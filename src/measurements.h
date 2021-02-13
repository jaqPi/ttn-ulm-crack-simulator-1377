#include <Arduino.h>
#include <sensors.h>
#include <Statistics.h>

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

struct Measurement  measureDistanceAndAmbientLightInterleaved(const TofSensor *tofSensor, uint8_t numberOfMeasurements);
#endif