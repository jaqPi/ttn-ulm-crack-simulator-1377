#include <Arduino.h>
#include <sensors.h>
#include <Statistics.h>

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

measurement_t measureDistanceAndAmbientLight(const TofSensor *tofSensor, uint8_t numberOfMeasurements);
measurement_t  measureDistanceAndAmbientLightInterleaved(const TofSensor *tofSensor, uint8_t numberOfMeasurements);
measurement_t  measureDistanceAndAmbientLightOneByOne(const TofSensor *tofSensor, uint8_t numberOfMeasurements);

#endif