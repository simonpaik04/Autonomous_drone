const int fan_pin = 19;
const int fan_channel = 1;
const int fan_resolution = 10;

const int melody[] = {
  262, 294, 330, 349, 393, 440, 494, 523
};

void setup() {
  ledcAttachPin(fan_pin, fan_channel);

  for (int note = 0; note < 8; note++) {
    ledcSetup(
      fan_channel,
      melody[note],
      fan_resolution
    );

    ledcWrite(fan_channel, 10);
    delay(500);
  }

  ledcWrite(fan_channel, 0);
}

void loop() {
}
