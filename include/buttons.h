#ifndef BUTTONS_H
#define BUTTONS_H
#include "globals.h"

void initButtons();
void handleButtons();
void enterSetMode();
void adjustValueUp();
void adjustValueDown();
void onButtonClick();
void adjustValueUpLongPress();
void adjustValueDownLongPress();
void adjust(enum SetStep currentStep, enum SetupItem currentItem, int direction);
void selectSetupItem();
void doubleClick();
#endif // BUTTONS_H