#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  // 하드웨어와 모델을 준비한 뒤 자동 비행 태스크를 시작한다.
  drone_setup();
  dnn_initialize();
  auto_setup();
}

void loop() {
  drone_loop();
}
