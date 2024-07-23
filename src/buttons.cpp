#include "buttons.h"
#include <OneButton.h>
#include "config.h"
#include "globals.h"
#include "display.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
//IPAddress local_IP(192, 168, 4, 1); // 静态IP地址
//IPAddress gateway(192, 168, 4, 1); // 网关IP地址
//WebServer server(80);

const char* updateIndex =
  "<html>"
  "<head>"
  "<meta charset=\"UTF-8\">"
  "<title>ESP32 OTA 更新</title>"
  "<style>"
  "body { font-family: Arial, sans-serif; text-align: center; }"
  "h1 { color: #333; }"
  "form { margin-top: 20px; }"
  "input[type=file] { display: block; margin: 20px auto; }"
  "input[type=submit] { margin-top: 20px; padding: 10px 20px; font-size: 18px; }"
  "</style>"
  "</head>"
  "<body>"
  "<h1>欢迎使用 ESP32 OTA 更新</h1>"
  "<form method='POST' action='/update' enctype='multipart/form-data'>"
  "<input type='file' name='update' accept='.bin'>"
  "<input type='submit' value='上传固件'>"
  "</form>"
  "</body>"
  "</html>";
void handleRoot() {
  server.sendHeader("Location", "/update");
  server.send(302, "text/plain", "");
}
void initButtons() {
    buttonSet.attachLongPressStart(enterSetMode);
    buttonUp.attachClick(adjustValueUp);
    buttonDown.attachClick(adjustValueDown);
    buttonSet.attachClick(onButtonClick);
    buttonSet.attachDoubleClick(doubleClick); // 处理双击事件
    buttonUp.attachDuringLongPress(adjustValueUpLongPress);
    buttonDown.attachDuringLongPress(adjustValueDownLongPress);
}

void handleButtons() {
    buttonSet.tick();
    buttonUp.tick();
    buttonDown.tick();
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
    isOTA = false;
    Serial.println("Enter Set Mode");
  }else {
    isInSetupMode = false;
    Serial.println("Exit Set Mode");  
    }
}

void adjustValueUp() {
  if (setMode == true && isInSetupMode == true) {
    if (currentSetupItem == BATTERY) {
      currentSetupItem = SET_TIME_DATE;
    } else {
      currentSetupItem = static_cast<SetupItem>(currentSetupItem + 1);
    }
  }else {
    if (currentSetupItem == SET_TIME_DATE || currentSetupItem == SET_TEMPERATURE_THRESHOLD || currentSetupItem == BATTERY){
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
      currentSetupItem = BATTERY;
    } else {
      currentSetupItem = static_cast<SetupItem>(currentSetupItem - 1);
    } 
  }else{
    if (currentSetupItem == SET_TIME_DATE || currentSetupItem == SET_TEMPERATURE_THRESHOLD || currentSetupItem == BATTERY){
      adjust(currentsetStep,currentSetupItem, -1); // Increase value
      }else{
        Serial.print("SET WORK MODE");
        secect_mode ++;
        if(secect_mode > 2)
          secect_mode = 1;
      }
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
void selectSetupItem() {
    if (currentSetupItem == SET_TIME_DATE) {
      currentsetStep = START_DATE_TIME;
      setMode = true;
    } else if (currentSetupItem == SET_TEMPERATURE_THRESHOLD) {
      currentsetStep = TIME_DATE_DONE;
      setMode = true;
    } else if (currentSetupItem == SET_WORK_MODE) {
      
    }else if (currentSetupItem == BATTERY) {
      
    }
}
void doubleClick() {
  Serial.print("doubleClick");
  isOTA = true;
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
  Serial.print("Access Point IP address: ");
  Serial.println(WiFi.softAPIP()); // 打印 ESP32 的 AP IP 地址
    // 设置服务器处理函数
  server.on("/", HTTP_GET, handleRoot); // 根路由重定向到 OTA 页面
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    //动态显示结果
    String message = Update.hasError() ? "更新失败" : "更新成功。重新启动…";
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.send(200, "text/html", "<span style='font-size: 24px;'>" + message + "</span>");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(15, 30);
    u8g2.print("Firmware Update");
    u8g2.setCursor(25, 45);
    u8g2.print("completed !");
    u8g2.sendBuffer();
    delay(1000);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload(); //用于处理上传的文件数据
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // 以最大可用大小开始
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        // 将接收到的数据写入Update对象
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { // 设置大小为当前大小
          Serial.printf("Update Success: %u bytes\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });
  server.begin();
  Serial.println("HTTP server started"); 
}
