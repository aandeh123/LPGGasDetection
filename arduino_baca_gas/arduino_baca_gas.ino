#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <MQUnifiedsensor.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 3 of your arduino
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-2") //MQ2
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ2CleanAir        (9.83) //RS / R0 = 9.83 ppm 

/*****************************Globals***********************************************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/*****************************Globals***********************************************/
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 7); // TX, RX


void setup() {
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  dht.begin();
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();

  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port
  mySerial.begin(9600);
  //Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(574.25); MQ2.setB(-2.222); // Configure the equation to to calculate LPG concentration
  /*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  /*****************************  MQ Init ********************************************/
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/
  MQ2.init();
  /*
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ2.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/
  // Explanation:
  // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recomend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1);
  }
  /*****************************  MQ CAlibration ********************************************/

  MQ2.serialDebug(true);
}
float h, t;
float ppm;
void loop() {
  MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
  ppm = MQ2.readSensor();
  if (ppm  > 150) {
    digitalWrite(3, LOW);
  }
  else {
    digitalWrite(3, HIGH);
  }
  lcd.setCursor(0, 0);
  lcd.print("PPM:");
  lcd.print(ppm);
  lcd.print("   ");

  h = dht.readHumidity();
  t = dht.readTemperature();
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(t);
  lcd.print("   ");
  lcd.setCursor(8, 1);
  lcd.print("K:");
  lcd.print(h);
  lcd.print("%  ");

  // Kirim lewat SoftwareSerial
  mySerial.print("T:");
  mySerial.print(t);
  mySerial.print(",H:");
  mySerial.print(h);
  mySerial.print(",PPM:");
  mySerial.println(ppm);

  delay(500); //Sampling frequency

}
