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