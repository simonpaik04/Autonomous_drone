const int LED = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char readData = Serial.read();

    Serial.print("read:");
    Serial.println(readData);

    if (readData == 'o') {
      digitalWrite(LED, HIGH);
    }
    else if (readData == 'f') {
      digitalWrite(LED, LOW);
    }
  }
}
