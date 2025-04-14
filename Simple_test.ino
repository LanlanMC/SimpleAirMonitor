#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Adafruit_ST7796S.h>
#include <BME280_Arduino_I2C.h>
#include "src/MQ7/MQ7.h"
#include "src/MQ135/MQ135.h"

#define TFT_CS           10
#define TFT_DC            9
#define TFT_RST           8

// 定义引脚
#define MQ135_PIN        A1  // MQ-135 模拟输入
#define MQ8_PIN          A4  // MQ-8   模拟输入
#define MQ7_PIN          A0  // MQ-7   模拟输入
#define MQ3_PIN          A3  // MQ-3   模拟输入 
#define LM75_PIN             // LM75   模拟输入
#define BME280_PIN       A5  // BME280 模拟输入
#define BUZZER_PIN        5  // 蜂鸣器控制引脚

// 定义安全范围
#define CO_MAX_PPM       50  // CO          (ppm)
#define CO2_MAX_PPM    1000  // CO₂         (ppm)
#define ALCOHOL         3.3  // CH₂CH₃OH    (%)    note: 爆炸下限LEL
#define H2                4  // H₂          (%)    note: 爆炸下限LEL


Adafruit_ST7796S tft = Adafruit_ST7796S(TFT_CS, TFT_DC, TFT_RST);
BME280_Arduino_I2C bme = BME280_Arduino_I2C(BME280_PIN);
MQ7 mq7 = MQ7(MQ7_PIN);
MQ135 mq135 = MQ135(MQ135_PIN);


void init_monitor()
{
  tft.init();
  tft.invertDisplay(1);
  tft.setRotation(0);
  tft.fillScreen(0x0000);
}

void render_table()
{
  // 格线
  for (uint8_t i=1;i<10;i++)
    tft.drawFastHLine(0, 48*i, 320, 0xFFFF);
  tft.drawFastVLine(32, 48, 432, 0xFFFF);
  tft.drawFastVLine(144, 48, 432, 0xFFFF);
  tft.drawFastVLine(256, 48, 432, 0xFFFF);
  
  // 序号
  tft.setFont(&FreeSans9pt7b);
  for (uint8_t i=49;i<58;i++)
  {
    tft.setCursor(9, 78 + (i-49)*48);
    tft.print((char)i);
  }

  // 项目
  tft.setCursor(36, 78);
  tft.print("Temperature");
  tft.setCursor(52, 124);
  tft.print("Humidity");
  tft.setCursor(70, 174);
  tft.print("atm");
  tft.setCursor(66, 222);
  tft.print("PM");              // TODO
  tft.setFont();
  tft.print("2.5");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(74, 270);
  tft.print("CO");
  tft.setCursor(56, 366);
  tft.print("Alcohol");         // TODO: MQ-3
  tft.setCursor(76, 414);
  tft.print("H");               // TODO: MQ-8
  tft.setFont();
  tft.print("2");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(70, 464);
  tft.print("CO");
  tft.setFont();
  tft.print("2");

  // 单位
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(288, 78);
  tft.print("C");
  tft.drawCircle(285, 68, 2, 0xFFFF);
  tft.setCursor(282, 124);
  tft.print("%");
  tft.setCursor(274, 174);
  tft.print("kPa");
  tft.setCursor(268, 220);
  tft.print("ug/m");
  tft.drawFastVLine(269, 220, 6, 0xFFFF);  // μ的尾巴
  tft.setFont();
  tft.setCursor(310, 206);
  tft.print("3");
  tft.setFont(&FreeSans9pt7b);
  for (uint16_t i=268;i<480;i+=48)
  {
    tft.setCursor(274, i);
    tft.print("ppm");
  }
}

void setup()
{
  init_monitor();
  tft.setFont(&FreeMonoBold12pt7b);
  tft.setTextColor(0xFFFF);
  tft.setCursor(82, 228);
  tft.print("Initializing");

  mq7.calibrate();
  tft.print(".");
  bme.begin();
  tft.print(".");

  // delay(100);
  tft.fillScreen(0x0000);
  
  tft.setCursor(24, 32);
  tft.print("Air Quality Monitor");
  render_table();
  tft.setFont(&FreeMonoBold12pt7b);
}

void loop()
{
  float CO = 0;
  float CO2 = 0;
  float temperature = 0;
  float humidity = 0;
  float pressure = 0;
  for (uint16_t i=0;i<1000;i++)
  {
    BME280Data* data = bme.read();
    humidity += data -> humidity;
    pressure += data -> pressure;
    CO += mq7.readPpm();
    CO2 += mq135.getPPM();
    
    Wire.beginTransmission(LM75_ADDRESS);
    Wire.write(0x00); // 温度寄存器地址
    Wire.endTransmission();
    Wire.requestFrom(LM75_ADDRESS, 2);
    if (Wire.available() >= 2) {
      int16_t rawTemp = (Wire.read() << 8) | Wire.read();
      temperature += rawTemp / 256.0; // 转换为摄氏度
    }

  }
  CO /= 1000;
  CO2 /= 1000;
  temperature /= 1000;
  humidity /= 1000;
  pressure /= 1000;


  if (CO > CO_MAX_PPM)
    tft.fillRect(145, 241, 111, 47, 0x20FD);
  else
    tft.fillRect(145, 241, 111, 47, 0x00E0);
  tft.setCursor(152, 270);
  tft.print(CO);

  if (CO2 > CO2_MAX_PPM)
    tft.fillRect(145, 433, 111, 47, 0x20FD);
  else
    tft.fillRect(145, 433, 111, 47, 0x00E0);
  tft.setCursor(152, 462);
  tft.print(CO2);

  if ()

  tft.fillRect(145, 49, 111, 47, 0x00E0);
  tft.setCursor(152, 78);
  tft.print(temperature);
  
  tft.fillRect(145, 97, 111, 47, 0x00E0);
  tft.setCursor(152, 124);
  tft.print(humidity * 100);

  tft.fillRect(145, 145, 111, 47, 0x00E0);
  tft.setCursor(152, 174);
  tft.print(pressure);
}
