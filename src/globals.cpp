#include "config.h"
#include "globals.h"
#include "WiFi.h"
#include "webServer.h"
#define I2C_ADDRESS 0x48 
// 全局变量定义
int year = 2024, month = 6, day = 22, hours = 12, minutes = 44, seconds = 0;
bool alarmState[NUM_CHANNELS] = {false, false, false, false, false, false};
float temperatureThresholds[NUM_CHANNELS] = {100.0, 100.0, 100.0, 100.0, 100.0, 100.0};
bool setMode = false, isInSetupMode = false, UpdateTime = false, Uptemperature = false, display_Meas = false,display_Bat = false;
char secect_mode = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = true;
bool isSettingMode = false;
// 定时更新值的变量
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 100;  // 100ms
bool isLongPressing = false;
volatile float multiplier = 0.015625F; /* ADS1115  @ +/- 0.512V gain (16-bit results) */
volatile float multiplier1 = 0.0078125F; /* ADS1115 @ +/- 0.256Vgain  (16-bit results) */
volatile bool tickerActive = false;  // 用于跟踪Ticker的状态
volatile  bool dataReady = false;
bool Flag_TimIT = false;
float R0 = 100.0;
float alpha = 0.00385;
float temperatures[NUM_CHANNELS] ={0,0,0,0,0,0};
int  currentChannel = 0;
int adcValues[NUM_CHANNELS];
int refResults[NUM_CHANNELS];
float battery_level = 0;
// 枚举变量定义
SetupItem currentSetupItem = SET_TIME_DATE;
SetStep currentsetStep = START_DATE_TIME;
const int controlPins[3] = {32, 33, 25};



// 全局对象定义
Ticker ticker;
Ticker buzzerTicker;
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RESET);
Adafruit_ADS1115 ads;
ThreeWire myWire(PIN_DAT, PIN_CLK, PIN_ENA);
RtcDS1302<ThreeWire> Rtc(myWire);
OneButton buttonSet(buttonSetPin, true, true);
OneButton buttonUp(buttonUpPin, true, true);
OneButton buttonDown(buttonDownPin, true, true);
SPIClass spi(VSPI);
File dataLog;
//OTA 相关
const char *ssid = "SYZ_ESP32";
const char *password = "123456789";
bool isOTA = false;
IPAddress local_IP(192, 168, 4, 1); // 静态IP地址
IPAddress gateway(192, 168, 4, 1); // 网关IP地址
WebServer server(80);
