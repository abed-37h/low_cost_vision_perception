# Arduino Mega and the stem Library

This document covers developing for the Arduino Mega robot: setting up the toolchain, installing the `stem` library, and a reference for the motor control functions used in the project.

## Overview

`stem` is the control library for the USAL robot. It drives the two wheel motors and reads the wheel encoders, and it also provides sensor, LCD and servo functions. The communication flow is:

`Arduino Mega → stem library → robot motors`

The project uses the motor control part (`Robot`, `moveMotor()`); the rest is listed under [Other Features](#other-features).

## Setup

### Board Support

The Mega uses the `arduino:avr` core.

**arduino-cli:**

```bash
arduino-cli core install arduino:avr
```

Find the serial port of the connected board with `arduino-cli board list`.

**Arduino IDE:**

1. Open **Tools > Board > Arduino AVR Boards** and select **Arduino Mega or Mega 2560**.
2. Open **Tools > Processor** and select the one that matches your board.
3. Connect the board over USB and select its port in **Tools > Port**.

### Libraries

`stem` is included in this repository (`libraries/stem`) and needs no external libraries. The build scripts add `libraries/` automatically. With the Arduino IDE, copy the `stem` folder into your sketchbook `libraries` folder.

## stem Library API

### Pins and Resources

Creating a `Robot` configures the library's pins and attaches the encoder interrupts. Pins used for motion:

| Function | Pins |
|---|---|
| Right motor (`motorR`) driver | 10, 11 |
| Left motor (`motorL`) driver | 46, 12 |
| Wheel encoder A | 2, 3 |
| Wheel encoder B | 18, 19 |

Pin 19 is also RX1 on the Mega, so `Serial1` cannot be used for another device. `Serial3` (TX3 on pin 14, RX3 on pin 15) does not overlap with any pin used by `stem`.

The library's speed control loop uses **Timer3**, so avoid other libraries that use Timer3 in the same sketch.

### `Robot`

Creates the robot object and sets up the pins and encoder interrupts.

```cpp
Robot(setController = controlON);
```

| Parameter | Description |
|---|---|
| `setController` | `controlON` (default) enables the library's encoder-based speed control; `controlOFF` disables it |

Usage:

```cpp
#include <stem.h>

Robot robot;
```

With speed control on, the constructor sets the default speed-loop gains (Kp 10, Ki 1, Kd 0.3) and a 10 ms loop interval.

`robot.begin()` initializes the LCD and servo. Call it before using `displayLCD()` or the `motorP` servo.

### `moveMotor()`

Drives one motor.

```cpp
void moveMotor(motorDirection direction, int power = 0);
```

| Parameter | Description |
|---|---|
| `direction` | `motorL` or `motorR` for the wheels; `motorP` for the servo |
| `power` | Signed value; the sign sets the direction and `0` stops that motor |

With speed control on (the default), `power` is the target speed of that wheel. The library measures the wheel speed from the encoders and corrects the motor output every 10 ms on Timer3. With speed control off, `power` drives the motor directly. The value is limited to ±100, or ±200 once speed control is running.

Stopping both motors (`power = 0` on `motorL` and `motorR`) also stops the control loop.

### `motorDirection`

```cpp
enum motorDirection { motorL, motorR, motorP, both, stopMotors };
```

### Motion Examples

Wheels driven with the same sign move the robot straight; opposite signs turn it in place. The library's `moveForward` and `pointTurn` examples use this pattern.

| Motion | `motorL` | `motorR` |
|---|---|---|
| Move straight | `+speed` | `+speed` |
| Turn in place | `+speed` | `-speed` |
| Turn in place (other way) | `-speed` | `+speed` |
| Stop | `0` | `0` |

Which sign turns the robot to the left or right depends on how the motors are wired, so test it before relying on it.

```cpp
robot.moveMotor(motorL, 30);
robot.moveMotor(motorR, -30);   // turn in place

robot.moveMotor(motorL, 0);
robot.moveMotor(motorR, 0);     // stop
```

### Other Features

`stem` also provides:

- LCD display, encoder readings and target-distance moves
- touch, ultrasonic, line and sound sensors
- PID tuning (`tunePID()`, `setPIDloopTime()`)

The project does not use these. See the sketches in `libraries/stem/examples/` for usage.

## References

- [Arduino IDE download page](https://www.arduino.cc/en/software/)