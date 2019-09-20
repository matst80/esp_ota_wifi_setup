#include "mqtt.h"
#include "otaWifi.h"

const char* mqttServer = "10.10.10.1";
const char* mqttClient = "tmpesp";
const char* testTopic = "esp/test";

mqttHelper mqtt(mqttServer, 1883);
OTA_WIFI ota_wifi;

void setup()
{
  Serial.begin(9600);
  ota_wifi.setup();
  mqtt.connect(mqttClient);
  
  mqtt.subscribe(testTopic, [](byte* data, u_int l){
    if (cmp(data,"ON",l)) {
      Serial.println("MATCHES");
    }
  });
}

void loop()
{
  ota_wifi.loop();
  mqtt.loop();
}