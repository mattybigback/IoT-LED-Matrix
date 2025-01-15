#ifndef MAIN_H
#define MAIN_H

// File system libraries
#include <FS.h>
#include <LittleFS.h>

// Network-related libraries
#include <DNSServer.h>
#include <NetBIOS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

// Matrix-related libraries
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// Web portal
#include "webpages/update.h"
#include "webPortal.hpp"

// Set the matrix type
// Read MD_Parola documentation for which option to use for other modules
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Set number of MAX72xx chips being used
#define MAX_DEVICES 4

#define CLK_PIN SCK  // GPIO2
#define DATA_PIN MOSI // GPIO4
#define CS_PIN SS   // GPIO5
#define SOFT_RESET 6

#ifdef RGB_BUILTIN
#undef RGB_BUILTIN
#endif
#define RGB_BUILTIN 8

// FS Paths
#define messagePath "/message.txt"
#define intensityConfPath "/intens.txt"
#define speedConfPath "/speed.txt"

#define BAUD_RATE 115200
#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial0.print(x)
#define debugln(x) Serial0.println(x)
#define debugSetup(x) Serial0.begin(x)
#else
#define debug(x)
#define debugln(x)
#define debugSetup(x)
#endif

#define FORMAT_LITTLEFS_IF_FAILED true

// Matrix display array
#define BUF_SIZE 500 // Be careful here.
// If you increase the buffer over 500 then you must
// also increase the size of mainPageBuffer. 500 is PLENTY.
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
extern WebServer server; // Server on port 80 (default HTTP port) - can change it to a different port if need be.

extern MD_Parola matrix;

// File objects for FS
extern File messageFile;
extern File scrollSpeedConfFile;
extern File intensityConfFile;

#endif