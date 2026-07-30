# Day 03 - 모터 출력과 PID 비행 제어

날짜: 2026-07-29

## Day 02와 비교

완성 코드 폴더를 GitHub의 Day 02 코드와 비교해 이미 있는 예제는 제외했습니다.

- `450`: Day 02의 `450_p_control`과 동일
- `470`: Day 02의 `470_motor_mixing`과 출력문 줄바꿈만 다르고 동작은 동일
- `3120`, `480`, `490`, `4100`, `4110`, `510`, `520`: GitHub에 없는 신규 예제

따라서 Day 03에는 신규 예제 7개만 정리했습니다.

## 공부한 내용

- MPU6050 자세 추정 부분을 독립 예제로 분리
- 시리얼 입력으로 스로틀을 `0~900` 범위에서 변경
- 계산된 모터 속도를 ESP32 LEDC PWM으로 실제 출력
- Bluetooth Classic으로 스로틀과 목표 자세를 원격 제어
- 각도 오차의 P 제어에 자이로 각속도 기반 D 제어 추가
- 목표 Roll 값을 이용한 좌우 조종
- I 제어를 추가한 PID 제어와 전후좌우 목표 자세 입력
- 스로틀이 0일 때 자세각과 제어 누적값 초기화

자세한 내용은 [학습 노트](./notes.md)에 정리했습니다.

## 예제 코드

| 원본 번호 | 폴더 | 내용 |
| --- | --- | --- |
| `3120` | [`3120_attitude_filter`](./examples/3120_attitude_filter/) | MPU6050 보정과 상보 필터 자세 추정 |
| `480` | [`480_serial_throttle`](./examples/480_serial_throttle/) | 시리얼 스로틀 입력과 모터 믹싱 계산 |
| `490` | [`490_motor_pwm`](./examples/490_motor_pwm/) | 4개 모터에 LEDC PWM 출력 |
| `4100` | [`4100_bluetooth_throttle`](./examples/4100_bluetooth_throttle/) | 블루투스 스로틀과 P 제어 |
| `4110` | [`4110_pd_control`](./examples/4110_pd_control/) | P 제어에 자이로 기반 D 제어 추가 |
| `510` | [`510_left_right_control`](./examples/510_left_right_control/) | 블루투스 `a`/`d`로 좌우 목표 자세 제어 |
| `520` | [`520_pid_direction_control`](./examples/520_pid_direction_control/) | PID 제어와 전후좌우 조종 |

각 예제는 완전한 Arduino 스케치입니다. 폴더명과 `.ino` 파일명이 같으므로 해당 폴더를 Arduino IDE에서 바로 열 수 있습니다.

> 실제 모터를 시험할 때는 먼저 프로펠러를 분리하고, `0` 입력으로 모터 정지를 확인한 뒤 낮은 출력부터 테스트합니다.
