#include "wifiManager.hpp"

void startWifiManager() {
    // Create instance of WiFiManager
    WiFiManager wifiManager;
    wifiManager.setAPCallback(wmCallback);
    // Buffer for AP Name
    char APName[32];
    // Puts the APNAME_PREFIX defined in the setup in the APName buffer and then adds the ESP module ID, formatted as 8 characters of Hex (leading zeros added)
    sprintf(APName, "%S%08X", APNAME_PREFIX, getChipId());
    // Sets a static IP so that it is easy to connect to while in AP mode
    // wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 0, 0, 0));
    wifiManager.autoConnect(APName);
    #if defined(HAS_NEOPIXEL)
    neopixelWrite(NEOPIXEL_PIN,GREEN);
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