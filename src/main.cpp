#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <VL6180X.h>
#include "TCA9548A.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#ifdef DEBUG
  #define print(x) Serial.print(x);
  #define println(x) Serial.println(x);
#else
  #define print(x)
  #define println(x)
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

struct TofSensor
{
    VL6180X &sensor;
    uint8_t resetPin, i2cAddress;
};

const TofSensor tofSensors[4] = {
  TofSensor { pololusensor1,   PIN_RESET_POLOLU_1,   25 },
  TofSensor { pololusensor2,   PIN_RESET_POLOLU_2,   26 },
  TofSensor { adafruitsensor1, PIN_RESET_ADAFRUIT_1, 27 },
  TofSensor { adafruitsensor2, PIN_RESET_ADAFRUIT_2, 28 }
};

const uint8_t numberOfSensors = sizeof(tofSensors) / sizeof(TofSensor);

struct Stats
{
    double mean, standardDeviation;
};

struct Measurement
{
    double meanDistance, standardDeviationDistance, meanAmbientLight, standardDeviationAmbientLight;
    uint8_t successfulMeasurementsDistance, successfulMeasurementsAmbientLight;
};


double calcMean(uint8_t successfulMeasurements, uint16_t measurementSeries[numberOfMeasurements]) {
    double mean = 0.0;

    if (successfulMeasurements > 0)
    {
        // Mean
        for(uint8_t i = 0; i < successfulMeasurements; i++) {
            mean += measurementSeries[i];
        }
        mean = mean/successfulMeasurements;
    }

    return mean;
}

double calcSD(uint8_t successfulMeasurements, uint16_t measurementSeries[numberOfMeasurements], double mean) {
    double standardDeviation = 0.0;
    if (successfulMeasurements > 0)
    {
        double variance = 0.0;
        // Variance
        for(uint8_t i = 0; i < successfulMeasurements; i++) {
            variance += sq(measurementSeries[i] - mean);
        }
        variance = variance/(successfulMeasurements-1);

        // Standard deviation
        standardDeviation = sqrt(variance);
    }

     return standardDeviation;
}

struct Stats calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[numberOfMeasurements]) {
    double mean = calcMean(successfulMeasurements, measurementSeries);
    return Stats { mean, calcSD(successfulMeasurements, measurementSeries, mean)};
}


struct Measurement  measureDistanceAndAmbientLight(uint8_t sensorNumber) {
    uint16_t measurementSeriesDistance[numberOfMeasurements];
    uint16_t measurementSeriesAmbientLight[numberOfMeasurements];

    print("Start measurement using ToF-Sensor ");
    print(tofSensors[sensorNumber].i2cAddress);
    print(", #Measurement: ");
    println(numberOfMeasurements);

    // calibrate sensor in terms of temperature
    // disabled
    //sensor.writeReg(sensor.SYSRANGE__VHV_RECALIBRATE, 0x01);    


    tofSensors[sensorNumber].sensor.startInterleavedContinuous();
    uint8_t successfulMeasurementsDistance = 0;
    uint8_t successfulMeasurementsAmbientLight = 0;

    for (uint8_t i = 0; i < numberOfMeasurements; i++) {
        // Ambient Light
        uint16_t currentAmbientLight = tofSensors[sensorNumber].sensor.readAmbientContinuous();
        if (!tofSensors[sensorNumber].sensor.timeoutOccurred()) {
            measurementSeriesAmbientLight[successfulMeasurementsAmbientLight] = currentAmbientLight;
            successfulMeasurementsAmbientLight += 1;

            #ifdef SINGLE_VALUES
                // Print current value to Serial
                if(i == numberOfMeasurements - 1) {
                    println(currentAmbientLight);
                }
                else {
                    print(currentAmbientLight);
                    print(",");
                }
            #endif
        }

        // Distance
        uint16_t currentDistance = (uint16_t) tofSensors[sensorNumber].sensor.readRangeContinuous();
        if (!tofSensors[sensorNumber].sensor.timeoutOccurred()) {
            measurementSeriesDistance[successfulMeasurementsDistance] = currentDistance;
            successfulMeasurementsDistance += 1;

            #ifdef SINGLE_VALUES
                // Print current value to Serial
                if(i == numberOfMeasurements - 1) {
                    println(currentDistance);
                }
                else {
                    print(currentDistance);
                    print(",");
                }
            #endif
        }
    }
    tofSensors[sensorNumber].sensor.stopContinuous();

    struct Stats statsAmbientLight = calcStats(successfulMeasurementsAmbientLight, measurementSeriesAmbientLight);
    struct Stats statsDistance = calcStats(successfulMeasurementsDistance, measurementSeriesDistance);



    // print("Distsucc: ");
    // print(successfulMeasurementsDistance);
    // print("/");
    // print(numberOfMeasurements);
    // println();
    print("DistM:");
    print(statsDistance.mean);
    println();

    print("DistSD:");
    print(statsDistance.standardDeviation);
    println();

    print("ALSsuc:");
    print(successfulMeasurementsAmbientLight);
    print("/");
    print(numberOfMeasurements);
    println();
    print("ALSM:");
    print(statsAmbientLight.mean);
    println();

    print("ALSSD:");
    print(statsAmbientLight.standardDeviation);
    println();

	struct Measurement measurement = { statsDistance.mean, statsDistance.standardDeviation, statsAmbientLight.mean, statsAmbientLight.standardDeviation, successfulMeasurementsDistance, successfulMeasurementsAmbientLight };

  return measurement;
}


void scanI2C() {
  println ("I2C scanner. Scanning ...");
  byte count = 0;


  for (byte i = 1; i < 120; i++)
  {

    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      print ("Found address: ");
      print (i);
      print (" (0x");
      print (i);
      println (")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  println ("Done.");
  print ("Found ");
  print (count);
  println (" device(s).");
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
    println(F("Strtng"));

    delay(500);
    Wire.begin();

    // Wake up one sensor afther the other to change its I2C address
    // and configure it
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      print("Configure ToF-Sensor #");
      print(i);
      print(": ");
      pinMode(tofSensors[i].resetPin, INPUT);
      delay(150);
      tofSensors[i].sensor.init();
      delay(100);
      tofSensors[i].sensor.setAddress(tofSensors[i].i2cAddress);
      print("I2C address set to ");
      print(tofSensors[i].sensor.getAddress());

      tofSensors[i].sensor.init();
      tofSensors[i].sensor.configureDefault();

      // Reduce range max convergence time and ALS integration
      // time to 30 ms and 50 ms, respectively, to allow 10 Hz
      // operation (as suggested by table "Interleaved mode
      // limits (10 Hz operation)" in the datasheet).
      tofSensors[i].sensor.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
      tofSensors[i].sensor.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
      tofSensors[i].sensor.setTimeout(500);

      // stop continuous mode if already active
      tofSensors[i].sensor.stopContinuous();
      // in case stopContinuous() triggered a single-shot
      // measurement, wait for it to complete
      delay(300);

      println(", configuration completed");
    }

    // IC2 Multiplex
    I2CMux.begin(Wire);             // TwoWire isntance can be passed here as 3rd argument
    I2CMux.closeAll();

    // BME280
    I2CMux.openChannel(0);
    // Setup BME280, use address 0x77 (default) or 0x76
    if (!bme.begin(0x76)) {
      println(F("noBME"));
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
    measurements[i] = measureDistanceAndAmbientLight(i);
    // short delay
    delay(300);
  }
  /*

  print("Ambient: ");
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    print(tofSensors[i].sensor.readAmbientContinuous());
    if (tofSensors[i].sensor.timeoutOccurred()) { print(" TIMEOUT"); }

    if(i < numberOfSensors-1) { print(" / "); }
  }


  print("\tRange: ");
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    print(tofSensors[i].sensor.readRangeContinuousMillimeters());
    if (tofSensors[i].sensor.timeoutOccurred()) { print(" TIMEOUT"); }

    if(i < numberOfSensors-1) { print(" / "); }
  }
  */

  print("\tTemp: ");
  I2CMux.openChannel(0);
  bme.takeForcedMeasurement();
  print(bme.readTemperature());
  print(" / ");
  I2CMux.closeChannel(0);
  I2CMux.openChannel(1);
  bme.takeForcedMeasurement();
  print(bme.readTemperature());
  I2CMux.closeChannel(1);

  print("\tHum: ");
  I2CMux.openChannel(0);
  print(bme.readHumidity());
  print(" / ");
  I2CMux.closeChannel(0);
  I2CMux.openChannel(1);
  print(bme.readHumidity());
  I2CMux.closeChannel(1);

  println();
  delay(1000);
}