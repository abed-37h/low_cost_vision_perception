#include <stem.h>

Robot r;

void setup() {
  // put your setup code here, to run once:
  r.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  r.displayEncoderAngles();
  delay(10);
}
