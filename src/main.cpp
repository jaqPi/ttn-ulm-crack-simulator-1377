#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <VL6180X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "measurements.h"
#include "sensors.h"
#include <Statistics.h>
#include "arduino_lmic.h"
#include <hal/hal.h>
#include <Queue.h>
#include "TCA9548A.h"

#include <Credentials.h> // TODO

VL6180X pololusensor1;
#define PIN_RESET_POLOLU_1 5
VL6180X pololusensor2;
#define PIN_RESET_POLOLU_2 12
VL6180X adafruitsensor1;
#define PIN_RESET_ADAFRUIT_1 10
VL6180X adafruitsensor2;
#define PIN_RESET_ADAFRUIT_2 11

TCA9548A I2CMux;                  // Address can be passed into the constructor

const uint8_t numberOfMeasurements = 50; // max 255!

enum MESSAGE_TYPE {
  TEMPS,
  SENSOR1,
  SENSOR2,
  SENSOR3,
  SENSOR4,
  ERROR
};

enum ERROR_CODES {
  NO_RESPONSE_I2C_ADDRESS,
  DEFAULT_I2C,
  NO_RESPONSE_SENSOR,
  SENSOR_ERROR
};

typedef struct LoRaPacket {
    byte payload[20];
    uint8_t length;
} loRaPacket_t;

Queue<loRaPacket_t> queue = Queue<loRaPacket_t>(5);

Adafruit_BME280 bme1;
Adafruit_BME280 bme2;

// Initially the same config for all sensors
const SensorConfig configInterleaved = { INTERLEAVED, MANUAL, 30, 50, 500};
const SensorConfig configOneByOne = { ONE_BY_ONE, MANUAL, 30, 50, 500};

const TofSensor tofSensors[4] = {
  TofSensor { pololusensor1,   configInterleaved, PIN_RESET_POLOLU_1,   0x29 },
  TofSensor { pololusensor2,   configInterleaved, PIN_RESET_POLOLU_2,   0x29 },
  TofSensor { adafruitsensor1, configInterleaved, PIN_RESET_ADAFRUIT_1, 0x29 },
  TofSensor { adafruitsensor2, configInterleaved, PIN_RESET_ADAFRUIT_2, 0x29 }
};

const uint8_t numberOfSensors = sizeof(tofSensors) / sizeof(TofSensor);


// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

const unsigned TX_INTERVAL = 12*60; // in seconds


// Pin mapping
// Pin mapping for Adafruit Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};



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


void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending"));
    } else {

        // temp             -> 2 byte
        // pressure         -> 2 byte
        // humidity         -> 2 byte
        // ============================
         //              SUM:  6 byte
        //        x2 Sensors: 12 byte

        // distance         -> 2 byte
        // distanceSD       -> 2 byte
        // distanceMD       -> 2 byte
        // distanceSucc     -> 1 byte
        // ambientLight     -> 2 byte
        // ambientLightSD   -> 2 byte
        // ambientLightMD   -> 2 byte
        // ambietnLightSucc -> 1 byte
        // ============================
        //              SUM:  14 byte
        //       x4 Sensors:  56 byte

        loRaPacket_t envPacket;

        envPacket.payload[0] = TEMPS;


        // first BME
      
        // Only needed in forced mode. Force update of BME values
        bme1.takeForcedMeasurement();

        // temp
        int temp = round(bme1.readTemperature() * 100);
        envPacket.payload[1] = highByte(temp);
        envPacket.payload[2] = lowByte(temp);

        // pressure
        int pressure = round(bme1.readPressure()/100);
        envPacket.payload[3] = highByte(pressure);
        envPacket.payload[4] = lowByte(pressure);

        // humidity
        int humidity = round(bme1.readHumidity() * 100);
        envPacket.payload[5] = highByte(humidity);
        envPacket.payload[6] = lowByte(humidity);


      
        // Only needed in forced mode. Force update of BME values
        bme2.takeForcedMeasurement();

        // temp
        int temp2 = round(bme2.readTemperature() * 100);
        envPacket.payload[7] = highByte(temp2);
        envPacket.payload[8] = lowByte(temp2);

        // pressure
        int pressure2 = round(bme2.readPressure()/100);
        envPacket.payload[9] = highByte(pressure2);
        envPacket.payload[10] = lowByte(pressure2);

        // humidity
        int humidity2 = round(bme2.readHumidity() * 100);
        envPacket.payload[11] = highByte(humidity2);
        envPacket.payload[12] = lowByte(humidity2);

        envPacket.length = 13;
        queue.push(envPacket);


        // ToF-Sensors
        for (uint8_t i = 0; i < numberOfSensors; i++)
        {
          I2CMux.openChannel(i); // TEMP
          
          loRaPacket_t distancePacket;
          
          // error handling PRE measurement

          // is sensor reachable?
          Wire.beginTransmission(tofSensors[i].i2cAddress);
          uint8_t i2cStatus = Wire.endTransmission();
          if(i2cStatus != 0) {
              // the sensor is not reachable under its assigned address.
              // check for sensor's factory default address:

              Wire.beginTransmission(0x29);
              if(Wire.endTransmission() == 0) {
                // sensor was somehow resettet to its default i2c address
                distancePacket.payload[0] = ERROR;
                distancePacket.payload[1] = i + 1; // sensor number
                distancePacket.payload[2] = DEFAULT_I2C;
                distancePacket.length = 3;
                queue.push(distancePacket);
                continue;
              }
              else {
                // sensor is gone :-(
                distancePacket.payload[0] = ERROR;
                distancePacket.payload[1] = i + 1; // sensor number
                distancePacket.payload[2] = NO_RESPONSE_I2C_ADDRESS;
                distancePacket.payload[3] = i2cStatus;
                distancePacket.length = 4;
                queue.push(distancePacket);
                continue;
              }
          }

          // is sensor present?
          if(tofSensors[i].sensor.readReg(VL6180X::IDENTIFICATION__MODEL_ID) != 0xB4) {
            distancePacket.payload[0] = ERROR;
            distancePacket.payload[1] = i + 1; // sensor number
            distancePacket.payload[2] = NO_RESPONSE_SENSOR;
            distancePacket.length = 3;
            queue.push(distancePacket);
            continue;
          }
          
          measurement_t currentMeasurement = measureDistanceAndAmbientLight(&tofSensors[i], numberOfMeasurements);
          
          // error handling POST measurement
          if(currentMeasurement.distance.mean == 255.0) {
            byte errorCode = tofSensors[i].sensor.readReg(VL6180X::RESULT__RANGE_STATUS);
            distancePacket.payload[0] = ERROR;
            distancePacket.payload[1] = i + 1; // sensor number
            distancePacket.payload[2] = SENSOR_ERROR;
            // Sensor Error Code Overview:
            // * https://www.st.com/resource/en/datasheet/vl6180x.pdf
            // 
            // Error Code Description
            // * https://www.st.com/resource/en/design_tip/dm00114111-vl6180x-range-status-error-codes-explanation-stmicroelectronics.pdf
            // * https://www.st.com/content/ccc/resource/sales_and_marketing/presentation/product_presentation/cc/96/42/b5/56/60/4d/e0/VL6180X_API_IntegrationGuide.pdf/files/VL6180X_API_IntegrationGuide.pdf/jcr:content/translations/en.VL6180X_API_IntegrationGuide.pdf
            distancePacket.payload[3] = errorCode;
            distancePacket.length = 4;
            queue.push(distancePacket);
            continue;
          }
          

          // number of Sensor
          distancePacket.payload[0] = i + 1;
          
          // encode config
          distancePacket.payload[1] = 0;
          if(tofSensors[i].config.calibrationMode == AUTO) {
            distancePacket.payload[1] |= 1 << 0;
          }

          if(tofSensors[i].config.measureMode == ONE_BY_ONE) {
            distancePacket.payload[1] |= 1 << 1;
          }

          
          // distance
          int meanDistance = round(currentMeasurement.distance.mean * 100);
          distancePacket.payload[2] = highByte(meanDistance);
          distancePacket.payload[3] = lowByte(meanDistance);

          int standardDeviationDistance = round(currentMeasurement.distance.standardDeviation * 100);
          distancePacket.payload[4] = highByte(standardDeviationDistance);
          distancePacket.payload[5] = lowByte(standardDeviationDistance);

          int medianDistance = round(currentMeasurement.distance.median * 100);
          distancePacket.payload[6] = highByte(medianDistance);
          distancePacket.payload[7] = lowByte(medianDistance);

          distancePacket.payload[8] = currentMeasurement.successfulMeasurementsDistance;

          // ambientLight
          int meanAmbientLight = round(currentMeasurement.light.mean * 100);
          distancePacket.payload[9] = highByte(meanAmbientLight);
          distancePacket.payload[10] = lowByte(meanAmbientLight);

          int standardDeviationAmbientLight = round(currentMeasurement.light.standardDeviation * 100);
          distancePacket.payload[11] = highByte(standardDeviationAmbientLight);
          distancePacket.payload[12] = lowByte(standardDeviationAmbientLight);

          int medianLight = round(currentMeasurement.light.median * 100);
          distancePacket.payload[13] = highByte(medianLight);
          distancePacket.payload[14] = lowByte(medianLight);

          distancePacket.payload[15] = currentMeasurement.successfulMeasurementsAmbientLight;

          distancePacket.length = 16;
          queue.push(distancePacket);
          I2CMux.closeChannel(i); // TEMP
          // short delay
          delay(100);
        }


        loRaPacket_t packetToSend = queue.pop();
        LMIC_setTxData2(1, (uint8_t*)packetToSend.payload, packetToSend.length, 0);
        Serial.println(F("Pckt qd"));
    }
}

void onEvent (ev_t ev) {
    // print(os_getTime());
    // print(": ");
    Serial.print(os_getTime());
    Serial.println(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println("EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            Serial.println("EV_REJOIN_FAILED");
            break;
        case EV_TXCOMPLETE:
            Serial.println("EV_TXCOMPLETE");
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println("Rcvd ack");
            if (LMIC.dataLen) {
              Serial.println("Received ");
              Serial.println(LMIC.dataLen);
              Serial.println(" bytes of payload");
            }

            if(queue.count() > 0) {
              loRaPacket_t packetToSend = queue.pop();
              LMIC_setTxData2(1, (uint8_t*)packetToSend.payload, packetToSend.length, 0);
              Serial.println(F("Pckt qd"));
            }
            else {
              // Schedule next transmission to be immediately after this
              os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
            }

            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}


void setup() {
    // Send shutdown signal to VL6180X sensors to be able to changer their I2C address
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      //pinMode(tofSensors[i].resetPin, OUTPUT); TEMP
      //digitalWrite(tofSensors[i].resetPin, LOW);
    }

    #ifdef DEBUG    
      Serial.begin(9600);
      while (!Serial);
    #endif
    Serial.println(F("Strtng"));

    delay(500);
    Wire.begin();

    // For test purposes we use a I2C multiplexer instead of changing the i2c address to each sensor
    I2CMux.begin(Wire);
    I2CMux.closeAll(); 

    // Wake up one sensor afther the other to change its I2C address
    // and configure it
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      I2CMux.openChannel(i); // TEMP
      Serial.print("Configure ToF-Sensor #");
      Serial.print(i);
      Serial.print(": ");
      //pinMode(tofSensors[i].resetPin, INPUT);
      //delay(150);
      tofSensors[i].sensor.init();
      delay(100);
      //tofSensors[i].sensor.setAddress(tofSensors[i].i2cAddress);
      //Serial.print("I2C address set to ");
      //Serial.print(tofSensors[i].sensor.getAddress());

      //tofSensors[i].sensor.init();
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
      I2CMux.closeChannel(i); // TEMP
    }

    scanI2C();

    // Setup BME280, use address 0x77 (default) or 0x76
    if (!bme1.begin(0x76)) {
      Serial.println(F("noBME1"));
      while (1);
    }

    // Setup BME280, use address 0x77 (default) or 0x76
    if (!bme2.begin(0x77)) {
      Serial.println(F("noBME2"));
      while (1);
    }

    
    // BME280 1
    bme1.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );


    
    bme2.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

#ifdef PROGMEM
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

#if defined(CFG_eu868)
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
#elif defined(CFG_us915)
    LMIC_selectSubBand(1);
#endif

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop()
{
  os_runloop_once();
}