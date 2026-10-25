#include "app_tracker.h"
#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "packet.h"

#define ENABLE_JPEG_ENC 0
#define ENABLE_TRACKER  0

#if ENABLE_TRACKER == 1
#include "BYTETracker.h"

const int FRAME_RATE = 10;
const int TRACK_BUFFER = 30;

BYTETracker tracker(FRAME_RATE, TRACK_BUFFER);

int missedFrames = 0;
int trackId = -1;

#endif

HardwareSerial atSerial(0);
HardwareSerial crSerial(1);

SSCMA AI;
JsonDocument doc;

const int16_t CENX = 120;
char response[10240];

void initApp() {
    atSerial.setRxBufferSize(4096);
    atSerial.begin(921600, SERIAL_8N1, -1, -1);
    crSerial.begin(115200, SERIAL_8N1, -1, 10);
    AI.begin(&atSerial);
    Serial.begin(115200);
    while (!Serial || !atSerial || !crSerial) delay(500);

#if STREAM_CHANNEL == 0
    const char* invoke_cmd = "AT+INVOKE=-1,0,1\r";
    #else
    const char* invoke_cmd = "AT+INVOKE=-1,0,0\r";
#endif
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

#if STREAM_CHANNEL != 0
    String b64Image = doc["data"]["image"];

    if (b64Image.length() == 0) {
        return false;
    }

    frame = b64Image;
#endif

    return true;
}

#if STREAM_CHANNEL != 0 && ENABLE_JPEG_ENC == 1
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

#if STREAM_CHANNEL != 0 && ENABLE_JPEG_ENC == 1
    if (!b64ToJpeg(frame)) {
        return;
    }
#endif

    JsonArray boxesJson = doc["data"]["boxes"].as<JsonArray>();

#if STREAM_CHANNEL != 0
    stream.println("IMG_START");
#endif

    if (!boxesJson.isNull() && boxesJson.size()) {
#if ENABLE_TRACKER == 1
        std::vector<BYTETracker::Object> objects;
#endif
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

#if ENABLE_TRACKER == 0
            int16_t boxCenter  = box.x;
            int16_t trackError = boxCenter - CENX;

            Packet packet;
            packet.detected = 1;
            packet.error    = trackError;

            sendPacket(packet, crSerial);
#endif
#if ENABLE_TRACKER == 1
            BYTETracker::Object obj;

            obj.rect.x      = box.x - box.w / 2.0f;
            obj.rect.y      = box.y - box.h / 2.0f;
            obj.rect.width  = box.w;
            obj.rect.height = box.h;

            obj.label = box.target;
            obj.prob  = box.score / 100.0f;

            objects.push_back(obj);
#endif
        }

#if ENABLE_TRACKER == 1
        std::vector<STrack> tracks = tracker.update(objects);

        bool found = false;

        for (auto& track : tracks) {
            stream.printf(
                "TRACK:%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                track.track_id,
                track.tlwh[0],
                track.tlwh[1],
                track.tlwh[2],
                track.tlwh[3],
                track.score
            );

            int16_t boxCenter  = (int16_t)(track.tlwh[0] + track.tlwh[2] / 2);
            int16_t trackError = boxCenter - CENX;

            if (trackId < 0) trackId = track.track_id;
            else if (trackId != track.track_id) continue;

            found = true;
            missedFrames = 0;

            Packet packet;
            packet.detected = 1;
            packet.error    = trackError;

            sendPacket(packet, crSerial);
        }

        if (!found && trackId >= 0) {
            missedFrames++;
            if (missedFrames > TRACK_BUFFER) {
                trackId      = -1;
                missedFrames = 0;
            }
        }
#endif
    }
    else {
        Packet packet;
        packet.detected = 0;
        packet.error    = 1000;

        sendPacket(packet, crSerial);
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
