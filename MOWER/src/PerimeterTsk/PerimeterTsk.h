#include <Arduino.h>
#ifndef perimetertask_h
#define perimetertask_h

/**
 * Perimeter processing task queue creation
 *
 */
void PerimeterQueueInit(void);

/**
 * Perimeter processing task trigger timer creation
 * 
 */
void PerimeterTimerInit(void);

/**
 * Perimeter processing task trigger timer code
 * 
 */
ICACHE_RAM_ATTR void PerimeterTimerISR(void);

/**
 * Perimeter data processing task Setup function
 * 
 */
void PerimeterProcessingSetup(void);

/**
 * Perimeter data raw calibration function to determine calibration offset to "zero value"
 * @param Samples is the number of samples to use to estblish calibration offset (g_PerimeterOffset)
 * @returns the ofset in the g_PerimeterOffset global variable
 */
void PerimeterRawValuesCalibration(int Samples);

/**
 * Perimeter data raw value conversion to [-127,+127] range
 * 
 * @param rawVal is the unsigned int value to convert
 * @param offset is the calibration offset
 * @return 8 bit integer converted value
 * */
int8_t PerimeterRawValuesConvert(uint16_t rawVal, uint16_t offset);

/**
 * Perimeter data raw value copy and pre-processing function (min/Max/Average)
 * 
 * @param Samples is the number of samples to copy and process
 */
void GetPerimeterRawValues(int Samples);

/**
 * Cross correlation digital matched filter
 *
 * @param H[] holds the double sided filter coeffs,
 * @param subsample is the number of times for each filter coeff to repeat 
 * @param M = H.length (number of points in FIR)
 * @param ip[] holds input data (length > nPts + M )
 * @param nPts is the length of the required output data
 * @return quality as the quality is ?????
 * @return the magnitude of the filter ouput 
 *
 * @note Originally based on Ardumower (www.ardumower.de) Perimeter Library
 * http://en.wikipedia.org/wiki/Cross-correlation
 */

int16_t corrFilter(int8_t *H, int8_t subsample, int16_t M, int8_t *ip, int16_t nPts, float &quality);

/**
 * Matched filter comparison and output 
 * @param Samples is the number of samples to be analysed through the matched filter
 */
void MatchedFilter(int16_t Samples);

/**
 * Perimeter processing task main loop
 * @param dummyParameter is unused but required
 */
void PerimeterProcessingLoopTask(void *dummyParameter);

/**
 * Perimeter processing task creation function
 */
void PerimeterProcessingLoopTaskCreate(void);

/**
 * Perimeter processing task suspension function
 */
void PerimeterProcessingLoopTaskSuspend(void);

/**
 * Perimeter processing task resume from suspension function
 */
void PerimeterProcessingLoopTaskResume(void);

#endif