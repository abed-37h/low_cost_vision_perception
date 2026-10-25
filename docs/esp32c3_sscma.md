# ESP32C3 and SSCMA

This document covers how the XIAO ESP32C3 communicates with the Grove Vision AI V2: setting up the toolchain, installing the SSCMA Arduino library, how the project uses it, and a reference for the library API.

## Overview

SSCMA (Seeed SenseCraft Model Assistant) provides a high-level interface for interacting with supported AI devices. The Seeed Arduino SSCMA library wraps the underlying AT protocol, so an application can initialize the device, invoke inference and read results without building every AT command by hand.

The communication flow is:

`XIAO ESP32C3 → UART → AT protocol → Grove Vision AI V2`

The library also supports I²C, but this project uses UART (see [Wiring](../README.md#wiring) for the pins and baud rate).

The tracker uses the library to initialize the UART link (`begin()`). It then sends the invoke command itself and parses the JSON response with ArduinoJson, so the other API functions below are not used by the tracker.

## Setup

### Board Support

Add the ESP32 board package URL (from the [XIAO ESP32C3 wiki](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#software-preparation)), then install the core:

```text
https://jihulab.com/esp-mirror/espressif/arduino-esp32.git
```

**arduino-cli:**

```bash
arduino-cli config add board_manager.additional_urls https://jihulab.com/esp-mirror/espressif/arduino-esp32.git
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

Find the serial port of the connected board with `arduino-cli board list`.

**Arduino IDE:**

1. Open **File > Preferences** and add the URL above to **Additional Boards Manager URLs**.
2. Open **Tools > Board > Boards Manager...**, search for `esp32` and install the latest **esp32** package.
3. Select **Tools > Board > ESP32 Arduino > XIAO_ESP32C3**. The list is long, so scroll down to find it.
4. Connect the board over USB and select its port in **Tools > Port**. The name depends on your computer; on Windows it looks like `COM3`.

### Libraries

The project needs these libraries (names as shown by `arduino-cli lib list`):

| Library | Install from | Repository |
|---|---|---|
| ArduinoJson | Library Manager | [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson) |
| ArxTypeTraits | Library Manager (dependency of Eigen) | [hideakitai/ArxTypeTraits](https://github.com/hideakitai/ArxTypeTraits) |
| Seeed Arduino SSCMA | Repository (GitHub or ZIP) | [Seeed-Studio/Seeed_Arduino_SSCMA](https://github.com/Seeed-Studio/Seeed_Arduino_SSCMA) |
| Eigen (ArduinoEigen, provides `ArduinoEigen.h`) | Repository (GitHub or ZIP) | [hideakitai/ArduinoEigen](https://github.com/hideakitai/ArduinoEigen) |

**From the Library Manager:**

```bash
arduino-cli lib install ArduinoJson
arduino-cli lib install ArxTypeTraits
```

In the Arduino IDE, use **Sketch > Include Library > Manage Libraries...**, search for the library and click **Install**.

**From a repository:**

- Arduino IDE: open the repository, click **Code > Download ZIP**, then open **Sketch > Include Library > Add .ZIP Library...** and select the file. A **Library added to your libraries** notification confirms the installation.
- `arduino-cli`: installing from a git URL has to be enabled first:

```bash
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/Seeed-Studio/Seeed_Arduino_SSCMA
arduino-cli lib install --git-url https://github.com/hideakitai/ArduinoEigen
```

## SSCMA Arduino Library API

The `Seeed_Arduino_SSCMA` library provides high-level functions for communicating with the Grove Vision AI V2.

### `begin()`

Initializes communication with the Grove Vision AI V2.

#### UART

```cpp
bool begin(
    HardwareSerial *serial,
    int32_t rst = -1,
    uint32_t baud = 921600,
    uint32_t wait_delay = 2
);
```

| Parameter | Description |
|---|---|
| `serial` | Hardware serial interface |
| `rst` | Reset pin; `-1` if not used |
| `baud` | UART baud rate; default is `921600` |
| `wait_delay` | Communication delay in milliseconds |

**Returns:** `true` if initialization succeeds; otherwise `false`.

#### I²C

```cpp
bool begin(
    TwoWire *wire = &Wire,
    int32_t rst = -1,
    uint16_t address = 0x62,
    uint32_t wait_delay = 2,
    uint32_t clock = 400000
);
```

| Parameter | Description |
|---|---|
| `wire` | I²C interface to use |
| `rst` | Reset pin; `-1` if not used |
| `address` | I²C device address; default is `0x62` |
| `wait_delay` | Communication delay in milliseconds |
| `clock` | I²C clock frequency in Hz; default is `400000` |

**Returns:** `true` if initialization succeeds; otherwise `false`.

### `invoke()`

Requests the Grove Vision AI V2 to perform inference using the currently deployed model.

```cpp
int invoke(
    int times = 1,
    bool filter = false,
    bool show = false
);
```

| Parameter | Description |
|---|---|
| `times` | Number of inference invocations |
| `filter` | If `true`, only sends the result when it differs from the previous frame's result, based on geometry and score |
| `show` | If `true`, requests image data in the response |

**Returns:** A status code. `0` (`CMD_OK`) indicates success. Other values indicate an error; for example, `3` (`CMD_ETIMEOUT`) indicates that the operation timed out.

### `available()`

Checks the amount of data available to be read from the Grove Vision AI V2.

```cpp
int available();
```

**Returns:** Number of bytes currently available.

### `read()`

Reads data received from the Grove Vision AI V2.

```cpp
int read(char *data, int length);
```

| Parameter | Description |
|---|---|
| `data` | Buffer where received data is stored |
| `length` | Maximum number of bytes to read |

**Returns:** Number of bytes actually read.

### `write()`

Sends data to the Grove Vision AI V2.

```cpp
int write(const char *data, int length);
```

| Parameter | Description |
|---|---|
| `data` | Data buffer to send |
| `length` | Number of bytes to send |

**Returns:** Number of bytes written.

### `boxes()`

Provides object-detection results from the most recent inference.

```cpp
std::vector<boxes_t>& boxes();
```

**Returns:** A reference to a vector of `boxes_t` structures.

| Member | Type | Description |
|---|---|---|
| `x` | `uint16_t` | Bounding-box x coordinate |
| `y` | `uint16_t` | Bounding-box y coordinate |
| `w` | `uint16_t` | Bounding-box width |
| `h` | `uint16_t` | Bounding-box height |
| `score` | `uint8_t` | Confidence score |
| `target` | `uint8_t` | Target/class ID |

### `classes()`

Provides classification results from the most recent inference.

```cpp
std::vector<classes_t>& classes();
```

**Returns:** A reference to a vector of `classes_t` structures.

| Member | Type | Description |
|---|---|---|
| `target` | `uint8_t` | Target/class ID |
| `score` | `uint8_t` | Confidence score |

### `points()`

Provides point/keypoint results from the most recent inference.

```cpp
std::vector<point_t>& points();
```

**Returns:** A reference to a vector of `point_t` structures.

| Member | Type | Description |
|---|---|---|
| `x` | `uint16_t` | X coordinate |
| `y` | `uint16_t` | Y coordinate |
| `z` | `uint16_t` | Z value |
| `score` | `uint8_t` | Confidence score |
| `target` | `uint8_t` | Target/class ID |

### `keypoints()`

Provides keypoint-detection results.

```cpp
std::vector<keypoints_t>& keypoints();
```

**Returns:** A reference to a vector of `keypoints_t` structures.

Each result contains:

- `box`: bounding box associated with the detected object
- `points`: vector of points associated with that object

### `perf()`

Provides performance information from the most recent inference.

```cpp
perf_t& perf();
```

**Returns:** A reference to the `perf_t` structure.

| Member | Description |
|---|---|
| `prepocess` | Preprocessing time |
| `inference` | Inference time |
| `postprocess` | Post-processing time |

### `last_image()`

Retrieves the most recent image received from the Grove Vision AI V2.

```cpp
String last_image();
```

**Returns:** A `String` containing the image data returned by the Grove Vision AI V2, stored as a Base64-encoded string.

The image is available when `invoke()` is called with `show = true` and the response contains image data.

### `save_jpeg()`

Requests the Grove Vision AI V2 to save the current image as a JPEG.

```cpp
int save_jpeg();
```

**Returns:** A status code. `0` (`CMD_OK`) indicates success. Other values indicate an error.

### `reset()`

Resets the Grove Vision AI V2.

```cpp
void reset();
```

### `info()`

Retrieves device information.

```cpp
String info(bool cache = true);
```

| Parameter | Description |
|---|---|
| `cache` | Use the previously retrieved value when available |

**Returns:** Device information as a `String`.

### `ID()` and `name()`

Retrieve the device ID and device name.

```cpp
char* ID(bool cache = true);
char* name(bool cache = true);
```

**Returns:** A pointer to the corresponding character string.

## References

- [Seeed Arduino SSCMA](https://github.com/Seeed-Studio/Seeed_Arduino_SSCMA) (includes the library's examples)
- [SSCMA AT Protocol documentation](https://github.com/Seeed-Studio/SSCMA-Micro/blob/1.0.x/docs/protocol/at_protocol.md)
- [Grove Vision AI V2 — AT Protocol](https://wiki.seeedstudio.com/grove_vision_ai_v2_at/)
- [XIAO ESP32C3 — Getting Started](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
- [Arduino IDE download page](https://www.arduino.cc/en/software/)