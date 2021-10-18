#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "IOExtender/IOExtender.h"

/**
 * I2C INA219 Perimeter load Current Sensor Setup function
 *
 */
void PerimeterLoadCurrentSensorSetup()
{
  if (!PerimeterCurrentSensor.begin())
  {
    LogPrintln("Perimeter current Sensor not found !", TAG_CHECK, DBG_ERROR);
  }
  else
  {
    DebugPrintln("Perimeter current Sensor found", DBG_VERBOSE, true);
  }

  PerimeterCurrentSensor.setCalibration_32V_1A();
  delay(100);
  PerimeterLoadCurrentRead();
}

/**
 * Checks to see if perimeter INA219 I2C current sensor is connected (and hopefully functionning)
 * 
 * @return true if INA219 current sensor is ok
 */
bool PerimeterLoadCurrentSensorCheck(void)
{
  DisplayClear();
  DisplayPrint(0, 0, F("Perim. sensor Test"));

  if (PerimeterCurrentSensor.success())
  {
    DebugPrintln("Perimeter load Sensor ok, Value: " + String(g_PerimeterCurrent, 0), DBG_INFO, true);
    DisplayPrint(1, 2, "Load OK: " + String(g_PerimeterCurrent, 0) + " mA");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Perimeter load Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2, F("ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to test and read Battery Charge current
 * 
 * * @param Now true to force an immediate read
 * * @param Reset true to force an average value reset
 * * @return true if charge current could be read
 */
bool PerimeterLoadCurrentRead(const bool Now, const bool Reset)
{
  static unsigned long LastPerimeterCurrentRead = 0;
  static float smoothedCurrent = UNKNOWN_FLOAT;
  float busvoltage = 0;
  float power = 0;

  if ((millis() - LastPerimeterCurrentRead > PERIMETER_CURRENT_READ_INTERVAL) || Now)
  {
    float current_mA = 0;

    // Ensure exlusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

    current_mA = PerimeterCurrentSensor.getCurrent_mA();
    busvoltage = PerimeterCurrentSensor.getBusVoltage_V();
    power = PerimeterCurrentSensor.getPower_mW() / 1000.0;

    // Free access to I2C for other tasks
    xSemaphoreGive(g_I2CSemaphore);

    if (PerimeterCurrentSensor.success())
    {
      // Filter out very low current values
      if (current_mA < PERIMETER_CURRENT_CURRENT_MIN)
      {
        current_mA = 0;
      }

    // Reset average on first value and on leaving base
      if (smoothedCurrent == UNKNOWN_FLOAT || Reset)
      {
        smoothedCurrent = abs(current_mA);
      }
      else
      {
        smoothedCurrent = 0.8 * smoothedCurrent + 0.2 * ((float)abs(current_mA));
      }

      g_PerimeterCurrent = smoothedCurrent;
      g_PerimeterVoltage = busvoltage;
      g_PerimeterPower = power;
      LastPerimeterCurrentRead = millis();

      DebugPrintln("Perimeter current value:" + String(g_PerimeterCurrent) + " mA (INA bus voltage:" + String(busvoltage,3) + " V)", DBG_VERBOSE, true);
      return true;
    }
    else
    {
      g_PerimeterCurrent = UNKNOWN_FLOAT;
      DebugPrintln("Perimeter current could not be read !", DBG_VERBOSE, true);
      return false;
    }
  }

  return true;
}
