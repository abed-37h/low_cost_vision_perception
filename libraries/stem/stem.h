/*
  stem.h - Header file of the library to control the USAL robot.
  Created by Bilal A. Komati, December 15, 2020.
  Updated on March 8, 2023.
  This update integrates PID control for the motors.
  Last update on December 12, 2025 to integrate PID control inside timer3.
  The last update compute PID control inside timer3 and disable ServoRob from using timer3.
  Moreover, two functions to display the Encoder values have been added.
  Released for Al Mabarrat Schools.
*/

#ifndef stem_h
#define stem_h

#include "Arduino.h"
#include "utility/LiquidCrystal_I2C.h"
#include "utility/ServoRob.h"
#include <inttypes.h>

#define Ain1 10
#define Ain2 11
#define Bin1 46
#define Bin2 12
#define EncAC1 2
#define EncAC2 3
#define EncBC1 18
#define EncBC2 19
#define touchSensorPin 53
#define ultrasonicTrigPin1 1 //old version
#define ultrasonicTrigPin6 6 //new version
#define ultrasonicEchoPin 23
#define lineDetectorLeftPin 40
#define lineDetectorRightPin 41
#define soundSensorPin7 7 // new version
//#define soundSensorPin33 33 // old version
#define servoPin 5

extern const unsigned int LED_BUILTIN_1;
extern const unsigned int LED_BUILTIN_2;
extern const unsigned int LED_BUILTIN_3;
extern const unsigned int LED_BUILTIN_4;

enum motorDirection{
	motorL,
	motorR,
	motorP,
	both,
	stopMotors
};

enum sensorName {
	touchSensor,
	ultrasonic,
	lineDetectorLeft,
	lineDetectorRight,
	motorEncoderL,
	motorEncoderR,
	soundSensor

};

enum setController: bool {
	controlON = true,
	controlOFF = false
};

// Function prototype for the Timer ISR
extern "C" void TIMER3_COMPA_vect(void) __attribute__((signal));

class Robot
{
  public:
	Robot(setController = controlON);
		void begin (void);
		void displayLCD(uint8_t, uint8_t, const char[]);
			void displayLCD(uint8_t, uint8_t, const long);
			void displayLCD(uint8_t, uint8_t, const double);
			void displayLCD(uint8_t, uint8_t, const int);
			void displayLCD(const int);
			void displayLCD(const double);
			void displayLCD(const char[]);
		void clearLCD(void);
		void moveMotor (motorDirection, int = 0);
		void setEncoderTarget (motorDirection, int);
		double getMotorEncoder (motorDirection);
		void resetMotorEncoder (motorDirection);
		void wait (int = 1000);
		int getSensorValue(sensorName);
		bool divisible(int, int);
		void tunePID(double, double, double);
		void setPIDloopTime (uint8_t);
		void displayEncoderValues (void);
		void displayEncoderAngles (void);

  private:
  	static Robot* _instance;
  	LiquidCrystal_I2C _lcd;
  	Servo _servo;
  	bool _startTargetAngle;
		bool _motorLisRunning;
		bool _motorRisRunning;
		bool _control;
		volatile long _encoderL;
		volatile long _encoderR;
		double _angleL;
		double _angleR;
		int _pwmOutput;
		int _targetAngle;
		// Variables for PID control
		int _maxPower;
		double _Kp;
		double _Kd;
		double _Ki;
		int _setSpeedL;
		int _setSpeedR;
		uint8_t _interval;		// in ms
		double _divisor;
		double _lastErrorR;
	  double _lastErrorL;
	  double _integralR;
	  double _integralL;
	  long _encoderPreviousL;
	  long _encoderPreviousR;

		// End of variables for PID control
		// Internal Functions
		void controlRight(void);
		void controlLeft(void);
		void controlBoth(void);
		void setMotorSpeed (motorDirection, int);
		void computeTimerTicks(int);
		void setTimerParam(void);
		void startTimer(void);
		void stopTimer(void);

		// Static Wrappers
		static void isrAC1(void);
    static void isrAC2(void);
    static void isrBC1(void);
    static void isrBC2(void);

    // Timer & Interrupt Handlers
    void handleTimerEvent(void);
    void handleEncAC1(void);
    void handleEncAC2(void);
    void handleEncBC1(void);
    void handleEncBC2(void);

    friend void TIMER3_COMPA_vect(void);
};

#endif