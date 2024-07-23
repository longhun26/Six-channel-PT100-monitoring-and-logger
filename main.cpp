#include <Arduino.h>
#include "../../include/my_header.h"
#include "my_library.h"
#include <freertos/timers.h>
#include <WiFi.h>  // 包含WiFi头文件
#include <BluetoothSerial.h>  // 包含蓝牙头文件
#include <esp_sleep.h>
#include <RtcDS1302.h>
#include <U8g2lib.h>
#include <Adafruit_ADS1X15.h>
#include <Ticker.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <OneButton.h>


#define alarmPin 2

#define PIN_ENA 26
#define PIN_DAT 27
#define PIN_CLK 14

// Variables to store time and date
 int year = 2024 ;
 int month = 06;
 int day = 22;
 int hours = 12;
 int minutes = 44;
 int seconds = 00;


// OLED定义SPI引脚
#define SD_CS 5      // 替换为你的SPI时钟引脚 
#define OLED_CLK 18      // 替换为你的SPI时钟引脚
#define OLED_MOSI 23     // 替换为你的SPI数据引脚
#define OLED_CS 13      // 替换为你的SPI片选引脚
#define OLED_DC 19      // 替换为你的OLED命令/数据选择引脚
#define OLED_RESET -1  // 替换为你的OLED复位引脚（如果没有复位引脚则设置为-1）

 // Button Pins
#define buttonSetPin  34 // Set button (long press)
#define buttonUpPin  35   // Up button
#define buttonDownPin  15 // Down button

// OneButton instances
OneButton buttonSet(buttonSetPin, true, true);
OneButton buttonUp(buttonUpPin, true, true);
OneButton buttonDown(buttonDownPin, true, true);

SPIClass spi = SPIClass(VSPI);


// 创建U8g2对象
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS,OLED_DC, OLED_RESET);
#define I2C_ADDRESS 0x48 
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */


//定义一个定时器
 Ticker ticker;

 #define NUM_CHANNELS 6
 float temperatures[NUM_CHANNELS];
 int  currentChannel = 0;
 int16_t adcValues[NUM_CHANNELS];
 int16_t refResults[NUM_CHANNELS];
 // 蜂鸣器报警引脚
#define  buzzerPin 12
bool isBuzzerOn = false;
Ticker buzzerTicker;

bool alarmState[NUM_CHANNELS] = {false, false, false, false, false, false};

bool setMode = false; //默认是正常模式，长按确定键进入设置模式
 float temperatureThresholds[NUM_CHANNELS] = {100.0, 100.0, 100.0, 100.0, 100.0, 100.0};
 // 设置项目
enum SetupItem {
  SET_TIME_DATE,
  SET_TEMPERATURE_THRESHOLD,
  SET_WORK_MODE 
};
SetupItem currentSetupItem = SET_TIME_DATE;

enum SetStep {
  START_DATE_TIME,
  SET_YEAR,
  SET_MONTH,
  SET_DAY,
  SET_HOUR,
  SET_MINUTE,
  SET_SECOND,
  TIME_DATE_DONE,
  SET_TEMPERATURE_THRESHOLD_0,
  SET_TEMPERATURE_THRESHOLD_1,
  SET_TEMPERATURE_THRESHOLD_2,
  SET_TEMPERATURE_THRESHOLD_3,
  SET_TEMPERATURE_THRESHOLD_4,
  SET_TEMPERATURE_THRESHOLD_5,
  TEMPERATURE_DONE
};
SetStep currentsetStep = START_DATE_TIME;
// State variables for setting time and date
unsigned long lastBlinkTime = 0;
bool blinkState = true;
bool isSettingMode = false;
// 定时更新值的变量
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 100;  // 100ms
bool isLongPressing = false;
// 定义是否处于设置模式
bool isInSetupMode = false;
bool UpdateTime = false;
bool Uptemperature = false;
bool display_Meas = false;

char secect_mode = 0;


 volatile float multiplier = 0.015625F; /* ADS1115  @ +/- 0.512V gain (16-bit results) */
 volatile float multiplier1 = 0.0078125F; /* ADS1115 @ +/- 0.256Vgain  (16-bit results) */
 volatile bool tickerActive = false;  // 用于跟踪Ticker的状态
 volatile  bool dataReady = false;
 bool Flag_TimIT = false;
 float R0 = 100.0;
 float alpha = 0.00385;
// DS1302 RTC instance
ThreeWire myWire(PIN_DAT,PIN_CLK,PIN_ENA);
RtcDS1302<ThreeWire> Rtc(myWire);
boolean sd_ok = 0;
File dataLog;

void enterSetMode();
void adjustValueUp();
void adjustValueDown();
void onButtonClick();
void adjustValueUpLongPress();
void adjustValueDownLongPress();
void checkAlarms();
void readADC();
void adjust(enum SetStep currentStep, enum SetupItem currentItem,int direction );
float adcToTemperature(int16_t* adcValue, int16_t* refResults, int index);
void displayValues();
void toggleBuzzer();
void logTemperaturesToSD(fs::FS &fs, const char *path);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void readFile(fs::FS &fs, const char *path);
void blinkLED(int times, int interval);
void selectSetupItem();
void selectDevice(int csPin);



void setup() {
  Serial.begin(115200);
  initLibrary(); //CD4051 控制引脚初始化
  // 禁用WiFi和蓝牙
 // WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
 // btStop();
  // 初始化LED报警引脚
  pinMode(alarmPin, OUTPUT);
  digitalWrite(alarmPin, HIGH);  // 关闭报警
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // 关闭报警
  if (!ads.begin(I2C_ADDRESS)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  Serial.println("ADS initialized.");
  ads.setGain(GAIN_EIGHT);
 // ads.setGain(GAIN_FOUR); 
  ads.setDataRate(RATE_ADS1115_16SPS);
  Rtc.Begin();
  RtcDateTime now = Rtc.GetDateTime();
  year = now.Year();
  month = now.Month();
  day = now.Day();
  hours = now.Hour();
  minutes = now.Minute();
  seconds = now.Second();
  Rtc.SetDateTime(RtcDateTime(year, month, day, hours, minutes, seconds));
  //spi.begin(OLED_CLK, OLED_MOSI, OLED_DC, OLED_CS); // 指定引脚
  selectDevice(OLED_CS);
  u8g2.begin();
  u8g2.begin();
  u8g2.setFont(u8g2_font_t0_11_mr);
  u8g2.clearBuffer(); 
  u8g2.drawStr(5, 35, "Booting the device...");
  u8g2.sendBuffer();
  delay(2000);
  /*// 设置定时器1，每秒触发一次中断
  timer = timerBegin(0, 80, true); // 80分频，1微秒定时器
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true); // 每秒触发一次  
  timerAlarmEnable(timer); // 启用定时器中断*/
  // 设置Ticker，每秒触发一次中断
  // 初始化OneButton库
  buttonSet.attachLongPressStart(enterSetMode);
  buttonUp.attachClick(adjustValueUp);
  buttonDown.attachClick(adjustValueDown);
  buttonSet.attachClick(onButtonClick);
  buttonUp.attachDuringLongPress(adjustValueUpLongPress);
  buttonDown.attachDuringLongPress(adjustValueDownLongPress);

  selectChannel(7);

  // 检查唤醒原因
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) {
    // 设置Ticker，每秒触发一次中断
     ticker.attach(1, readADC);  // 每1秒触发一次 readADC
    tickerActive = true;  // 更新Ticker状态
  } else {
    // 如果从深度睡眠唤醒，立即启动定时器
    //ticker.attach(1, readADC);
   // tickerActive = true;
  } 
}

void loop() {
  buttonSet.tick();
  buttonUp.tick();
  buttonDown.tick();
  displayValues();
  if (!tickerActive && Flag_TimIT) {
    ticker.attach(1, readADC);  // 重新启动定时器中断
    tickerActive = true;  // 更新Ticker状态
  }   
} 
void adjustValueUpLongPress() {
  isLongPressing = true;
  if (millis() - lastUpdateTime > updateInterval) {
    adjust(currentsetStep,currentSetupItem, 1); // Increase value
    lastUpdateTime = millis();
  }
}
void adjustValueDownLongPress() {
  isLongPressing = true;
  if (millis() - lastUpdateTime > updateInterval) {
    adjust(currentsetStep,currentSetupItem, -1); // Decrease value
    lastUpdateTime = millis();
  }
}
void onButtonClick() {
   if (setMode == true && isInSetupMode == true) {
      selectSetupItem();
      isInSetupMode = false;
    }
  if (setMode == true && isInSetupMode == false){
    if (currentSetupItem == SET_TIME_DATE || currentSetupItem == SET_TEMPERATURE_THRESHOLD){
     currentsetStep = static_cast<SetStep>(currentsetStep + 1);
     if (currentsetStep == TIME_DATE_DONE) { // 如果当前编辑时间日期完成  
      currentsetStep = START_DATE_TIME;
      Serial.print(currentsetStep);
      isInSetupMode = true; // 退出设置更新
      UpdateTime = true;
 
      Rtc.SetDateTime(RtcDateTime(year, month, day, hours, minutes, seconds));
      } 
      if (currentsetStep == TEMPERATURE_DONE) { // 如果当前编辑温度完成
      currentsetStep = TIME_DATE_DONE;
      isInSetupMode = true; // 退出设置更新
      Uptemperature = true;
      } 
    }else{
      Serial.print("\r\nEnter logger or display Measure");
      if(secect_mode == 1){
        secect_mode = 0;
        display_Meas = false;
        setMode = false;
        //isInSetupMode = true; // 退出设置更新
      }
      if(secect_mode >= 2){
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(5, 20);
        u8g2.print("Enter SD Logger mode");
        u8g2.setCursor(0, 40);
        u8g2.print("Please insert SD card");
        u8g2.setCursor(10, 60);
        u8g2.print(" display turn off");
        u8g2.sendBuffer();
        delay(1000);
        u8g2.setPowerSave(1);
        selectDevice(SD_CS);
        delay(10);
        spi.begin();
        if(!SD.begin(SD_CS,spi,10000000)){
          u8g2.setPowerSave(0);
          Serial.println("Card Mount Failed");
          selectDevice(OLED_CS); 
          delay(10);
          u8g2.begin();
          u8g2.setPowerSave(0);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_6x10_tf);
          u8g2.setCursor(10, 20);
          u8g2.print("Card Mount Failed");
          u8g2.setCursor(5, 40);
          u8g2.print("Enter Logger Failed");
          u8g2.sendBuffer();
          delay(1000);
        } else{
          Serial.println("initialization done.");
          if( SD.exists("/datelog.txt") == 0 ){
              Serial.print("\r\nCreate 'Log.txt' file ... ");
              }
          else
            Serial.println("error creating file.");
      
        }        
        secect_mode = 0;
        display_Meas = true;
        //isInSetupMode = true; // 退出设置更新
        setMode = false;

      }

    }
  }

}
void enterSetMode() {
  setMode = !setMode;
  selectDevice(OLED_CS);
  delay(10);
  u8g2.begin();
  u8g2.setPowerSave(0);
  currentSetupItem = SET_TIME_DATE;
  if (setMode) {
    isInSetupMode = true;
    //UpdateTime = false;
   // Uptemperature = false;
    display_Meas = false;
    Serial.println("Enter Set Mode");
  }else {
    
    isInSetupMode = false;
    Serial.println("Exit Set Mode");
    
  }
}
void adjustValueUp() {
  if (setMode == true && isInSetupMode == true) {
    if (currentSetupItem == SET_WORK_MODE) {
      currentSetupItem = SET_TIME_DATE;
    } else {
      currentSetupItem = static_cast<SetupItem>(currentSetupItem + 1);
    }
  }else {
    if (currentSetupItem == SET_TIME_DATE || currentSetupItem == SET_TEMPERATURE_THRESHOLD){
      adjust(currentsetStep,currentSetupItem, 1); // Increase value
      }else{
        Serial.print("SET WORK MODE");
        secect_mode ++;
        if(secect_mode > 2)
          secect_mode = 1;
      }
    }
}
void adjustValueDown() {
  if (setMode == true && isInSetupMode == true) {
    if (currentSetupItem == SET_TIME_DATE) {
      currentSetupItem = SET_WORK_MODE;
    } else {
      currentSetupItem = static_cast<SetupItem>(currentSetupItem - 1);
    } 
  }else{
    if (currentSetupItem == SET_TIME_DATE || currentSetupItem == SET_TEMPERATURE_THRESHOLD){
      adjust(currentsetStep,currentSetupItem, -1); // Increase value
      }else{
        Serial.print("SET WORK MODE");
        secect_mode ++;
        if(secect_mode > 2)
          secect_mode = 1;
      }
  }
}


void adjust(enum SetStep currentsetStep, enum SetupItem currentItem,int direction ){
  //RtcDateTime t = Rtc.GetDateTime();
  if(currentItem == SET_TIME_DATE){
  switch (currentsetStep) {
    case SET_YEAR:
      year += direction;
      break;
    case SET_MONTH:
      month += direction;
      if (month > 12) month = 1;
      if (month < 1)  month = 12;
      break;
    case SET_DAY:
      day += direction;
      if (day > 31) day = 1;
      if (day < 1) day = 31;
      break;
    case SET_HOUR:
      hours += direction;
      if (hours> 23) hours = 0;
      if (hours < 0) hours= 23;
      break;
    case SET_MINUTE:
      minutes += direction;
      if (minutes > 59) minutes = 0;
      if (minutes < 0) minutes = 59;
      break;
    case SET_SECOND:
      seconds += direction;
      if (seconds > 59) seconds = 0;
      if (seconds< 0) seconds = 59;
      break;
    }
}else {
  if(currentItem == SET_TEMPERATURE_THRESHOLD){
      switch (currentsetStep){
        case SET_TEMPERATURE_THRESHOLD_0:
          temperatureThresholds[0] += direction;
          if (temperatureThresholds[0] > 200) 
            temperatureThresholds[0] =  200;
          if (temperatureThresholds[0] < -50) 
            temperatureThresholds[0] = -50;
           break;

        case SET_TEMPERATURE_THRESHOLD_1:
          temperatureThresholds[1] += direction;
          if (temperatureThresholds[1] > 200) 
            temperatureThresholds[1] =  200;
          if (temperatureThresholds[1] < -50) 
            temperatureThresholds[1] = -50;
           break;
        case SET_TEMPERATURE_THRESHOLD_2:
          temperatureThresholds[2] += direction;
          if (temperatureThresholds[2] > 200) 
            temperatureThresholds[2] =  200;
          if (temperatureThresholds[2] < -50) 
            temperatureThresholds[2] = -50;
           break;
        case SET_TEMPERATURE_THRESHOLD_3:
          temperatureThresholds[3] += direction;
          if (temperatureThresholds[3] > 200) 
            temperatureThresholds[3] =  200;
          if (temperatureThresholds[3] < -50) 
            temperatureThresholds[3] = -50;
           break;
        case SET_TEMPERATURE_THRESHOLD_4:
          temperatureThresholds[4] += direction;
          if (temperatureThresholds[4] > 200) 
            temperatureThresholds[4] =  200;
          if (temperatureThresholds[4] < -50) 
            temperatureThresholds[4] = -50;
           break;
        case SET_TEMPERATURE_THRESHOLD_5:
          temperatureThresholds[5] += direction;
          if (temperatureThresholds[5] > 200) 
            temperatureThresholds[5] =  200;
          if (temperatureThresholds[5] < -50) 
            temperatureThresholds[5] = -50;
           break;  
      }
    }else{

    }
  }
  //Rtc.SetDateTime(RtcDateTime(year, month, day, hours, minutes, seconds));
  //displayValues();
}


void readADC() {
  int  volt1 =0;
  int  volt2 =0;
  char N = 16;
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  selectChannel(currentChannel);
  adcValues[currentChannel] = 0;
  refResults[currentChannel] = 0;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  for(char i = 0; i < N; i++){
    
   // volt1 = ads.readADC_SingleEnded(1);
   // Serial.print(volt1); Serial.print("volt1 ("); Serial.print(volt1 * multiplier); Serial.println("mV)");
   // volt2 = ads.readADC_SingleEnded(2);
    
   // Serial.print(volt2); Serial.print("volt2 ("); Serial.print(volt2 * multiplier); Serial.println("mV)");
   // adcValues[currentChannel] = volt1 - volt2;
   // volt1 =0.0;
   // volt2 =0.0;
    ads.setGain(GAIN_EIGHT); 
    refResults[currentChannel] = ads.readADC_Differential_2_3();
    ads.setGain(GAIN_SIXTEEN);  
    adcValues[currentChannel] = ads.readADC_Differential_0_1();
    //Serial.println(String(i));
    Serial.print(adcValues[currentChannel]); Serial.print("("); Serial.print(adcValues[currentChannel] * multiplier1); Serial.println("mV)");
    Serial.print(refResults[currentChannel]); Serial.print("("); Serial.print(refResults[currentChannel] * multiplier); Serial.println("mV)");
    volt1 +=  adcValues[currentChannel];
    volt2 +=  refResults[currentChannel];
    Serial.println(volt1);
    Serial.println(volt2);
   
  }
  adcValues[currentChannel] = volt1 / N;
  refResults[currentChannel] = volt2/ N;
  temperatures[currentChannel] = adcToTemperature(adcValues,refResults,currentChannel);
  //adcValues[currentChannel] = 0;
  //refResults[currentChannel] = 0;
  if (temperatures[currentChannel] >= 200 || temperatures[currentChannel] <= -50)
  {
    temperatures[currentChannel] = -127;
  }
 // selectChannel(7);


  currentChannel++;
  if (currentChannel >= NUM_CHANNELS) {
    selectChannel(6);
    currentChannel = 0;
    checkAlarms();  // 检查报警
    if(display_Meas == true) {
      logTemperaturesToSD(SD, "/datelog.txt");
    }
    Flag_TimIT = true; // 更新数据就绪状态
    ticker.detach();  // 停止定时器中断
    tickerActive = false;  // 更新Ticker状态    
  }
}

float adcToTemperature(int16_t* adcValue, int16_t* refResults, int index) {

  volatile float multiplier = 0.015625F; /* ADS1115  @ +/- 0.512V gain (16-bit results) */
  volatile float multiplier1 = 0.0078125F; /* ADS1115 @ +/- 0.256Vgain  (16-bit results) */
  float pt_voltage = adcValue[index] * multiplier1;
  float ref_voltage = refResults[index] * multiplier;
  float Rtd = pt_voltage * 499.0 / ref_voltage;
  float temperature= (Rtd/R0-1.0)/alpha;
  Serial.print("rtd Resistance : "); Serial.print(Rtd);Serial.println("欧姆)");
  Serial.print(temperature);Serial.println("度");

  return  temperature;

}

void displayValues(){
  if(setMode == true && isInSetupMode == false){
    u8g2.clearBuffer();
    switch(currentSetupItem){
      case SET_TIME_DATE: {
        if (millis() - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        lastBlinkTime = millis();
        }
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 10, "Set Date Time");
        u8g2.drawLine(7, 12, 90,12); // Adjust the length (24) as needed
        int y = 30; // Y position for the date/time text
        u8g2.setCursor(0, y);
        int x = 0; // Track the x position for the cursor
        u8g2.printf("%04d",year);
        u8g2.print("/");
        u8g2.printf("%02d",month);
        u8g2.print("/");
        u8g2.printf("%02d",day); 
        u8g2.print(" ");
        u8g2.printf("%02d",hours); 
        u8g2.print(":");
        u8g2.printf("%02d",minutes);
        u8g2.print(":");
        u8g2.printf("%02d",seconds);
        if (blinkState){
          Serial.print(currentsetStep);
          switch (currentsetStep){
            case SET_YEAR:
              u8g2.drawLine(x, y + 2, x + 22, y + 2); // Adjust the length (24) as needed
              break;
          case SET_MONTH:
              u8g2.drawLine(x + 30, y + 2, x + 40, y + 2); // Adjust the length (24) as needed
              break;
          case SET_DAY:
              u8g2.drawLine(x + 48, y + 2, x + 58, y + 2); // Adjust the length (24) as needed
              break;
          case SET_HOUR:
              u8g2.drawLine(x + 66, y + 2, x + 76, y + 2); // Adjust the length (24) as needed
              break;
          case SET_MINUTE:
              u8g2.drawLine(x + 84, y + 2, x + 94, y + 2); // Adjust the length (24) as needed
              break;
          case SET_SECOND:
              u8g2.drawLine(x + 102, y + 2, x + 112, y + 2); // Adjust the length (24) as needed
              break;
          }
        }
        break;
      } 
      case SET_TEMPERATURE_THRESHOLD:{
        int j = 1;
        for (int i = 0; i < NUM_CHANNELS; i++) {
        u8g2.setCursor(10, 10 + i * 10);
        u8g2.print("CH");
        u8g2.print(j++);
        u8g2.print(" TLV: ");
        u8g2.print(temperatureThresholds[i]);
        if (millis() - lastBlinkTime > 500) {
          blinkState = !blinkState;
          lastBlinkTime = millis();
        }
        if (blinkState && ((static_cast<int>(currentsetStep ) - 8)== i)) {
        u8g2.drawBox(0, 5 + i * 10, 5, 6); // Blink threshold
          }
        }
        j = 0;
        break;
      }
      case SET_WORK_MODE:{
        u8g2.setFont(u8g2_font_t0_11_mr); 
        u8g2.drawStr(10, 15, "Select mode");
        u8g2.drawLine(7, 17, 83,17); // Adjust the length (24) as needed

        u8g2.drawStr(10, 35, "Display Measure");
        u8g2.drawStr(10, 55, "SD card Logger");
        if (secect_mode){
          u8g2.drawBox(0, 8 + secect_mode *20, 5, 6);
        }
        break;
      }
    }
  u8g2.sendBuffer();
  } 
  if(setMode == true && isInSetupMode == true){
    if(UpdateTime == false || Uptemperature == false ){
      Serial.println("oled 显示");
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_t0_11_mr); 
      u8g2.drawStr(10, 15, "Set Time Date");
      u8g2.drawStr(10, 35, "Set Temperature");
      u8g2.drawStr(10, 55, "Set Work Mode");
      u8g2.drawBox(0, 8 + currentSetupItem * 20, 5, 6); // Blink threshold
      u8g2.sendBuffer();
    }
    if (UpdateTime == true){
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 30);
      u8g2.print("time update completed");
      u8g2.sendBuffer();
      delay(1000);
      UpdateTime = false;
    }
    if (Uptemperature == true){

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 30);
      u8g2.print("Temp update completed");
      u8g2.sendBuffer();
      delay(1000);
      Uptemperature = false;
    }
  }
  if (setMode == false && display_Meas == false){
      
      u8g2.clearBuffer();
      RtcDateTime now = Rtc.GetDateTime();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 10);
      u8g2.printf("%04d/%02d/%02d %02d:%02d:%02d", 
                now.Year(), now.Month(), now.Day(), 
                now.Hour(), now.Minute(), now.Second());
      u8g2.drawFrame(0, 11, 128, 52);
      u8g2.drawLine(0, 27, 127, 27);
      u8g2.drawLine(0, 43, 127, 43);
      u8g2.drawVLine(21, 11, 52);
      u8g2.drawVLine(65, 11, 52);
      u8g2.drawVLine(84, 11, 52);
      u8g2.setFont(u8g2_font_t0_11_mr);
      u8g2.setCursor(2, 25);
      u8g2.print("CH1");
      u8g2.setCursor(66, 25);
      u8g2.print("CH4");
      u8g2.setCursor(2, 41);
      u8g2.print("CH2");
      u8g2.setCursor(66, 41);
      u8g2.print("CH5");
      u8g2.setCursor(2, 57);
      u8g2.print("CH3");
      u8g2.setCursor(66, 57);
      u8g2.print("CH6");
      for (int i = 0; i < NUM_CHANNELS; i++) {
        if(i < 3){
          u8g2.setCursor(22, 25 + i * 16 );
          u8g2.print(temperatures[i]);
        } else {
          int j = i - 3;
          u8g2.setCursor(85, 25 + j * 16);
          u8g2.print(temperatures[i]);
        }
      }
      u8g2.sendBuffer();
      Serial.println("开机 显示");
  }
  /*if (Flag_TimIT){
    Flag_TimIT = false;
   }*/
}


void checkAlarms() {
  bool anyAlarm  = false;
  for (int i = 0; i < NUM_CHANNELS; i++) {
    Serial.println(temperatures[i]);
    Serial.println(temperatureThresholds[i]);
    if (temperatures[i] > temperatureThresholds[i]) {
      anyAlarm = true;
      alarmState[i] = true;
      Serial.println(alarmState[i]);
      digitalWrite(alarmPin, LOW);  // 触发报警
    } else {
      
        alarmState[i] = false;
        digitalWrite(alarmPin, HIGH);  // 关闭报警
      }  
  }
  if (anyAlarm) {
  buzzerTicker.attach(0.1, toggleBuzzer);  // 每0.5秒切换一次蜂鸣器状态
  } else {
    buzzerTicker.detach();  // 关闭蜂鸣器定时器
    digitalWrite(buzzerPin, LOW);  // 确保蜂鸣器关闭
  }
}
void toggleBuzzer() {
  isBuzzerOn = !isBuzzerOn;
  digitalWrite(buzzerPin, isBuzzerOn ? HIGH : LOW);
}
void logTemperaturesToSD(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);
  File dataFile = fs.open(path, FILE_APPEND);
  if (!dataFile){
    Serial.println("Failed to open /datelog.txt");
    return; 
  }
  Serial.println("OK");
  RtcDateTime t = Rtc.GetDateTime();
  dataFile.print(t.Year());
  dataFile.print("-");
  dataFile.print(t.Month());
  dataFile.print("-");
  dataFile.print(t.Day());
  dataFile.print(" ");
  dataFile.print(t.Hour());
  dataFile.print(":");
  dataFile.print(t.Minute());
  dataFile.print(":");
  dataFile.print(t.Second());
  dataFile.print(", ");
  for (int i = 0; i < NUM_CHANNELS; i++) {
    dataFile.print("Channel ");
    dataFile.print(i);
    dataFile.print(": ");
    dataFile.print(temperatures[i]);
    dataFile.print(" C");
    if (i < NUM_CHANNELS - 1) {
      dataFile.print(", ");
    }
  }
    dataFile.println();
    dataFile.close();
    Serial.println("Temperature data logged to SD card."); 
    blinkLED(3, 100);
}
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.print(file.name());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf(
        "  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour,
        tmstruct->tm_min, tmstruct->tm_sec
      );
      if (levels) {
        // listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.print(file.size());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf(
        "  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour,
        tmstruct->tm_min, tmstruct->tm_sec
      );
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void blinkLED(int times, int interval) {
  for (int i = 0; i < times; i++) {
    digitalWrite(alarmPin, LOW);   // LED亮
    delay(interval);              // 延时
    digitalWrite(alarmPin, HIGH);    // LED灭
    delay(interval);              // 延时
  }
}
void selectSetupItem() {
    if (currentSetupItem == SET_TIME_DATE) {
      currentsetStep = START_DATE_TIME;
      setMode = true;
    } else if (currentSetupItem == SET_TEMPERATURE_THRESHOLD) {
      currentsetStep = TIME_DATE_DONE;
      setMode = true;
    } else if (currentSetupItem == SET_WORK_MODE) {
      
    }
  
  displayValues();
}
void selectDevice(int csPin) {
  pinMode(SD_CS, OUTPUT);
  pinMode(OLED_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH); // 确保SD卡未被选择
  digitalWrite(OLED_CS, HIGH);  // 确保U8g2未被选择

  digitalWrite(csPin, LOW); // 选择所需设备
}