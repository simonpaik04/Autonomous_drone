# 모터 출력과 PID 비행 제어 학습 기록

날짜: 2026-07-29

## 1. Day 02 코드와 신규 코드 구분

Day 03 완성 코드 폴더에는 `3120`, `4100`, `4110`, `450`, `470`, `480`, `490`, `510`, `520` 예제가 들어 있다.

GitHub Day 02와 비교한 결과 `450`은 기존 `450_p_control`과 동일했다. `470`은 긴 `Serial.printf()` 문장의 줄바꿈만 다르고 계산과 동작은 기존 `470_motor_mixing`과 동일했다. 두 코드는 중복으로 판단해 Day 03에서 제외했다.

GitHub에 없던 다음 7개 예제를 Day 03 신규 코드로 정리했다.

```text
3120 -> 자세 추정
480  -> 시리얼 스로틀
490  -> 실제 모터 PWM
4100 -> 블루투스 스로틀
4110 -> PD 자세 제어
510  -> 좌우 조종
520  -> PID 전후좌우 조종
```

## 2. `3120`: 자세 추정 독립 예제

`3120`은 MPU6050에서 가속도와 자이로 값을 읽고, 시작 시 1,000개 샘플로 오프셋을 보정한다. 이후 실제 반복 시간 `dt`를 구해 자이로 값을 적분하고 가속도 기반 각도와 상보 필터로 결합한다.

```cpp
const double ALPHA = 0.96;

cmAngleX =
    ALPHA * (cmAngleX + GyXR * dt)
    + (1.0 - ALPHA) * acAngleX;

cmAngleY =
    ALPHA * (cmAngleY + GyYR * dt)
    + (1.0 - ALPHA) * acAngleY;
```

제어와 모터 출력 없이 `X`, `Y`, `Z` 자세각만 시리얼 모니터에 출력하므로 센서 방향과 보정 상태를 먼저 점검할 때 사용할 수 있다.

## 3. `480`: 시리얼 스로틀 입력

`480`은 Day 02의 P 제어와 모터 믹싱에 시리얼 스로틀 입력을 추가한다.

```cpp
if (userInput >= '0' && userInput <= '9') {
  throttle = (userInput - '0') * 100;
}
```

문자 `0`부터 `9`까지를 입력하면 스로틀이 `0`, `100`, ..., `900`으로 바뀐다. 이 예제는 네 모터의 속도를 계산하고 출력하지만 아직 GPIO에 PWM을 내보내지는 않는다.

## 4. `490`: LEDC 모터 PWM 출력

`490`은 네 모터 핀과 LEDC 채널을 연결하고 계산된 속도를 실제 PWM으로 출력한다.

| 모터 | GPIO | LEDC 채널 |
| --- | ---: | ---: |
| A | 23 | 0 |
| B | 19 | 1 |
| C | 18 | 2 |
| D | 26 | 3 |

PWM 주파수는 `5000 Hz`, 해상도는 `10 bit`다. 모터 믹싱 결과는 `0~1000`으로 제한한 뒤 각 채널에 전달한다.

```cpp
int iSpeedA = constrain((int)speedA, 0, 1000);
int iSpeedB = constrain((int)speedB, 0, 1000);
int iSpeedC = constrain((int)speedC, 0, 1000);
int iSpeedD = constrain((int)speedD, 0, 1000);

ledcWrite(CHANNEL_A, iSpeedA);
ledcWrite(CHANNEL_B, iSpeedB);
ledcWrite(CHANNEL_C, iSpeedC);
ledcWrite(CHANNEL_D, iSpeedD);
```

## 5. `4100`: 블루투스 스로틀

`4100`은 USB 시리얼 대신 ESP32의 Bluetooth Classic 입력으로 스로틀을 받는다.

```cpp
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
  SerialBT.begin("ESP32_DRONE");
}
```

블루투스에서 숫자 문자를 읽어 스로틀을 변경한다. 스로틀이 `0`이면 자세 추정값과 제어값을 초기화해 정지 후 다시 시작할 때 이전 각도 누적값이 남지 않도록 한다.

```cpp
if (throttle == 0) {
  cmAngleX = cmAngleY = cmAngleZ = 0.0;
  gyAngleX = gyAngleY = gyAngleZ = 0.0;
  BalX = BalY = BalZ = 0.0;
}
```

## 6. `4110`: PD 자세 제어

P 제어는 목표 각도와 현재 각도의 오차에 비례해 보정한다. `4110`은 여기에 자이로 각속도를 반대 방향으로 더하는 D 항을 추가한다.

```cpp
double Kp = 5.0;
double BalX = Kp * eAngleX;
double BalY = Kp * eAngleY;
double BalZ = Kp * eAngleZ;

double Kd = 1.0;
BalX += Kd * -GyXR;
BalY += Kd * -GyYR;
BalZ += Kd * -GyZR;
```

D 항은 빠른 회전 움직임을 감쇠해 목표 각도 주변의 흔들림을 줄이는 역할을 한다. 이 예제의 초기 스로틀은 `100`이며 블루투스 장치 이름은 `PAIKdrone`이다.

## 7. `510`: 좌우 목표 자세 조종

`510`은 블루투스 명령으로 목표 Roll 각도 `tAngleX`를 바꾼다.

| 입력 | 목표 동작 | `tAngleX` |
| --- | --- | ---: |
| `a` | 왼쪽 | `-10°` |
| `d` | 오른쪽 | `10°` |
| `s` | 수평 복귀 | `0°` |
| `0`~`9` | 스로틀 | `0`~`900` |

현재 자세를 직접 바꾸는 대신 목표 각도를 바꾸고, PD 제어기가 목표 자세를 따라가도록 모터 출력을 조절한다.

## 8. `520`: PID와 전후좌우 조종

`520`은 오차를 시간에 따라 누적하는 I 항을 추가한다.

```cpp
double Ki = 1.0;
ResX += Ki * eAngleX * dt;
ResY += Ki * eAngleY * dt;
ResZ += Ki * eAngleZ * dt;

BalX += ResX;
BalY += ResY;
BalZ += ResZ;
```

I 항은 작은 오차가 오래 남는 정상 상태 오차를 줄인다. 스로틀이 0이 되면 `ResX`, `ResY`, `ResZ`도 0으로 초기화한다.

조종 입력은 목표 Roll과 Pitch를 함께 설정한다.

| 입력 | 목표 동작 | 목표 자세 |
| --- | --- | --- |
| `w` | 전진 | `X=0°`, `Y=-10°` |
| `a` | 왼쪽 | `X=-10°`, `Y=0°` |
| `d` | 오른쪽 | `X=10°`, `Y=0°` |
| `s` | 후진 | `X=0°`, `Y=10°` |
| `f` | 수평 복귀 | `X=0°`, `Y=0°` |
| `0`~`9` | 스로틀 | `0`~`900` |

이 예제의 제어 상수는 `Kp=4.0`, `Kd=1.0`, `Ki=1.0`이고 블루투스 장치 이름은 `PAIkdrone`이다.

## 9. 전체 제어 흐름

```text
MPU6050 데이터 읽기
-> 시작 오프셋 보정
-> 상보 필터 자세 추정
-> 블루투스에서 스로틀과 목표 자세 입력
-> 목표 자세와 현재 자세의 오차 계산
-> P/PD/PID 보정값 계산
-> 4개 모터 믹싱
-> 0~1000 범위 제한
-> LEDC PWM 출력
```

## 10. 시험할 때 주의할 점

- 프로펠러를 분리한 상태에서 모터 방향과 PWM 출력을 먼저 확인한다.
- 전원을 넣은 직후 약 1초간 센서를 평평하게 두고 움직이지 않는다.
- 블루투스 연결 후 가장 먼저 `0`을 보내 정지 명령이 동작하는지 확인한다.
- 낮은 스로틀부터 단계적으로 올린다.
- `520`의 I 항은 스로틀이 0이 아닌 동안 계속 누적되므로 실제 비행 전에는 게인 조정과 적분 포화 방지 범위를 검토한다.
