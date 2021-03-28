# ttn-ulm-crack-simulator-1377

## Payload Format

Per step the uC sends five packets: a packet containing environmental data and four packets containing distance data.

## Environment Packet

First packet sent by uC.

| Name | Unit | Description |
| --- | --- | --- |
| temp1 | °C | Temperature measured by BME280 #1 |
| pressure1 | hPa | Atmospheric pressure measured by BMEW280 #1 |
| humidity1 | % | Relative humidity measured by BME280 #1 |
| temp2 | °C | Temperature measured by BME280 #2 |
| pressure2 | hPa | Atmospheric pressure measured by BMEW280 #2 |
| humidity2 | % | Relative humidity measured by BME280 #2 |

## Distance and Ambient Light (ALS) Packet

There are four VL6180X ToF distance sensors connected to the uC. Two manufactured by Pololu (#1+2) and two by Adafruit (#3+4).

Due to packet size restrictions of the LMiC library (max 53 Byte / Packet), the measured data is sent separately per sensor. In general, each value is calculated on the basis of a series of 50 single measurements.

| Name | Unit | Description |
| --- | --- | --- |
| meanDistanceX | mm | Mean of 50 x distance measurements |
| standardDeviationDistanceX | mm | Standard deviation of 50 x distance measurements | 
| medianDistanceX | mm | Median of 50 x distance measurements |
| successfulMeasurementsDistance | - | If a single measurement times out, it is excluded from the values above. Remainder: a series consists of 50 measurements. Thus, the max value is 50. |
| meanAmbientLightX | ? | Mean of 50 x ALS measurements |
| standardDeviationAmbientLightX | ? | Standard deviation of 50 x ALS measurements | 
| medianAmbientLightX | ? | Median of 50 x ALS measurements |
| successfulMeasurementsAmbientLightX | - | If a single measurement times out, it is excluded from the values above. Remainder: a series consists of 50 measurements. Thus, the max value is 50. |
