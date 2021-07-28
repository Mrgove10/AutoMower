#include <Arduino.h>
#ifndef anareadtask_h
#define anareadtask_h

int I2SAnalogRead(void);

void initI2S(void);
//void dacTask(void *params);

/**
 * Fast timer ISR function
 * 
 */
ICACHE_RAM_ATTR void FastAnaReadTimerISR(void);

/**
 * Slow Timer ISR function
 * 
 */
ICACHE_RAM_ATTR void SlowAnaReadTimerISR(void);

/**
 * Analog Read Setup function
 * 
 */
void AnaReadSetup(void);

/**
 * Analog Read permanent loop task
 * 
 */
void AnaReadLoopTask(void);

#endif