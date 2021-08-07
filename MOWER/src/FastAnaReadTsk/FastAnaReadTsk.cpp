#include <Arduino.h>
#include "Environment_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "FastAnaReadTsk/FastAnaReadTsk.h"
#include "Utils/Utils.h"

#include <driver/adc.h>
#include <driver/i2s.h>

/**
 * Read raw analog values form I2S diver buffers
 * @param Samples number of samples to read from I2S buffer
 */
void I2SAnalogRead(int Samples)
{
  size_t bytesNeeded = Samples * sizeof(uint16_t); // Size of DMA buffer read: each sample is 2 bytes long
  uint16_t i2sData[Samples];                       // Array to recevive content of DM buffer
  size_t bytesRead = 0;                            // number of bytes red returned by I2S buffer read function
  size_t total_timeout = 0;

  // Ensure exlusive access to ADC
  xSemaphoreTake(g_ADCinUse, portMAX_DELAY);

  // Read I2S buffer with very short timeout (1 RTOS tick) as buffer should be full as we are reading after
  // I2S driver has notified data is available through associated queue.
  i2s_read(I2S_PORT, i2sData, bytesNeeded, &bytesRead, I2S_READ_TIMEOUT);

  // Free access to ADC for other tasks
  xSemaphoreGive(g_ADCinUse);

  // To monitor normal task behaviour, we check that the number of bytes transfered from the I2S
  // DMA buffer is consistent with the expected number of bytes.
  // If not, this is the sign of a timeout while reading teh buffer, which is not normal.
  if (bytesRead != bytesNeeded)
  {
    total_timeout = total_timeout + 1;
  }

  // Ensure exlusive access to shared global variables to ensure data integrity
  // within arrays and other data updated by task
  xSemaphoreTake(g_RawValuesSemaphore, portMAX_DELAY);

  // Copy content of red DMA buffer to global shred variable for use by other tasks and update pointer to last value
  for (int i = 0; i < Samples; i++)
  {
    g_raw[g_rawWritePtr] = i2sData[i] & 0x0FFF; // raw data provided in [0, 4095] value range
    // Serial.print(String(g_raw[g_rawWritePtr]) + " ");
    g_rawWritePtr = g_rawWritePtr + 1; // update pointer to last updated value in g_raw rotating buffer
    if (g_rawWritePtr == PERIMETER_RAW_SAMPLES)
    {
      g_rawWritePtr = 0;
    }
  }
  //  Serial.println();

  // Free access to shared global variables with Perimter data processing task
  xSemaphoreGive(g_RawValuesSemaphore);

  // Decided not to protect with a semaphore the access to timeout counter as this a non critical variable and this avoids unecessary system overload
  //  xSemaphoreTake(g_MyglobalSemaphore, portMAX_DELAY);
  g_FastAnaReadTimeout = g_FastAnaReadTimeout + total_timeout;
  //  xSemaphoreGive(g_MyglobalSemaphore);
}

/**
 * Initialise I2S driver for high frequency analog data reading from ADC
 * 
 */
void initI2S(void)
{
  // Setup I2S configuration descriptor
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // we use 16 bit depth sampling
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // avoids the sample rate being doubled vs RIGHT_LEFT
      .communication_format = I2S_COMM_FORMAT_I2S_LSB,
      .intr_alloc_flags = 0,
      .dma_buf_count = I2S_DMA_BUFFERS,     // must be between 2 and 128
      .dma_buf_len = I2S_DMA_BUFFER_LENGTH, // must between 8 and 1024
      .use_apll = false,                    // we do not use RTC clock timer
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  // Install and start I2S driver to write to a queue to notify for new data available
  i2s_driver_install(I2S_PORT, &i2s_config, 1, &g_I2SQueueHandle);

  // Initialise ADC pad for use by I2S driver and assign appropriate ADC channel (corresponding to connected pin)
  i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);

  // Initialise DMA buffers
  i2s_zero_dma_buffer(I2S_PORT);

  // Start I2S driver ADC access
  i2s_adc_enable(I2S_PORT);

  // invert the ADC so that the results match those from the normal adc read api
  adc_set_data_inv(I2S_ADC_UNIT, true);

  DebugPrintln("I2S Clock rate:" + String(i2s_get_clk(I2S_PORT)), DBG_VERBOSE, true);
  //  DebugPrintln("initI2S I2Squeue:" + String(uxQueueMessagesWaiting(g_I2SQueueHandle)));
}

/**
 * High frequency analog read task main loop
 * @param dummyParameter is unused but required
 */
void FastAnaReadLoopTask(void *dummyParameter)
{
  static bool SetupDone = false;

  // Task is supposed to run forever
  for (;;)
  {
    //------------------------------------------------------------------
    // Task Setup (done only on 1st call)
    //------------------------------------------------------------------
    if (!SetupDone)
    {
      DebugPrintln("Fast analog aquisition Task started on Core " + String(xPortGetCoreID()), DBG_VERBOSE, true);

      // Initialise I2S driver
      initI2S();

      // initialise shared array containing raw sample values
      for (int i = 0; i < PERIMETER_RAW_SAMPLES; i++)
      {
        g_raw[i] = 0;
      }

      DebugPrintln("Fast analog aquisition Task setup complete: I2Squeue:" + String(uxQueueMessagesWaiting(g_I2SQueueHandle)), DBG_VERBOSE, true);
      SetupDone = true;
    }

    //------------------------------------------------------------------
    // Task Loop (done on each timer semaphore release)
    //------------------------------------------------------------------

    i2s_event_t evt;

    // Endless loop waiting on I2S Driver queue
    while (xQueueReceive(g_I2SQueueHandle, &evt, portMAX_DELAY) == pdPASS)
    {
      // To monitor correct operation of the reading task, the number of unread events in the queue is monitored (should be zero)
      unsigned int inQueue = uxQueueMessagesWaiting(g_I2SQueueHandle);
      // Decided not to protect with a semaphore the access to monotoring shared variables as they are non critical variable and this avoids unecessary system overload
      g_inQueue = g_inQueue + inQueue;
      g_inQueueMax = max(inQueue, g_inQueueMax);

      // if Queue entry is a DMA buffer Ready event, then read the samples from the I2S DMA buffers
      if (evt.type == I2S_EVENT_RX_DONE)
      {
        I2SAnalogRead(I2S_DMA_BUFFER_LENGTH);
      }
    }
  }
}

/**
 * High frequency analog read task creation function
 */
void FastAnaReadLoopTaskCreate(void)
{
  // Create semaphores used by task
  g_ADCinUse = xSemaphoreCreateMutex();
  g_RawValuesSemaphore = xSemaphoreCreateMutex();

  // Create RTOS high speed analog read task
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      FastAnaReadLoopTask,           /* Task function. */
      FAST_ANA_READ_TASK_NAME,       /* String with name of task. */
      FAST_ANA_READ_TASK_STACK_SIZE, /* Stack size in bytes. */
      NULL,                          /* Parameter passed as input of the task */
      FAST_ANA_READ_TASK_PRIORITY,   /* Priority of the task */
      &g_FastAnaReadTaskHandle,      /* Task handle */
      FAST_ANA_READ_TASK_ESP_CORE);  /* Task core */

  if (xReturned == pdPASS)
  {
    DebugPrintln("Fast analog aquisition Task created on Core " + String(FAST_ANA_READ_TASK_ESP_CORE), DBG_VERBOSE, true);
  }
  else
  {
    DebugPrintln("Fast analog aquisition Task creation failled (" + String(xReturned) + ")", DBG_ERROR, true);
    //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
    //errQUEUE_BLOCKED						( -4 )
    //errQUEUE_YIELD							( -5 )
  }
}

/**
 * High frequency analog read task suspension function
 */
void FastAnaReadLoopTaskSuspend(void)
{
  vTaskSuspend(g_FastAnaReadTaskHandle);
  DebugPrintln("Fast analog aquisition Task suspended", DBG_VERBOSE, true);
}

/**
 * High frequency analog read task resume from suspension function
 */
void FastAnaReadLoopTaskResume(void)
{
  vTaskResume(g_FastAnaReadTaskHandle);
  DebugPrintln("Fast analog aquisition Task resumed", DBG_VERBOSE, true);
}

/**
 * AnalogRead function that is compatible with I2S ADC use
 * @param pin to read from
 * @return pin analog value
 */
int ProtectedAnalogRead(int pin)
{
  // Using I2S to read from the ADC causes conflict with normal analogRead function as it locks the ADC
  // This function is to be used to perform "usual" analog reads whilst managing the conflict with the I2S driver through the use on
  // application semaphore
  // IMPORTANT NOTES:
  // 1- This function may not be used before the semaphore has been initialised (initlisation is taken care of in the FastAnaReadLoopTaskCreate() function)
  // 2- This function only works for ADC1 (pins 32 to 39)

  // obtain exclusive access to ADC (or wait for it to be available)
  xSemaphoreTake(g_ADCinUse, portMAX_DELAY);

  // Stop I2S driver
  i2s_stop(I2S_PORT);

  // Stop I2S driver using ADC
  i2s_adc_disable(I2S_PORT);

  // read ADC channel corresponding to Pin
  adc1_channel_t ADC1Channel;

  switch (pin)
  {
  case 32:
    ADC1Channel = ADC1_GPIO32_CHANNEL;
    break;
  case 33:
    ADC1Channel = ADC1_GPIO33_CHANNEL;
    break;
  case 34:
    ADC1Channel = ADC1_GPIO34_CHANNEL;
    break;
  case 35:
    ADC1Channel = ADC1_GPIO35_CHANNEL;
    break;
  case 36:
    ADC1Channel = ADC1_GPIO36_CHANNEL;
    break;
  case 37:
    ADC1Channel = ADC1_GPIO37_CHANNEL;
    break;
  case 38:
    ADC1Channel = ADC1_GPIO38_CHANNEL;
    break;
  case 39:
    ADC1Channel = ADC1_GPIO39_CHANNEL;
    break;
  default:
    DebugPrintln("Pin " + String(pin) + " is not an ADC1 pin !", DBG_ERROR, true);
    return -1;
  }

  int rawval = adc1_get_raw(ADC1Channel);

  // Restore I2S driver using ADC
  i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);

  // Clear I2S DMA buffers
  i2s_zero_dma_buffer(I2S_PORT);

  // Assign ADC to I2S driver
  i2s_adc_enable(I2S_PORT);

  // Start I2S driver
  i2s_start(I2S_PORT);

  xSemaphoreGive(g_ADCinUse);
  // Free access to ADC

  return rawval;
}
