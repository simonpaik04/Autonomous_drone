# Day 04 - 자동 비행과 학습 데이터 수집

날짜: 2026-07-30

## Day 03과 비교

Day 03에서는 하나의 Arduino 스케치 안에서 MPU6050 자세 추정, PID 제어, 블루투스 조종, 모터 출력을 순서대로 구현했습니다.

Day 04에서는 이 코드를 역할별 파일로 나누고, FreeRTOS 태스크로 정해진 스로틀 프로파일을 실행하면서 센서 입력과 PID 보정값을 학습 데이터로 수집합니다.

## 공부한 내용

- Arduino 스케치를 메인·드론 제어·자동 비행 태스크 파일로 분리
- ESP32 FreeRTOS 태스크를 코어 0에 고정해 자동 비행 시퀀스 실행
- MPU6050 가속도·자이로 6개 값을 입력 데이터로 저장
- PID의 Roll·Pitch·Yaw 보정값 3개를 목표 데이터로 저장
- 최대 2,800개 샘플을 ESP32 RAM에 수집
- 시리얼 명령 `p`로 NumPy 코드 출력
- 시리얼 명령 `c`로 C/C++ 배열 코드 출력
- 태스크 삭제 전 데이터 수집·출력 상태를 안전하게 변경
- LEDC 채널을 초기화한 뒤 모터 핀을 연결하도록 순서 수정

자세한 내용은 [학습 노트](./notes.md)에 정리했습니다.

## 예제 코드

| 파일 | 역할 |
| --- | --- |
| [`auto_flight_data_collection.ino`](./examples/auto_flight_data_collection/auto_flight_data_collection.ino) | Arduino `setup()`·`loop()` 진입점 |
| [`drone.ino`](./examples/auto_flight_data_collection/drone.ino) | MPU6050, PID, 모터 PWM, 데이터 수집·출력 |
| [`task_auto_avi.ino`](./examples/auto_flight_data_collection/task_auto_avi.ino) | FreeRTOS 자동 비행 태스크와 스로틀 프로파일 |

세 파일은 각각 독립 스케치가 아니라 `auto_flight_data_collection`이라는 하나의 다중 파일 Arduino 스케치를 구성합니다. 폴더 전체를 Arduino IDE에서 열어야 합니다.

> 실제 모터를 시험할 때는 프로펠러를 분리한 상태에서 먼저 확인하고, 배터리와 USB 전원을 동시에 연결할 때는 보드 전원 구성을 점검합니다.
