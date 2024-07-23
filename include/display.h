#ifndef DISPLAY_H
#define DISPLAY_H

#include "SPI.h"
#include "Wire.h"
#include <U8g2lib.h>
#include "config.h"


void selectDevice(int csPin);
void initDisplay();
void displayValues();
void displayMessage(const char* message);


#endif // DISPLAY_H