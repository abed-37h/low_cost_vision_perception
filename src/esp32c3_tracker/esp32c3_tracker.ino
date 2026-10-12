#include <WiFi.h>
#include "app_tracker.h"
#include "env.h"

WiFiServer server(port);

void setup() {
    initApp();
#if STREAM_CHANNEL == 2
#if ENABLE_WIFI_CONFIG == 1
    IPAddress Local_IP(192, 168, 1, 100);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.config(Local_IP, gateway, subnet);
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    server.begin();

    Serial.printf("Server started listening at '%s:%u'\n", WiFi.localIP().toString(), port);
#endif

    Serial.println("Object Tracker Started on ESP32C3");
}

void loop() {
    double latency, fps;
#if STREAM_CHANNEL == 2
    static WiFiClient client = server.available();
    if (client) {
        while (client.connected()) {
            measurePerf(latency, fps, []() { handleInferenceResults(client); });
        }
        client.stop();
    }
#else
    measurePerf(latency, fps, []() { handleInferenceResults(Serial); });
#endif
    printPerf(latency, fps);
}
