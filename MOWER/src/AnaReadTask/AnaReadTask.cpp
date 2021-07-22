#include <Arduino.h>
#include "Environment_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "AnaReadTask/AnaReadTask.h"
#include "Utils/Utils.h"
//#include <sys/time.h>
//#include <time.h>


/**
 * Analog Read permanent loop task
 * 
 */
void AnaReadLoopTask(void * dummyParameter )
{
  static bool SetupDone = false;
  static unsigned long FastCalls = 0;
  static unsigned long FastDoNothing = 0;
  static unsigned long FastMissed = 0;
  static unsigned long LastFastCalltime = 0;
  static unsigned long SlowCalls = 0;
  static unsigned long LastSlowCalltime = 0;
  static unsigned long AnalogReadMillis = 0;

  uint8_t temprature_sens_read();

  for (;;)
  {
    if (ESP_ANA_READ_TASK_CORE == 0)
    {
      delayMicroseconds(TIMER_FAST_FREQUENCY*4);
    } 
    if (!SetupDone)
    {
      // insert 1st execution (setup) code here
      adcAttachPin(PIN_ESP_PERIMETER);
      analogReadResolution(10);
      analogSetClockDiv(1);

      DebugPrintln("Analog aquisition Task Started on core " + String(xPortGetCoreID()), DBG_INFO, true);
      SetupDone = true;
    }

    //---------------------------
    // Fast timer related work
    //---------------------------

    portENTER_CRITICAL_ISR(&g_FastTimerMux);
    int MemFastTimerCount = g_FastTimerCount;
    portEXIT_CRITICAL_ISR(&g_FastTimerMux);

    if (MemFastTimerCount > 0)
    {
      FastCalls = FastCalls + 1;
      unsigned long now = millis();

      if (MemFastTimerCount > 1)
      {
        FastMissed = FastMissed + MemFastTimerCount - 1;
      }

      if (millis() - LastFastCalltime > 5000)
      {
        if (FastMissed > 0)
        {
          DebugPrintln("");
          DebugPrintln("Fast analog aquisition timer count missed " + String(FastMissed) + " time(s)", DBG_WARNING, true);
//          FastCalls = FastCalls + MemFastTimerCount;
          MemFastTimerCount = 1;
          FastMissed = 0;
        }
        DebugPrintln("");
        DebugPrintln("Total Do nothing calls :" + String(FastDoNothing) + 
                     "| Analog aquisition calls :" + String(FastCalls) + 
                     "| Total iterations:" + String(FastCalls + FastDoNothing) + 
                     " => " + String((float) FastCalls/(now-LastFastCalltime),3) + 
                     "kHz | Read Time " + String((float) 1000*AnalogReadMillis/FastCalls,3) + "microseconds", DBG_VERBOSE, true);
        FastCalls = 0;
        FastDoNothing = 0;
        LastFastCalltime = millis();
        AnalogReadMillis = 0;
      }
      //  Do what ever elseneeds to be done here......
      unsigned long start = millis();

//      for (int i=0; i<1;i++)
//      {
      analogRead(PIN_ESP_PERIMETER);    // for tests
//      delay(1);
//      }

      AnalogReadMillis = AnalogReadMillis + millis()-start;
      portENTER_CRITICAL_ISR(&g_FastTimerMux);
      g_FastTimerCount = MemFastTimerCount - 1;
      portEXIT_CRITICAL_ISR(&g_FastTimerMux);
    }
    else
    {
      FastDoNothing = FastDoNothing + 1;
    }

    //---------------------------
    // Slow timer related work
    //---------------------------

    portENTER_CRITICAL_ISR(&g_FastTimerMux);
    int MemSlowTimerCount = g_SlowTimerCount;
    portEXIT_CRITICAL_ISR(&g_FastTimerMux);

    if (MemSlowTimerCount > 0)
    {
      SlowCalls = SlowCalls + 1;

      if (millis() - LastSlowCalltime > 10000)
      {
        if (MemSlowTimerCount > 1)
        {
          DebugPrintln("Slow analog aquisition timer count missed " + String(MemSlowTimerCount-1) + " time(s)", DBG_WARNING, true);
          MemSlowTimerCount = 1;
        }

//        DebugPrintln("Slow analog aquisition calls :" + String(SlowCalls) + "| Chip temperature:" + String(temprature_sens_read()), DBG_VERBOSE, true);
//        DebugPrintln("Slow analog aquisition calls :" + String(SlowCalls) + "| Chip temperature:" + String((float(temprature_sens_read())-32.0f)/1.8f,1), DBG_VERBOSE, true);
        DebugPrintln("Slow analog aquisition calls :" + String(SlowCalls) + "| Chip temperature:" + String(temperatureRead()), DBG_VERBOSE, true);
        DebugPrintln("Free heap size:" + String(esp_get_free_heap_size()) + "| Free int heap size:" + String(esp_get_free_internal_heap_size())+ "| Free min heap size:" + String(esp_get_minimum_free_heap_size()), DBG_VERBOSE, true);
        SlowCalls = 0;
        LastSlowCalltime = millis();
      }

      //  Do what ever else needs to be done here......


      portENTER_CRITICAL_ISR(&g_SlowTimerMux);
      g_SlowTimerCount = MemSlowTimerCount - 1;
      portEXIT_CRITICAL_ISR(&g_SlowTimerMux);
    }
  }
}

/**
 * Fast timer ISR function
 * 
 */
ICACHE_RAM_ATTR void FastAnaReadTimerISR(void)
{
  portENTER_CRITICAL_ISR(&g_FastTimerMux);
  g_FastTimerCount = g_FastTimerCount + 1;
  portEXIT_CRITICAL_ISR(&g_FastTimerMux);
}

/**
 * Slow Timer ISR function
 * 
 */
ICACHE_RAM_ATTR void SlowAnaReadTimerISR(void)
{
  portENTER_CRITICAL_ISR(&g_SlowTimerMux);
  g_SlowTimerCount = g_SlowTimerCount + 1;
  portEXIT_CRITICAL_ISR(&g_SlowTimerMux);
}

/**
 * Analog Read Setup function
 * 
 */
void AnaReadSetup(void)
{
  // Create new task to monitor analog channels

    BaseType_t xReturned;

    xReturned = xTaskCreatePinnedToCore(
              AnaReadLoopTask,  /* Task function. */
              "AnaReadTsk",     /* String with name of task. */
              10000,            /* Stack size in bytes. */
              NULL,             /* Parameter passed as input of the task */
              1,                /* Priority of the task. */
              &g_AnaReadTask,   /* Task handle. */
              ESP_ANA_READ_TASK_CORE);        /* Core (core 0  "reserved for system tasks") */

    if(xReturned == pdPASS)
    {
      DebugPrintln("Fast analog aquisition Task created on Core " + String(ESP_ANA_READ_TASK_CORE), DBG_INFO, true);
    }
    else
    {
      DebugPrintln("Fast analog aquisition Task creation failled (" + String(xReturned) + ")", DBG_INFO, true);
      //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
      //errQUEUE_BLOCKED						( -4 )
      //errQUEUE_YIELD							( -5 )
    }

  // Fast timer setup
  g_FastTimer = timerBegin(TIMER_FAST_NUMBER, TIMER_PRESCALER, true);
  timerAttachInterrupt(g_FastTimer, &FastAnaReadTimerISR, true);
  timerAlarmWrite(g_FastTimer, TIMER_FAST_FREQUENCY, true);
  timerAlarmEnable(g_FastTimer);
  DebugPrintln("Fast analog aquisition timer initialised at " + String(TIMER_FAST_FREQUENCY) + " microseconds", DBG_INFO, true);

  // Slow timer setup
  g_SlowTimer = timerBegin(TIMER_SLOW_NUMBER, TIMER_PRESCALER, true);
  timerAttachInterrupt(g_SlowTimer, &SlowAnaReadTimerISR, true);
  timerAlarmWrite(g_SlowTimer, TIMER_SLOW_FREQUENCY, true);
  timerAlarmEnable(g_SlowTimer);
  DebugPrintln("Slow analog aquisition timer initialised at " + String(TIMER_SLOW_FREQUENCY) + " microseconds", DBG_INFO, true);
}
