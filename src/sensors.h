#include <Arduino.h>
#include <VL6180X.h>

#ifndef SENSORS_H
#define SENSORS_H

enum measureMode {
    INTERLEAVED,
    SUCCESSIVELY
};

struct SensorConfig
{
    measureMode mode;
    uint16_t SYSRANGE__MAX_CONVERGENCE_TIME, SYSALS__INTEGRATION_PERIOD,timeout;
};


struct TofSensor
{
    VL6180X &sensor;
    SensorConfig config;
    uint8_t resetPin, i2cAddress;
};

#endif