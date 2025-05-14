#include "webPortal.hpp"

// Server response to a request for root page
void handleRoot() {
// Turn the LED or NeoPixel orange to indicate action
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, ORANGE); // Orange
#endif
    debugln("Web portal index requested");
    // Open file in read mode
    File webpage = LittleFS.open("/confpage/index.html", "r");
    if (!webpage) {
        server.send(404, "text/plain", "Not found");
        return;
    }
    String pageContent = webpage.readString();
    webpage.close();
    debugln("File loaded succesfully");

    char mainPageBuffer[PAGE_BUF_SIZE];

    int len = snprintf(
        mainPageBuffer,      // destination buffer
        PAGE_BUF_SIZE,       // max page size (incl. NUL)
        pageContent.c_str(), // HTML template
        curMessage,          // %s → current message
        MSG_BUF_SIZE - 1,    // %d → max message length
        intensity,           // %d → intensity value
        INTENSITY_MIN,       // %d → min intensity value
        INTENSITY_MAX,       // %d → max intensity value
        scrollSpeed,         // %d → scroll speed value
        SCROLL_SPEED_MIN,    // %d → min scroll speed value
        SCROLL_SPEED_MAX     // %d → max scroll speed value
    );

    if (len < 0 || (size_t)len >= PAGE_BUF_SIZE) {
        debugln("ERROR: output truncated or snprintf failed");
        server.send(500, "text/plain", "Server error");
        return;
    }

    server.send(200, "text/html", mainPageBuffer);
}

// Server response to incoming data from form
void handleForm() {
// visual feedback
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, ORANGE);
#endif

    char intensityBuf[3];
    char speedBuf[4];
    debugln("Web portal update requested");
    // Open file in read mode
    File webpage = LittleFS.open("/confpage/update.html", "r");
    if (!webpage) {
        server.send(404, "text/plain", "Not found");
        return;
    }
    String pageContent = webpage.readString();
    webpage.close();
    debugln("File loaded succesfully");

    // Read and convert in one go—no named String variables
    server.arg("messageToScroll").toCharArray(newMessage, MSG_BUF_SIZE);
    server.arg("intensity").toCharArray(intensityBuf, sizeof(intensityBuf));
    server.arg("speed").toCharArray(speedBuf, sizeof(speedBuf));
    debug("Message: ");
    debugln(newMessage);
    debug("Message length: ");
    debugln(strlen(newMessage));
    debug("Intensity: ");
    debugln(intensityBuf);
    debug("Speed: ");
    debugln(speedBuf);

    int parsedIntensity = atoi(intensityBuf);
    int parsedScrollSpeed = atoi(speedBuf);

    // 1) check intensity range
    if (parsedIntensity < INTENSITY_MIN || parsedIntensity > INTENSITY_MAX) {
        char errBuf[50];
        snprintf(errBuf, sizeof(errBuf), "Error: intensity must be between %d and %d", INTENSITY_MIN, INTENSITY_MAX);
        debugln(errBuf);
        server.send(400, "text/plain", errBuf);
        return;
    }

    // 2) check speed range
    if (parsedScrollSpeed < SCROLL_SPEED_MIN || parsedScrollSpeed > SCROLL_SPEED_MAX) {
        char errBuf[50];
        snprintf(errBuf, sizeof(errBuf), "Error: speed must be between %d and %d", SCROLL_SPEED_MIN, SCROLL_SPEED_MAX);
        server.send(400, "text/plain", errBuf);
        debugln(errBuf);
        return;
    }

    // all good — commit settings
    intensity = parsedIntensity;
    scrollSpeed = parsedScrollSpeed;

    LittleFS.open(messagePath, "w").print(newMessage);
    LittleFS.open(speedConfPath, "w").print(scrollSpeed);
    LittleFS.open(intensityConfPath, "w").print(intensity);

    // Update flags, clear/reset display, and debug-log

    newMessageAvailable = true;
    matrix.displayClear();
    matrix.displayReset();
    resetDisplay = true;

    // Send HTTP response
    server.send(200, "text/html", pageContent);
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, GREEN);
#endif
}

void handleAPI() {
    if (server.method() == HTTP_PUT) {
        // Bool flags to track if the parameters have changed
        bool messageChanged = false;
        bool intensityChanged = false;
        bool speedChanged = false;
        bool displayFlippedChanged = false;

        // Turn the LED or NeoPixel orange to indicate action
        #if defined(HAS_NEOPIXEL)
            neopixelWrite(NEOPIXEL_PIN, ORANGE); // Orange
        #endif

        if (!server.hasArg("plain")) {
            server.send(400, "application/json",
                        "{\"errors\":{\"body\":\"Missing JSON body\"}}");
            return;
        }
        // Parse http header into JsonDocument
        JsonDocument reqDoc;
        // JsonObject reqObj = reqDoc.to<JsonObject>();
        DeserializationError err = deserializeJson(reqDoc, server.arg("plain"));
        if (err) {
            server.send(400, "application/json",
                        "{\"errors\":{\"json\":\"Invalid JSON\"}}");
            return;
        }

        // JSON document for logging validation errors
        JsonDocument errDoc;
        JsonObject rootErr = errDoc.to<JsonObject>();           // to<JsonObject>() creates the root object
        JsonObject errors = rootErr["errors"].to<JsonObject>(); // nested “errors” object

        JsonVariant messageVariant = reqDoc["message"];
        if (!messageVariant.isNull()) {
            debugln("Message received");
            if (!messageVariant.is<String>()) {
                debugln("Message not a string");
                errors["message"] = "Value must be a string";
            } else {
                debugln("Message is a string");
            }
            size_t messageLength = messageVariant.as<String>().length();
            if (messageLength > MSG_BUF_SIZE - 1) {
                debugln("Message too long");
                char errBuf[50];
                snprintf(errBuf, sizeof(errBuf), "Too long (%d). Max allowed length is %d", messageLength, MSG_BUF_SIZE - 1);
                errors["message"] = errBuf;
            }
            messageChanged = true;
        }
        JsonVariant intensityVariant = reqDoc["intensity"];
        if (!reqDoc["intensity"].isNull()) {
            debugln("Intensity received");
            if (!intensityVariant.is<int>()) {
                debugln("Intensity not an int");
                errors["intensity"] = "Value must be an integer";
            } else {
                int intensityValue = reqDoc["intensity"];
                if (intensityValue < INTENSITY_MIN || intensityValue > INTENSITY_MAX) {
                    debugln("Intensity out of range");
                    char errBuf[76];
                    snprintf(errBuf, sizeof(errBuf), "Value (%d) out of range. Must be be between %d and %d (inclusive)", intensityValue, INTENSITY_MIN, INTENSITY_MAX);
                    errors["intensity"] = errBuf;
                }
            }
            intensityChanged = true;
        }
        JsonVariant speedVariant = reqDoc["speed"];
        if (!speedVariant.isNull()) {
            debugln("Speed received");
            if (!speedVariant.is<int>()) {
                debugln("Speed not an int");
                errors["speed"] = "Value must be an integer";
            } else {
                int speedValue = reqDoc["speed"];
                if (speedValue < SCROLL_SPEED_MIN || speedValue > SCROLL_SPEED_MAX) {
                    debugln("Speed out of range");
                    char errBuf[76];
                    snprintf(errBuf, sizeof(errBuf), "Value (%d) out of range. Must be be between %d and %d (inclusive)", speedValue, SCROLL_SPEED_MIN, SCROLL_SPEED_MAX);
                    errors["speed"] = errBuf;
                }
            }
            speedChanged = true;
        }
        JsonVariant flipVariant = reqDoc["display_flipped"];
        if (!flipVariant.isNull()) {
            debugln("Display flipped received");
            if (!reqDoc["display_flipped"].is<bool>()) {
                debugln("Display flipped not a bool");
                errors["display_flipped"] = "Must be true or false";
            }
            displayFlippedChanged = true;
        }

        // If errors are found, send a 400 response with the errors and return
        if (errors.size() > 0) {
            debugln("Errors found");
            String errJson;
            debug("Errors: ");
            serializeJson(errDoc, errJson);
            debugln(errJson);
            server.send(400, "application/json", errJson);
            return;
        }
        if (messageChanged) {
            debug("Message: ");
            debugln(messageVariant.as<const char *>());
            newMessageAvailable = true;
            snprintf(newMessage, MSG_BUF_SIZE, "%s", messageVariant.as<const char *>());
            debugln ("New message copied into buffer");
            LittleFS.open(messagePath, "w").print(newMessage);

        }
        if (intensityChanged) {
            debug("Intensity: ");
            debugln(intensityVariant.as<int>());
            intensity = intensityVariant.as<int>();
            LittleFS.open(intensityConfPath, "w").print(intensity);
            displaySettingsChanged = true;
        }
        if (speedChanged) {
            debug("Speed: ");
            debugln(speedVariant.as<int>());
            scrollSpeed = speedVariant.as<int>();
            LittleFS.open(speedConfPath, "w").print(scrollSpeed);
            displaySettingsChanged = true;
        }
        if (displayFlippedChanged) {
            debug("Display flipped: ");
            debugln(flipVariant.as<bool>());
            displayFlipped = flipVariant.as<bool>();
            LittleFS.open(flipConfPath, "w").print(displayFlipped);
            displaySettingsChanged = true;
        }
            
    // Set reset display flag to true if new message is recieved or settings have changed
    resetDisplay |= newMessageAvailable || displaySettingsChanged;

    // Send HTTP response
    char response[16] = "{\"status\":\"ok\"}";
    //serializeJson(reqDoc, response);
    server.send(200, "application/json", response);
    #if defined(HAS_NEOPIXEL)
        neopixelWrite(NEOPIXEL_PIN, GREEN);
    #endif    
    return;
    }

    if (server.method() == HTTP_GET) {

        JsonDocument settings;

        settings["message"] = curMessage;
        settings["message_max_len"] = MSG_BUF_SIZE - 1;
        settings["intensity"] = intensity;
        settings["intensity_min"] = INTENSITY_MIN;
        settings["intensity_max"] = INTENSITY_MAX;
        settings["speed"] = scrollSpeed;
        settings["speed_min"] = SCROLL_SPEED_MIN;
        settings["speed_max"] = SCROLL_SPEED_MAX;
        settings["display_flipped"] = displayFlipped;

        String response;
        serializeJson(settings, response);
        server.send(200, "application/json", response);
        return;
    }
    server.send(405, "application/json",
                "{\"errors\":{\"method\":\"Method not allowed\"}}");
}

void handleFlip() {
// Turn the LED or NeoPixel orange to indicate action
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, ORANGE); // Orange
#endif
    debugln("Web portal flip requested");
    displayFlipped = setMatrixOrientation(!displayFlipped);
    char pageContent[64];
    snprintf(pageContent, sizeof(pageContent), "Display flipped: %s", displayFlipped ? "true" : "false");
    server.send(200, "text/html", pageContent);
}