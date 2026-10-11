#ifndef APP_TRACKER_H
#define APP_TRACKER_H

#include "Stream.h"

void initApp();
bool getInferenceResults();
void handleInferenceResults();
void measurePerf(double& latency, double& fps, void (*fn)());
void printPerf(double latency, double fps);

#endif