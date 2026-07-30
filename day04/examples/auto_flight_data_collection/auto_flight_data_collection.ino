#include <Arduino.h>

void drone_setup();
void drone_loop();
void auto_setup();

void setup() {
  drone_setup();
  auto_setup();
}

void loop() {
  drone_loop();
}
