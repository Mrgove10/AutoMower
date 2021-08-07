#include <Arduino.h>
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "PerimeterTsk/PerimeterTsk.h"
#include "Utils/Utils.h"

/**
 * Perimeter processing task queue creation
 *
 */
void PerimeterQueueInit(void)
{
  // Perimeter queue creation

  /* Create a queue capable of containing bytes (used as commands to premieter processing task */
  g_PerimeterTimerQueue = xQueueCreate(PERIMETER_QUEUE_LEN, sizeof(byte) );
  if( g_PerimeterTimerQueue == NULL )
  {
    DebugPrintln("Perimeter Queue creation problem !!", DBG_ERROR, true);
  }
  else
  {
    DebugPrintln("PerimeterQueue created for " + String(PERIMETER_QUEUE_LEN) + " events", DBG_VERBOSE, true);
  }
}

/**
 * Perimeter processing task trigger timer creation
 * 
 */
void PerimeterTimerInit(void)
{
  // Perimeter timer setup

  // Create timer handle and set prescaler value
  g_PerimeterTimerhandle = timerBegin(PERIMETER_TIMER_NUMBER, TIMER_PRESCALER, true);

  // Attach Timer ISR function to Timer
  timerAttachInterrupt(g_PerimeterTimerhandle, &PerimeterTimerISR, true);

  // Setup tigger alarm associated to timer set at chosen duration
  timerAlarmWrite(g_PerimeterTimerhandle, PERIMETER_TIMER_PERIOD, true);

  // Activate the alarm
  timerAlarmEnable(g_PerimeterTimerhandle);

  DebugPrintln("Perimeter task Timer init at " + String(PERIMETER_TIMER_PERIOD) + " us", DBG_VERBOSE, true);
}

/**
 * Perimeter processing task trigger timer code
 * 
 */
ICACHE_RAM_ATTR void PerimeterTimerISR(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;     // We have not woken a task at the start of the ISR. 
  BaseType_t QueueReturn;
  byte Message = PERIMETER_TASK_PROCESSING_TRIGGER;       // for perimeter data processing

  // Tigger Perimeter processing task by sending an event in the queue
  QueueReturn = xQueueSendToBackFromISR(g_PerimeterTimerQueue, &Message, &xHigherPriorityTaskWoken );

  // To monitor normal task behaviour, we check that the queue is not full, indicating that the processing task is not runing or is running too slowly 
  if (QueueReturn != pdPASS) {

  // Decided not to protect with a semaphore the access to full queue counter as this a non critical variable and this avoids unecessary system overload
//    xSemaphoreTake(g_MyglobalSemaphore, portMAX_DELAY);
    g_PerimeterQueuefull = g_PerimeterQueuefull + 1;
//    xSemaphoreGive(g_MyglobalSemaphore);
  }
  if( xHigherPriorityTaskWoken )
  {
  	portYIELD_FROM_ISR ();
  }
}

/**
 * Perimeter data processing task Setup function
 * 
 */
void PerimeterProcessingSetup(void)
{
  g_PerimeterRawMax = 0;
  g_PerimeterRawMin = SCHAR_MAX;
  g_PerimeterRawAvg = 0;
  g_isInsidePerimeter = false;
  g_PerimetersignalTimedOut = false;
  g_PerimeterMagnitude = 0;
  g_PerimeterSmoothMagnitude = 0;
  g_PerimeterFilterQuality = 0;
  g_signalCounter = 0;

  // Trigger calibration to calculate offset
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  BaseType_t QueueReturn;
  byte Message = PERIMETER_TASK_PROCESSING_CALIBRATION;       // for perimeter Readings calibration 

  QueueReturn = xQueueSendToBackFromISR(g_PerimeterTimerQueue, &Message, &xHigherPriorityTaskWoken );
  if (QueueReturn == pdPASS) {
    DebugPrintln("Perimeter readings calibration requested", DBG_VERBOSE, true);
  }
  if( xHigherPriorityTaskWoken )
  {
    portYIELD_FROM_ISR ();
  }
}

/**
 * Perimeter data raw calibration function to determine calibration offset to "zero value"
 * @param Samples is the number of samples to use to estblish calibration offset (g_PerimeterOffset)
 * @returns the ofset in the g_PerimeterOffset global variable
 */
void PerimeterRawValuesCalibration(int Samples)
{
  DebugPrintln("Start of calibration on " + String(Samples) + " Perimeter values ", DBG_VERBOSE, true);
  
  // Wait to ensure that enough data has been captured by fast analog read task
  delay(100); 
  uint16_t RawCopy[PERIMETER_RAW_SAMPLES];
  uint16_t minVal = UINT_MAX;
  uint16_t maxVal = 0;
  
  // Get exclusive access to shared global variable and get a copy of the g_Raw circular buffer and associated pointer
  xSemaphoreTake(g_RawValuesSemaphore, portMAX_DELAY);
  memcpy(&RawCopy, &g_raw, PERIMETER_RAW_SAMPLES*sizeof(uint16_t));
  int Ptr = g_rawWritePtr;
  xSemaphoreGive(g_RawValuesSemaphore);

  // Get "Samples" values from g_Raw circular buffer and determine min/max/Total
  for (int i=0; i<Samples; i++)
  {
    maxVal = max(RawCopy[i], maxVal);
    minVal = min(RawCopy[i], minVal);
  }

  // Calculate the offset 
  int16_t center = minVal + (maxVal -  minVal) / 2;
  // Decided not to protect with a semaphore the access to offset this value is only written here and this avoids unecessary system overload
//   xSemaphoreTake(g_MyglobalSemaphore, portMAX_DELAY);
  g_PerimeterOffset = center;
//   xSemaphoreGive(g_MyglobalSemaphore);
  LogPrintln("End of calibration: Offset=" + String(center), TAG_VALUE, DBG_INFO);
}

/**
 * Perimeter data raw value conversion to [-127,+127] range
 * 
 * @param rawVal is the unsigned int value to convert
 * @param offset is the calibration offset
 * @return 8 bit integer converted value
 * */
int8_t PerimeterRawValuesConvert(uint16_t rawVal, uint16_t offset)
{
  int16_t calibratedValue = 0;
  int8_t relativeValue = 0;

  calibratedValue = rawVal - offset;
  relativeValue = min(SCHAR_MAX,  max(SCHAR_MIN, calibratedValue / 16));
  
  return relativeValue;
}

/**
 * Perimeter data raw value copy and pre-processing function (min/Max/Average)
 * 
 * @param Samples is the number of samples to copy and process
 */
void GetPerimeterRawValues(int Samples)
{
  long rawTotal = 0;

  // Get exclusive access to shared global variable and get a copy of the g_Raw circular buffer and associated pointer
  xSemaphoreTake(g_RawValuesSemaphore, portMAX_DELAY);
  memcpy(&g_RawCopy, &g_raw, PERIMETER_RAW_SAMPLES*sizeof(uint16_t));
  int Ptr = g_rawWritePtr;
  g_rawWritePtrCopy = g_rawWritePtr;
  uint16_t offset = g_PerimeterOffset;
  xSemaphoreGive(g_RawValuesSemaphore);

  // Determine start and end point in g_Raw circular buffer to get number of samples requested

  int endPtr = Ptr - 1;
  if (endPtr < 0)
  {
    endPtr = PERIMETER_RAW_SAMPLES - 1;
  }
  int startPtr = endPtr - Samples;
  if (startPtr < 0)
  {
    startPtr = PERIMETER_RAW_SAMPLES + startPtr;
  }

  int i = startPtr;

  // Get "Samples" values from g_RawCopy circular buffer, determine min/max/Total and convert values for preparation of values to be used by matched filter
  int8_t ConvertedValue = 0;
  int8_t minBuf = SCHAR_MAX;
  int8_t maxBuf = 0;
  for (int l = 0; l < Samples; l++)
  {
    // Convert value by appliying calibration offset and shifting to appropriate range and store in buffer to be used by matched filter

    // Serial.print(String(g_RawCopy[i]) + " ");

    ConvertedValue = PerimeterRawValuesConvert(g_RawCopy[i], offset);
    g_PerimeterSamplesForMatchedFilter[l] = ConvertedValue; 
    maxBuf = max(ConvertedValue, maxBuf);
    minBuf = min(ConvertedValue, minBuf);
    rawTotal = rawTotal + ConvertedValue;
    i = i + 1;
    if (i == PERIMETER_RAW_SAMPLES)
    {
      i = 0;
    }
  }

  // Serial.println();

  // Decided not to protect with a semaphore the access as these values are only written here and this avoids unecessary system overload
//   xSemaphoreTake(g_MyglobalSemaphore, portMAX_DELAY);
  g_PerimeterRawMax = maxBuf;
  g_PerimeterRawMin = minBuf;
  // Calculate average of samples extracted from g_Raw circular buffer
  g_PerimeterRawAvg = rawTotal / Samples;
//   xSemaphoreGive(g_MyglobalSemaphore);
}

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

int16_t corrFilter(int8_t *H, int8_t subsample, int16_t M, int8_t *ip, int16_t nPts, float &quality)
{  
  int16_t sumMax = 0; // max correlation sum
  int16_t sumMin = 0; // min correlation sum
  int16_t Ms = M * subsample; // number of filter coeffs including subsampling

  // compute sum of absolute filter coeffs
  int16_t Hsum = 0;
  for (int16_t i=0; i<M; i++) Hsum += abs(H[i]); 
  Hsum *= subsample;

  // compute correlation
  // for each input value
  for (int16_t j=0; j<nPts; j++)
  {
      int16_t sum = 0;      
      int8_t *Hi = H;
      int8_t ss = 0;
      int8_t *ipi = ip;      
      // for each filter coeffs
      for (int16_t i=0; i<Ms; i++)
      {        
        sum += ((int16_t)(*Hi)) * ((int16_t)(*ipi));
        ss++;
        if (ss == subsample) {
          ss=0;
          Hi++; // next filter coeffs
        }
        ipi++;
      }      
      if (sum > sumMax) sumMax = sum;
      if (sum < sumMin) sumMin = sum;
      ip++;
  }      
  // normalize to 4095
  sumMin = ((float)sumMin) / ((float)(Hsum*127)) * 4095.0;
  sumMax = ((float)sumMax) / ((float)(Hsum*127)) * 4095.0;
  
  // compute ratio min/max 
  if (sumMax > -sumMin) {
    quality = ((float)sumMax) / ((float)-sumMin);
    return sumMax;
  } else {
    quality = ((float)-sumMin) / ((float)sumMax);
    return sumMin;
  }  
}

/**
 * Matched filter comparison and output 
 * @param Samples is the number of samples to be analysed through the matched filter
 */
void MatchedFilter(int16_t Samples)
{
  // int16_t sampleCount = Samples;
  int16_t mag; // perimeter magnitude
  static float smoothMag;
  float FilterQuality = 0;
  static unsigned long lastInsideTime;
//  static int callCounter;

  // int8_t *samples = ADCMan.getCapture(idxPin[idx]);    
  // signalMin[idx] = ADCMan.getADCMin(idxPin[idx]);
  // signalMax[idx] = ADCMan.getADCMax(idxPin[idx]);
  // signalAvg[idx] = ADCMan.getADCAvg(idxPin[idx]);    

  // magnitude for tracking (fast but inaccurate)    

  if (PERIMETER_USE_DIFFERENTIAL_SIGNAL)
  {
    mag = corrFilter(g_sigcode_diff, 
                     PERIMETER_SUBSAMPLE, 
                     sizeof(g_sigcode_diff), 
                     g_PerimeterSamplesForMatchedFilter, 
                     Samples-sizeof(g_sigcode_diff)*PERIMETER_SUBSAMPLE, 
                     FilterQuality);
  }
  else
  {
    mag = corrFilter(g_sigcode_norm, 
                     PERIMETER_SUBSAMPLE, 
                     sizeof(g_sigcode_norm), 
                     g_PerimeterSamplesForMatchedFilter, 
                     Samples-sizeof(g_sigcode_norm)*PERIMETER_SUBSAMPLE, 
                     FilterQuality);
  } 

  if (PERIMETER_SWAP_COIL_POLARITY)
  {
    mag = mag * -1;
  }        
  
  // smoothed magnitude used for signal-off detection  
  smoothMag = 0.99 * smoothMag + 0.01 * ((float)abs(mag));  
  //smoothMag[idx] = 0.99 * smoothMag[idx] + 0.01 * ((float)mag[idx]);  

  // perimeter inside/outside detection
  if (mag > 0){
    g_signalCounter = min(g_signalCounter + 1, 5);    
  } else {
    g_signalCounter = max(g_signalCounter - 1, -5);    
  }
  if (g_signalCounter < 0){
    lastInsideTime = millis();
  } 

  boolean isInside;
  if (abs(mag) > PERIMETER_IN_OUT_DETECTION_THRESHOLD) {
    // Large signal, the in/out detection is reliable.
    // Using mag yields very fast in/out transition reporting.
    isInside = (mag < 0);
  } else {
    // Low signal, use filtered value for increased reliability
    isInside =  (g_signalCounter < 0);
  }

  // Decided not to protect with a semaphore the access match filter values as these values are only written here and this avoids unecessary system overload
//  xSemaphoreTake(g_MyglobalSemaphore, portMAX_DELAY);
  g_PerimeterMagnitude = mag;
  g_PerimeterSmoothMagnitude = smoothMag;
  g_isInsidePerimeter = isInside;
  g_PerimeterFilterQuality = FilterQuality;
//  xSemaphoreGive(g_MyglobalSemaphore);
}

/**
 * Perimeter processing task main loop
 * @param dummyParameter is unused but required
 */
void PerimeterProcessingLoopTask(void *dummyParameter)
{
{
  static bool SetupDone = false;

  for (;;)
  {
    //------------------------------------------------------------------
    // Task Setup (done only on 1st call)
    //------------------------------------------------------------------

    if (!SetupDone)
    {
      DebugPrintln("Perimeter data processing Task Started on core " + String(xPortGetCoreID()) , DBG_VERBOSE, true);

      PerimeterQueueInit();         // Create queue used by timer based ISR
      PerimeterTimerInit();         // Create and start Timer based ISR
      PerimeterProcessingSetup();   // Initialise task value and results

      SetupDone = true;
      delayMicroseconds(PERIMETER_TIMER_PERIOD);

      DebugPrintln("Perimeter data processing Task setup complete: Perimeterqueue:" + String(uxQueueMessagesWaiting(g_PerimeterTimerQueue)) , DBG_VERBOSE, true);
    }

    //------------------------------------------------------------------
    // Task Loop (done on each timer semaphore release)
    //------------------------------------------------------------------

    byte evt;
    static int count = 0;
    // Wait on queue event
    while (xQueueReceive(g_PerimeterTimerQueue, &evt, portMAX_DELAY) == pdPASS)
    {
      // To monitor correct operation of the reading task, the number of unread events in the queue is monitored (should be zero)
      unsigned int inPerimterQueue = uxQueueMessagesWaiting(g_PerimeterTimerQueue);
      // Decided not to protect with a semaphore the access to monotoring shared variables as they are non critical variable and this avoids unecessary system overload
      g_inPerimeterQueue = g_inPerimeterQueue + inPerimterQueue;
      g_inPerimeterQueueMax = max(inPerimterQueue, g_inPerimeterQueueMax);

      if (evt == PERIMETER_TASK_PROCESSING_TRIGGER)     // Perimeter data processing
      {
        // Get values from fast aquisition Task and Calculate Min/Max/Avg
        GetPerimeterRawValues(I2S_DMA_BUFFER_LENGTH);

        // Run MatchFilter and Determine Perimeter status variables
        MatchedFilter(I2S_DMA_BUFFER_LENGTH);
        
        // Display Perimeter task and filter summary information
        count = count + 1;
        if (count ==  6 || abs(g_PerimeterMagnitude) > 400 ) {
          DebugPrintln("Perim: inQ:" + String(g_inPerimeterQueue) +
                     " QMax:" + String(g_inPerimeterQueueMax) +
                     " Qfull:" + String(g_PerimeterQueuefull) +
                     " |RawAvg " + String(g_PerimeterRawAvg) + " [" + String(g_PerimeterRawMin) + "," + String(g_PerimeterRawMax) + "]" +
                     " |FiltMag:" + String(g_PerimeterMagnitude) +
                     " SMag:" + String(g_PerimeterSmoothMagnitude) +
                     " FiltQual:" + String(g_PerimeterFilterQuality,2) + 
                     " Sigcount:" + String(g_signalCounter) +
                     " in?:" + String(g_isInsidePerimeter),
                 DBG_DEBUG, true);

          // Reset displayed values
          g_PerimeterQueuefull = 0;
          g_inPerimeterQueueMax = 0;
          g_inPerimeterQueue = 0;
          count = 0;
        }
// plot Match filter results
#ifdef SERIAL_PLOTTER
        int Ptr = g_rawWritePtrCopy;
        uint16_t offset = g_PerimeterOffset;

        int endPtr = Ptr - 1;
        if (endPtr < 0)
        {
          endPtr = PERIMETER_RAW_SAMPLES - 1;
        }
        int startPtr = endPtr - I2S_DMA_BUFFER_LENGTH;
        if (startPtr < 0)
        {
          startPtr = PERIMETER_RAW_SAMPLES + startPtr;
        }
        int i = startPtr;

        for (int l = 0; l < I2S_DMA_BUFFER_LENGTH; l++)
        {
          Serial2.println(String(micros() & 0x0FFFFF) + ";;;" + 
                          String(i) + ";" + String(l) + ";" + String(g_RawCopy[i]) + ";" + 
                          String(PerimeterRawValuesConvert(g_RawCopy[i],offset)) + ";" +
                          String(g_PerimeterMagnitude) + ";" + String(g_PerimeterSmoothMagnitude) + ";" +
                          String(g_PerimeterFilterQuality,2) + ";" + String(g_signalCounter)+ ";" + String(g_isInsidePerimeter));
          i = i + 1;
          if (i == PERIMETER_RAW_SAMPLES)
          {
              i = 0;
          }
        }
#endif
      }
      else if (evt == PERIMETER_TASK_PROCESSING_CALIBRATION)    // Calibration
      {
        PerimeterRawValuesCalibration(PERIMETER_RAW_SAMPLES);
      }
    }
  }
}

}

/**
 * Perimeter processing task creation function
 */
void PerimeterProcessingLoopTaskCreate(void)
{
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      PerimeterProcessingLoopTask, /* Task function. */
      PERIMETER_TASK_NAME,         /* String with name of task. */
      PERIMETER_TASK_STACK_SIZE,   /* Stack size in bytes. */
      NULL,                        /* Parameter passed as input of the task */
      PERIMETER_TASK_PRIORITY,     /* Priority of the task. */
      &g_PerimeterProcTaskHandle,  /* Task handle. */
      PERIMETER_TASK_ESP_CORE);

  if (xReturned == pdPASS)
  {
    DebugPrintln("Perimeter data processing Task created on Core " + String(PERIMETER_TASK_ESP_CORE), DBG_VERBOSE, true);
  }
  else
  {
    DebugPrintln("Perimeter data processing Task creation failled (" + String(xReturned) + ")", DBG_ERROR, true);
    //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
    //errQUEUE_BLOCKED						( -4 )
    //errQUEUE_YIELD							( -5 )
  }
}

/**
 * Perimeter processing task suspension function
 */
void PerimeterProcessingLoopTaskSuspend(void)
{
  vTaskSuspend(g_PerimeterProcTaskHandle);
  Serial.println("Perimeter data processing Task suspended");
}

/**
 * Perimeter processing task resume from suspension function
 */
void PerimeterProcessingLoopTaskResume(void)
{
  vTaskResume(g_PerimeterProcTaskHandle);
  DebugPrintln("Perimeter data processing Task resumed", DBG_VERBOSE, true);
}
