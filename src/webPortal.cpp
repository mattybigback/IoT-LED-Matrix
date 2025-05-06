#include "webPortal.hpp"

// Server response to a request for root page
void handleRoot() {
    // Turn the LED or NeoPixel orange to indicate action
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(RGB_BUILTIN, ORANGE); // Orange
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
        MSG_BUF_SIZE-1,      // %d → max message length
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
    neopixelWrite(RGB_BRIGHTNESS, ORANGE);
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
}
