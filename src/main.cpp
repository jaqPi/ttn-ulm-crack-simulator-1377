#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <VL6180X.h>
#include "TCA9548A.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


VL6180X pololusensor1;
#define PIN_RESET_POLOLU_1 5
VL6180X pololusensor2;
#define PIN_RESET_POLOLU_2 6
VL6180X adafruitsensor1;
#define PIN_RESET_ADAFRUIT_1 10
VL6180X adafruitsensor2;
#define PIN_RESET_ADAFRUIT_2 11

TCA9548A I2CMux; 
Adafruit_BME280 bme;

void scanI2C() {
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;


  for (byte i = 1; i < 120; i++)
  {

    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}


/*
https://github.com/luetzel/VL6180x
https://forum.pololu.com/t/vl53l0x-maximum-sensors-on-i2c-arduino-bus/10845/20
https://github.com/pololu/vl53l0x-arduino/issues/1

*/

void setup() {
    pinMode(PIN_RESET_POLOLU_1, OUTPUT);
    digitalWrite(PIN_RESET_POLOLU_1, LOW);

    pinMode(PIN_RESET_POLOLU_2, OUTPUT);
    digitalWrite(PIN_RESET_POLOLU_2, LOW);

    pinMode(PIN_RESET_ADAFRUIT_1, OUTPUT);
    digitalWrite(PIN_RESET_ADAFRUIT_1, LOW);

    pinMode(PIN_RESET_ADAFRUIT_2, OUTPUT);
    digitalWrite(PIN_RESET_ADAFRUIT_2, LOW);

    #ifdef DEBUG    
      Serial.begin(9600);
      while (!Serial);
    #endif
    Serial.println(F("Strtng"));

    delay(500);
    Wire.begin();

    // Sensor 1 Pololu
    pinMode(PIN_RESET_POLOLU_1, INPUT);
    delay(150);
    pololusensor1.init();
    Serial.println("03");
    delay(100);
    pololusensor1.setAddress((uint8_t)25);
    Serial.println("04");

    Serial.println("addresses set");

    pololusensor1.init();
    pololusensor1.configureDefault();


    // Sensor 2 Polulu
    pinMode(PIN_RESET_POLOLU_2, INPUT);
    delay(150);
    pololusensor2.init();
    Serial.println("03");
    delay(100);
    pololusensor2.setAddress((uint8_t)27);
    Serial.println("04");

    Serial.println("addresses set");

    pololusensor2.init();
    pololusensor2.configureDefault();

    // Sensor 3 Adafruit (with pololu lib)
    pinMode(PIN_RESET_ADAFRUIT_1, INPUT);
    delay(150);
    adafruitsensor1.init();
    Serial.println("03");
    delay(100);
    adafruitsensor1.setAddress((uint8_t)29);
    Serial.println("04");

    Serial.println("addresses set");

    adafruitsensor1.init();
    adafruitsensor1.configureDefault();

    // Sensor 4 is left to default address
    pinMode(PIN_RESET_ADAFRUIT_2, INPUT);
    delay(150);
    adafruitsensor2.init();
    adafruitsensor2.configureDefault();


    // Reduce range max convergence time and ALS integration
    // time to 30 ms and 50 ms, respectively, to allow 10 Hz
    // operation (as suggested by table "Interleaved mode
    // limits (10 Hz operation)" in the datasheet).
    pololusensor1.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    pololusensor2.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    adafruitsensor1.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    adafruitsensor2.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);


    pololusensor1.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    pololusensor2.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    adafruitsensor1.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    adafruitsensor2.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);

    pololusensor1.setTimeout(500);
    pololusensor2.setTimeout(500);
    adafruitsensor1.setTimeout(500);
    adafruitsensor2.setTimeout(500);

    // stop continuous mode if already active
    pololusensor1.stopContinuous();
    pololusensor2.stopContinuous();
    adafruitsensor1.stopContinuous();
    adafruitsensor2.stopContinuous();
    // in case stopContinuous() triggered a single-shot
    // measurement, wait for it to complete
    delay(300);


    // IC2 Multiplex
    I2CMux.begin(Wire);             // TwoWire isntance can be passed here as 3rd argument
    I2CMux.closeAll();

    // BME280
    I2CMux.openChannel(0);
    // Setup BME280, use address 0x77 (default) or 0x76
    if (!bme.begin(0x76)) {
      Serial.println(F("noBME"));
      while (1);
    }

    
    // BME280 1
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
    I2CMux.closeChannel(0);


    // BME280 1
    I2CMux.openChannel(1);
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
    I2CMux.closeChannel(1);


    scanI2C();

    // start interleaved continuous mode with period of 100 ms
    pololusensor1.startInterleavedContinuous(100);
    pololusensor2.startInterleavedContinuous(100);
    adafruitsensor1.startInterleavedContinuous(100);
    adafruitsensor2.startInterleavedContinuous(100);
}

void loop()
{
  Serial.print("Ambient: ");
  Serial.print(pololusensor1.readAmbientContinuous());
  if (pololusensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print(" / ");

  Serial.print(pololusensor2.readAmbientContinuous());
  if (pololusensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print(" / ");

  Serial.print(adafruitsensor1.readAmbientContinuous());
  if (adafruitsensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  
  Serial.print(" / ");

  Serial.print(adafruitsensor2.readAmbientContinuous());
  if (adafruitsensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }



  Serial.print("\tRange: ");
  Serial.print(pololusensor1.readRangeContinuousMillimeters());
  if (pololusensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print(" / ");

  Serial.print(pololusensor2.readRangeContinuousMillimeters());
  if (pololusensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print(" / ");
  
  Serial.print(adafruitsensor1.readRangeContinuousMillimeters());
  if (adafruitsensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.print(" / ");
  
  Serial.print(adafruitsensor2.readRangeContinuousMillimeters());
  if (adafruitsensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }


  Serial.print("\tTemp: ");
  I2CMux.openChannel(0);
  bme.takeForcedMeasurement();
  Serial.print(bme.readTemperature());
  Serial.print(" / ");
  I2CMux.closeChannel(0);
  I2CMux.openChannel(1);
  bme.takeForcedMeasurement();
  Serial.print(bme.readTemperature());
  I2CMux.closeChannel(1);

  Serial.print("\tHum: ");
  I2CMux.openChannel(0);
  Serial.print(bme.readHumidity());
  Serial.print(" / ");
  I2CMux.closeChannel(0);
  I2CMux.openChannel(1);
  Serial.print(bme.readHumidity());
  I2CMux.closeChannel(1);

  Serial.println();
  delay(1000);
}