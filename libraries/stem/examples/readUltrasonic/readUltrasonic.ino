#include <stem.h>

Robot r;

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
	Serial.println(r.getSensorValue(ultrasonic));
	delay(100);
}