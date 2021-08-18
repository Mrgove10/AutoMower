#ifndef anareadtsk_h
#define anareadtsk_h

/**
 * AnalogRead function that is compatible with I2S ADC use
 * @param pin to read from
 * @return pin analog value
 */
int ProtectedAnalogRead(int pin);

/**
 * Analog Read task main loop
 * @param dummyParameter is unused but required
 */
void AnaReadLoopTask(void *dummyParameter);

/**
 * Analog Read task creation function
 */
void AnaReadLoopTaskCreate(void);

/**
 * Analog Read task suspension function
 */
void AnaReadLoopTaskSuspend(void);

/**
 * Analog Read task resume from suspension function
 */
void AnaReadLoopTaskResume(void);

#endif