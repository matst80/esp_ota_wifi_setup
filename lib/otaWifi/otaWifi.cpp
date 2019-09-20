#include "otaWifi.h"
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const int _eprom_start = 0;
volatile boolean isUploading = false;
const byte DNS_PORT = 53;

const String portalContent = "<body><form action=\"/\" method=\"POST\"><div><label>SSID</label><input name=\"ssid\" /><label>Password</label><input name=\"password\" /></div><input type=\"submit\" value=\"Save\" /></form></body>";

ESP8266WebServer _server(80);
DNSServer dnsServer;

wifiSettings readSettings(int start)
{
    EEPROM.begin(50);
    wifiSettings ret = wifiSettings();
    EEPROM.get(start, ret);
    EEPROM.end();
    return ret;
}

void writeSettings(int start, wifiSettings settings)
{
    EEPROM.begin(50);
    EEPROM.put(start, settings);
    EEPROM.commit();
    EEPROM.end();
}

void _clearConfig()
{
    wifiSettings clearSettings = {false, "", ""};
    writeSettings(_eprom_start, clearSettings);
}

void waitForWifi()
{
    int count = 0;
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Connection Failed! retrying...");
        delay(1000);
        if (count++ > 20)
        {
            if (!isUploading)
            {
                _clearConfig();
                ESP.restart();
            }
        }
        else {
            WiFi.reconnect();
        }
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void _handleWeb()
{
    _server.send(200, "text/html", portalContent);
}

void scpy(void *destination, String source, uint8_t size)
{
    char buff[size];
    source.toCharArray(buff, size);
    memcpy(destination, buff, size);
}

void _handleSave()
{
    if (!_server.hasArg("ssid") || !_server.hasArg("password"))
    {
        _server.send(400, "text/plain", "400: Invalid Request");
        return;
    }
    wifiSettings tmp = {true, "", ""};
    scpy(tmp.ssid, _server.arg("ssid"), 35);
    scpy(tmp.password, _server.arg("password"), 30);

    writeSettings(_eprom_start, tmp);
    _server.send(200, "text/html", "Saved... rebooting...");
    ESP.reset();
}

void _setupWebServer()
{
    _server.begin();
    _server.on("/", HTTP_GET, _handleWeb);
    _server.on("/reboot", HTTP_GET, []() {
        ESP.reset();
    });
    _server.on("/", HTTP_POST, _handleSave);
    _server.onNotFound([]() {
        const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.5.1/\" /></head><body><p>redirecting...</p></body>";
        _server.send(200, "text/html", metaRefreshStr);
    });
}

void _setupOTA()
{
    ArduinoOTA.onStart([]() {
        isUploading = true;
        String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        isUploading = false;
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        isUploading = false;
        Serial.printf("Error[%u]: ", error);
    });
    ArduinoOTA.begin();
}

OTA_WIFI::OTA_WIFI()
{
}

void OTA_WIFI::save(wifiSettings settings)
{
    writeSettings(_eprom_start, settings);
}

void OTA_WIFI::_startWifi()
{
    if (_currentSettings.hasConfig)
    {
        WiFi.mode(WIFI_STA);
        Serial.print("Connecting to ");
        Serial.println(_currentSettings.ssid);

        WiFi.begin(_currentSettings.ssid, _currentSettings.password);
        waitForWifi();
    }
    else
    {
        IPAddress staticIP(192, 168, 5, 1);
        IPAddress subnet(255, 255, 255, 0);

        WiFi.mode(WIFI_AP);
        char name[18];
        sprintf(name, "leet-%08X\n", ESP.getChipId());

        WiFi.softAPConfig(staticIP, staticIP, subnet);
        WiFi.softAP(name);

        Serial.print("Setup IP address ");

        dnsServer.start(DNS_PORT, "*", staticIP);

        Serial.print(WiFi.softAPIP());
        Serial.print(", AP: ");
        Serial.println(name);
    }
}

void OTA_WIFI::setup()
{
    _currentSettings = readSettings(_eprom_start);
    _startWifi();
    _setupWebServer();
    _setupOTA();
}

boolean OTA_WIFI::hasConfig() {
    return _currentSettings.hasConfig;
}

void OTA_WIFI::loop()
{
    ArduinoOTA.handle();
    _server.handleClient();
    if (!_currentSettings.hasConfig)
    {
        dnsServer.processNextRequest();
    }
}
