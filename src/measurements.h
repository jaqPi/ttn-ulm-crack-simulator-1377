#include <Arduino.h>
#include "sensors.h"

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H


struct Stats
{
    double mean, standardDeviation, median;
};

struct Measurement
{
    double meanDistance, standardDeviationDistance, meanAmbientLight, standardDeviationAmbientLight;
    uint8_t successfulMeasurementsDistance, successfulMeasurementsAmbientLight;
};

double calcMean(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
double calcSD(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements, double mean);
double calcMedian(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
struct Stats calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements);
struct Measurement  measureDistanceAndAmbientLight(const TofSensor *tofSensor, uint8_t numberOfMeasurements);
#endif