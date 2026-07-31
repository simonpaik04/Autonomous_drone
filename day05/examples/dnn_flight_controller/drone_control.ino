#include <Wire.h>

const int MOTOR_A = 23;
const int MOTOR_B = 19;
const int MOTOR_C = 18;
const int MOTOR_D = 26;

const int CHANNEL_A = 0;
const int CHANNEL_B = 1;
const int CHANNEL_C = 2;
const int CHANNEL_D = 3;

const int MOTOR_FREQ = 5000;
const int MOTOR_RESOLUTION = 10;

volatile int throttle = 0;

void stop_motors() {
  ledcWrite(CHANNEL_A, 0);
  ledcWrite(CHANNEL_B, 0);
  ledcWrite(CHANNEL_C, 0);
  ledcWrite(CHANNEL_D, 0);
}

void drone_setup() {
  Wire.begin();
  Wire.setClock(400000);

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Day 04에서 확인한 것처럼 채널 설정을 먼저 하고 핀을 연결한다.
  ledcSetup(CHANNEL_A, MOTOR_FREQ, MOTOR_RESOLUTION);
  ledcSetup(CHANNEL_B, MOTOR_FREQ, MOTOR_RESOLUTION);
  ledcSetup(CHANNEL_C, MOTOR_FREQ, MOTOR_RESOLUTION);
  ledcSetup(CHANNEL_D, MOTOR_FREQ, MOTOR_RESOLUTION);

  ledcAttachPin(MOTOR_A, CHANNEL_A);
  ledcAttachPin(MOTOR_B, CHANNEL_B);
  ledcAttachPin(MOTOR_C, CHANNEL_C);
  ledcAttachPin(MOTOR_D, CHANNEL_D);

  stop_motors();
}

void drone_loop() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)0x68, (uint8_t)14, true);

  if (Wire.available() < 14) {
    throttle = 0;
    stop_motors();
    return;
  }

  int16_t AcXH = Wire.read();
  int16_t AcXL = Wire.read();
  int16_t AcYH = Wire.read();
  int16_t AcYL = Wire.read();
  int16_t AcZH = Wire.read();
  int16_t AcZL = Wire.read();
  int16_t TmpH = Wire.read();
  int16_t TmpL = Wire.read();
  int16_t GyXH = Wire.read();
  int16_t GyXL = Wire.read();
  int16_t GyYH = Wire.read();
  int16_t GyYL = Wire.read();
  int16_t GyZH = Wire.read();
  int16_t GyZL = Wire.read();

  (void)TmpH;
  (void)TmpL;

  int16_t AcX = AcXH << 8 | AcXL;
  int16_t AcY = AcYH << 8 | AcYL;
  int16_t AcZ = AcZH << 8 | AcZL;
  int16_t GyX = GyXH << 8 | GyXL;
  int16_t GyY = GyYH << 8 | GyYL;
  int16_t GyZ = GyZH << 8 | GyZL;

  // 모델 학습 때 사용한 것과 같은 순서와 정규화를 유지한다.
  double x[6] = {
      AcX / 32768.0,
      AcY / 32768.0,
      AcZ / 32768.0,
      GyX / 32768.0,
      GyY / 32768.0,
      GyZ / 32768.0,
  };
  double y[3] = {0.0, 0.0, 0.0};

  dnn_predict(x, 6, y, 3);

  double BalX = y[0];
  double BalY = y[1];
  double BalZ = y[2];

  if (!isfinite(BalX) || !isfinite(BalY) || !isfinite(BalZ)) {
    throttle = 0;
    stop_motors();
    return;
  }

  int currentThrottle = throttle;
  if (currentThrottle <= 0) {
    BalX = 0.0;
    BalY = 0.0;
    BalZ = 0.0;
  }

  double speedA = currentThrottle + BalX - BalY + BalZ;
  double speedB = currentThrottle - BalX - BalY - BalZ;
  double speedC = currentThrottle - BalX + BalY + BalZ;
  double speedD = currentThrottle + BalX + BalY - BalZ;

  int iSpeedA = constrain((int)speedA, 0, 1000);
  int iSpeedB = constrain((int)speedB, 0, 1000);
  int iSpeedC = constrain((int)speedC, 0, 1000);
  int iSpeedD = constrain((int)speedD, 0, 1000);

  ledcWrite(CHANNEL_A, iSpeedA);
  ledcWrite(CHANNEL_B, iSpeedB);
  ledcWrite(CHANNEL_C, iSpeedC);
  ledcWrite(CHANNEL_D, iSpeedD);
}
