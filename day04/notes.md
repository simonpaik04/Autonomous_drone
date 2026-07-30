# 자동 비행과 학습 데이터 수집 학습 기록

날짜: 2026-07-30

## 1. 다중 파일 Arduino 스케치

Day 04 코드는 기능에 따라 세 파일로 분리했다.

```text
auto_flight_data_collection.ino
├── setup(): 드론과 자동 비행 태스크 초기화
└── loop(): 드론 센서·제어 루프 반복

drone.ino
├── MPU6050 읽기와 자세 추정
├── PID 보정과 모터 믹싱
├── LEDC PWM 출력
└── 학습 데이터 수집·시리얼 출력

task_auto_avi.ino
├── 코어 0 FreeRTOS 태스크 생성
└── 시간에 따른 스로틀 자동 변경
```

Arduino IDE에서는 같은 폴더의 `.ino` 파일을 하나의 스케치로 합쳐 빌드한다. 따라서 세 파일을 따로 열지 않고 `auto_flight_data_collection` 폴더 전체를 연다.

## 2. ESP32 코어 역할 분리

자동 비행 시퀀스는 `xTaskCreatePinnedToCore()`를 이용해 코어 0에서 실행한다.

```cpp
xTaskCreatePinnedToCore(
    TaskAutoAviMain,
    "TaskAutoAvi",
    10000,
    NULL,
    1,
    &TaskAutoAviHandle,
    0);
```

Arduino의 기본 `loop()`에서는 `drone_loop()`가 계속 실행되며 MPU6050 측정, 자세 추정, PID 계산, 모터 출력을 담당한다. 두 실행 흐름은 `throttle`, `data_sampling_en`, `data_streaming_en`을 공유한다.

## 3. 자동 스로틀 프로파일

자동 비행 태스크는 시작 후 5초간 대기한 다음 데이터 수집을 켜고 아래 순서로 스로틀을 변경한다.

| 구간 | 스로틀 | 간격 | 예상 시간 |
| --- | --- | ---: | ---: |
| 상승 | `0 → 474` | 4 ms | 1,900 ms |
| 1차 하강 | `475 → 301` | 12 ms | 2,100 ms |
| 2차 하강 | `300 → 1` | 5 ms | 1,500 ms |
| 정지 | `0` | - | - |

대기 시간을 제외한 스로틀 시퀀스는 약 5.5초다.

## 4. 수집 데이터

한 샘플은 입력 6개와 목표 출력 3개로 구성한다.

### 입력 `input`

| 인덱스 | 값 |
| ---: | --- |
| 0 | 가속도 X `AcX` |
| 1 | 가속도 Y `AcY` |
| 2 | 가속도 Z `AcZ` |
| 3 | 자이로 X `GyX` |
| 4 | 자이로 Y `GyY` |
| 5 | 자이로 Z `GyZ` |

### 목표 `target`

| 인덱스 | 값 |
| ---: | --- |
| 0 | Roll 보정값 `BalX` |
| 1 | Pitch 보정값 `BalY` |
| 2 | Yaw 보정값 `BalZ` |

최대 샘플 수는 `SAMPLE_COUNT = 2800`이다. 입력 배열은 약 33.6 KB, 목표 배열은 약 67.2 KB로 두 버퍼가 약 100.8 KB의 RAM을 사용한다.

## 5. 데이터 수집 상태

공유 플래그로 수집과 출력을 구분한다.

```cpp
volatile int data_sampling_en = 0;
volatile int data_streaming_en = 0;
```

- `data_sampling_en == 1`: 센서와 PID 보정값을 RAM에 저장
- `data_streaming_en == 1`: 시리얼에서 `p` 또는 `c` 명령 처리

비행이 끝나면 수집을 끄고 시리얼 출력을 활성화한다.

```cpp
data_sampling_en = 0;
data_streaming_en = 1;
```

## 6. Python·C/C++ 형식 출력

자동 비행 종료 메시지가 나타난 뒤 시리얼 모니터에서 한 글자를 전송한다.

| 명령 | 출력 |
| --- | --- |
| `p` | NumPy의 `I`, `T` 배열 코드 |
| `c` | C/C++의 `I`, `T` 배열 코드 |

시리얼 모니터의 통신 속도는 `115200`으로 맞춘다. 2,800개 샘플은 출력량이 크므로 전송이 끝날 때까지 시간이 걸릴 수 있다.

## 7. 확인한 오류와 수정

### 태스크 삭제 뒤 코드가 실행되지 않는 문제

현재 태스크를 `vTaskDelete()`로 삭제하면 그 다음 문장은 실행되지 않는다. 따라서 수집 종료와 출력 허용 상태를 먼저 설정하고 마지막에 현재 태스크를 삭제한다.

```cpp
data_sampling_en = 0;
data_streaming_en = 1;

TaskAutoAviHandle = NULL;
vTaskDelete(NULL);
```

### LEDC 초기화 오류

`ledcAttachPin()`이 초기화되지 않은 채널의 duty를 읽지 않도록 `ledcSetup()`을 먼저 실행한다.

```cpp
ledcSetup(CHANNEL_A, MOTOR_FREQ, MOTOR_RESOLUTION);
ledcAttachPin(MOTOR_A, CHANNEL_A);
ledcWrite(CHANNEL_A, 0);
```

### 시리얼 출력

ESP32의 USB 시리얼로 확실하게 내보내기 위해 일반 `printf()` 대신 `Serial.printf()`를 사용한다.

## 8. USB 재연결과 RAM 데이터

수집 데이터는 플래시가 아니라 RAM에만 저장된다. USB를 다시 연결하거나 시리얼 포트를 열 때 ESP32가 재부팅되면 수집한 데이터가 모두 사라진다.

- USB를 분리하기 전에 Arduino IDE의 시리얼 모니터를 닫는다.
- 주행이 끝나도 ESP32의 배터리 전원은 유지한다.
- USB를 다시 연결한 뒤 COM 포트를 다시 선택하고 시리얼 모니터를 새로 연다.
- 부팅 로그에 `rst:0x...`가 나타나면 재부팅된 것이므로 RAM 데이터는 복구할 수 없다.
- 확실한 보존이 필요하면 SD카드나 플래시에 저장하는 기능을 추가한다.

## 9. 전체 동작 흐름

```text
드론 하드웨어와 데이터 버퍼 초기화
-> 코어 0에 자동 비행 태스크 생성
-> MPU6050 오프셋 보정
-> 5초 대기 후 데이터 수집 시작
-> 자동 스로틀 상승·하강
-> 센서 입력과 PID 보정값 저장
-> 모터 정지
-> 데이터 수집 종료와 시리얼 출력 허용
-> p 또는 c 명령으로 학습 데이터 출력
```

## 10. 시험할 때 주의할 점

- 처음에는 프로펠러를 분리하고 자동 스로틀 변화와 모터 정지를 확인한다.
- 비행 전 `input`과 `target` 메모리 할당 성공 메시지를 확인한다.
- MPU6050 보정 중에는 기체를 평평한 곳에 두고 움직이지 않는다.
- ESP32와 모터 전원의 접지 및 USB 연결 방식을 확인한다.
- 자동 비행 태스크가 끝났더라도 모터 출력이 실제로 0인지 확인한 뒤 기체에 접근한다.
