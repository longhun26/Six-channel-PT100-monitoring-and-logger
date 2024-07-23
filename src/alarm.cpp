#include "alarm.h"
#include "config.h"
#include <Arduino.h>
#include "globals.h"
bool isBuzzerOn = false;

void initAlarm() {
    pinMode(alarmPin, OUTPUT);
    digitalWrite(alarmPin, HIGH);  // 关闭报警
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);  // 关闭蜂鸣器
}

void handleAlarm() {
    checkAlarms();
}

void checkAlarms() {
    bool anyAlarm  = false;
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (temperatures[i] > temperatureThresholds[i]) {
            anyAlarm = true;
            alarmState[i] = true;
            digitalWrite(alarmPin, LOW);  // 触发报警
        } else {
            alarmState[i] = false;
            digitalWrite(alarmPin, HIGH);  // 关闭报警
        }  
    }
    if (anyAlarm) {
        buzzerTicker.attach(0.1, toggleBuzzer);  // 每0.1秒切换一次蜂鸣器状态
    } else {
        buzzerTicker.detach();  // 关闭蜂鸣器定时器
        digitalWrite(buzzerPin, LOW);  // 确保蜂鸣器关闭
    }
    if (battery_level <= 10.0){

        display_Bat = true ;

    } else{
        display_Bat = false ;
    }

}

void toggleBuzzer() {
    isBuzzerOn = !isBuzzerOn;
    digitalWrite(buzzerPin, isBuzzerOn ? HIGH : LOW);
}