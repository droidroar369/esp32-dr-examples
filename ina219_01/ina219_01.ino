/*
* Program to perform basic measurementes with an INA219 module, and show them in an Oled screen
*/
#include <Wire.h>
#include <Adafruit_INA219.h>

//For showing measurements on an Oled screen. Comment if you won't use an Oled
#define SDA_OLED 5
#define SCL_OLED 6

#define INA_ADDR 0x40

//OLed object, there is no 72x40 constructor in u8g2 hence the 72x40 screen is
// mapped in the middle of the 132x64 pixel buffer of the SSD1306 controller
#if defined(SDA_OLED) && defined(SCL_OLED)
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_OLED, SDA_OLED);

const int width = 70;
const int height = 40;
const int xOffset = 30; // = (132-w)/2
const int yOffset = 24; // = (64-h)/2
#endif

Adafruit_INA219 ina219(INA_ADDR);


void setup(void) 
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello!");
  
  //If Oled is present, begin Wire at Oled pins, and connect INA module to these same pins
  #if defined(SDA_OLED) && defined(SCL_OLED)
  if(!Wire.begin(SDA_OLED, SCL_OLED)) {
    Serial.println("Error while initializing I2C bus");
    for(;;);
  }
  #endif

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while(1) delay(10);
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");

  //Begin Oled
  #if defined(SDA_OLED) && defined(SCL_OLED)
  if(!u8g2.begin()) {
    Serial.println("Failed to initialize Oled. Using only Serial");
    return;
  }
  u8g2.setContrast(255); // set contrast to maximum 
  u8g2.setBusClock(400000); //400kHz I2C 
  u8g2.enableUTF8Print();
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_17_tr);
  u8g2.setCursor(xOffset+width/3, yOffset+height/2);
  u8g2.print("Hello!");
  u8g2.sendBuffer();
  Serial.println("OLed iniciada.");

  #endif
}


void loop(void) 
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  #if defined(SDA_OLED) && defined(SCL_OLED)
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr); //u8g2_font_t0_17_tr
  //Print load voltage
  u8g2.setCursor(xOffset+3, yOffset+3+u8g2.getMaxCharHeight());
  u8g2.printf("V=%.2f V", loadvoltage);
  //Print current
  u8g2.setCursor(xOffset+3, yOffset+3+2*u8g2.getMaxCharHeight());
  u8g2.printf("I=%.2f mA", current_mA);
  u8g2.sendBuffer();
  #endif

  delay(2000);
}
