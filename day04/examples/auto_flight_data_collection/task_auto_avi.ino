#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

TaskHandle_t TaskAutoAviHandle = NULL;

extern volatile int throttle;
extern volatile int data_sampling_en;
extern volatile int data_streaming_en;
extern volatile int samples_collected;

void TaskAutoAviMain(void *pvParameters);

void auto_setup() {
  BaseType_t taskCreated = xTaskCreatePinnedToCore(
      TaskAutoAviMain,
      "TaskAutoAvi",
      10000,
      NULL,
      1,
      &TaskAutoAviHandle,
      0);

  if (taskCreated != pdPASS) {
    TaskAutoAviHandle = NULL;
    Serial.println("Failed to create TaskAutoAvi.");
    return;
  }

  delay(2000);
}

void TaskAutoAviMain(void *pvParameters) {
  (void)pvParameters;

  delay(5000);

  samples_collected = 0;
  data_streaming_en = 0;
  data_sampling_en = 1;

  // 475 * 4 ms = 1,900 ms 동안 상승한다.
  for (int thr = 0; thr < 475; thr++) {
    throttle = thr;
    delay(4);
  }

  // (475 - 300) * 12 ms = 2,100 ms 동안 하강한다.
  for (int thr = 475; thr > 300; thr--) {
    throttle = thr;
    delay(12);
  }

  // 300 * 5 ms = 1,500 ms 동안 0까지 하강한다.
  for (int thr = 300; thr > 0; thr--) {
    throttle = thr;
    delay(5);
  }

  throttle = 0;

  // 현재 태스크를 삭제하기 전에 상태를 변경해야 한다.
  data_sampling_en = 0;
  data_streaming_en = 1;

  Serial.printf(
      "Sampling finished: %d samples. Enter p or c.\n",
      samples_collected);

  TaskAutoAviHandle = NULL;
  vTaskDelete(NULL);
}
