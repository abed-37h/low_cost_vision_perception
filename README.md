# Low-Cost Vision Perception

A low-cost vision system that lets a car robot track an object moving horizontally and keep it centered in the camera frame.

## Hardware

- Grove Vision AI Module V2 with OV5647-67 camera
- XIAO ESP32C3
- Arduino Mega
- Prebuilt car robot

## Pipeline

1. The camera feeds the Grove Vision AI V2, which runs an object detection model.
2. The XIAO ESP32C3 reads the detection results over UART.
3. The ESP32C3 sends target information to the Arduino Mega as serial packets.
4. The Arduino Mega steers the robot to keep the target centered.

## How It Works

For every frame, the Vision AI module reports the bounding box of each detected object. The ESP32C3 takes the horizontal center of the box and subtracts the center of the frame. The result is the **error**, in pixels: zero means the target is centered, a positive value means it is to the right, and a negative value means it is to the left. The frame center is a constant in the tracker code (currently `120`); set it to half of your frame width.

The ESP32C3 sends the error to the Mega in a small packet (see [Packet Protocol](#packet-protocol)). The Mega turns the robot in place toward the target with a PD controller: the turning speed grows with the error, and a filtered derivative term damps overshoot. The robot stops when:

- the error is inside a small deadband (the target is centered),
- the packet says no target was detected, or
- no packet has arrived for 150 ms.

Optionally, the ESP32C3 can run BYTETracker to follow one object consistently across frames when several are visible. The camera stream to a PC is also optional and only used for monitoring.

## Repository Structure

| Path | Description |
|---|---|
| `src/esp32c3_tracker/` | ESP32C3 side: reads detections, computes the error, sends packets, optional camera stream |
| `src/mega_controller/` | Mega side: receives packets and controls the robot |
| `libraries/packet/` | Packet format shared by both sides |
| `libraries/BYTETracker/` | Object tracker, taken from the Seeed_Arduino_SSCMA `camera_web_server` example (license included) |
| `libraries/stem/` | Robot control library provided by the supervisor |
| `scripts/` | `arduino-cli` build and upload scripts |
| `tools/camera_preview/` | Python viewer for the camera stream |
| `Makefile` | Shortcuts for the scripts |

## Prerequisites

- `arduino-cli` (with the `esp32:esp32` and `arduino:avr` cores) or the Arduino IDE. The
  scripts below use `arduino-cli`; with the IDE, copy the sketches and the `libraries/`
  folders into your Arduino folders manually (not tested).
- Arduino libraries (install steps: [ESP32C3 and SSCMA documentation](docs/esp32c3_sscma.md#libraries)):
  - Seeed_Arduino_SSCMA
  - ArduinoJson
  - Eigen (ArduinoEigen, for BYTETracker)
  - ArxTypeTraits (required by Eigen)
- Python 3, preferably in a virtual environment (only for the camera preview tool)
- A detection model deployed to the Grove Vision AI V2 (see [Model Deployment documentation](docs/model_deployment.md))

## Wiring

| XIAO ESP32C3 | Connected to |
|---|---|
| D6 (TX) | Grove Vision AI V2 RXD |
| D7 (RX) | Grove Vision AI V2 TXD |
| 3.3V | Grove Vision AI V2 3.3V |
| D10 (TX) | Arduino Mega RX3 (pin 15) |
| GND | Vision AI GND and Mega GND (common ground) |
| 5V | Arduino Mega 5V (**proposed, not tested**) |

Serial links: Vision AI at 921600 baud, Mega at 115200 baud (`Serial3`).

> **Warning:** the ESP32C3 5V to Mega 5V connection has not been tested. If the Mega is
> powered another way (USB or the robot battery), do not connect the 5V pins. The common
> GND is the only connection the boards need.

## Build and Upload

> **Run all `make` commands from the repository root.** The Makefile lives there and is
> not global, so `make` can't find it from another directory.

Both commands take the same arguments. Everything after `--` is passed to the script.

    make build  -- --board <board> <sketch_dir>                          # compile only
    make upload -- --board <board> --port <port> --compile <sketch_dir>  # compile and upload

`build` only compiles, so it can be used to check the code without hardware. Find your
port with `arduino-cli board list`. The scripts add `libraries/` to the build
automatically, so the `packet`, `stem` and `BYTETracker` libraries need no extra setup.

| Flag | Meaning |
|---|---|
| `-b`, `--board` | `xiao_esp32c3`, `arduino_mega` or `arduino_uno` |
| `-p`, `--port` | Serial port, for example `/dev/ttyACM0` or `COM3` (upload only) |
| `-c`, `--compile` | Optional. Compile before uploading (upload only) |
| `-m`, `--monitor` | Optional. Open the serial monitor after upload (upload only) |
| `<path>` or `--path` | Sketch directory |

## Usage

### ESP32C3 tracker (`src/esp32c3_tracker`)

Before building, copy `env.example.h` to `env.h` in the same folder (`env.h` is git-ignored).

Options are compile-time settings; edit them before building:

| Option | Defined in | Meaning |
|---|---|---|
| `STREAM_CHANNEL` | `app_tracker.h` | `0` no stream (default), `1` USB serial, `2` TCP (Wi-Fi settings in `env.h`) |
| `ENABLE_WIFI_CONFIG` | `app_tracker.h` | `1` uses the static IP set in `esp32c3_tracker.ino` |
| `ENABLE_TRACKER` | `app_tracker.cpp` | `1` enables BYTETracker, which is only built when this is `1` (default `0`) |
| `ENABLE_JPEG_ENC` | `app_tracker.cpp` | `1` sends frames as binary JPEG instead of base64 (default `0`) |

> Note (TCP): with `STREAM_CHANNEL 2` the tracker only runs while a viewer is connected.

Compile only:

    make build -- --board xiao_esp32c3 ./src/esp32c3_tracker

Compile and upload:

    make upload -- --board xiao_esp32c3 --port /dev/ttyACM0 --compile ./src/esp32c3_tracker

### Mega controller (`src/mega_controller`)

Receives packets on `Serial3` and runs the PD controller described in [How It Works](#how-it-works). Tuning constants are at the top of the sketch:

| Constant | Value | Meaning |
|---|---|---|
| `Kp`, `Kd` | `0.5`, `0.075` | Proportional and derivative gains |
| `EMAX` | `20` | Deadband: the robot stops when the absolute error is below this |
| `VMAX` | `30` | Maximum turning speed |

The packet timeout is 150 ms. Debug output goes to `Serial` at 9600 baud. Board setup and the `stem` motor functions are described in the [Arduino Mega and the stem Library documentation](docs/mega_controller.md).

Compile only:

    make build -- --board arduino_mega ./src/mega_controller

Compile and upload:

    make upload -- --board arduino_mega --port /dev/ttyACM1 --compile ./src/mega_controller

Add `--monitor` to open the serial monitor after upload.

### Camera preview (`tools/camera_preview`)

Shows the camera stream with detection boxes. When `ENABLE_TRACKER` is `1`, it shows the tracker's boxes and IDs instead. Both frame formats (base64 and binary JPEG) are handled automatically. The ESP32C3 must have streaming enabled: `STREAM_CHANNEL 1` for serial, `STREAM_CHANNEL 2` for TCP.

Set up a virtual environment once (from the repository root):

    python3 -m venv .venv
    source .venv/bin/activate        # Windows: .venv\Scripts\activate
    pip install -r tools/camera_preview/requirements.txt

All flags are optional:

| Flag | Default | Notes |
|---|---|---|
| `--mode` | `serial` | `serial` or `tcp` |
| `--port` | `COM3` | Serial mode only |
| `--baud` | `115200` | Serial mode only |
| `--host` | `192.168.1.100` | TCP mode only |
| `--tcp-port` | `8080` | TCP mode only |

Then run **one** of the following, depending on the stream channel you chose.

Serial (`STREAM_CHANNEL 1`):

    python tools/camera_preview/camera_preview.py --mode serial --port COM3 --baud 115200

TCP (`STREAM_CHANNEL 2`):

    python tools/camera_preview/camera_preview.py --mode tcp --host 192.168.1.100 --tcp-port 8080

Press `Esc` to quit.

## Packet Protocol

The ESP32C3 sends the Mega one 7-byte packet for each detected object in a frame, or a single no-target packet when nothing is detected.

| Byte | Field | Description |
|---|---|---|
| 0-1 | Header | `0xAA 0x55` |
| 2 | Type | `0x01` |
| 3 | Detected | `1` if a target was found, otherwise `0` |
| 4-5 | Error | `int16`, little-endian: target center x minus frame center x (positive = target to the right) |
| 6 | Checksum | XOR of type, detected and the two error bytes |

When nothing is detected, the packet has `detected = 0` and `error = 1000`.

## Status

This code was rebuilt from an earlier tested prototype with the same logic. The restructured version has not been fully re-tested yet.