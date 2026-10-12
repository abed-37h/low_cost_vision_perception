// In our robot, there exist 4 built-in LEDs
// named from LED_BUILTIN_1 to LED_BUILTIN_4

#include <stem.h>

Robot r;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN_1,HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN_1,LOW);
  delay(1000);
}