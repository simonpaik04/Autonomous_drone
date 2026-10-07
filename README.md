# Autonomous Drone Study

- **학습 방식:** ESP32·Arduino와 MPU6050을 이용해 모터 PWM, 자세 추정, P·PD·PID 제어를 단계별 예제로 정리했습니다.
- **구현 내용:** FreeRTOS 기반 스로틀 시퀀스와 센서·PID 데이터 수집을 구성하고, 학습된 `6 → 16 → 16 → 3` DNN의 모터 보정값 추론 예제를 작성했습니다.
- **구성 특징:** Day 01–05별 코드·학습 노트를 통해 기초 제어부터 데이터 수집과 임베디드 신경망 추론까지 이어지는 학습 과정을 기록합니다.

ESP32를 이용한 드론 공부 기록입니다.

## 진행 상황

- [Day 01 - ESP32 기초와 모터 제어](./day01/README.md)
- [Day 02 - MPU6050 자세 추정과 P 제어](./day02/README.md)
- [Day 03 - 모터 출력과 PID 비행 제어](./day03/README.md)
- [Day 04 - 자동 비행과 학습 데이터 수집](./day04/README.md)
- [Day 05 - 선형모델 기초와 DNN 비행 제어 추론](./day05/README.md)
