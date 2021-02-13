#include <Statistics.h>
#include <unity.h>
#include <stdio.h>


void test_calcMean_WithNoFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 10;
    uint8_t test_numberOfMeasurements = test_succesfulMeasurements;
    uint16_t series[10] = {5, 7, 8, 7, 10, 10, 7, 6, 48, 2};
    double preCalculatedMean = 11.0;

    double mean = calcMean(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMean, mean);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMean, mean);
}

void test_calcMean_WithFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 8;
    uint8_t test_numberOfMeasurements = 10;
    // Note: failed measurements are indicated by a value of 0,
    // therefore, calcMean divides by the number of succesfulMeasurements
    // not (total) number of measurements
    uint16_t series[10] = {5, 7, 8, 7, 10, 10, 7, 6, 0, 0};
    double preCalculatedMean = 7.5;

    double mean = calcMean(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMean, mean);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMean, mean);
}

void test_calcMean_WithOnlyFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 0;
    uint8_t test_numberOfMeasurements = 10;
    uint16_t series[10] = {0};
    double preCalculatedMean = 0;

    double mean = calcMean(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMean, mean);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMean, mean);
}

void test_calcSD_withNoFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 10;
    uint8_t test_numberOfMeasurements = test_succesfulMeasurements;
    uint16_t series[10] = {5, 7, 8, 7, 10, 10, 7, 6, 48, 2};
    double mean = 11.0 ;
    double preCalculatedMeanSD = 13.20774;
    
    double sd = calcSD(test_succesfulMeasurements, series, test_numberOfMeasurements, mean);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMeanSD, sd);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMeanSD, sd);
}

void test_calcSD_withFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 8;
    uint8_t test_numberOfMeasurements = 10;
    uint16_t series[10] = {5, 7, 8, 7, 10, 10, 7, 6, 0, 0};
    double mean = 7.5 ;
    double preCalculatedMeanSD = 1.772811;
    
    double sd = calcSD(test_succesfulMeasurements, series, test_numberOfMeasurements, mean);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMeanSD, sd);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMeanSD, sd);
}

void test_calcSD_withOnlyFailedMeasurements(void) {
    uint8_t test_succesfulMeasurements = 0;
    uint8_t test_numberOfMeasurements = 10;
    uint16_t series[10] = {0};
    double mean = 0.0 ;
    double preCalculatedMeanSD = 0.0;
    
    double sd = calcSD(test_succesfulMeasurements, series, test_numberOfMeasurements, mean);
    
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMeanSD, sd);
    printf("Expected: %f \t Actual: %f\n", preCalculatedMeanSD, sd);
}

void test_calcMedian_withNoFailedMeasurements_evenN(void) {
    uint8_t test_succesfulMeasurements = 12;
    uint8_t test_numberOfMeasurements = 12;
    uint16_t series[12] = {1, 7, 13, 5, 10, 9, 11, 6, 48, 2, 20, 23};
    uint16_t series_before[12] = {1, 7, 13, 5, 10, 9, 11, 6, 48, 2, 20, 23};

    float preCalculatedMedian = 9.5;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_calcMedian_withNoFailedMeasurements_oddN(void) {
    uint8_t test_succesfulMeasurements = 11;
    uint8_t test_numberOfMeasurements = 11;
    uint16_t series[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 20, 23};
    uint16_t series_before[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 20, 23};

    float preCalculatedMedian = 9.0;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_calcMedian_withFailedMeasurements_evenN(void) {
    uint8_t test_succesfulMeasurements = 10;
    uint8_t test_numberOfMeasurements = 12;
    uint16_t series[12] = {1, 7, 13, 5, 10, 9, 11, 6, 48, 2, 0, 0};
    uint16_t series_before[12] = {1, 7, 13, 5, 10, 9, 11, 6, 48, 2, 0, 0};

    float preCalculatedMedian = 8.0;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_calcMedian_withFailedMeasurements_oddN(void) {
    uint8_t test_succesfulMeasurements = 9;
    uint8_t test_numberOfMeasurements = 11;
    uint16_t series[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 0, 0};
    uint16_t series_before[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 0, 0};

    float preCalculatedMedian = 7.0;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_calcMedian_withOnlyFailedMeasurements_evenN(void) {
    uint8_t test_succesfulMeasurements = 0;
    uint8_t test_numberOfMeasurements = 12;
    uint16_t series[12] = {0};
    uint16_t series_before[12] = {0};

    float preCalculatedMedian = 0.0;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_calcMedian_withOnlyFailedMeasurements_oddN(void) {
    uint8_t test_succesfulMeasurements = 0;
    uint8_t test_numberOfMeasurements = 11;
    uint16_t series[11] = {0};
    uint16_t series_before[11] = {0};

    float preCalculatedMedian = 0.0;
    
    float median = calcMedian(test_succesfulMeasurements, series, test_numberOfMeasurements);
    
    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, median);
    // Test if array remains unmodified
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_before, series, test_numberOfMeasurements);

    printf("Expected: %f \t Actual: %f\n", preCalculatedMedian, median);
}

void test_quickSort(void) {
    uint16_t series[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 20, 23};
    uint16_t series_sorted[11] = {1,  2,  5,  6,  7,  9, 10, 11, 20, 23, 48};
    uint16_t series_before[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 20, 23};

    quickSort(series, 0, 11-1);
    
    TEST_ASSERT_EQUAL_UINT16_ARRAY(series_sorted, series, 11);
}

void test_calcStats(void) {
    uint8_t test_succesfulMeasurements = 9;
    uint8_t test_numberOfMeasurements = 11;
    uint16_t series[11] = {1, 7, 5, 10, 9, 11, 6, 48, 2, 0, 0};

    float preCalculatedMedian = 7.0;
    double preCalculatedSd = 14.28286;
    double preCalculatedMean = 11.0;

    struct Stats actual = calcStats(test_succesfulMeasurements, series, test_numberOfMeasurements);

    TEST_ASSERT_EQUAL_FLOAT(preCalculatedMedian, actual.median);
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedSd, actual.standardDeviation);
    TEST_ASSERT_EQUAL_DOUBLE(preCalculatedMean, actual.mean);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_calcMean_WithNoFailedMeasurements);
    RUN_TEST(test_calcMean_WithFailedMeasurements);
    RUN_TEST(test_calcMean_WithOnlyFailedMeasurements);

    RUN_TEST(test_calcSD_withNoFailedMeasurements);
    RUN_TEST(test_calcSD_withFailedMeasurements);
    RUN_TEST(test_calcSD_withOnlyFailedMeasurements);

    RUN_TEST(test_calcMedian_withNoFailedMeasurements_evenN);
    RUN_TEST(test_calcMedian_withNoFailedMeasurements_oddN);
    RUN_TEST(test_calcMedian_withFailedMeasurements_evenN);
    RUN_TEST(test_calcMedian_withFailedMeasurements_oddN);
    RUN_TEST(test_calcMedian_withOnlyFailedMeasurements_evenN);
    RUN_TEST(test_calcMedian_withOnlyFailedMeasurements_oddN);

    RUN_TEST(test_calcStats);

    RUN_TEST(test_quickSort);

    UNITY_END();

    return 0;
}