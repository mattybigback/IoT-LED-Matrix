#ifndef WIFIMANAGER_HPP
#define WIFIMANAGER_HPP

#include "main.hpp"

void startWifiManager();
void saveConfigCallback();
void wmCallback(WiFiManager *myWiFiManager);

#endif