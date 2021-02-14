#include "Statistics.h"

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
            variance += pow((measurementSeries[i] - mean),2);
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


float calcMedian(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements) {
    uint16_t succesfulMeasurementSeries[successfulMeasurements];

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
        uint8_t lowerIndex = (uint8_t) round(successfulMeasurements/2)-1;
        uint8_t upperIndex = (uint8_t) round(successfulMeasurements/2)-1+1;
        return (
                succesfulMeasurementSeries[lowerIndex]
                + succesfulMeasurementSeries[upperIndex]
                )
                / 2.0;
    }
    else {
            
            uint8_t index = (uint8_t) round((successfulMeasurements)/2.0)-1;
            return succesfulMeasurementSeries[index];
    }
}

stats_t calcStats(uint8_t successfulMeasurements, uint16_t measurementSeries[], uint8_t numberOfMeasurements) {
    double mean = calcMean(successfulMeasurements, measurementSeries, numberOfMeasurements);
    double sd = calcSD(successfulMeasurements, measurementSeries, numberOfMeasurements, mean);
    double median = calcMedian(successfulMeasurements, measurementSeries, numberOfMeasurements);
    stats_t stats = {mean, sd, median};
    return stats;
}