#include "sd_card.h"
#include <SD.h>
#include "globals.h"
#include "config.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "rtc.h"
#include "adc.h"


void initSD() {
    if (!SD.begin(SD_CS)) {
        Serial.println("SD卡初始化失败！");
        return;
    }
    Serial.println("SD卡初始化成功！");
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