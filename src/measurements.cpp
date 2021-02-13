#include "measurements.h"

struct Measurement measureDistanceAndAmbientLightInterleaved(const TofSensor *tofSensor, uint8_t numberOfMeasurements) {
    uint16_t measurementSeriesDistance[numberOfMeasurements];
    uint16_t measurementSeriesAmbientLight[numberOfMeasurements];

    Serial.println("Start measurement using ToF-Sensor ");
    Serial.println(tofSensor->i2cAddress);
    Serial.print(", #Measurement: ");
    Serial.println(numberOfMeasurements);

    // calibrate sensor in terms of temperature
    // disabled
    //sensor.writeReg(sensor.SYSRANGE__VHV_RECALIBRATE, 0x01);    


    tofSensor->sensor.startInterleavedContinuous();
    uint8_t successfulMeasurementsDistance = 0;
    uint8_t successfulMeasurementsAmbientLight = 0;

    for (uint8_t i = 0; i < numberOfMeasurements; i++) {
        // Ambient Light
        uint16_t currentAmbientLight = tofSensor->sensor.readAmbientContinuous();
        if (!tofSensor->sensor.timeoutOccurred()) {
            measurementSeriesAmbientLight[successfulMeasurementsAmbientLight] = currentAmbientLight;
            successfulMeasurementsAmbientLight += 1;

            #ifdef SINGLE_VALUES
                // Print current value to Serial
                if(i == numberOfMeasurements - 1) {
                    Serial.println(currentAmbientLight);
                }
                else {
                    Serial.print(currentAmbientLight);
                    Serial.print(",");
                }
            #endif
        }

        // Distance
        uint16_t currentDistance = (uint16_t) tofSensor->sensor.readRangeContinuous();
        if (!tofSensor->sensor.timeoutOccurred()) {
            measurementSeriesDistance[successfulMeasurementsDistance] = currentDistance;
            successfulMeasurementsDistance += 1;

            #ifdef SINGLE_VALUES
                // Print current value to Serial
                if(i == numberOfMeasurements - 1) {
                    Serial.println(currentDistance);
                }
                else {
                    Serial.print(currentDistance);
                    Serial.print(",");
                }
            #endif
        }
    }
    tofSensor->sensor.stopContinuous();

    struct Stats statsAmbientLight = calcStats(successfulMeasurementsAmbientLight, measurementSeriesAmbientLight, numberOfMeasurements);
    struct Stats statsDistance = calcStats(successfulMeasurementsDistance, measurementSeriesDistance, numberOfMeasurements);



    // Serial.print("Distsucc: ");
    // Serial.print(successfulMeasurementsDistance);
    // Serial.print("/");
    // Serial.print(numberOfMeasurements);
    // Serial.println();
    Serial.print("DistM:");
    Serial.print(statsDistance.mean);
    Serial.println();

    Serial.print("DistSD:");
    Serial.print(statsDistance.standardDeviation);
    Serial.println();

    Serial.print("ALSsuc:");
    Serial.print(successfulMeasurementsAmbientLight);
    Serial.print("/");
    Serial.print(numberOfMeasurements);
    Serial.println();
    Serial.print("ALSM:");
    Serial.print(statsAmbientLight.mean);
    Serial.println();

    Serial.print("ALSSD:");
    Serial.print(statsAmbientLight.standardDeviation);
    Serial.println();

	struct Measurement measurement = { statsDistance.mean, statsDistance.standardDeviation, statsAmbientLight.mean, statsAmbientLight.standardDeviation, successfulMeasurementsDistance, successfulMeasurementsAmbientLight };

  return measurement;
}
