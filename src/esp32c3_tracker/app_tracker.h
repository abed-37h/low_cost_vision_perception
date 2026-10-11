#ifndef APP_TRACKER_H
#define APP_TRACKER_H

#include "Stream.h"

#define STREAM_CHANNEL     0
#define ENABLE_WIFI_CONFIG 0

void initApp();
bool getInferenceResults(String& frame);
void handleInferenceResults(Stream& stream);
void measurePerf(double& latency, double& fps, void (*fn)());
void printPerf(double latency, double fps);

#endif