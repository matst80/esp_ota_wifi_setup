#ifndef mqtt_h
#define mqtt_h

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

typedef std::function<void(byte* message,unsigned int length)> THandlerFunction;

boolean cmp(byte* a, const char* b, unsigned int l);

class mqttSub {
  public:
    mqttSub();
    mqttSub(const char* topic, THandlerFunction hdlr);
    const char* topic;
    THandlerFunction handler;
};

class mqttHelper {
    public:
        mqttHelper(const char* server, int mqttPort);
        void loop();
        void connect(const char* clientName);
        void publish(char* topic, char* message);
        void subscribe(const char* topic, THandlerFunction handler);
    private:
        const char* _clientName;
};

#endif