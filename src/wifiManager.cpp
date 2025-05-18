#include "wifiManager.hpp"

void startWifiManager() {
    // Create instance of WiFiManager
    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(wmCallback);
    // Buffer for AP Name
    char APName[32];
    // Puts the APNAME_PREFIX defined in the setup in the APName buffer and then adds the ESP module ID, formatted as 8 characters of Hex (leading zeros added)
    sprintf(APName, "%s%06X", APNAME_PREFIX, getChipId());
    wifiManager.autoConnect(APName);
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN,GREEN);
    #endif
    
}

void saveConfigCallback() {
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN, CYAN); // Optional: Indicate saving config
    #endif
    matrix.print("SAVED");
    /* 
    Restarts ESP8266 to make sure mdns responder is started correctly
    with new IP address. This is not needed for ESP32 as the mDNS library
    is more robust.
    */
    #if defined(ARDUINO_ARCH_ESP8266)
    debugln("Restarting ESP8266...");
    ESP.restart(); // Restart the ESP8266
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