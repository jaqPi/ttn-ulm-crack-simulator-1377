#ifndef UNIT_TEST
    #include <Arduino.h>
#else
    #include <stdint.h>
    #include <math.h>
#endif

#ifndef STATISTICS_H
#define STATISTICS_H


typedef struct Stats
{
    double mean, standardDeviation;
    float median;
} stats_t;

typedef struct Measurement
{
    stats_t distance, light;
    uint8_t successfulMeasurementsDistance, successfulMeasurementsAmbientLight;
} measurement_t;

double calcMean(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
double calcSD(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements, double mean);
float calcMedian(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
stats_t calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);

void quickSort(uint16_t arrayToSort[], int lowerIndex, int higherIndex);

#endif