#include "app_tracker.h"
#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#define ENABLE_JPEG_ENC 0

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

bool getInferenceResults(String& frame) {
    size_t n = atSerial.readBytesUntil('\n', response, sizeof(response) - 1);
    response[n] = '\0';

    doc.clear();

    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.printf("JSON error: %s\n", error.c_str());
        return false;
    }

    String b64Image = doc["data"]["image"];

    if (b64Image.length() == 0) {
        return false;
    }

    frame = b64Image;
    return true;
}

#if ENABLE_JPEG_ENC == 1
#include "mbedtls/base64.h"
#define MAX_JPEG_SIZE (15 * 1024)
uint8_t jpeg[MAX_JPEG_SIZE];
size_t jpegSize = 0;

bool b64ToJpeg(String b64Image) {
    size_t b64Size = b64Image.length();

    size_t outputSize = 0;
    mbedtls_base64_decode(nullptr, 0, &outputSize, (const unsigned char*) b64Image.c_str(), b64Size);

    if (outputSize == 0 || outputSize > MAX_JPEG_SIZE) {
        Serial.printf("Invalid JPEG size: %d\n", outputSize);
        return false;
    }

    int ret = mbedtls_base64_decode(jpeg, MAX_JPEG_SIZE, &outputSize, (const unsigned char*) b64Image.c_str(), b64Size);

    if (ret != 0) {
        Serial.printf("Base64 decode failed: %d\n", ret);
        return false;
    }

    jpegSize = outputSize;

    return true;
}

#endif

void handleInferenceResults(Stream& stream) {
    String frame = "";
    if (!getInferenceResults(frame)) {
        return;
    }

#if ENABLE_JPEG_ENC == 1
    if (!b64ToJpeg(frame)) {
        return;
    }
#endif

    JsonArray boxesJson = doc["data"]["boxes"].as<JsonArray>();

#if STREAM_CHANNEL != 0
    stream.println("IMG_START");
#endif

    if (!boxesJson.isNull() && boxesJson.size()) {
        /* ----------------- Get image processing and inference time ---------------- */
        JsonArray perfJson = doc["data"]["perf"].as<JsonArray>();
        perf_t perf;

        perf.prepocess   = perfJson[0].as<uint16_t>();
        perf.inference   = perfJson[1].as<uint16_t>();
        perf.postprocess = perfJson[2].as<uint16_t>();
        /* -------------------------------------------------------------------------- */
        
        stream.printf(
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

            stream.printf(
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

#if STREAM_CHANNEL != 0
#if ENABLE_JPEG_ENC == 1
    stream.printf("IMG_BIN:%u\n", jpegSize);
    stream.write(jpeg, jpegSize);
#else
    stream.println(frame);
#endif
    
    stream.println("IMG_END");
#endif
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
