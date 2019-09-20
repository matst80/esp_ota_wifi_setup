#ifndef otaWIFI_h
#define otaWIFI_h
#include <ESP8266WebServer.h>

struct wifiSettings
{
  bool hasConfig;
  char ssid[20];
  char password[25];
};

class OTA_WIFI {
    public:
        OTA_WIFI();
        void setup();
        void loop();
        boolean hasConfig();
        void save(wifiSettings settings);
    private:
        wifiSettings _currentSettings;
        void _startWifi();
        void _restart();
};

#endif