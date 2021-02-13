#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <VL6180X.h>
#include "TCA9548A.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "measurements.h"
#include "sensors.h"
#include <Statistics.h>

#ifndef DEBUG
  #define Serial
#endif

VL6180X pololusensor1;
#define PIN_RESET_POLOLU_1 5
VL6180X pololusensor2;
#define PIN_RESET_POLOLU_2 6
VL6180X adafruitsensor1;
#define PIN_RESET_ADAFRUIT_1 10
VL6180X adafruitsensor2;
#define PIN_RESET_ADAFRUIT_2 11

const uint8_t numberOfMeasurements = 50; // max 255!

TCA9548A I2CMux; 
Adafruit_BME280 bme;

// Initially the same config for all sensors
const SensorConfig defaultConfig = { INTERLEAVED, MANUAL, 30, 50, 500};

const TofSensor tofSensors[4] = {
  TofSensor { pololusensor1,   defaultConfig, PIN_RESET_POLOLU_1,   25 },
  TofSensor { pololusensor2,   defaultConfig, PIN_RESET_POLOLU_2,   26 },
  TofSensor { adafruitsensor1, defaultConfig, PIN_RESET_ADAFRUIT_1, 27 },
  TofSensor { adafruitsensor2, defaultConfig, PIN_RESET_ADAFRUIT_2, 28 }
};

const uint8_t numberOfSensors = sizeof(tofSensors) / sizeof(TofSensor);




void scanI2C() {
  Serial.println("I2C scanner. Scanning ...");
  byte count = 0;


  for (byte i = 1; i < 120; i++)
  {

    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print("Found address: ");
      Serial.print(i);
      Serial.print(" (0x");
      Serial.print(i);
      Serial.println(")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  Serial.println("Done.");
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" device(s).");
}


/*
https://github.com/luetzel/VL6180x
https://forum.pololu.com/t/vl53l0x-maximum-sensors-on-i2c-arduino-bus/10845/20
https://github.com/pololu/vl53l0x-arduino/issues/1

*/

void setup() {
    // Send shutdown signal to VL6180X sensors to be able to changer their I2C address
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      pinMode(tofSensors[i].resetPin, OUTPUT);
      digitalWrite(tofSensors[i].resetPin, LOW);
    }

    #ifdef DEBUG    
      Serial.begin(9600);
      while (!Serial);
    #endif
    Serial.println(F("Strtng"));

    delay(500);
    Wire.begin();

    // Wake up one sensor afther the other to change its I2C address
    // and configure it
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      Serial.print("Configure ToF-Sensor #");
      Serial.print(i);
      Serial.print(": ");
      pinMode(tofSensors[i].resetPin, INPUT);
      delay(150);
      tofSensors[i].sensor.init();
      delay(100);
      tofSensors[i].sensor.setAddress(tofSensors[i].i2cAddress);
      Serial.print("I2C address set to ");
      Serial.print(tofSensors[i].sensor.getAddress());

      tofSensors[i].sensor.init();
      tofSensors[i].sensor.configureDefault();

      // Reduce range max convergence time and ALS integration
      // time to 30 ms and 50 ms, respectively, to allow 10 Hz
      // operation (as suggested by table "Interleaved mode
      // limits (10 Hz operation)" in the datasheet).
      tofSensors[i].sensor.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, tofSensors[i].config.rangeMaxConvergenceTime);
      tofSensors[i].sensor.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, tofSensors[i].config.alsMaxIntegrationPeriod);
      tofSensors[i].sensor.setTimeout(tofSensors[i].config.timeout);

      // stop continuous mode if already active
      tofSensors[i].sensor.stopContinuous();
      // in case stopContinuous() triggered a single-shot
      // measurement, wait for it to complete
      delay(300);

      if(tofSensors[i].config.calibrationMode == MANUAL) {
        // disable auto calibrate (to do it manually before every series)
        tofSensors[i].sensor.writeReg(VL6180X::SYSRANGE__VHV_REPEAT_RATE, 0x00);    
        // calibrate single time (actually the sensor should have done it during start up)
        tofSensors[i].sensor.writeReg(VL6180X::SYSRANGE__VHV_RECALIBRATE, 0x01);   
      }

      Serial.println(", configuration completed");
    }

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
    //pololusensor1.startInterleavedContinuous(100);

}

void loop()
{
  Measurement measurements[numberOfSensors];

  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    measurements[i] = measureDistanceAndAmbientLightInterleaved(&tofSensors[i], numberOfMeasurements);
    // short delay
    delay(100);
  }
  /*

  Serial.print("Ambient: ");
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    Serial.print(tofSensors[i].sensor.readAmbientContinuous());
    if (tofSensors[i].sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

    if(i < numberOfSensors-1) { Serial.print(" / "); }
  }


  Serial.print("\tRange: ");
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    Serial.print(tofSensors[i].sensor.readRangeContinuousMillimeters());
    if (tofSensors[i].sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

    if(i < numberOfSensors-1) { Serial.print(" / "); }
  }
  */

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