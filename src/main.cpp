
#include "main.hpp"

char curMessage[MSG_BUF_SIZE] = {""};
char newMessage[MSG_BUF_SIZE] = {""};
bool newMessageAvailable;
bool displaySettingsChanged;
bool resetDisplay;
int intensity = 15;
int scrollSpeed = 30;
bool displayFlipped = false;
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
        matrix.displayClear();
        if (newMessageAvailable) {            // If a new message has been set
            debugln("New message available"); // Debug message
            resetDisplay = false;             // Clear the resetDisplay flag
            snprintf(curMessage, MSG_BUF_SIZE, "%s", newMessage); // Copy the newMessage buffer to curMessage
            newMessageAvailable = false;      // Clear the newMessageAvailabe flag

        }
        if (displaySettingsChanged) { // If the display settings have changed
            debugln("Display settings changed"); // Debug message
            resetDisplay = false;             // Clear the resetDisplay flag
            matrix.setSpeed(scrollSpeed);        // Set the scroll speed
            debug("Scroll speed: ");
            debugln(scrollSpeed);               // Debug message
            matrix.setIntensity(intensity);      // Set the intensity
            debug("Intensity: ");
            debugln(intensity);                 // Debug message
            setMatrixOrientation(displayFlipped); // Set the display orientation
            debug("Display flipped: ");
            debugln(displayFlipped);           // Debug message
            displaySettingsChanged = false;      // Clear the displaySettingsChanged flag
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
    //matrix.displayReset();
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
#if DEBUG==1
void memoryUsage() {
    // Print the free heap memory
    debug("Free heap: ");
    debugln(ESP.getFreeHeap());
    debug("Largest free block: ");
    debugln((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    debug("Stack Highwater: ");
    debugln((unsigned)uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

}
#endif

void setup() {
    debugSetup(BAUD_RATE);
    debugln("Booting...");
     // explicitly set mode, esp defaults to STA+AP and hold until mode is set
    while (!WiFi.mode(WIFI_STA)){
        
    }
    debugln("Wifi mode set");
    #if defined(HAS_NEOPIXEL)
        neopixelWrite(NEOPIXEL_PIN, YELLOW); // yellow
    #endif
    pinMode(SOFT_RESET, INPUT_PULLUP);

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

    if (!LittleFS.exists(flipConfPath)) {
        // debugln("No flip file exists. Creating one now...");
        file = LittleFS.open(flipConfPath, "w");
        file.print("0");
        file.close();
    }
    // Read flip conf file, convert it to a bool and store it in the displayFlipped global variable
    file = LittleFS.open(flipConfPath, "r");
    incomingFS = file.readStringUntil('\n');
    if (incomingFS == "1") {
        displayFlipped = true;
    } else {
        displayFlipped = false;
    }

    // Set new message flag
    // By not also setting the displayReset flag the first message can continue to scroll
    newMessageAvailable = true;
    displaySettingsChanged = true;


    // Begin matrix, set scroll speed and intensity from global variables
    matrix.begin();
    setMatrixOrientation(displayFlipped);
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
    server.on("/api/message", handleAPI);
    server.serveStatic("/", LittleFS, "/confpage/");
    server.begin(); // Start http server

    char hostnameBuffer[32];
    sprintf(hostnameBuffer, "%S%08X", APNAME_PREFIX, getChipId());
    WiFi.hostname(hostnameBuffer);

    char ipAddress[16]; // Char array to store human readable IPv4 address
    sprintf(ipAddress, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);

    // Copy IP address to newMessage display buffer so that it is scrolled across the display
    strcpy(newMessage, ipAddress);
    // Reset Display
    matrix.displayClear();
    // Set up text scroll animation
    matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    messageScroll();

    // Read message file from LittleFS and store it in the newMessage char array
    file = LittleFS.open(messagePath, "r");
    incomingFS = file.readStringUntil('\n');
    incomingFS.toCharArray(newMessage, MSG_BUF_SIZE);
    // Set flags to notify that a new message is available and that the display settings have changed
    // This is done so that the display will reset and scroll the new message when the IP address is finished scrolling
    newMessageAvailable = true;
    displaySettingsChanged = true;
    
}


void loop() {
#if DEBUG==1
    static unsigned long lastMemoryUsageTime = 0; // Tracks the last time memoryUsage() was called
    unsigned long currentMillis = millis();      // Get the current time
    if (currentMillis - lastMemoryUsageTime >= 10000) { // If 60 seconds have passed
        memoryUsage(); // Call memoryUsage() function
        lastMemoryUsageTime = currentMillis; // Update the last time memoryUsage() was called
    }
#endif
    messageScroll();       // Scroll the message
    server.handleClient(); // Keep web server ticking over
    // Reset if the reset button is pressed
    if (!digitalRead(SOFT_RESET)) {
        factoryReset();
    }
}