# ESP32 드론 코딩 학습 기록

날짜: 2026-07-27

## 1. Arduino 프로그램의 기본 구조

ESP32를 Arduino IDE에서 사용하면 C++ 문법과 Arduino 함수를 이용해 프로그래밍한다.

```cpp
void setup() {
  // 처음 한 번만 실행
}

void loop() {
  // 계속 반복 실행
}
```

`setup()`은 전원을 켜거나 리셋했을 때 한 번 실행된다. 센서, 시리얼 통신, GPIO, PWM 채널 등을 초기화할 때 사용한다.

`loop()`는 `setup()` 실행 후 계속 반복된다. 센서 측정, 사용자 입력 확인, 모터 출력 계산 등의 반복 작업을 작성한다.

## 2. 변수와 상수 선언

```cpp
const int fan_pin = 19;
```

- `const`: 선언한 값을 이후에 변경하지 못하게 한다.
- `int`: 정수 자료형이다.
- `fan_pin`: 변수 이름이다.
- `19`: GPIO 핀 번호다.

핀 번호나 PWM 채널처럼 실행 중 변경하지 않을 값은 `const`로 선언한다.

```cpp
const int MOTOR_A = 23;
const int CHANNEL_A = 0;
```

반복 횟수처럼 실행 중 값이 바뀌는 변수에는 `const`를 사용하지 않는다.

```cpp
unsigned int howMany = 50;
howMany--;
```

`unsigned int`는 음수가 없는 정수 자료형이다. `--`는 변수값을 1 감소시킨다.

전역변수는 함수 밖에서 선언하며 모든 함수에서 사용할 수 있다. 지역변수는 함수나 조건문 안에서 선언하며 해당 영역 안에서만 사용할 수 있다.

## 3. GPIO 출력 설정

```cpp
pinMode(fan_pin, OUTPUT);
digitalWrite(fan_pin, HIGH);
digitalWrite(fan_pin, LOW);
```

`pinMode()`는 GPIO 핀의 입력·출력 방향을 설정한다. ESP32에서 HIGH는 논리값 1이며 GPIO에서 약 3.3V가 출력된다. LOW는 논리값 0이며 약 0V가 출력된다.

GPIO가 모터를 직접 구동하는 것은 아니다. ESP32의 HIGH/LOW 신호가 드론 쉴드의 모터 드라이버를 제어하고, 모터 드라이버가 배터리 전원을 모터에 공급한다.

## 4. `delay()` 함수

```cpp
delay(1000);
```

`delay()`는 지정한 밀리초 동안 프로그램 실행을 멈춘다.

- `delay(1)`: 1ms
- `delay(10)`: 10ms
- `delay(100)`: 0.1초
- `delay(1000)`: 1초

`delay()`가 실행되는 동안에는 다른 코드를 처리하지 못한다. PWM 원리 학습이나 간단한 테스트에는 사용할 수 있지만, IMU 측정과 자세 제어를 계속 수행해야 하는 실제 드론 코드에서는 사용을 줄여야 한다.

## 5. `digitalWrite()`를 이용한 소프트웨어 PWM

```cpp
digitalWrite(fan_pin, HIGH);
delay(10);

digitalWrite(fan_pin, LOW);
delay(90);
```

위 코드는 모터를 10ms 동안 켜고 90ms 동안 끈다.

```text
한 주기 = 10ms + 90ms = 100ms
주파수 = 1 / 0.1초 = 10Hz
듀티비 = 10 / 100 × 100 = 10%
```

모터는 실제로 HIGH와 LOW만 입력받지만, 빠른 ON/OFF 반복과 모터의 관성에 의해 평균 출력이 조절된다.

```cpp
unsigned int howMany = 50;

void loop() {
  if (howMany > 0) {
    howMany--;

    digitalWrite(fan_pin, HIGH);
    delay(10);

    digitalWrite(fan_pin, LOW);
    delay(90);
  }
}
```

한 주기가 100ms이고 이를 50번 반복하므로 약 5초 동안 실행된다.

## 6. `if` 조건문

```cpp
if (howMany > 0) {
  howMany--;
}
```

`if`는 괄호 안의 조건이 참일 때만 중괄호 내부 코드를 실행한다.

```cpp
userInput == '0'; // 비교
userInput = '0';  // 값 저장
```

여러 조건을 동시에 만족해야 할 때 `&&`를 사용한다.

```cpp
if (userInput >= '0' && userInput <= '5') {
}
```

위 조건은 입력 문자가 `'0'` 이상이고 동시에 `'5'` 이하인지 확인한다.

## 7. ESP32 LEDC 하드웨어 PWM

LEDC는 ESP32 내부 하드웨어가 PWM을 생성하는 기능이다. `digitalWrite()`와 `delay()`를 계속 실행하지 않아도 PWM 신호가 자동으로 출력된다.

```cpp
const int fan_pin = 19;
const int fan_channel = 1;
const int fan_freq = 5000;
const int fan_resolution = 10;
```

- `fan_pin`: PWM 신호가 출력되는 실제 GPIO 번호
- `fan_channel`: ESP32 내부 PWM 발생기 번호
- `fan_freq`: PWM 주파수
- `fan_resolution`: PWM 듀티값의 해상도

```cpp
ledcSetup(fan_channel, fan_freq, fan_resolution);
ledcAttachPin(fan_pin, fan_channel);
ledcWrite(fan_channel, 100);
```

10비트 해상도의 출력 범위는 0~1023이다.

```cpp
ledcWrite(fan_channel, 0);     // 0%
ledcWrite(fan_channel, 100);   // 약 9.8%
ledcWrite(fan_channel, 512);   // 약 50%
ledcWrite(fan_channel, 1023);  // 100%
```

```text
듀티비 = ledcWrite 값 / 1023 × 100
```

해상도가 높을수록 0%에서 100% 사이를 더 세밀하게 나눌 수 있다. 해상도가 높아져도 최대 출력이 커지는 것은 아니다.

## 8. PWM 주파수

```cpp
const int fan_freq = 5000;
```

5000Hz는 HIGH와 LOW로 구성된 PWM 한 주기가 1초에 5,000번 반복된다는 의미다.

```text
주기 = 1 / 5000 = 0.0002초 = 200μs
```

- 주파수: ON/OFF 반복 속도
- 듀티비: 한 주기에서 ON 상태가 차지하는 비율
- 모터 출력: 주로 듀티비로 조절

주파수가 너무 낮으면 모터가 회전과 정지를 반복하며 덜컥거릴 수 있다. 주파수를 높이면 ON/OFF 간격이 짧아져 비교적 부드럽게 회전한다.

## 9. 4개 모터와 PWM 채널 설정

각 모터를 독립적으로 제어하기 위해 서로 다른 GPIO 핀과 PWM 채널을 사용한다.

```cpp
const int MOTOR_A = 23;
const int MOTOR_B = 19;
const int MOTOR_C = 18;
const int MOTOR_D = 26;

const int CHANNEL_A = 0;
const int CHANNEL_B = 1;
const int CHANNEL_C = 2;
const int CHANNEL_D = 3;
```

각 채널에 주파수와 해상도를 설정하고 모터 핀을 연결한다.

```cpp
ledcSetup(CHANNEL_A, MOTOR_FREQ, MOTOR_RESOLUTION);
ledcAttachPin(MOTOR_A, CHANNEL_A);
```

초기화 과정에서는 모든 모터 출력을 0으로 설정한다.

```cpp
ledcWrite(CHANNEL_A, 0);
ledcWrite(CHANNEL_B, 0);
ledcWrite(CHANNEL_C, 0);
ledcWrite(CHANNEL_D, 0);
```

## 10. USB 시리얼 통신

```cpp
Serial.begin(115200);
```

`Serial.begin()`은 PC와 ESP32의 시리얼 통신을 시작한다. `115200`은 통신 속도다.

```cpp
if (Serial.available() > 0) {
  char userInput = Serial.read();
}
```

`Serial.available()`은 수신 버퍼에 저장된 데이터의 바이트 수를 반환한다. `Serial.read()`는 수신 버퍼에서 문자 한 개를 읽는다.

```cpp
Serial.print(userInput);
Serial.println(userInput);
```

- `Serial.print()`: 출력 후 줄을 바꾸지 않음
- `Serial.println()`: 출력 후 다음 줄로 이동

## 11. 문자와 숫자 변환

시리얼에서 입력한 `'3'`은 정수 3이 아니라 문자 `'3'`이다. 문자 숫자를 실제 정수로 변환하려면 문자 `'0'`의 값을 뺀다.

```cpp
int number = userInput - '0';
int throttle = (userInput - '0') * 100;
```

```text
'0' -> 0
'1' -> 100
'2' -> 200
'3' -> 300
'4' -> 400
'5' -> 500
```

계산한 `throttle` 값을 4개 모터에 동일하게 적용할 수 있다.

## 12. 블루투스 시리얼 통신

```cpp
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
```

ESP32의 블루투스 이름을 `PAIKdrone`으로 설정하고 통신을 시작한다.

```cpp
SerialBT.begin("PAIKdrone");
```

블루투스로 받은 문자 한 개를 읽고 다시 전송할 수 있다.

```cpp
if (SerialBT.available() > 0) {
  char userInput = SerialBT.read();
  SerialBT.println(userInput);
}
```

## 13. 블루투스 입력으로 모터 속도 제어

```cpp
if (SerialBT.available() > 0) {
  char userInput = SerialBT.read();

  if (userInput >= '0' && userInput <= '5') {
    int throttle = (userInput - '0') * 100;

    ledcWrite(CHANNEL_A, throttle);
    ledcWrite(CHANNEL_B, throttle);
    ledcWrite(CHANNEL_C, throttle);
    ledcWrite(CHANNEL_D, throttle);
  }
}
```

블루투스로 문자가 들어오면 `'0'~'5'` 범위인지 확인한 뒤 숫자로 변환한다. 변환된 숫자에 100을 곱해 모터 PWM 출력값으로 사용한다.

현재 코드는 마지막으로 입력받은 모터 출력이 계속 유지된다. 통신이 끊겼을 때 자동으로 모터를 정지시키는 기능은 포함되어 있지 않다.

## 14. 모터 쌍을 이용한 Yaw 회전 테스트

드론의 대각선 모터 쌍에 서로 다른 출력을 주면 반작용 토크 차이로 Yaw 방향 회전을 만들 수 있다.

```cpp
// A와 C 모터 구동
ledcWrite(CHANNEL_A, throttle);
ledcWrite(CHANNEL_B, 0);
ledcWrite(CHANNEL_C, throttle);
ledcWrite(CHANNEL_D, 0);
delay(2000);

// 전체 정지
ledcWrite(CHANNEL_A, 0);
ledcWrite(CHANNEL_B, 0);
ledcWrite(CHANNEL_C, 0);
ledcWrite(CHANNEL_D, 0);
delay(2000);

// B와 D 모터 구동
ledcWrite(CHANNEL_A, 0);
ledcWrite(CHANNEL_B, throttle);
ledcWrite(CHANNEL_C, 0);
ledcWrite(CHANNEL_D, throttle);
delay(2000);
```

이 코드는 모터 회전 원리를 확인하는 개방루프 테스트다. 실제 비행에서는 기본 추력을 유지하면서 모터 쌍의 출력을 미세하게 증감하고, IMU와 PID 제어를 이용해 원하는 각속도를 유지해야 한다.
