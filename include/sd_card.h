#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "buttons.h"
#include "config.h"
#include <globals.h>

void initSD();
void logTemperaturesToSD(fs::FS &fs, const char *path);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void readFile(fs::FS &fs, const char *path);
void blinkLED(int times, int interval);

#endif // SD_CARD_H