const int fan_pin = 19;
const int fan_channel = 1;
const int fan_freq = 261;
const int fan_resolution = 10;

void setup() {
  ledcAttachPin(fan_pin, fan_channel);
  ledcSetup(fan_channel, fan_freq, fan_resolution);

  ledcWrite(fan_channel, 100);
  delay(5000);

  ledcWrite(fan_channel, 0);
}

void loop() {
}
