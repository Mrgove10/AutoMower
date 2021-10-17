#include <Arduino.h>
#ifndef perimetersendtask_h
#define perimetersendtask_h

/**
 * Perimeter signal code send function
 */
void PerimeterCodeSend(void);

/**
 * Perimeter signal sender timer ISR
 */
ICACHE_RAM_ATTR void PerimeterSendTimerISR(void);

/**
 * Perimeter signal sender pins initialisation
 */
void InitPerimeterSendPins(void);

/**
 * Perimeter signal sender queue initialisation
 */
void InitPerimeterSendQueue(void);

/**
 * Perimeter signal sender timer initialisation
 */
void InitPerimeterSendFastTimer(void);

/**
 * Perimeter signal sender task main loop
 * @param dummyParameter is unused but required
 */
void PerimeterSendLoopTask(void *dummyParameter);

/**
 * Perimeter signal sender task creation function
 */
void PerimeterSendLoopTaskCreate(void);

/**
 * Perimeter signal sender task suspension function
 */
void PerimeterSendLoopTaskSuspend(void);

/**
 * Perimeter signal sender task resume from suspension function
 */
void PerimeterSendLoopTaskResume(void);

#endif