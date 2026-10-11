#include "app_tracker.h"

void setup() {
    initApp();
}

void loop() {
    double latency, fps;
    measurePerf(latency, fps, handleInferenceResults);
    printPerf(latency, fps);
}
