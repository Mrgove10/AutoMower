#include <Arduino.h>
#include "Environment_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "AnaReadTsk/AnaReadTsk.h"
#include "Battery/Battery.h"
#include "Battery/Battery.h"
#include "MotorCurrent/MotorCurrent.h"
#include "Compass/Compass.h"
#include "GPS/GPS.h"
#include "GyroAccel/GyroAccel.h"
#include "Utils/Utils.h"

#include <driver/adc.h>
#include <driver/i2s.h>

/**
 * AnalogRead function which is compatible with I2S ADC use
 * @param pin to read from
 * @return pin analog value
 */
int ProtectedAnalogRead(int pin)
{
  // Using I2S to read from the ADC causes conflict with normal analogRead function as it locks the ADC
  // This function is to be used to perform "usual" (slow) analog reads whilst managing the conflict with the I2S driver through the use of an
  // application semaphore
  // IMPORTANT NOTES:
  // 1- This function may not be used before the semaphore has been initialised (initlisation is taken care of in the FastAnaReadLoopTaskCreate() function)
  // 2- This function only works for ADC1 connected pins (pins 32 to 39)

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

/**
 * Analog Read task main loop
 * @param dummyParameter is unused but required
 */
void AnaReadLoopTask(void *dummyParameter)
{
  static bool SetupDone = false;
  static int loopCount = 0;
  static unsigned long startime = millis();

  for (;;)
  {
    //------------------------------------------------------------------
    // Task Setup (done only on 1st call)
    //------------------------------------------------------------------

    if (!SetupDone)
    {
      DebugPrintln("Analog read Task Started on core " + String(xPortGetCoreID()), DBG_VERBOSE, true);

      //   SonarSensorSetup();       // Setup Sensors

      // Motion motor current sensor setup
      MotorCurrentSensorSetup();

      SetupDone = true;
      DebugPrintln("Analog read Task setup complete", DBG_VERBOSE, true);
    }
    
    //------------------------------------------------------------------
    // Task Loop (done on each loop if read is not suspended)
    //------------------------------------------------------------------

    // Read battery voltage value
    BatteryVoltageRead(false); // the read interval check is performed by the function

    // Read battery charge current value
    BatteryChargeCheck();

    // Read motion motor current value
    MotorCurrentRead(MOTOR_CURRENT_RIGHT, false); // The read interval check is performed by the function
    MotorCurrentRead(MOTOR_CURRENT_LEFT, false);  // The read interval check is performed by the function

    // Read cut motor current value
    MotorCurrentRead(MOTOR_CURRENT_CUT, false); // The read interval check is performed by the function

    // Read GPS data
    GPSRead(false); // The read interval check is performed by the function

    // Read Gyro / Accel Angle data
    PitchRollCalc(false);

    // Read Compass data
    CompassRead(false); // The read interval check is performed by the function

    loopCount ++;
    if (loopCount > 100)
    {
      DebugPrintln("Analog read Task loop time: " + String(millis() - startime) + " ms", DBG_INFO, true);
      loopCount = 0;
      startime = millis();
    }
    // Wait for the next loop
    delay(ANA_READ_TASK_LOOP_WAIT);
  }
}

/**
 * Analog Read task creation function
 */
void AnaReadLoopTaskCreate(void)
{
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      AnaReadLoopTask,          /* Task function. */
      ANA_READ_TASK_NAME,       /* String with name of task. */
      ANA_READ_TASK_STACK_SIZE, /* Stack size in bytes. */
      NULL,                     /* Parameter passed as input of the task */
      ANA_READ_TASK_PRIORITY,   /* Priority of the task. */
      &g_AnaReadTaskHandle,     /* Task handle. */
      ANA_READ_TASK_ESP_CORE);

  if (xReturned == pdPASS)
  {
    DebugPrintln("Analog read Task created on Core " + String(ANA_READ_TASK_ESP_CORE), DBG_VERBOSE, true);
  }
  else
  {
    DebugPrintln("Analog read Task creation failled (" + String(xReturned) + ")", DBG_ERROR, true);
    //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
    //errQUEUE_BLOCKED						( -4 )
    //errQUEUE_YIELD						( -5 )
  }
}

/**
 * Analog Read task suspension function
 */
void AnaReadLoopTaskSuspend(void)
{
  vTaskSuspend(g_AnaReadTaskHandle);
  DebugPrintln("Analog read Task suspended", DBG_INFO, true);

}

/**
 * Analog Read task resume from suspension function
 */
void AnaReadLoopTaskResume(void)
{
  vTaskResume(g_AnaReadTaskHandle);
  DebugPrintln("Analog read Task resumed", DBG_INFO, true);
}
