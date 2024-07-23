#include "adc.h"
#include "config.h"
#include "alarm.h"
#include "sd_card.h"
#include "Arduino.h"
#include "globals.h"
#include "esp32-hal-adc.h"

// ADC 对象
//static esp_adc_cal_characteristics_t *adcChar;
adc_attenuation_t adc_att = ADC_2_5db;
const float battery_min = 3.3; // (V) minimum voltage of battery before shutdown
const float battery_max = 4.2; // (V) maximum voltage of battery
/* R1 & R2 */
const float R1 = 300000.0;     //300k
const float R2 = 100000.0;     //100k
/* adc电压手动校准（adc接地读出来多少这里就写多少） */
const float adc_offset = 0.00;
/* 电池电压手动补偿 */
const float vol_offset = 0.00;
void initADC() {
    if (!ads.begin()) {
        Serial.println("Failed to initialize ADS.");
        while (1);
    }
    Serial.println("ADS initialized.");
    ads.setGain(GAIN_EIGHT);
    ads.setDataRate(RATE_ADS1115_16SPS);
    for (char i = 0;i < 3;i++){
      pinMode(controlPins[i], OUTPUT);  //初始化CD4501 控制脚
    }
  pinMode(ADC_CHANNEL, INPUT);
  adcAttachPin(ADC_CHANNEL);   
  analogReadResolution(12); 
  analogSetPinAttenuation(ADC_CHANNEL, adc_att);             
  

}
int adc_read_val()
{

  int samplingFrequency = 500; // 采样频率（可调）
  long sum = 0;             // 采样和
  float samples = 0.0;      // 采样平均值

  for (int i = 0; i < samplingFrequency; i++)
  {
      sum += analogReadMilliVolts(ADC_CHANNEL); // Take an ADC1 reading from a single channel.
      delayMicroseconds(1000);
  }
  samples = sum / (float)samplingFrequency;

    //uint voltage = esp_adc_cal_raw_to_voltage(samples, adcChar); // 2.调用 API 函数将 ADC 值转换为电压（在调用此函数之前，必须初始化特征结构，也就是步骤 1）
  //int voltage = (samples ) / 4095; //通过公式转换为电压
  //return voltage; //单位(mV)
  return samples;
}

void handleADC() {
  int  volt1 =0;
  int  volt2 =0;
  char N = 16;
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  selectChannel(currentChannel);
  adcValues[currentChannel] = 0;
  refResults[currentChannel] = 0;
  //temperatures[currentChannel]= 0;
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
  currentChannel++;
  if (currentChannel >= NUM_CHANNELS) {
    /*adc端电压 */
  //adcStart(ADC_CHANNEL);
   float adc_voltage = (float)adc_read_val() / 1000.0f + adc_offset;
    Serial.print("ADC voltage: ");
    Serial.print(adc_voltage);
    Serial.println("V");

    /*通过R1&R2推算电池电压*/
    float battery_voltage = (R1 + R2) / R2 * adc_voltage + vol_offset;
    Serial.print("Battery voltage: ");
    Serial.print(battery_voltage);
    Serial.println("V");

    /*通过电池最低与最高电压推算电池电量*/
    battery_level = ((battery_voltage - battery_min) / (battery_max - battery_min)) * 100;
    Serial.print("Battery Level: ");
    if (battery_level < 100)   
       Serial.print(battery_level);
    else {
    battery_level = 100;
    Serial.print(battery_level); 
    Serial.println("%");
    Serial.print("\n");
    }                       
      Serial.print(100.0); 
    Serial.println("%");
    Serial.print("\n");
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


float adcToTemperature(int* adcValue, int* refResults, int index) {

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

void selectChannel(int channel) {
  // 确保通道号在有效范围内
  if (channel < 0 || channel > 7) {
    Serial.println("通道号无效。请输入0到7之间的数字。");
    return;
  }

  // 根据通道号设置地址选择引脚
  digitalWrite(controlPins[0], channel & 0x01);
  digitalWrite(controlPins[1], (channel >> 1) & 0x01);
  digitalWrite(controlPins[2], (channel >> 2) & 0x01);
}

