# Day 01 - ESP32 기초와 모터 제어

날짜: 2026-07-27

## 공부한 내용

- Arduino의 `setup()`과 `loop()` 구조
- GPIO와 시리얼 통신을 이용한 LED 제어
- PWM의 주파수, 주기, 듀티비
- ESP32 LEDC를 이용한 단일 모터 제어
- 4개 모터의 PWM 채널 설정
- 블루투스 입력을 이용한 모터 출력 제어

자세한 내용은 [학습 노트](./notes.md)에 정리했습니다.

## 예제 코드

| 폴더 | 내용 |
| --- | --- |
| [`01_serial_output`](./examples/01_serial_output/) | 시리얼 모니터 출력 |
| [`02_led_serial_control`](./examples/02_led_serial_control/) | 시리얼 입력으로 LED 켜고 끄기 |
| [`03_led_pwm_test`](./examples/03_led_pwm_test/) | LED로 PWM 듀티비 변화 확인 |
| [`04_single_motor_pwm`](./examples/04_single_motor_pwm/) | 단일 모터 PWM 테스트 |
| [`05_motor_melody`](./examples/05_motor_melody/) | PWM 주파수를 바꾸는 모터 테스트 |
| [`06_four_motor_bluetooth`](./examples/06_four_motor_bluetooth/) | 블루투스로 4개 모터 출력 제어 |

> 모터 코드는 프로펠러를 분리한 상태에서 테스트합니다.
