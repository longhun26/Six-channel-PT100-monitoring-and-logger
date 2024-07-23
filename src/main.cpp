#include <Arduino.h>
#include <WebServer.h>
#include "config.h"
#include "adc.h"
#include "alarm.h"
#include "buttons.h"
#include "display.h"
#include "rtc.h"
#include "sd_card.h"
#include "globals.h"

void setup() {
    Serial.begin(115200);
    //定义一个定时器
    
    // 初始化各个模块
    initDisplay();
    initButtons();
    initADC();
    initRTC();
   // initSD();
    initAlarm();
    // 检查唤醒原因
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) {
    // 设置Ticker，每秒触发一次中断
     ticker.attach(1, handleADC);  // 每1秒触发一次 handleADC
    tickerActive = true;  // 更新Ticker状态
  } else {
    // 如果从深度睡眠唤醒，立即启动定时器
    //ticker.attach(1, readADC);
   // tickerActive = true;
  } 

}

void loop() {
    handleButtons();
    displayValues();
    if (!tickerActive && Flag_TimIT) {
        ticker.attach(1, handleADC);  // 重新启动定时器中断
        tickerActive = true;  // 更新Ticker状态
    }
    if(isOTA == true && setMode == false && display_Meas == false){
       server.handleClient();
       digitalWrite(alarmPin, LOW);   // LED亮
       vTaskDelay(400 / portTICK_PERIOD_MS);
       digitalWrite(alarmPin, HIGH);    // LED灭
       vTaskDelay(400 / portTICK_PERIOD_MS);

    }
    
    
   // handleADC();
   // handleAlarm();

    // 其他循环代码
    // ...
}