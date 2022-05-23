#include <Arduino.h>
#include "Environment_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "PerimeterSendTsk/PerimeterSendTsk.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "MQTT/MQTT.h"
#include "Utils/Utils.h"

/**
 * Perimeter signal code send function
 */
void PerimeterCodeSend(void)
{
//   unsigned long startSend = micros();

  if (g_enableSender)
  {
    int Level = int(map(int(g_PerimeterPowerLevel), 0, 100, 0, PERIMETER_SEND_POINTS)); // convert speed (in %) into PWM range

    ledcWrite(PERIMETER_SEND_PWM_CHANNEL, Level);
    // digitalWrite(PIN_ESP_SENDER_PERIMETER_PWM, HIGH);

    if (g_sigcode[g_SentStep] == 1)
    {
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN1, LOW);
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN2, HIGH);
    }
    else if (g_sigcode[g_SentStep] == -1)
    {
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN1, HIGH);
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN2, LOW);
    }
    else
    {
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN1, LOW);
      digitalWrite(PIN_ESP_SENDER_PERIMETER_IN2, LOW);
    }
    g_SentStep = g_SentStep + 1;
    if (g_SentStep == sizeof(g_sigcode))
    {
      g_SentStep = 0;
    }
  }
  else
  {
    digitalWrite(PIN_ESP_SENDER_PERIMETER_IN1, LOW);
    digitalWrite(PIN_ESP_SENDER_PERIMETER_IN2, LOW);
    ledcWrite(PERIMETER_SEND_PWM_CHANNEL, 0);
    // digitalWrite(PIN_ESP_SENDER_PERIMETER_PWM, LOW);
  }

//   g_writeTime = g_writeTime + micros()- startSend;
}

/**
 * Perimeter signal sender timer ISR
 */
ICACHE_RAM_ATTR void PerimeterSendTimerISR(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // We have not woken a task at the start of the ISR. 
  BaseType_t QueueReturn;
  byte Message = 1;

  portENTER_CRITICAL_ISR(&g_PerimeterSendTimerMux);
  g_PerimeterSendTimerCount = g_PerimeterSendTimerCount + 1;

  QueueReturn = xQueueSendToBackFromISR(g_PerimeterSendQueue, &Message, &xHigherPriorityTaskWoken);
  if (QueueReturn != pdPASS) {
    g_PerimeterSendQfull = g_PerimeterSendQfull + 1;
  }
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR ();
  }
  portEXIT_CRITICAL_ISR(&g_PerimeterSendTimerMux);
}

/**
 * Perimeter signal sender pins initialisation
 */
void InitPerimeterSendPins(void)
{
  pinMode(PIN_ESP_SENDER_PERIMETER_IN1, OUTPUT);
  pinMode(PIN_ESP_SENDER_PERIMETER_IN2, OUTPUT);

  // configure LED PWM functionalities
  ledcSetup(PERIMETER_SEND_PWM_CHANNEL, PERIMETER_SEND_PWM_FREQUENCY, PERIMETER_SEND_PWM_RESOLUTION);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PIN_ESP_SENDER_PERIMETER_PWM, PERIMETER_SEND_PWM_CHANNEL);

  // pinMode(PIN_ESP_SENDER_PERIMETER_PWM, OUTPUT);
}


/**
 * Perimeter signal sender queue initialisation
 */
void InitPerimeterSendQueue(void)
{
  // Fast timer queue

  /* Create a queue capable of containing bytes (used as booleans) */
  g_PerimeterSendQueue = xQueueCreate(PERIMETER_SEND_QUEUE_LEN, sizeof(byte) );
  if( g_PerimeterSendQueue == NULL )
  {
    DebugPrintln("Perimeter signal send Task Queue creation problem !!", DBG_ERROR, true);
  }
  else
  {
    DebugPrintln("Perimeter signal send Task Queue init for "+ String(PERIMETER_SEND_QUEUE_LEN), DBG_VERBOSE, true);
  }
}

/**
 * Perimeter signal sender timer initialisation
 */
void InitPerimeterSendFastTimer(void)
{
  // Fast timer setup
  g_PerimeterSendTimerSemaphore = xSemaphoreCreateMutex();

  g_PerimeterSendTimerhandle = timerBegin(PERIMETER_SEND_TIMER_NUMBER, PERIMETER_SEND_TIMER_PRESCALER, true);

  timerAttachInterrupt(g_PerimeterSendTimerhandle, &PerimeterSendTimerISR, true);
  timerAlarmWrite(g_PerimeterSendTimerhandle, PERIMETER_SEND_TIMER_INTERVAL, true);
  timerAlarmEnable(g_PerimeterSendTimerhandle);

  DebugPrintln("Perimeter Send timer init at " + String(PERIMETER_SEND_TIMER_INTERVAL), DBG_VERBOSE, true);
}

/**
 * Perimeter signal sender task main loop
 * @param dummyParameter is unused but required
 */
void PerimeterSendLoopTask(void *dummyParameter)
{
//   static unsigned long StartMicros = micros();
  static bool SetupDone = false;
//   static unsigned long Delay = 0;
//   long duration = 0;

  for (;;)
  {

//-----------------------------------
// Task Setup (done only on 1st call)
//-----------------------------------

    if (!SetupDone)
    {
      delay(100);
      DebugPrintln("Perimeter signal send Task Started on core " + String(xPortGetCoreID()), DBG_VERBOSE, true);

      InitPerimeterSendQueue();
      InitPerimeterSendFastTimer();

      SetupDone = true;
      xQueueReset(g_PerimeterSendQueue);
    }

//-----------------------------------
// Task Loop (done on each timer semaphore release)
//-----------------------------------

    bool evt;
    while (xQueueReceive(g_PerimeterSendQueue, &evt, portMAX_DELAY) == pdPASS)
    {
    //   int inQueue = uxQueueMessagesWaiting(g_PerimeterSendQueue);
      if (evt == 1)
      {
        // StartMicros = micros();

        portENTER_CRITICAL_SAFE(&g_PerimeterSendLoopMux);     
        PerimeterCodeSend();
        portEXIT_CRITICAL_SAFE(&g_PerimeterSendLoopMux);

        // xSemaphoreTake(g_GlobalVariablesAccessSemaphore, portMAX_DELAY);
        // g_inQueue = g_inQueue + inQueue;
        // g_inQueueMax = max(inQueue, (int) g_inQueueMax);
        // g_FastLoopCount = g_FastLoopCount + 1;
        // g_SenderLoopDuration = g_SenderLoopDuration + duration;
        // g_SenderLoopDurationMax = max((long) g_SenderLoopDurationMax, duration);
        // g_SenderLoopDurationMin = min((long) g_SenderLoopDurationMin, duration);
        // xSemaphoreGive(g_GlobalVariablesAccessSemaphore);
      }
    }
  }
}

/**
 * Perimeter signal sender task creation function
 */
void PerimeterSendLoopTaskCreate(void)
{
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      PerimeterSendLoopTask, /* Task function. */
      PERIMETER_SEND_TASK_NAME,         /* String with name of task. */
      PERIMETER_SEND_TASK_STACK_SIZE,   /* Stack size in bytes. */
      NULL,                        /* Parameter passed as input of the task */
      PERIMETER_SEND_TASK_PRIORITY,     /* Priority of the task. */
      &g_PerimeterSendTaskHandle,  /* Task handle. */
      PERIMETER_SEND_TASK_ESP_CORE);

  if (xReturned == pdPASS)
  {
    DebugPrintln("Perimeter signal send Task created on Core " + String(PERIMETER_SEND_TASK_ESP_CORE), DBG_VERBOSE, true);
  }
  else
  {
    DebugPrintln("Perimeter signal send Task creation failed (" + String(xReturned) + ")", DBG_ERROR, true);
    //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
    //errQUEUE_BLOCKED						( -4 )
    //errQUEUE_YIELD							( -5 )
  }
}

/**
 * Perimeter signal sender task suspension function
 */
void PerimeterSendLoopTaskSuspend(void)
{
  vTaskSuspend(g_PerimeterSendTaskHandle);
  DebugPrintln("Perimeter signal send Task suspended", DBG_VERBOSE, true);
}

/**
 * Perimeter signal sender task resume from suspension function
 */
void PerimeterSendLoopTaskResume(void)
{
  vTaskResume(g_PerimeterSendTaskHandle);
  DebugPrintln("Perimeter signal send Task resumed", DBG_VERBOSE, true);
}

/**
 * Perimeter signal stop
 */
void PerimeterSignalStop(void)
{
  DebugPrintln("Perimeter signal stopped", DBG_DEBUG, true);

  g_enableSender = false;
  PerimeterSignalStatusSend(true);
  delay(200);
  PerimeterLoadCurrentRead(true, true);
}

/**
 * Perimeter signal start
 */
void PerimeterSignalStart(void)
{
  DebugPrintln("Perimeter signal start", DBG_DEBUG, true);

  g_enableSender = true;
  PerimeterSignalStatusSend(true);
  delay(200);
  PerimeterLoadCurrentRead(true, true);
}
