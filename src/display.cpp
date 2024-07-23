#include "display.h"
#include "SPI.h"
#include <U8g2lib.h>
#include "config.h"
#include "globals.h"

// 'scrollbar_background', 8x64px
const unsigned char bitmap_scrollbar_background [] PROGMEM = {
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00
};
// 'item_sel_outline', 128x21px
const unsigned char bitmap_item_sel_outline [] PROGMEM = {
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0
};

void selectDevice(int csPin) {
    pinMode(SD_CS, OUTPUT);
    pinMode(OLED_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH); // 确保SD卡未被选择
    digitalWrite(OLED_CS, HIGH); // 确保OLED未被选择
    digitalWrite(csPin, LOW); // 选择所需设备
}

void initDisplay() {
    selectDevice(OLED_CS);
    u8g2.begin();
    u8g2.begin(); // 调用两次以确保初始化成功
    u8g2.setFont(u8g2_font_t0_11_mr);
    u8g2.clearBuffer();
    u8g2.drawStr(2, 35, "Booting the device...");
    u8g2.sendBuffer();
    delay(2000);
}

void displayValues() {
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
        u8g2.setFont(u8g2_font_6x10_tf);
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
        u8g2.setFont(u8g_font_7x14); 
        u8g2.drawStr(10, 15, "Select Mode");
        u8g2.drawLine(7, 17, 83,17); // Adjust the length (24) as needed

        u8g2.drawStr(10, 35, "Display Measure");
        u8g2.drawStr(10, 55, "SD card Logger");
        if (secect_mode){
          u8g2.drawBox(0, 8 + secect_mode *20, 5, 6);
        }
        break;
      }
      case BATTERY:{
        u8g2.setFont(u8g2_font_t0_11_mr); 
        u8g2.drawStr(10, 15, "Battery Levels");
        u8g2.drawLine(7, 17, 95,17); // Adjust the length (24) as needed
        u8g2.setCursor(15, 40);
        u8g2.print(battery_level);
        u8g2.drawStr(50, 40, "%");
        break;
      }
    }
  u8g2.sendBuffer();
  } 
  if(setMode == true && isInSetupMode == true){
    if(UpdateTime == false || Uptemperature == false ){
      //Serial.println("oled 显示");
      u8g2.clearBuffer();
      //u8g2.drawBitmap(0, 22, 128/4, 21, bitmap_item_sel_outline);
      u8g2.setFont(u8g_font_7x14); 
      u8g2.drawStr(10, 15, "Set Time Date");
      //u8g2.setFont(u8g_font_7x14);
      u8g2.drawStr(10, 30, "Set Temperature");
      u8g2.drawStr(10, 45, "Set Work Mode");
      u8g2.drawStr(10, 60, "Battery");
      // draw scrollbar background
      u8g2.drawBitmap(128-8, 0, 8/8, 64, bitmap_scrollbar_background);
      // draw scrollbar handle
      u8g2.drawBox(125, 68/4 * currentSetupItem, 3, 60/5);
      //u8g2.drawBox(0, 8 + currentSetupItem * 15, 5, 6); // Blink threshold
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
  }
  /*if (Flag_TimIT){
    Flag_TimIT = false;
   }*/
  if (display_Bat == true){
    selectDevice(OLED_CS);
    u8g2.begin();
    u8g2.setFont(u8g2_font_t0_11_mr);
    u8g2.clearBuffer();
    u8g2.drawStr(5, 30, "Battery Levels low");
    u8g2.drawStr(5, 50, "Please charge");
    u8g2.sendBuffer();
  }
}

void displayMessage(const char* message) {
    u8g2.clearBuffer();
    u8g2.drawStr(10, 30, message);
    u8g2.sendBuffer();
}

