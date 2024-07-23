#ifndef GLOBALS_H
#define GLOBALS_H

#include <Ticker.h>
#include <U8g2lib.h>
#include <RtcDS1302.h>
#include <Adafruit_ADS1X15.h>
#include <OneButton.h>
#include <config.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <Wifi.h>
#include <WebServer.h >

// 全局变量声明
extern int year, month, day, hours, minutes, seconds;
extern float temperatures[NUM_CHANNELS];
extern bool alarmState[NUM_CHANNELS];
extern float temperatureThresholds[NUM_CHANNELS];
extern bool setMode, isInSetupMode, UpdateTime, Uptemperature, display_Meas,display_Bat,isOTA;
extern char secect_mode;
// State variables for setting time and date
extern unsigned long lastBlinkTime;
extern bool blinkState;
extern bool isSettingMode ;
// 定时更新值的变量
extern unsigned long lastUpdateTime ;
extern const unsigned long updateInterval ;  // 
extern bool isLongPressing ;

extern volatile float multiplier; /* ADS1115  @ +/- 0.512V gain (16-bit results) */
extern volatile float multiplier1 ; /* ADS1115 @ +/- 0.256Vgain  (16-bit results) */
extern volatile bool tickerActive;  // 用于跟踪Ticker的状态
extern volatile  bool dataReady ;
extern bool Flag_TimIT ;
extern float R0 ;
extern float alpha ;
extern float temperatures[NUM_CHANNELS];
extern int  currentChannel ;
extern int adcValues[NUM_CHANNELS];
extern int refResults[NUM_CHANNELS];
extern float battery_level;
extern const int controlPins[3];
extern IPAddress local_IP; // 静态IP地址
extern IPAddress gateway; // 网关IP地址

extern const char *ssid ;
extern const char *password ;
extern WebServer server;

// 枚举声明
enum SetupItem { SET_TIME_DATE, SET_TEMPERATURE_THRESHOLD, SET_WORK_MODE,BATTERY};
extern SetupItem currentSetupItem;

enum SetStep { START_DATE_TIME, SET_YEAR, SET_MONTH, SET_DAY, SET_HOUR, SET_MINUTE, SET_SECOND, TIME_DATE_DONE, SET_TEMPERATURE_THRESHOLD_0, SET_TEMPERATURE_THRESHOLD_1, SET_TEMPERATURE_THRESHOLD_2, SET_TEMPERATURE_THRESHOLD_3, SET_TEMPERATURE_THRESHOLD_4, SET_TEMPERATURE_THRESHOLD_5, TEMPERATURE_DONE };
extern SetStep currentsetStep;
// 全局对象声明
extern Ticker ticker;
extern Ticker buzzerTicker;
extern U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2;
extern Adafruit_ADS1115 ads;
extern ThreeWire myWire;
extern RtcDS1302<ThreeWire> Rtc;
extern OneButton buttonSet;
extern OneButton buttonUp;
extern OneButton buttonDown;
extern SPIClass spi;
extern File dataLog;

#endif // GLOBALS_H