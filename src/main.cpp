/* WiFi scrolling display by Matt H
   https://github.com/mattybigback

   Required hardware:

    * ESP8266 or ESP32 (tested with the below modules)
        Wemos D1 - ESP8266
        Generic ESP32-C3 devkit (using Wemos Lolin C3 pin definitions) - ESP32-C3
        

    * MAX7219 matrix module (tested with the below modules)
        FC-16 32x8 module

   MD_MAX72xx and MD_Parola libraries written and maintainesd by Marco Colli (MajicDesigns)
   https://github.com/MajicDesigns
   https://github.com/MajicDesigns/MD_MAX72XX
   https://github.com/MajicDesigns/MD_Parola

   WiFiMagager written and maintained by tzapu
   https://github.com/tzapu/WiFiManager


   Use this code at your own risk. I know I do.
*/

#include "main.hpp"

// Matrix display array
char curMessage[MSG_BUF_SIZE] = {""};
char newMessage[MSG_BUF_SIZE] = {""};

// New Message Flag
bool newMessageAvailable = true;

// Reset Display Flag
bool resetDisplay = false;

// Matrix display properties used on startup to display IP address
int intensity = 15;
int scrollSpeed = 90;

// WiFi AP Name - Should not exceed 24 chracters as the maximum length for an SSID is 32 characters, and 8 are used for the board ID
const char *APNamePrefix = "Mattrix";


// Scrolling effects
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
const int scrollPause = 0; // in milliseconds. Not used by default - holds the screen at the end of the message

// Instantiate objects
#if defined(ARDUINO_ARCH_ESP32)
WebServer server(80); 
#elif defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer server(80);
#endif
/*
   Uncommand the relevant line

   Top line for hardware SPI (ESP-12 etc)
   Bottom line for software SPI (ESP-01)
*/

MD_Parola matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
//MD_Parola matrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint32_t getChipId();
void wmCallback(WiFiManager *myWiFiManager);
void factoryReset();

void messageScroll() {
    // If the display is still animating OR the resetDisplay flag has been set
    if (matrix.displayAnimate() || resetDisplay) {

        if (newMessageAvailable) {          // If a new message has been set
            debugln("New message available"); // Debug message
            resetDisplay = false;           // Clear the resetDisplay flag
            strcpy(curMessage, newMessage); // Copy the newMessage buffer to curMessage
            newMessageAvailable = false;    // Clear the newMessageAvailabe flag
            matrix.setSpeed(scrollSpeed);   // Set scroll speed from global variable
            matrix.setIntensity(intensity); // Set intensity from global variable
        }
        matrix.displayReset(); // If display has finished animating reset the animation
    }
}


void startWifiManager() {
    // Create instance of WiFiManager
    WiFiManager wifiManager;
    wifiManager.setAPCallback(wmCallback);
    // Buffer for AP Name
    char APName[32];
    // Puts the APNamePrefix defined in the setup in the APName buffer and then adds the ESP module ID, formatted as 8 characters of Hex (leading zeros added)
    sprintf(APName, "%S%08X", APNamePrefix, getChipId());
    // Sets a static IP so that it is easy to connect to while in AP mode
    // wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 0, 0, 0));
    wifiManager.autoConnect(APName);
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN,0,20,0);
    #endif
}

void wmCallback(WiFiManager *myWiFiManager) {
    matrix.print("SETUP");
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, BLUE);
    #endif
    if (!digitalRead(SOFT_RESET)) {
        #if defined(HAS_NEOPIXEL)
        neopixelWrite(NEOPIXEL_PIN, RED);
        #endif
        factoryReset();
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
    WiFi.disconnect(false,true); // Disconnect from WiFi and clear credentials
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    LittleFS.format();     // Wipe the file system
    matrix.begin();        // Initialise the display
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
    server.begin();                   // Start http server

    char hostnameBuffer[32];
    sprintf(hostnameBuffer, "%S%08X", APNamePrefix, getChipId());
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
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
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
    // Read intensity conf file until it reaches the null terminator character, convert it to an int and store it in the intensity global variable
    file = LittleFS.open(intensityConfPath, "r");
    incomingFS = file.readStringUntil('\n');
    intensity = incomingFS.toInt();

    // Set new message flag
    // By not also setting the displayReset flag the first message can continue to scroll
    newMessageAvailable = true;
}

void loop() {
    // put your main code here, to run repeatedly:
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(RGB_BRIGHTNESS, GREEN);
    #endif
    messageScroll();       // Scroll the message
    server.handleClient(); // Keep web server ticking over
        if (!digitalRead(SOFT_RESET)) {
        factoryReset();
    }
}
