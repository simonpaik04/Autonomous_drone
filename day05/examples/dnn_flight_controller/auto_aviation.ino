#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

TaskHandle_t TaskAutoAviHandle = NULL;

extern volatile int throttle;

void TaskAutoAviMain(void *pvParameters);

void auto_setup() {
  pinMode(0, INPUT_PULLUP);

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
    throttle = 0;
    Serial.println("Failed to create TaskAutoAvi.");
    return;
  }

  Serial.println("DNN controller ready. Press the GPIO 0 button to start.");
}

void TaskAutoAviMain(void *pvParameters) {
  (void)pvParameters;

  // INPUT_PULLUP이므로 버튼을 누르면 LOW가 된다.
  while (digitalRead(0) != LOW) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // 475 * 4 ms = 1,900 ms 동안 상승한다.
  for (int thr = 0; thr < 475; thr++) {
    throttle = thr;
    vTaskDelay(pdMS_TO_TICKS(4));
  }

  // (475 - 300) * 12 ms = 2,100 ms 동안 하강한다.
  for (int thr = 475; thr > 300; thr--) {
    throttle = thr;
    vTaskDelay(pdMS_TO_TICKS(12));
  }

  // 300 * 5 ms = 1,500 ms 동안 0까지 하강한다.
  for (int thr = 300; thr > 0; thr--) {
    throttle = thr;
    vTaskDelay(pdMS_TO_TICKS(5));
  }

  throttle = 0;
  Serial.println("Automatic throttle sequence finished.");

  TaskAutoAviHandle = NULL;
  vTaskDelete(NULL);
}
