#ifndef UNIT_TEST
    #include <Arduino.h>
#else
    #include <stdint.h>
    #include <math.h>
#endif

#ifndef STATISTICS_H
#define STATISTICS_H


struct Stats
{
    double mean, standardDeviation;
    float median;
};

struct Measurement
{
    double meanDistance, standardDeviationDistance, meanAmbientLight, standardDeviationAmbientLight;
    uint8_t successfulMeasurementsDistance, successfulMeasurementsAmbientLight;
};

double calcMean(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
double calcSD(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements, double mean);
float calcMedian(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
struct Stats calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);

void quickSort(uint16_t arrayToSort[], int lowerIndex, int higherIndex);

#endif