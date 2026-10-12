#include <stem.h>
#include "packet.h"

#define EMAX 20
#define VMAX 30
#define VMIN 2

#define VDEF 30

Robot robot;

unsigned long lastPacketTime = 0;
const unsigned long PACKET_TIME = 150;

float Kp = 0.5;
float Kd = 0.075;

// Derivative filtering.
// Higher alpha = more smoothing / slower response.
// 0.7 is a reasonable starting point.
const float DERIVATIVE_ALPHA = 0.7;

int16_t prevError = 0;
float prevDerivative = 0;

unsigned long prevTime = 0;
bool havePrevious = false;

void turn(int vel = VDEF) {
    robot.moveMotor(motorL, vel);
    robot.moveMotor(motorR, -vel);
}

void stop() {
    robot.moveMotor(motorL, 0);
    robot.moveMotor(motorR, 0);

    // Reset PID history when stopped.
    prevError = 0;
    prevDerivative = 0;
    prevTime = millis();
    havePrevious = false;
}

void turnLeft(int vel = VDEF) {
    robot.moveMotor(motorL, -vel);
    robot.moveMotor(motorR, vel);
}

void turnRight(int vel = VDEF) {
    robot.moveMotor(motorL, vel);
    robot.moveMotor(motorR, -vel);
}

void trackObject(Packet& p) {
    if (!p.detected) {
        stop();
        return;
    }

    unsigned long now = millis();

    float rawDerivative = 0;

    if (havePrevious) {
        float dt = (now - prevTime) / 1000.0f;

        if (dt > 0.001f) {
            rawDerivative = (p.error - prevError) / dt;
        }
    }

    // Low-pass filter the derivative to reduce vision noise.
    float derivative =
        DERIVATIVE_ALPHA * prevDerivative +
        (1.0f - DERIVATIVE_ALPHA) * rawDerivative;

    float output = Kp * p.error + Kd * derivative;

    prevError = p.error;
    prevDerivative = derivative;
    prevTime = now;
    havePrevious = true;

    if (abs(p.error) < EMAX) {
        stop();
        return;
    }

    int velocity = constrain((int)fabs(output), 0, VMAX);

    if (output > 0) {
        turnRight(velocity);
    }
    else {
        turnLeft(velocity);
    }
}

void setup() {
    stop();

    Serial.begin(9600);
    Serial3.begin(115200);

    while (!Serial || !Serial3)
        delay(1000);

    Serial.println("Robot Control Started on Arduino Mega");
}

void loop() {
    Packet packet;

    if (receivePacket(packet, Serial3)) {
        trackObject(packet);
        lastPacketTime = millis();
    }

    if (millis() - lastPacketTime > PACKET_TIME) stop();
}
