
#include "main.hpp"

char curMessage[MSG_BUF_SIZE] = {""};
char newMessage[MSG_BUF_SIZE] = {""};
bool newMessageAvailable;
bool resetDisplay;
int intensity;
int scrollSpeed;
bool displayFlipped;
textEffect_t scrollEffect; 
textPosition_t scrollAlign; 
int scrollPause;          
#if defined(ARDUINO_ARCH_ESP32)
    WebServer server(WEB_SERVER_PORT); // ESP32 web server
#elif defined(ARDUINO_ARCH_ESP8266)
    ESP8266WebServer server(WEB_SERVER_PORT); // ESP8266 web server
#endif
MD_Parola matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX72XX_DEVICE_COUNT); 

void messageScroll() {
    // If the display is still animating OR the resetDisplay flag has been set
    if (matrix.displayAnimate() || resetDisplay) {

        if (newMessageAvailable) {            // If a new message has been set
            debugln("New message available"); // Debug message
            resetDisplay = false;             // Clear the resetDisplay flag
            strcpy(curMessage, newMessage);   // Copy the newMessage buffer to curMessage
            newMessageAvailable = false;      // Clear the newMessageAvailabe flag
            matrix.setSpeed(scrollSpeed);     // Set scroll speed from global variable
            matrix.setIntensity(intensity);   // Set intensity from global variable
        }
        matrix.displayReset(); // If display has finished animating reset the animation
#if defined(HAS_NEOPIXEL)
        neopixelWrite(NEOPIXEL_PIN, GREEN);
#endif
    }
}

void factoryReset() {
// Holds execution until reset button is released
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, WHITE);
#endif
    while (digitalRead(SOFT_RESET) == LOW) {
        yield(); // Hands execution over to network stack to stop the ESP crashing
    }
    WiFi.disconnect(false, true); // Disconnect from WiFi and clear credentials
    WiFi.mode(WIFI_STA);          // explicitly set mode, esp defaults to STA+AP
    scrubUserData();              // Wipe user data
    matrix.begin();               // Initialise the display
    matrix.print("RESET");
    delay(3000);
    ESP.restart(); // System reset
}

uint32_t getChipId() {

#if defined(ARDUINO_ARCH_ESP32)
    uint32_t id = 0;
    for (int i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
#elif defined(ARDUINO_ARCH_ESP8266)
    uint32_t id = ESP.getChipId(); // Get the chip ID
#endif
    return id;
}

#include <LittleFS.h>

void scrubUserData() {
    File root = LittleFS.open("/");
    if (!root)
        return; // safety check

    File entry;
    while ((entry = root.openNextFile())) { // iterate over everything in “/”
        if (!entry.isDirectory()) {
        #if defined(ARDUINO_ARCH_ESP32)
            String name = entry.path();
        #endif
        #if defined(ARDUINO_ARCH_ESP8266)
            String name = entry.name();
        #endif
            entry.close(); // close the file so that it can be deleted
            if (name.endsWith(".dat")) { // “*.dat” test
                debug("Removing ");
                debugln(name);
                LittleFS.remove(name); // delete file
            }
        }
    }
}

bool setMatrixOrientation(bool flipDisplay) {
    matrix.displayClear();
    matrix.displayReset();
    matrix.setZoneEffect(0, flipDisplay, PA_FLIP_LR);
    matrix.setZoneEffect(0, flipDisplay, PA_FLIP_UD);

    // Set the matrix orientation
    if (flipDisplay) {
        debugln("Flipping display");
        scrollEffect = PA_SCROLL_RIGHT;
        scrollAlign = PA_RIGHT;

    } else {
        // Set the matrix to the default orientation
        debugln("Setting display to default orientation");
        scrollEffect = PA_SCROLL_LEFT;
        scrollAlign = PA_LEFT;

    }

    matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    return flipDisplay;
}

void setup() {
    debugSetup(BAUD_RATE);
    debugln("Booting...");
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    debugln("Wifi mode set");
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, YELLOW); // yellow
#endif
    delay(250);
    pinMode(SOFT_RESET, INPUT_PULLUP);

    // Begin matrix, set scroll speed and intensity from global variables
    matrix.begin();
    displayFlipped = setMatrixOrientation(0);
    matrix.setSpeed(scrollSpeed);
    matrix.setIntensity(intensity);

    // Call factory reset funcion if pin is low
    if (!digitalRead(SOFT_RESET)) {
        factoryReset();
    }

    matrix.print("BOOT");

    // Start WiFiManager
    startWifiManager();
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, GREEN); // green
#endif

    server.on("/", handleRoot);       // Function to call when root page is loaded
    server.on("/update", handleForm); // Function to call when form is submitted and update page is loaded
    server.on("/api", handleAPI);
    server.on("/flip", handleFlip); // Function to call when flip page is loaded
    server.begin(); // Start http server

    char hostnameBuffer[32];
    sprintf(hostnameBuffer, "%S%08X", APNAME_PREFIX, getChipId());
    WiFi.hostname(hostnameBuffer);

    char ipAddress[16]; // Char array to store human readable IP address
    sprintf(ipAddress, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);

    // Copy IP address to newMessage display buffer so that it is scrolled across the display
    strcpy(newMessage, ipAddress);
    // Reset Display
    matrix.displayClear();
    // Set up text scroll animation
    matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

    // Set newMessageAvailable and resetDisplay flags
    newMessageAvailable = true;
    resetDisplay = true;
    // Trigger scroll animation
    messageScroll();

    // Begin LittleFS and check if it successfully mounts FS.
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        debugln("LittleFS Mount Failed");
        return;
    }
    String incomingFS; // String object for passing data to and from LittleFS

    // Check if message file exists, and if it doesn't then create one
    File file;
    if (!LittleFS.exists(messagePath)) {
        debugln("No message file exists. Creating one now...");
        file = LittleFS.open(messagePath, "w");
        file.print("Hello, World!");
        file.close();
    }
    // Read message file until it reaches the null terminator character and store contents in the newMessage char array
    file = LittleFS.open(messagePath, "r");
    incomingFS = file.readStringUntil('\n');
    incomingFS.toCharArray(newMessage, MSG_BUF_SIZE);

    // Check if speed conf file exists, and if it doesn't then create one
    if (!LittleFS.exists(speedConfPath)) {
        // debugln("No speed file exists. Creating one now...");
        file = LittleFS.open(speedConfPath, "w");
        file.print("50");
        file.close();
    }
    // Read speed conf file until it reaches the null terminator character, convert it to an int and store it in the scrollSpeed global variable
    file = LittleFS.open(speedConfPath, "r");
    incomingFS = file.readStringUntil('\n');
    scrollSpeed = incomingFS.toInt();

    // Check if intensity conf file exists, and if it doesn't then create one
    if (!LittleFS.exists(intensityConfPath)) {
        // debugln("No intensity file exists. Creating one now...");
        file = LittleFS.open(intensityConfPath, "w");
        file.print("7");
        file.close();
    }
    // Read intensity conf file, convert it to an int and store it in the intensity global variable
    file = LittleFS.open(intensityConfPath, "r");
    incomingFS = file.readStringUntil('\n');
    intensity = incomingFS.toInt();

    // Set new message flag
    // By not also setting the displayReset flag the first message can continue to scroll
    newMessageAvailable = true;
}

void loop() {
    messageScroll();       // Scroll the message
    server.handleClient(); // Keep web server ticking over
    // Reset if the reset button is pressed
    if (!digitalRead(SOFT_RESET)) {
        factoryReset();
    }
}
