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


void factoryReset() {
// Holds execution until reset button is released
#if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, WHITE);
#endif
    while (digitalRead(FACTORY_RESET_PIN) == LOW) {
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
    // Get the chip ID
    // ESP32: 48 bits, 6 bytes
    // ESP8266: 32 bits, 4 bytes
    // The chip ID is a unique identifier for the ESP32/ESP8266 chip
    // It is used to identify the device on the network
    // The chip ID is not the same as the MAC address
#if defined(ARDUINO_ARCH_ESP32)
    uint32_t id = 0;
    for (int i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
#elif defined(ARDUINO_ARCH_ESP8266)
    uint32_t id = ESP.getChipId(); // Get the chip ID
#endif
    debug("Chip ID: ");
    debugln(id); // Print the chip ID to the serial monitor
    return id;
}

#include <LittleFS.h>

void scrubUserData() {
    #if defined(ARDUINO_ARCH_ESP32)
    File root = LittleFS.open("/");
    if (!root)
        return; // safety check

    File entry;
    while ((entry = root.openNextFile())) { // iterate over everything in “/”
        if (!entry.isDirectory()) {
        #if defined(ARDUINO_ARCH_ESP32)
            String name = entry.path();
        #endif
            entry.close(); // close the file so that it can be deleted
            if (name.endsWith(".dat")) { // “*.dat” test
                debug("Removing ");
                debugln(name);
                LittleFS.remove(name); // delete file
            }
        }
    }
    #elif defined(ARDUINO_ARCH_ESP8266)
    Dir dir = LittleFS.openDir("/"); // Use openDir for ESP8266
    while (dir.next()) { // Iterate over files in the directory
        String name = dir.fileName();
        if (name.endsWith(".dat")) { // Check for ".dat" files
            debug("Removing ");
            debugln(name);
            if (LittleFS.remove(name)) {
                debugln("File removed successfully.");
            } else {
                debugln("Failed to remove file.");
            }
        }
    }
    #endif
}


#if DEBUG==1 && defined(ARDUINO_ARCH_ESP32)
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

// Function to get IP address as a cstring
void getIpAddress(char* buffer, size_t bufferSize, bool includePort) {
    String ip = WiFi.localIP().toString();
    if (includePort && WEB_SERVER_PORT != 80) {
        snprintf(buffer, bufferSize, "%s:%d", ip.c_str(), WEB_SERVER_PORT);
    } else {
        snprintf(buffer, bufferSize, "%s", ip.c_str());
    }
}

char* getURL() {
    static char url[51]; // Char array to store the URL
    #if WEB_SERVER_PORT != 80
        snprintf(url, sizeof(url), "http://%s%06x.local:%d", HOSTNAME_PREFIX, getChipId(), WEB_SERVER_PORT);
    #else
        snprintf(url, sizeof(url), "http://%s%06x.local", HOSTNAME_PREFIX, getChipId());
    #endif
    debug("URL: ");
    debugln(url); // Print the URL to the serial monitor
    return url;
}

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
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    #if defined(ADDRESS_SCROLL_BUTTON)
    pinMode(ADDRESS_SCROLL_BUTTON, INPUT_PULLUP);
    #endif

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
    if (!digitalRead(FACTORY_RESET_PIN)) {
        factoryReset();
    }

    matrix.print("BOOT");

    // Start WiFiManager
    startWifiManager();
    WiFi.setAutoReconnect(true);
    #if defined(ARDUINO_ARCH_ESP32)
    WiFi.persistent(true);
    #endif
    #if defined(HAS_NEOPIXEL)
        neopixelWrite(NEOPIXEL_PIN, GREEN); // green
    #endif

    char hostnameBuffer[40]; // Char array to store hostname
    // Set the hostname
    snprintf(hostnameBuffer, sizeof(hostnameBuffer), "%s%06x", HOSTNAME_PREFIX, getChipId());
    WiFi.hostname(hostnameBuffer);
    debug("Hostname: ");
    debugln(hostnameBuffer); // Print the hostname to the serial monitor

    // Start mDNS responder
    if (!MDNS.begin(hostnameBuffer)) {
        debugln("Error setting up MDNS responder!");
    } else {
        debug("MDNS responder started: ");
        debugln(getURL());
    }
    
    // Get the IP address
    char ipAddressBuffer[22]; // Char array to store the IP address
    getIpAddress(ipAddressBuffer, sizeof(ipAddressBuffer), true); // Get the IP address
    debug("IP Address: ");
    debugln(ipAddressBuffer); // Print the IP address to the serial monitor


    // Read message file from LittleFS and store it in the newMessage char array
    file = LittleFS.open(messagePath, "r");
    incomingFS = file.readStringUntil('\n');
    incomingFS.toCharArray(newMessage, MSG_BUF_SIZE);

    // Set up the web server and define the routes
    server.on("/", handleRoot);       // Function to call when root page is loaded
    server.on("/api/message", handleAPI);
    server.serveStatic("/", LittleFS, "/confpage/");
    server.begin(); // Start http server

    // Copy IP address and mDNS url to newMessage display buffer so that it is scrolled across the display
    
    snprintf(curMessage, MSG_BUF_SIZE, "%s    %s", ipAddressBuffer, getURL());
    // Reset Display
    matrix.displayClear();
    // Set up text scroll animation
    matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    messageScroll();
    
    // Set flags to notify that a new message is available and that the display settings have changed
    // This is done so that the display will reset and scroll the new message when the IP address is finished scrolling
    newMessageAvailable = true;
    displaySettingsChanged = true;

    
}


void loop() {
#if DEBUG==1 && defined(ARDUINO_ARCH_ESP32)
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
    if (!digitalRead(FACTORY_RESET_PIN)) {
        factoryReset();
    }
    #if defined(ADDRESS_SCROLL_BUTTON)
        static bool buttonPreviouslyPressed = false; // Track the previous state of the button
        if (!digitalRead(ADDRESS_SCROLL_BUTTON)) {
            if (!buttonPreviouslyPressed) { // Only execute if the button was not previously pressed
                buttonPreviouslyPressed = true; // Update the state to indicate the button is now pressed
                char ipAddressBuffer[22]; // Char array to store the IP address
                getIpAddress(ipAddressBuffer, sizeof(ipAddressBuffer), true);
                snprintf(curMessage, MSG_BUF_SIZE, "%s    %s", ipAddressBuffer, getURL());
                // Reset Display
                matrix.displayClear();
                // Set up text scroll animation
                matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
                //messageScroll();
                
                // Set flags to notify that a new message is available and that the display settings have changed
                // This is done so that the display will reset and scroll the new message when the IP address is finished scrolling
                newMessageAvailable = true;
                displaySettingsChanged = true;
            }
        } else {
            buttonPreviouslyPressed = false; // Reset the state when the button is released
        }
    #endif
    #if defined(ARDUINO_ARCH_ESP8266)
        MDNS.update();
    #endif
}