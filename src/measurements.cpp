#include "measurements.h"


double calcMean(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements) {
    double mean = 0.0;

    if (successfulMeasurements > 0 && successfulMeasurements <= numberOfMeasurements)
    {
        // Mean
        for(uint8_t i = 0; i < successfulMeasurements; i++) {
            mean += measurementSeries[i];
        }
        mean = mean/successfulMeasurements;
    }

    return mean;
}


double calcSD(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements, double mean) {
    double standardDeviation = 0.0;
    if (successfulMeasurements > 0 && successfulMeasurements <= numberOfMeasurements)
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

void exchangeNumbers(uint16_t arrayToSort[], int i, int j) {
 int temp = arrayToSort[i];
 arrayToSort[i] = arrayToSort[j];
 arrayToSort[j] = temp;
}

// quick sort & and exchange numbers code adopted from
// https://create.arduino.cc/projecthub/Alfodr/quicksort-algorithm-and-esp8266-web-server-6126bf
// License: GPL3+
void quickSort(uint16_t arrayToSort[], int lowerIndex, int higherIndex) {
 int i = lowerIndex;
 int j = higherIndex;
 int pivot = arrayToSort[lowerIndex + (higherIndex - lowerIndex) / 2];
 while (i <= j) {
   while (arrayToSort[i] < pivot) {
     i++;
   }
   while (arrayToSort[j] > pivot) {
     j--;
   }
   if (i <= j) {
     exchangeNumbers(arrayToSort, i, j);
     i++;
     j--;
   }
 }
 if (lowerIndex < j)
   quickSort(arrayToSort, lowerIndex, j);
 if (i < higherIndex)
   quickSort(arrayToSort, i, higherIndex);
}


double calcMedian(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements) {
    uint16_t succesfulMeasurementSeries[successfulMeasurements] = {0};

    uint8_t j = 0;
    for (uint8_t i = 0; i < numberOfMeasurements; i++)
    {
        if(measurementSeries[i] != 0 && j < successfulMeasurements) {
            succesfulMeasurementSeries[j] = measurementSeries[i];
            j +=1;
        }
    }
    quickSort(succesfulMeasurementSeries, 0, successfulMeasurements -1);
    
    if(successfulMeasurements % 2 == 0) {
        return (
                succesfulMeasurementSeries[successfulMeasurements/2]
                + succesfulMeasurementSeries[(successfulMeasurements/2)+1]
                )
                / 2;
    }
    else {
            return succesfulMeasurementSeries[round(successfulMeasurements/2)];
    }
}

struct Stats calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements) {
    double mean = calcMean(successfulMeasurements, measurementSeries, numberOfMeasurements);
    double sd = calcSD(successfulMeasurements, measurementSeries, numberOfMeasurements, mean);
    double median = calcMedian(successfulMeasurements, measurementSeries, numberOfMeasurements);
    return Stats { mean, sd, median};
}


struct Measurement measureDistanceAndAmbientLight(const TofSensor *tofSensor, uint8_t numberOfMeasurements) {
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
