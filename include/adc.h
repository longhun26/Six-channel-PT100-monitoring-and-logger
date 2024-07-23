#ifndef ADC_H
#define ADC_H
#include "config.h"
void selectChannel(int channel);
void initADC();
int adc_read_val();
float adcToTemperature(int* adcValue, int* refResults, int index);
void handleADC();


#endif // ADC_H