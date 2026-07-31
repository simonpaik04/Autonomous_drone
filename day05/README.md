# Day 05 - 선형모델 기초와 DNN 비행 제어 추론

날짜: 2026-07-31

## Day 04와 비교

Day 04에서는 MPU6050의 가속도·자이로 6개 값을 입력으로, PID의 Roll·Pitch·Yaw 보정값 3개를 정답으로 수집했습니다.

Day 05에서는 선형모델의 순전파·손실·경사하강법을 작은 Arduino 예제로 먼저 확인한 뒤, 오프라인에서 학습된 DNN 가중치를 ESP32에 넣어 센서값으로부터 모터 보정값을 직접 추론합니다.

| 구분 | Day 04 | Day 05 |
| --- | --- | --- |
| 목적 | 학습 데이터 수집 | 학습된 모델로 실시간 추론 |
| 입력 | MPU6050 원시값 6개 | 정규화한 MPU6050 값 6개 |
| 출력 | PID 보정값 `BalX/Y/Z` 저장 | DNN이 `BalX/Y/Z` 예측 |
| 제어 핵심 | PID 계산 | `6 → 16 → 16 → 3` DNN |
| 모델 데이터 | 없음 | 가중치·편향 C++ 배열 |
| 자동 비행 시작 | 5초 지연 | GPIO 0 버튼 입력 |

## 새로 공부한 내용

- 선형식 `y = xw + b`의 순전파와 평균제곱오차
- 오차의 기울기로 가중치와 편향을 갱신하는 경사하강법
- 입력이 여러 개인 선형모델로 계산 확장
- ArduinoEigen의 `MatrixXd`와 `Map`을 이용한 배열·행렬 연결
- ReLU를 사용하는 `6 → 16 → 16 → 3` 완전연결 신경망
- 외부에서 학습한 가중치를 `model_data.cpp`에 저장해 ESP32에 배포
- MPU6050 6축 입력으로 Roll·Pitch·Yaw 모터 보정값 추론
- 기존 PID 계산을 DNN 출력으로 교체하는 비행 제어 흐름
- GPIO 0 버튼과 FreeRTOS 태스크를 이용한 자동 비행 시작
- Day 04에서 확인한 초기화 순서와 모터 안전 규칙을 DNN 코드에 적용

자세한 계산 과정과 주의점은 [학습 노트](./notes.md)에 정리했습니다.

## 예제 코드

| 폴더 | 내용 |
| --- | --- |
| [`gpio0_digital_input_monitor`](./examples/gpio0_digital_input_monitor/) | GPIO 0의 HIGH·LOW 상태를 시리얼로 확인 |
| [`linear_model_forward_loss_demo`](./examples/linear_model_forward_loss_demo/) | `y = xw + b` 순전파와 제곱오차 계산 |
| [`single_input_linear_regression_training`](./examples/single_input_linear_regression_training/) | 1개 입력의 가중치·편향 경사하강 학습 |
| [`two_input_linear_regression_training`](./examples/two_input_linear_regression_training/) | 2개 입력의 가중치·편향 경사하강 학습 |
| [`dnn_flight_controller`](./examples/dnn_flight_controller/) | 학습된 DNN으로 3축 모터 보정값을 추론하는 다중 파일 스케치 |

`dnn_flight_controller`는 ArduinoEigen 라이브러리가 필요하며, 폴더 전체를 Arduino IDE에서 열어야 합니다.

> DNN 출력은 학습 데이터 범위를 벗어나면 예상보다 커질 수 있습니다. 실제 모터 시험 전에는 프로펠러를 분리하고, 시리얼·테스트 입력으로 출력 범위와 정지 동작을 먼저 검증합니다.
