#include <Arduino.h>
#include "Environment_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "AnaReadTask/AnaReadTask.h"
#include "Utils/Utils.h"


#include <driver/adc.h>
#include <driver/i2s.h>



// storage for the samples
#define NSAMPLES 2
uint16_t samples[NSAMPLES] = {0};

// storage for the DAC waveform at 10Hz
const uint32_t nWavePoints = ANA_READ_TASK_SAMPLE_RATE/10;
uint16_t wave[nWavePoints];

#define BYTES_NEEDED 2

int I2SAnalogRead(void)
{
  size_t bytes_read;
  uint16_t sample = 0;    // samples are 16 bit integers

  i2s_read(I2S_NUM_0, (void*) &sample, BYTES_NEEDED, &bytes_read, portMAX_DELAY); // this will wait until enough data is ready
  if (sample >> 12 == ANA_READ_TASK_ADC_CHANNEL && bytes_read == BYTES_NEEDED)     // channel is in the top 4 bits
  {
    return sample & 0x0FFF;
  }
  else
  { 
    return -1;
  }
}

void initI2S(void)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = ANA_READ_TASK_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,  // stops the sample rate being doubled vs RIGHT_LEFT
        .communication_format = I2S_COMM_FORMAT_I2S_LSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = 1,
        .tx_desc_auto_clear = 0,
        .fixed_mclk = 1
    };

    //install and start i2s driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

    //init ADC pad
    i2s_set_adc_mode(ADC_UNIT_1, ANA_READ_TASK_ADC_CHANNEL);

// NOT NEEDED IN THEORY - JUST FOR TEST
    // init DAC
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); // right is GPIO25

    // invert the ADC so that the results match those from the normal adc read api
    adc_set_data_inv(ADC_UNIT_1, true);
}

/*
void dacTask(void *params)
{
    size_t written;
    for(;;) {
        i2s_write(I2S_NUM_0, &wave[0], 1000, &written, portMAX_DELAY);
        i2s_write(I2S_NUM_0, &wave[500], 1000, &written, portMAX_DELAY);
    }
}
*/

/**
 * Analog Read permanent loop task
 * 
 */
void AnaReadLoopTask(void * dummyParameter )
{
  static bool SetupDone = false;
  static unsigned long FastTimerCountTotal = 0;
//  static unsigned long FastDoNothing = 0;
//  static unsigned long FastMissed = 0;
  static unsigned long LastFastCalltime = 0;
  static unsigned long SlowCalls = 0;
  static unsigned long LastSlowCalltime = 0;
  static unsigned long AnalogReadMicros = 0;

  static unsigned long MemFastTimerCount = 0;

//  size_t bytes_read, total_read;
//  const size_t bytesNeeded = NSAMPLES * 2;


  uint8_t temprature_sens_read();

  for (;;)
  {
//    if (ANA_READ_TASK_ESP_CORE == 0)
//    {
//      delayMicroseconds(TIMER_FAST_FREQUENCY*4);
//    } 
    if (!SetupDone)
    {
      // insert 1st execution (setup) code here
      adcAttachPin(PIN_ESP_PERIMETER);
      analogReadResolution(10);
      analogSetClockDiv(1);
      analogRead(PIN_ESP_PERIMETER);    // for tests

      DebugPrintln("Analog aquisition Task Started on core " + String(xPortGetCoreID()), DBG_INFO, true);
      SetupDone = true;
      MemFastTimerCount = 0;
    }

    //---------------------------
    // Fast timer related work
    //---------------------------

    if (xSemaphoreTake(g_FastTimerSemaphore, portMAX_DELAY) == pdTRUE)
    {

    portENTER_CRITICAL_ISR(&g_AnaReadMux);
    MemFastTimerCount = g_FastTimerCount;
//        Serial.print("MemFastTimerCount:");
//       Serial.print(MemFastTimerCount);
//        Serial.print(" g_FastTimerCount:");
//        Serial.println(g_FastTimerCount);
    portEXIT_CRITICAL_ISR(&g_AnaReadMux);

//        delay(500);

    FastTimerCountTotal = FastTimerCountTotal + 1;

// read the value
//      unsigned long start = micros();

      int val=-1;
//      val=I2SAnalogRead();
      if (val != -1) 
      {
        g_readAnaBuffer[g_readAnaBufferPtr] = val;
        if (g_readAnaBufferPtr == ANA_READ_BUFFER_SIZE - 1)
        {
          g_readAnaBufferPtr = 0;
        }
        else
        {
          g_readAnaBufferPtr = g_readAnaBufferPtr + 1;
        }
      }
//      AnalogReadMicros = AnalogReadMicros + micros()-start;

//      if (MemFastTimerCount > 1)
//      {
//        FastMissed = FastMissed + MemFastTimerCount - 1;
//        MemFastTimerCount = 1;
//      }

      if (millis() - LastFastCalltime > 10000)
      {
/*
        if (FastMissed > 0)
        {
          DebugPrintln("");
          DebugPrintln("Fast analog aquisition timer count missed " + String(FastMissed) + " time(s)", DBG_WARNING, true);
          FastMissed = 0;
        }
*/
//        unsigned long now = millis();

        g_MissedReadings = MemFastTimerCount - FastTimerCountTotal;
        g_rate = (float) FastTimerCountTotal/(millis()-LastFastCalltime);
        g_Triggers = MemFastTimerCount;
/*
        DebugPrintln("");
        DebugPrintln("Timer calls :" + String(MemFastTimerCount) +
                     "| Read iterations:" + String(FastTimerCountTotal) + 
                     "| Missed readings :" + String(MemFastTimerCount - FastTimerCountTotal) + 
                     " " + String((float) (MemFastTimerCount - FastTimerCountTotal)/MemFastTimerCount*100,2) + "%" +
                     " => " + String((float) FastTimerCountTotal/(now-LastFastCalltime),3) + 
                     "kHz | Read Time " + String((float) AnalogReadMicros/FastTimerCountTotal,3) + "microseconds/sample", DBG_INFO, true);
*/
//        FastCalls = 0;
//        FastDoNothing = 0;
        MemFastTimerCount = 0;
        FastTimerCountTotal = 0;
        LastFastCalltime = millis();
        AnalogReadMicros = 0;
      }
      //  Do what ever elseneeds to be done here......

//      for (int i=0; i<1;i++)
//      {
//analogRead(PIN_ESP_PERIMETER);    // for tests
//        adc1_get_raw(ADC1_CHANNEL_3);
//      delay(1);
//      }

//      g_AnalogReadMicrosTotal = 0;
//      g_timerCallCounter = 0;

      portENTER_CRITICAL_ISR(&g_AnaReadMux);
      if(MemFastTimerCount > 0) 
      {
        g_FastTimerCount = g_FastTimerCount - 1;
      }
      else
      {
        g_FastTimerCount = 0;
      }
      portEXIT_CRITICAL_ISR(&g_AnaReadMux);
    }
    else
    {
      DebugPrintln("Semaphore timeout expired !!!", DBG_WARNING, true);
    }
/*    }
    else
    {
      FastDoNothing = FastDoNothing + 1;
    }
*/

/*
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
    */
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
  xSemaphoreGive(g_FastTimerSemaphore);
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

    initI2S();

    // task for writing out the waveform to the DAC
//    xTaskCreatePinnedToCore(dacTask, "dacTask", 3000, NULL, 10, NULL, 1);

  g_FastTimerSemaphore = xSemaphoreCreateMutex();


    BaseType_t xReturned;

    xReturned = xTaskCreatePinnedToCore(
              AnaReadLoopTask,  /* Task function. */
              "AnaReadTsk",     /* String with name of task. */
              10000,            /* Stack size in bytes. */
              NULL,             /* Parameter passed as input of the task */
              1,                /* Priority of the task. */
              &g_AnaReadTask,   /* Task handle. */
              ANA_READ_TASK_ESP_CORE);        /* Core (core 0  "reserved for system tasks") */

    if(xReturned == pdPASS)
    {
      DebugPrintln("Fast analog aquisition Task created on Core " + String(ANA_READ_TASK_ESP_CORE), DBG_INFO, true);
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
