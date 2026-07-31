void setup() {
  pinMode(0, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  int pinState = digitalRead(0);
  Serial.println(pinState);
  delay(20);
}
