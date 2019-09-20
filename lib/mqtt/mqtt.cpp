#include "mqtt.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

mqttSub _subs[16];
uint8_t _currentSub;

THandlerFunction findHandler(char *topic)
{
    int len = sizeof(topic);
    for (int i = 0; i < _currentSub; i++)
    {
        if (memcmp(_subs[i].topic, topic, len) == 0)
        {
            return _subs[i].handler;
        }
    }
    return [](byte *data, unsigned int legth) {};
}

boolean cmp(byte *a, const char *b, unsigned int l)
{
    return memcmp(a, b, l) == 0;
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("got topic: ");
    Serial.println(topic),
        findHandler(topic)(payload, length);
}

mqttSub::mqttSub() {}

mqttSub::mqttSub(const char *topicToListenTo, THandlerFunction hdlr)
{
    topic = topicToListenTo;
    handler = hdlr;
}

mqttHelper::mqttHelper(const char *server, int port)
{
    client.setServer(server, port);
    client.setCallback(callback);
}

void mqttHelper::subscribe(const char *topic, THandlerFunction handler)
{
    _subs[_currentSub++] = mqttSub(topic, handler);
    client.subscribe(topic);
}

void mqttHelper::loop()
{
    if (!client.connected())
    {
        client.connect(_clientName);
    }
    client.loop();
}

void mqttHelper::publish(char *topic, char *data)
{
    client.publish(topic, data);
}

void mqttHelper::connect(const char *clientName)
{
    _clientName = clientName;
    client.connect(clientName);
}
