#include "app_tracker.h"
#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <Arduino.h>

HardwareSerial atSerial(0);

SSCMA AI;
JsonDocument doc;

char response[10240];

void initApp() {
    atSerial.begin(921600, SERIAL_8N1, -1, -1);
    AI.begin(&atSerial);
    Serial.begin(115200);

    while (!atSerial) delay(1000);
    
    const char* invoke_cmd = "AT+INVOKE=-1,0,1\r";

    atSerial.write(invoke_cmd, strlen(invoke_cmd));

    while (!atSerial.available()) delay(1000);
    size_t n = atSerial.readBytesUntil('\n', response, sizeof(response) - 1);

    Serial.println("ESP32C3 Tracker Started");
}

bool getInferenceResults() {
    size_t n = atSerial.readBytesUntil('\n', response, sizeof(response) - 1);
    response[n] = '\0';

    doc.clear();

    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.printf("JSON error: %s\n", error.c_str());
        return false;
    }

    return true;
}

void handleInferenceResults() {
    getInferenceResults();

    JsonArray boxesJson = doc["data"]["boxes"].as<JsonArray>();

    if (!boxesJson.isNull() && boxesJson.size()) {
        /* ----------------- Get image processing and inference time ---------------- */
        JsonArray perfJson = doc["data"]["perf"].as<JsonArray>();
        perf_t perf;

        perf.prepocess   = perfJson[0].as<uint16_t>();
        perf.inference   = perfJson[1].as<uint16_t>();
        perf.postprocess = perfJson[2].as<uint16_t>();
        /* -------------------------------------------------------------------------- */
        
        Serial.printf(
            "PERF: prepocess=%u, inference=%u, postpocess=%u\n",
            perf.prepocess,
            perf.inference,
            perf.postprocess
        );

        for (JsonArray boxJson : boxesJson) {
            /* ------------------------- Get detected object box ------------------------ */
            boxes_t box;

            box.x      = boxJson[0].as<uint16_t>();
            box.y      = boxJson[1].as<uint16_t>();
            box.w      = boxJson[2].as<uint16_t>();
            box.h      = boxJson[3].as<uint16_t>();
            box.score  = boxJson[4].as<uint8_t>();
            box.target = boxJson[5].as<uint8_t>();
            /* -------------------------------------------------------------------------- */

            Serial.printf(
                "BOX:%u,%u,%u,%u,%u,%u\n",
                box.x,
                box.y,
                box.w,
                box.h,
                box.score,
                box.target
            );
        }
    }
}

void measurePerf(double& latency, double& fps, void (*fn)()) {
    unsigned long start = micros();
    fn();
    unsigned long end = micros();

    latency = (end - start) / 1000.0f;
    fps = 1000.0f / latency;
}

void printPerf(double latency, double fps) {
    Serial.printf("Elapsed Time = %.2f\n", latency);
    Serial.printf("FPS = %.2f\n", fps);
}
