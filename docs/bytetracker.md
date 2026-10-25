# BYTETracker

This document covers the basic usage of the BYTETracker library: what it does, how to enable it, and a reference for the functions and fields used in the project.

## Overview

BYTETracker is a multi-object tracker based on the ByteTrack algorithm. It gives each detected object a persistent ID across frames, by matching new detections to existing tracks with IoU and smoothing their positions with a Kalman filter. The library in this repository (`libraries/BYTETracker`) is adapted by Seeed Studio from the `camera_web_server` example of Seeed_Arduino_SSCMA, and is released under the MIT license (see `libraries/BYTETracker/LICENSE`).

The flow is:

`detections (per frame) → BYTETracker::update() → tracks with IDs`

The project uses it as an option, to keep following the same object when several are visible.

## Setup

The library is included in this repository and registers as `ByteTracker`. It depends on two external libraries, Eigen (ArduinoEigen) and ArxTypeTraits; see [Libraries](esp32c3_sscma.md#libraries) for how to install them.

Enable it in the ESP32C3 tracker by setting `ENABLE_TRACKER` to `1` in `app_tracker.cpp`. The tracker is only built when this option is `1`. In your own sketch, include it with:

```cpp
#include "BYTETracker.h"
```

## BYTETracker API

### `BYTETracker`

Creates a tracker.

```cpp
BYTETracker(int frame_rate = 10, int track_buffer = 30);
```

| Parameter | Description |
|---|---|
| `frame_rate` | Expected number of frames per second; default is `10` |
| `track_buffer` | Frame buffer for lost tracks; default is `30` |

A track that stops being matched is removed after `frame_rate / 30 * track_buffer` frames (integer division of the result), which is `10` frames with the defaults.

### `BYTETracker::Object`

One detection passed to the tracker.

```cpp
struct Object {
    Rect4f rect;
    int    label;
    float  prob;
};
```

| Member | Type | Description |
|---|---|---|
| `rect` | `Rect4f` | Bounding box with members `x`, `y`, `width` and `height`; `x`, `y` is the **top-left** corner |
| `label` | `int` | Class ID |
| `prob` | `float` | Confidence from `0.0` to `1.0` |

Two conversions are needed when the detections come from SSCMA:

- SSCMA boxes are center-based (`x`, `y` is the box center), so use `x - w / 2` and `y - h / 2` for the top-left corner.
- SSCMA scores are `0` to `100`, so divide by `100` to get `prob`. The tracker's thresholds below assume `0.0` to `1.0`.

### `update()`

Updates the tracker with the detections of one frame and returns the current tracks.

```cpp
std::vector<STrack> update(const std::vector<Object>& objects);
```

| Parameter | Description |
|---|---|
| `objects` | Detections of the current frame (may be empty) |

**Returns:** A vector of `STrack`, one for each active track.

Call `update()` once per frame, including frames with no detections (pass an empty vector). The tracker counts frames internally, so lost tracks only expire when it is called every frame.

A new object is reported starting from its second frame of detection, once the tracker has matched it again. The only exception is the very first frame the tracker ever sees, where objects are reported immediately.

### `STrack`

One tracked object.

| Member | Type | Description |
|---|---|---|
| `track_id` | `int` | Track ID, unique for the run; IDs increase from `1` and are never reused |
| `tlwh` | `std::vector<float>` | Box as `{x, y, width, height}`, with `x`, `y` at the top-left corner |
| `tlbr` | `std::vector<float>` | Box as `{x1, y1, x2, y2}`: top-left and bottom-right corners |
| `score` | `float` | Confidence of the matched detection |
| `label` | `int` | Class ID |
| `state` | `int` | `TrackState`: `New`, `Tracked`, `Lost` or `Removed` |
| `is_activated` | `bool` | `true` once the track is confirmed |

The tracks returned by `update()` are already confirmed, so the box is in `tlwh`. To get the horizontal center of a track, use `tlwh[0] + tlwh[2] / 2`.

### Thresholds

These values are fixed in the `BYTETracker` constructor in `BYTETracker.cpp` and are not constructor parameters:

| Threshold | Value | Meaning |
|---|---|---|
| `track_thresh` | `0.5` | Detections at or above this are matched to existing tracks first; lower ones are only used in a second matching pass |
| `high_thresh` | `0.6` | A new track is only started from a detection with at least this score |
| `match_thresh` | `0.8` | Matching limit for the first pass (second pass uses `0.5`) |

Change them in `BYTETracker.cpp` if needed.

## Basic Usage

The code below is adapted from the tracker code in the project.

```cpp
#include <Seeed_Arduino_SSCMA.h>
#include "BYTETracker.h"

BYTETracker tracker(10, 30);   // frame_rate, track_buffer

void processFrame(std::vector<boxes_t>& boxes) {
    std::vector<BYTETracker::Object> objects;

    for (auto& box : boxes) {
        BYTETracker::Object obj;

        obj.rect.x      = box.x - box.w / 2.0f;   // center -> top-left
        obj.rect.y      = box.y - box.h / 2.0f;
        obj.rect.width  = box.w;
        obj.rect.height = box.h;

        obj.label = box.target;
        obj.prob  = box.score / 100.0f;           // 0-100 -> 0.0-1.0

        objects.push_back(obj);
    }

    // Call every frame, even when objects is empty.
    std::vector<STrack> tracks = tracker.update(objects);

    for (auto& track : tracks) {
        float centerX = track.tlwh[0] + track.tlwh[2] / 2;
        // track.track_id identifies the same object across frames
    }
}
```

To follow one object, remember the `track_id` of the first track you pick and ignore the other IDs. When that ID has not appeared for several frames, pick a new one.

## References

- [Seeed Arduino SSCMA `camera_web_server` example](https://github.com/Seeed-Studio/Seeed_Arduino_SSCMA/tree/main/examples/camera_web_server)
- [ByteTrack](https://github.com/FoundationVision/ByteTrack)