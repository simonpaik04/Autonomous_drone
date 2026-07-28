# MPU6050 자세 추정과 드론 P 제어 학습 기록

날짜: 2026-07-28

## 1. MPU6050과 I2C 통신

MPU6050은 3축 가속도와 3축 자이로 값을 측정하는 IMU 센서다. ESP32에서는 `Wire` 라이브러리로 I2C 통신을 사용한다.

```cpp
#include <Wire.h>

Wire.begin();
Wire.setClock(400000);
```

MPU6050의 기본 I2C 주소는 `0x68`이다. 전원 관리 레지스터 `0x6B`에 `0`을 기록하면 절전 모드가 해제된다.

```cpp
Wire.beginTransmission(0x68);
Wire.write(0x6B);
Wire.write(0x00);
Wire.endTransmission(true);
```

가속도 X축 데이터가 시작되는 `0x3B`부터 14바이트를 읽으면 가속도 3축, 온도, 자이로 3축 값을 한 번에 얻을 수 있다.

```cpp
Wire.beginTransmission(0x68);
Wire.write(0x3B);
Wire.endTransmission(false);
Wire.requestFrom((uint16_t)0x68, (uint8_t)14, true);
```

각 측정값은 상위 8비트와 하위 8비트로 나뉘어 있으므로 하나의 16비트 부호 있는 정수로 합친다.

```cpp
int16_t AcX = AcXH << 8 | AcXL;
int16_t GyX = GyXH << 8 | GyXL;
```

## 2. 정지 상태 오프셋 보정

센서가 움직이지 않아도 가속도 X·Y와 자이로 값이 정확히 0이 되지 않는다. 이 오차를 줄이기 위해 시작 직후 1,000개의 값을 누적하고 평균을 오프셋으로 저장한다.

```cpp
AcXOff = AcXSum / 1000.0;
AcYOff = AcYSum / 1000.0;
AcZOff = AcZSum / 1000.0;

GyXOff = GyXSum / 1000.0;
GyYOff = GyYSum / 1000.0;
GyZOff = GyZSum / 1000.0;
```

각 샘플마다 `delay(1)`이 있으므로 계산과 I2C 통신 시간을 포함해 약 1초 정도 걸린다. 이 시간 동안 센서를 평평한 곳에 두고 움직이지 않아야 한다. 움직인 상태에서 얻은 평균값을 오프셋으로 사용하면 이후의 각도 계산도 틀어진다.

가속도 Z축에는 정지 상태에서도 중력 가속도 약 `16384`가 측정된다. 보정 후에도 중력 성분을 유지하기 위해 다음과 같이 계산한다.

```cpp
double AcXD = AcX - AcXOff;
double AcYD = AcY - AcYOff;
double AcZD = AcZ - AcZOff + 16384;
```

## 3. 실제 반복 시간 `dt`

자이로 각속도를 각도로 바꾸려면 이전 측정부터 현재 측정까지 걸린 시간이 필요하다.

```cpp
static unsigned long t_prev = micros();
unsigned long t_now = micros();
double dt = (t_now - t_prev) / 1000000.0;
t_prev = t_now;
```

`micros()`는 마이크로초 단위이므로 `1,000,000`으로 나누어 초 단위로 변환한다. 반복문의 실행 시간은 항상 완전히 같지 않기 때문에 고정된 시간을 사용하는 것보다 실제 `dt`를 측정하는 편이 정확하다.

## 4. 자이로 센서로 각도 구하기

MPU6050의 자이로 범위가 기본값인 ±250°/s일 때 감도는 `131 LSB/(°/s)`다.

```cpp
double GyXR = GyXD / 131.0;
double GyYR = GyYD / 131.0;
double GyZR = GyZD / 131.0;
```

각속도에 시간을 곱해 계속 누적하면 회전각을 구할 수 있다.

```cpp
gyAngleX += GyXR * dt;
gyAngleY += GyYR * dt;
gyAngleZ += GyZR * dt;
```

자이로는 빠른 움직임을 잘 따라가지만 작은 오차도 계속 적분되므로 시간이 지날수록 각도가 천천히 벗어나는 드리프트가 생긴다.

## 5. 가속도 센서로 Roll과 Pitch 구하기

정지하거나 천천히 움직일 때 가속도 센서가 측정하는 중력 방향을 이용하면 X축과 Y축 기울기를 계산할 수 있다.

```cpp
double AcYZD = sqrt(pow(AcYD, 2) + pow(AcZD, 2));
double AcXZD = sqrt(pow(AcXD, 2) + pow(AcZD, 2));

double acAngleY = atan(-AcXD / AcYZD) * 180 / 3.14159;
double acAngleX = atan(AcYD / AcXZD) * 180 / 3.14159;
```

가속도 센서는 장시간 기준이 안정적이지만 진동이나 이동 가속도가 생기면 각도값이 흔들린다. 중력 방향만으로는 Z축 회전인 Yaw를 구할 수 없다.

## 6. 상보 필터

상보 필터는 자이로의 빠른 반응과 가속도의 장시간 안정성을 결합한다.

```cpp
const double ALPHA = 0.96;

cmAngleX =
    ALPHA * (cmAngleX + GyXR * dt)
    + (1.0 - ALPHA) * acAngleX;

cmAngleY =
    ALPHA * (cmAngleY + GyYR * dt)
    + (1.0 - ALPHA) * acAngleY;
```

현재 설정에서는 자이로 계산값을 96%, 가속도 계산값을 4% 반영한다. Z축은 가속도만으로 기준각을 얻을 수 없기 때문에 예제에서는 자이로 적분값을 사용한다.

```cpp
cmAngleZ = gyAngleZ;
```

## 7. P 제어

목표 각도에서 현재 각도를 빼면 자세 오차를 얻는다.

```cpp
double eAngleX = tAngleX - cmAngleX;
double eAngleY = tAngleY - cmAngleY;
double eAngleZ = tAngleZ - cmAngleZ;
```

P 제어는 오차에 비례 이득 `Kp`를 곱해 균형 보정값을 만든다.

```cpp
double Kp = 1.0;
double BalX = Kp * eAngleX;
double BalY = Kp * eAngleY;
double BalZ = Kp * eAngleZ;
```

기울기가 커질수록 반대 방향으로 되돌리기 위한 보정값도 커진다. `Kp`가 너무 작으면 복원력이 약하고, 너무 크면 목표 각도 주변에서 심하게 흔들릴 수 있다.

## 8. 4개 모터 속도 분배

기본 스로틀에 Roll, Pitch, Yaw 보정값을 서로 다른 부호로 더하거나 빼서 각 모터의 속도를 계산한다.

```cpp
double speedA = throttle + BalX - BalY + BalZ;
double speedB = throttle - BalX - BalY - BalZ;
double speedC = throttle - BalX + BalY + BalZ;
double speedD = throttle + BalX + BalY - BalZ;
```

| 보정 | 증가하는 모터 | 감소하는 모터 |
| --- | --- | --- |
| `BalX` | A, D | B, C |
| `BalY` | C, D | A, B |
| `BalZ` | A, C | B, D |

`470` 예제는 계산된 모터 속도를 시리얼 모니터에 출력하는 단계다. 실제 모터에 적용할 때에는 값을 PWM 범위로 제한한 뒤 각 LEDC 채널에 전달해야 한다.

## 9. 시리얼 포트 문제 해결

업로드 로그에 `Hard resetting via RTS pin`까지 표시되면 ESP32 플래시 기록은 완료된 것이다. 이후 `Port busy` 오류는 다른 프로그램이나 시리얼 모니터가 같은 COM 포트를 사용하고 있다는 뜻이다.

- Arduino IDE의 시리얼 모니터와 시리얼 플로터를 닫는다.
- 다른 Arduino IDE 창이나 터미널 프로그램을 종료한다.
- Windows 장치 관리자에서 ESP32가 연결된 실제 COM 포트를 확인한다.
- Arduino IDE의 `도구 > 포트`에서 같은 포트를 선택한다.
- 해결되지 않으면 USB 케이블을 다시 연결하거나 IDE를 재시작한다.

시리얼 모니터에 `⸮⸮⸮` 같은 문자가 출력되면 코드의 `Serial.begin(115200)`과 시리얼 모니터의 보드레이트를 모두 `115200`으로 맞춘다.

## 10. 이번 단계의 흐름

```text
MPU6050 원시값 읽기
→ 정지 오프셋 보정
→ 자이로 각속도와 가속도 각도 계산
→ 상보 필터로 자세각 추정
→ 목표 각도와의 오차 계산
→ P 제어 보정값 계산
→ 4개 모터 속도로 분배
```

`450` 예제는 자세 추정과 P 제어까지 구현한다. `470` 예제는 여기에 기본 스로틀과 모터 믹싱 계산을 추가한다.
