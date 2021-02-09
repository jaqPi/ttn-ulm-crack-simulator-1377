#include <Arduino.h>
#include <VL6180X.h>

#ifndef SENSORS_H
#define SENSORS_H

enum MeasureMode {
    INTERLEAVED,
    ONE_BY_ONE
};

enum CalibrationMode {
    MANUAL,
    AUTO
};

struct SensorConfig
{
    MeasureMode measureMode;
    CalibrationMode calibrationMode;
    uint16_t rangeMaxConvergenceTime, alsMaxIntegrationPeriod, timeout;
};


struct TofSensor
{
    VL6180X &sensor;
    SensorConfig config;
    uint8_t resetPin, i2cAddress;
};

#endif