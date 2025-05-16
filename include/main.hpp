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

#ifndef MAIN_HPP
#define MAIN_HPP

// File system libraries
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Network-related libraries
#include <DNSServer.h>  // Needed for WiFiManager captive portal
#if defined(ARDUINO_ARCH_ESP32)
    #include <ESPmDNS.h>
    #include <WebServer.h>
    #include <WiFi.h>
    #elif defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266mDNS.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266WiFi.h>
#endif
#include <WiFiManager.h>

// Matrix-related libraries
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

/* --  Project headers  -- */

#include "webPortal.hpp"    // Web portal functions
#include "colours.hpp"      // Neopixel colour lookup
#include "wifiManager.hpp"  // Wifi manager functions

/* --  Matrix hardware type  -- */
// Read MD_Parola documentation for which option to use for other modules
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW   // MD_MAX72XX::FC16_HW for FC-16 32x8 module
#define MAX72XX_DEVICE_COUNT 4              // Number of MAX7219 devices in the chain

/* --  SPI pins  -- */
#define CLK_PIN SCK     // SPI clock pin
#define DATA_PIN MOSI   // SPI data out pin
#define CS_PIN SS       // SPI chip select pin

// Set the platformio env does not specify NEOPIXEL_PIN then use RGB_BUILTIN (Defined by the board definition)
#if !defined(NEOPIXEL_PIN)
    #define NEOPIXEL_PIN RGB_BUILTIN
#endif

/* --  Settings file paths  -- */
#define messagePath "/message.dat"
#define intensityConfPath "/intens.dat"
#define speedConfPath "/speed.dat"
#define flipConfPath "/flip.dat"

/* --  Serial debugging macro definitions  -- */
#define BAUD_RATE 115200
#define DEBUG 1 // Set to 1 to enable debug messages
#if DEBUG == 1                      // If DEBUG is set to 1 then enable debug macros
    #if defined(ALT_SERIAL)         // If ALT_SERIAL is defined then use that for debugging
        #define debug(x) ALT_SERIAL.print(x)
        #define debugln(x) ALT_SERIAL.println(x)
        #define debugSetup(x) ALT_SERIAL.begin(x)
    #else                           // Otherwise use Serial for debugging
        #define debug(x) Serial.print(x)
        #define debugln(x) Serial.println(x)
        #define debugSetup(x) Serial.begin(x)
    #endif
#else                               // If DEBUG is set to 0 then disable debug macros
    #define debug(x)
    #define debugln(x)
    #define debugSetup(x)
#endif

/* --  LittleFS initialisaton macros  -- */
#if defined(ARDUINO_ARCH_ESP32)
    #define FORMAT_LITTLEFS_IF_FAILED true  // Format the filesystem if it fails to mount//
#elif defined(ARDUINO_ARCH_ESP8266)
    #define FORMAT_LITTLEFS_IF_FAILED       // Not supported on ESP8266
#endif


/* --  Text buffer sizes  -- */
#define MSG_BUF_SIZE 501 // 500 characters for the message buffer including the null terminator
#define PAGE_BUF_SIZE 2048 // 2kB for the HTML page buffer

/* --  Matrix parameter limits  -- */
#define INTENSITY_MIN 0
#define INTENSITY_MAX 15
#define SCROLL_SPEED_MIN 10
#define SCROLL_SPEED_MAX 200

/*  
    WiFi AP Name - Should not exceed 25 chracters as 
    the maximum length for an SSID is 31 characters (plus null terminator), 
    and 6 are used for the board ID
*/
#define APNAME_PREFIX "MATRIX_SETUP_"
static_assert(
    sizeof(APNAME_PREFIX) - 1 <= 25,
        "APNAME_PREFIX is too long: SSID (prefix + 6 hex chars) must be <= 31 characters."
);

/*  
    Hostname - Should not exceed 25 chracters as 
    the maximum length for an hostname is 31 characters (plus null terminator), 
    and 6 are used for the board ID
*/

#define HOSTNAME_PREFIX "espmatrix-"
static_assert(
    sizeof(HOSTNAME_PREFIX) - 1 <= 25,
        "APNAME_PREFIX is too long: hostname (prefix + 6 hex chars) must be <= 31 characters."
);

// Matrix display arrays
extern char curMessage[MSG_BUF_SIZE];   // Current message to be displayed
extern char newMessage[MSG_BUF_SIZE];   // New message to be displayed

/* --  Status flags  -- */
extern bool newMessageAvailable;    // true if a new message is available
extern bool displaySettingsChanged;   // true if the display settings have changed
// Reset Display Flag
extern bool resetDisplay;           // true if the display needs to be reset

/* --  Matrix display properties  -- */
extern int intensity;
extern int scrollSpeed;
extern bool displayFlipped;

/* --  Parola scrolling effects  -- */
extern textEffect_t scrollEffect;   // PA_SCROLL_LEFT, PA_SCROLL_RIGHT
extern textPosition_t scrollAlign;  // PA_LEFT, PA_RIGHT
extern int scrollPause;             // in milliseconds. Not used by default - holds the screen at the end of the message


/* --  Instantiate objects  -- */
// Instantiate the web server object depending on the platform
#if defined(ARDUINO_ARCH_ESP32)
    extern WebServer server; // ESP32 web server
#elif defined(ARDUINO_ARCH_ESP8266)
    extern ESP8266WebServer server; // ESP8266 web server
#endif

// Instantiate the matrix object
extern MD_Parola matrix; 

/* --  Function definitions  -- */

uint32_t getChipId();
void getIpAddress(char* buffer, size_t bufferSize, bool includePort);
char* getHostname();
void factoryReset();
void messageScroll();
void scrubUserData();
bool setMatrixOrientation(bool flipDisplay);
#if DEBUG==1
void memoryUsage();
#endif
void setup();
void loop();

#endif