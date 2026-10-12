/*
  stem.cpp - Source file of the library to control the USAL robot.
  Created by Bilal A. Komati, December 15, 2020.
  Updated on March 8, 2023.
  This update integrates PID control for the motors.
  The last update compute PID control inside timer3 and disable ServoRob from using timer3.
  Moreover, two functions to display the Encoder values have been added.
  Released for Al Mabarrat Schools.
*/

#include "stem.h"

// Initialize static pointer to nullptr for safety
Robot* Robot::_instance = nullptr;

// Static Wrappers (The Bridge)
void Robot::isrAC1() { if(_instance) _instance->handleEncAC1(); }
void Robot::isrAC2() { if(_instance) _instance->handleEncAC2(); }
void Robot::isrBC1() { if(_instance) _instance->handleEncBC1(); }
void Robot::isrBC2() { if(_instance) _instance->handleEncBC2(); }

// Global Timer ISR
ISR(TIMER3_COMPA_vect) {
    if (Robot::_instance) Robot::_instance->handleTimerEvent();
}

const unsigned int LED_BUILTIN_1 = 4;
const unsigned int LED_BUILTIN_2 = 45;
const unsigned int LED_BUILTIN_3 = 44;
const unsigned int LED_BUILTIN_4 = 13;

// Class members
Robot::Robot(setController controller) : _lcd(0x27,16,2)
{
  pinMode(Ain1, OUTPUT);
  pinMode(Ain2, OUTPUT);
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(touchSensorPin,INPUT);
  pinMode(lineDetectorLeftPin,INPUT);
  pinMode(lineDetectorRightPin,INPUT);
  pinMode(ultrasonicTrigPin1, OUTPUT);//old version 
  pinMode(ultrasonicTrigPin6, OUTPUT);// new version
  pinMode(soundSensorPin7, INPUT);
 // pinMode(soundSensorPin33, INPUT);
  pinMode(ultrasonicEchoPin,INPUT);
  pinMode(LED_BUILTIN_1,OUTPUT);
  pinMode(LED_BUILTIN_2,OUTPUT);
  pinMode(LED_BUILTIN_3,OUTPUT);
  pinMode(LED_BUILTIN_4,OUTPUT);
  pinMode(EncAC1,INPUT);
  pinMode(EncAC2,INPUT);
  pinMode(EncBC1,INPUT);
  pinMode(EncBC2,INPUT);
  _startTargetAngle = false;
  _motorLisRunning = _motorRisRunning = false;
  _encoderR = _encoderL = 0;
  _control = controller;
  _Kp = 10;
  _Ki = 1.0;
  _Kd = 0.3;
  _divisor = 10;
  _interval = 10;		// set interval to 10 ms
  _lastErrorR = 0.0;
  _lastErrorL = 0.0;
  _integralR = 0.0;
  _integralL = 0.0; 
  _encoderPreviousL = _encoderPreviousR = 0;
  _maxPower = 100;
  _instance = this; // assign current instance to pointer (IMPORTANT!!!)
  attachInterrupt(digitalPinToInterrupt(EncAC1), Robot::isrAC1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncAC2), Robot::isrAC2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncBC1), Robot::isrBC1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(EncBC2), Robot::isrBC2, CHANGE);
}

bool Robot::divisible(int a, int b)
{
	return (a % b) == 0;
}

void Robot::begin (void) // To start LCD and Servo Functions. These functions should not be called in the constructor
{
  _servo.attach(servoPin);
  _servo.write(0);
  delay(100);
  _lcd.init();
  _lcd.backlight();
  _lcd.clear();
}

void Robot::displayLCD(uint8_t row, uint8_t column, const char msg[])
{
	_lcd.setCursor(column, row);
	_lcd.print(msg);
}

void Robot::displayLCD(uint8_t row, uint8_t column, const long msg)
{
	_lcd.setCursor(column, row);
	_lcd.print(msg);
}

void Robot::displayLCD(uint8_t row, uint8_t column, const double msg)
{
	_lcd.setCursor(column, row);
	_lcd.print(msg);
}
void Robot::displayLCD(uint8_t row, uint8_t column, const int msg)
{
	_lcd.setCursor(column, row);
	_lcd.print(msg);
}

void Robot::displayLCD(const int msg)
{
	_lcd.setCursor(0, 0);
	_lcd.print(msg);
}
void Robot::displayLCD(const char msg[])
{
	_lcd.setCursor(0, 0);
	_lcd.print(msg);
}

void Robot::displayLCD(const double msg)
{
	_lcd.setCursor(0, 0);
	_lcd.print(msg);
}

void Robot::clearLCD(void)
{
	_lcd.clear();
}

void Robot::moveMotor (motorDirection direction, int power)
{
	if (_control)
	{
		setMotorSpeed(direction, power);
		_encoderPreviousL = _encoderL;
		_encoderPreviousR = _encoderR;
		_maxPower = 200;
		//power = 5;
		startTimer();
	}
	switch (direction)
	{
		case motorR:
			_motorRisRunning = true;
			power = constrain(power, -_maxPower, _maxPower);
			if (power > 0)
			{
				_pwmOutput = 240 - power;
				digitalWrite(Ain2, HIGH);
				analogWrite(Ain1, _pwmOutput);
			}
			else if (power < 0)
			{
				_pwmOutput = 240 + power;
				digitalWrite(Ain1, HIGH);
				analogWrite(Ain2, _pwmOutput);
			}
			else
			{
				digitalWrite(Ain1, HIGH);
				digitalWrite(Ain2, HIGH);
				_motorRisRunning = false;
				if (!_motorLisRunning)
					stopTimer();
			}
			break;
		case motorL:
			_motorLisRunning = true;
			power = constrain(power, -_maxPower, _maxPower);
			if (power > 0)
			{
				_pwmOutput = 240 - power;
				digitalWrite(Bin1, HIGH);
				analogWrite(Bin2, _pwmOutput);
			}
			else if (power < 0)
			{
				_pwmOutput = 240 + power;
				digitalWrite(Bin2, HIGH);
				analogWrite(Bin1, _pwmOutput);
			}
			else
			{
				digitalWrite(Bin1, HIGH);
				digitalWrite(Bin2, HIGH);
				_motorLisRunning = false;
				if(!_motorRisRunning)
					stopTimer();
			}
			break;
		case motorP:
			power = constrain(power, 0, 100);
			_servo.write(power);
			break;
		default:
			_motorLisRunning = _motorRisRunning = false;
			_pwmOutput = 0;
			digitalWrite(Ain1, HIGH);
			digitalWrite(Ain2, HIGH);
			digitalWrite(Bin1, HIGH);
			digitalWrite(Bin2, HIGH);
			stopTimer();
  	}
}

void Robot::setEncoderTarget (motorDirection direction, int targetAngle)
{
	_targetAngle = abs(targetAngle);
	_startTargetAngle = true;
}

void Robot::wait (int time)
{
	if(!_startTargetAngle)
		delay(time);
	else
	{
		if (_motorRisRunning && _motorLisRunning)
		{
			while ( (abs(getMotorEncoder(motorR)) < _targetAngle) && (abs(getMotorEncoder(motorL)) < _targetAngle) )
				{ /* Loop to wait until the movement is finished */ }
		}
		else if (_motorRisRunning)
		{
			while (abs(getMotorEncoder(motorR)) < _targetAngle)
				{ /* Loop to wait until the movement is finished */ }
		}
		else if (_motorLisRunning)
		{
			while (abs(getMotorEncoder(motorL)) < _targetAngle)
				{ /* Loop to wait until the movement is finished */ }
		}
		moveMotor(stopMotors,0);
		_startTargetAngle = false;
	}
}

double Robot::getMotorEncoder (motorDirection direction)
{
	switch (direction)
	{
		case motorL:
			_angleL = 0.25714 * _encoderL;
			return _angleL;
			break;
		case motorR:
			_angleR = 0.25714 * _encoderR;
			return _angleR;
			break;
		default: return 0 ;
	}
}

void Robot::resetMotorEncoder (motorDirection direction)
{
	switch (direction)
	{
		case motorL:
			_encoderL = 0;
			break;
		case motorR:
			_encoderR = 0;
			break;
		default:
			_encoderL = _encoderR = 0 ;
	}
}

int Robot::getSensorValue(sensorName sensor)
{
	switch (sensor)
	{
		case motorEncoderL:			// added to the new version
			return(_encoderL);
		case motorEncoderR:			// added to the new version
			return(_encoderR);
		case touchSensor:
			return(digitalRead(touchSensorPin));
		case lineDetectorLeft:
			return(digitalRead(lineDetectorLeftPin));
		case lineDetectorRight:
			return(digitalRead(lineDetectorRightPin));
		case soundSensor:
			return(digitalRead(soundSensorPin7)); 
		case ultrasonic:
			int duration, distance;
			digitalWrite(ultrasonicTrigPin1, LOW);
			digitalWrite(ultrasonicTrigPin6, LOW);

			delayMicroseconds(2);
			digitalWrite(ultrasonicTrigPin1, HIGH);
			digitalWrite(ultrasonicTrigPin6, HIGH);

			delayMicroseconds(10);
			digitalWrite(ultrasonicTrigPin1, LOW);
			digitalWrite(ultrasonicTrigPin6, LOW);

			duration = pulseIn(ultrasonicEchoPin, HIGH);
			distance = duration*0.034/2;
			delay(10);
			return distance;
		default:
			return -1;
	}
}

void Robot::displayEncoderValues (void)
{
	displayLCD("Enc Left: ");
  	displayLCD(0,11,_encoderL);
  	displayLCD(1,0,"Enc Right: ");
  	displayLCD(1,11,_encoderR);
}

void Robot::displayEncoderAngles (void)
{

	displayLCD("Enc Left: ");
  	displayLCD(0,10,getMotorEncoder(motorL));
  	displayLCD(1,0,"Enc Right: ");
  	displayLCD(1,10,getMotorEncoder(motorR));
}

// Encoder A: Channel 1 (Pin 2, PORTE Bit 4) Changed
void Robot::handleEncAC1(void)
{
    // Read the whole Port E register once for synchronization
    uint8_t state = PINE;    
    // Check if AC1 (Bit 4) is HIGH
    if (state & (1 << 4)) { 
        // Rising Edge of AC1. Check AC2 (Bit 5).
        if (state & (1 << 5)) _encoderR++; // CW
        else                  _encoderR--; // CCW
    } else {
        // Falling Edge of AC1. Check AC2 (Bit 5).
        if (!(state & (1 << 5))) _encoderR++; // CW
        else                     _encoderR--; // CCW
    }
}

// Encoder A: Channel 2 (Pin 3, PORTE Bit 5) Changed
void Robot::handleEncAC2(void)
{
    uint8_t state = PINE;
    if (state & (1 << 5)) { // AC2 Rising
        if (!(state & (1 << 4))) _encoderR++;
        else                     _encoderR--;
    } else { // AC2 Falling
        if (state & (1 << 4))    _encoderR++;
        else                     _encoderR--;
    }
}

// Encoder B: Channel 1 (Pin 18, PORTD Bit 3) Changed
void Robot::handleEncBC1(void)
{
    // Use PORT D for pins 18/19
    uint8_t state = PIND;
    
    // Check BC1 (Bit 3)
    if (state & (1 << 3)) { // BC1 Rising
        // Check BC2 (Bit 2)
        if (state & (1 << 2)) _encoderL--;
        else                  _encoderL++;
    } else { // BC1 Falling
        if (!(state & (1 << 2))) _encoderL--;
        else                     _encoderL++;
    }
}

// Encoder B: Channel 2 (Pin 19, PORTD Bit 2) Changed
void Robot::handleEncBC2(void) {
    uint8_t state = PIND;
    if (state & (1 << 2)) { // BC2 Rising
        if (!(state & (1 << 3))) _encoderL--;
        else                     _encoderL++;
    } else { // BC2 Falling
        if (state & (1 << 3))    _encoderL--;
        else                     _encoderL++;
    }
}

void Robot::setMotorSpeed (motorDirection direction, int newSetpoint)
{
	if (_control)
	{
		switch (direction)
		{
			case motorR:
				_setSpeedR = newSetpoint;
				break;
			case motorL:
				_setSpeedL = newSetpoint;
				break;
		}
	}
}

void Robot::tunePID(double Kp, double Ki, double Kd)
{
	if (Kp < 0 || Ki < 0 || Kd < 0) return;
	_Kp = Kp;
	_Ki = Ki;
	_Kd = Kd;
}

void Robot::setPIDloopTime(uint8_t time)
{
	_interval = time;
}

void Robot::startTimer(void)
{
	if (_interval < 261)
	{
		// Calculate Ticks (Prescaler 64 logic)
	  uint16_t required_ticks = (uint16_t)((float)_interval * 250.0);
	  if (required_ticks == 0) required_ticks = 1;
	  // Configure Timer3 Registers
	  TCCR3A = 0;
	  TCCR3B = 0;
	  // Set CTC Mode (WGM32 = 1)
	  TCCR3B |= (1 << WGM32);
	  // Set Prescaler 64 (CS31=1, CS30=1)
	  TCCR3B |= (1 << CS31) | (1 << CS30);
	  // Set Compare Match
	  OCR3A = required_ticks - 1;
	  // Reset Counter so it starts fresh from 0
	  TCNT3 = 0;
	  // Enable Interrupt
	  TIMSK3 |= (1 << OCIE3A);
	}
}

void Robot::stopTimer(void)
{
	// Disable Interrupt
  TIMSK3 &= ~(1 << OCIE3A);
  // Stop Clock (Optional, saves power)
  TCCR3B &= ~((1 << CS32) | (1 << CS31) | (1 << CS30));
  _control = true;
}

void Robot::handleTimerEvent(void)
{
	_control = false;
	if (_motorRisRunning && _motorLisRunning)
		controlBoth();
	else if (_motorRisRunning)
		controlRight();
	else if (_motorLisRunning)
		controlLeft();
	else
		stopTimer();
}

void Robot::controlBoth(void)
{
	long encoderL = _encoderL;
	long encoderR = _encoderR;
	double rpmL = (encoderL - _encoderPreviousL)*30.0/7.0;
	double rpmR = (encoderR - _encoderPreviousR)*30.0/7.0;
	double errorL = _setSpeedL - rpmL;
	double errorR = _setSpeedR - rpmR;
	_integralL += errorL * _interval;
	_integralR += errorR * _interval;
	double dErrL = (errorL - _lastErrorL) / _interval;
	double dErrR = (errorR - _lastErrorR) / _interval;
	double newOutputL = (_Kp * errorL + _Ki * _integralL + _Kd * dErrL) / _divisor;
	double newOutputR = (_Kp * errorR + _Ki * _integralR + _Kd * dErrR) / _divisor;
	_lastErrorL = errorL;
	_lastErrorR = errorR;
	moveMotor(motorL, newOutputL);
	moveMotor(motorR, newOutputR);
	_encoderPreviousL = encoderL;
	_encoderPreviousR = encoderR;
}

void Robot::controlRight(void)
{
	long encoderR = _encoderR;
	double rpmR = (encoderR - _encoderPreviousR)*30.0/7.0;
	double errorR = _setSpeedR - rpmR;
	_integralR += errorR * _interval;
	double dErrR = (errorR - _lastErrorR) / _interval;
	double newOutputR = (_Kp * errorR + _Ki * _integralR + _Kd * dErrR) / _divisor;
	_lastErrorR = errorR;
	moveMotor(motorR, newOutputR);
	_encoderPreviousR = encoderR;
}

void Robot::controlLeft(void)
{
	long encoderL = _encoderL;
	double rpmL = (encoderL - _encoderPreviousL)*30.0/7.0;
	double errorL = _setSpeedL - rpmL;
	_integralL += errorL * _interval;
	double dErrL = (errorL - _lastErrorL) / _interval;
	double newOutputL = (_Kp * errorL + _Ki * _integralL + _Kd * dErrL) / _divisor;
	_lastErrorL = errorL;
	moveMotor(motorL, newOutputL);
	_encoderPreviousL = encoderL;
}