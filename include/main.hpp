#ifndef MAIN_H
#define MAIN_H

// File system libraries
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Network-related libraries
#include <DNSServer.h>
#if defined(ARDUINO_ARCH_ESP32)
    #include <NetBIOS.h>
    #include <WebServer.h>
    #include <WiFi.h>
    #elif defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266NetBIOS.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266WiFi.h>
#endif
#include <WiFiManager.h>

// Matrix-related libraries
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// Web portal
#include "webPortal.hpp"

// Neopixel colour lookup
#include "colours.hpp"

// Wifi manager functions
#include "wifiManager.hpp"

// Set the matrix type
// Read MD_Parola documentation for which option to use for other modules
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Set number of MAX72xx chips being used
#define MAX_DEVICES 4

#define CLK_PIN SCK
#define DATA_PIN MOSI
#define CS_PIN SS

#if !defined(NEOPIXEL_PIN)
    #define NEOPIXEL_PIN RGB_BUILTIN
#endif

// Settings file paths
#define messagePath "/message.txt"
#define intensityConfPath "/intens.txt"
#define speedConfPath "/speed.txt"

#define BAUD_RATE 115200
#define DEBUG 1 // Set to 1 to enable debug messages

#if DEBUG == 1
    #if defined(ALT_SERIAL)
        #define Serial ALT_SERIAL
    #endif
    #define debug(x) Serial.print(x)
    #define debugln(x) Serial.println(x)
    #define debugSetup(x) Serial.begin(x)
#else
    #define debug(x)
    #define debugln(x)
    #define debugSetup(x)
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define FORMAT_LITTLEFS_IF_FAILED true
#elif defined(ARDUINO_ARCH_ESP8266)
#define FORMAT_LITTLEFS_IF_FAILED
#endif


// Matrix display array
#define MSG_BUF_SIZE 501 // Be careful here.
// If you increase the buffer over 500 then you must
// also increase the size of PAGE_BUF_SIZE.
#define PAGE_BUF_SIZE 2048 // 2kB for the HTML page buffer
#define INTENSITY_MIN 0
#define INTENSITY_MAX 15
#define SCROLL_SPEED_MIN 10
#define SCROLL_SPEED_MAX 200

/*  
    WiFi AP Name - Should not exceed 24 chracters as 
    the maximum length for an SSID is 32 characters, 
    and 8 are used for the board ID
*/
#define APNAME_PREFIX "Mattrix"

extern char curMessage[];
extern char newMessage[];

// New Message Flag
extern bool newMessageAvailable;

// Reset Display Flag
extern bool resetDisplay;

// Matrix display properties used on startup to display IP address
extern int intensity;
extern int scrollSpeed;

extern textEffect_t scrollEffect;
extern textPosition_t scrollAlign;

// Instantiate objects
#if defined(ARDUINO_ARCH_ESP32)
extern WebServer server; // Server on port 80 (default HTTP port) - can change it to a different port if need be.
#elif defined(ARDUINO_ARCH_ESP8266)
extern ESP8266WebServer server; // Server on port 80 (default HTTP port) - can change it to a different port if need be.
#endif
extern MD_Parola matrix;

uint32_t getChipId();
void factoryReset();
void messageScroll();
void setup();
void loop();

#endif