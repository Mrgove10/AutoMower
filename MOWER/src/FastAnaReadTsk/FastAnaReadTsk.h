#include <Arduino.h>
#ifndef fastanareadtask_h
#define fastanareadtask_h

/**
 * Read raw analog values form I2S diver buffers
 * @param Samples number of samples to read from I2S buffer
 */
void I2SAnalogRead(int Samples);

/**
 * Initialise I2S driver for high frequency analog data reading from ADC
 * 
 */
void initI2S(void);

/**
 * High frequency analog read task main loop
 * @param dummyParameter is unused but required
 */
void FastAnaReadLoopTask(void *dummyParameter);

/**
 * High frequency analog read task creation function
 */
void FastAnaReadLoopTaskCreate(void);

/**
 * High frequency analog read task suspension function
 */
void FastAnaReadLoopTaskSuspend(void);

/**
 * High frequency analog read task resume from suspension function
 */
void FastAnaReadLoopTaskResume(void);

/**
 * AnalogRead function that is compatible with I2S ADC use
 * @param pin to read from
 * @return pin analog value
 */
int ProtectedAnalogRead(int pin);

#endif