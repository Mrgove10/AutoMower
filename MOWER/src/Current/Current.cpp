#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Current/Current.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "AnaReadTsk/AnaReadTsk.h"

/**
 * I2C INA219 Current Sensor Setup function
 */

void MotorCurrentSensorSetup()
{
  for (int sensor = 0; sensor < MOTOR_CURRENT_COUNT; sensor++)
  {
    if (!MotorCurrentSensor[sensor].begin())
    {
      DebugPrintln("Motor current Sensor # " + String(sensor) + " not found !", DBG_VERBOSE, true);
      LogPrintln("Motor current Sensor # " + String(sensor) + " not found !", TAG_CHECK, DBG_ERROR);
    }
    else
    {
      DebugPrintln("Motor current Sensor # " + String(sensor) + " found !", DBG_VERBOSE, true);
    }

    MotorCurrentSensor[sensor].setCalibration_32V_1A();
    //  g_MotorCurrent[sensor].setCalibration_16V_400mA();
    delay(100);
    MotorCurrentRead(sensor);
  }
}

/**
 * Checks to see if Motor I2C INA219 Current Sensor is connected (and hopefully functionning)
 * @param sensor int sensor to check
 * @return true if current sensor is ok
 */
bool MotorCurrentSensorCheck(int sensor)
{
  String sensorStr[MOTOR_CURRENT_COUNT] = {"Right", "Left", "Cut"};

  if (sensor == 0)
  {
    DisplayClear();
    DisplayPrint(0, 0, F("Motor Current Test"));
  }

  DisplayPrint(2, sensor + 1, sensorStr[sensor]);

  if (MotorCurrentSensor[sensor].success())
  {
    DebugPrintln(sensorStr[sensor] + " Motor Current Sensor ok", DBG_INFO, true);
    DisplayPrint(8, sensor + 1, F("OK"));

    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(sensorStr[sensor] + " Motor Current Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(8, sensor + 1, F("ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to test and read I2C INA219 Current Sensor
 *  
 * @param sensor int sensor to read
 * @param Now true to force an immediate read
 * @return true if current could be read
 */
bool MotorCurrentRead(const int sensor, const bool Now)
{
  static unsigned long LastMotorCurrentRead[MOTOR_CURRENT_COUNT] = {0, 0, 0};
  static float smoothedCurrent[MOTOR_CURRENT_COUNT] = {UNKNOWN_FLOAT, UNKNOWN_FLOAT, UNKNOWN_FLOAT};

  if ((millis() - LastMotorCurrentRead[sensor] > MOTOR_CURRENT_READ_INTERVAL) || Now)
  {
    //    float shuntvoltage = 0;
    //    float busvoltage = 0;
    float current_mA = 0;
    //    float loadvoltage = 0;
    //    float power_mW = 0;

    //    shuntvoltage = MotorCurrentSensor[sensor].getShuntVoltage_mV();
    //    busvoltage = MotorCurrentSensor[sensor].getBusVoltage_V();
    current_mA = MotorCurrentSensor[sensor].getCurrent_mA();
    //    power_mW = MotorCurrentSensor[sensor].getPower_mW();
    //    loadvoltage = busvoltage + (shuntvoltage / 1000);

    if(smoothedCurrent[sensor] == UNKNOWN_FLOAT)
    {
      smoothedCurrent[sensor] = abs(current_mA);
    }
    else
    {
      smoothedCurrent[sensor] = 0.8 * smoothedCurrent[sensor] + 0.2 * ((float)abs(current_mA));
    }

    /*
    DebugPrintln("Sensor" + String(sensor) + " Bus Voltage: " + String(busvoltage) + " V" + 
              " Shunt Voltage: " + String(shuntvoltage) + " mV" + 
              " Load Voltage: " + String(loadvoltage) + " V" + 
              " Current: " + String(current_mA) + " mA" + 
              " Power: " + String(power_mW) + " mW" , DBG_VERBOSE, true);
  */
    g_MotorCurrent[sensor] = smoothedCurrent[sensor];
    LastMotorCurrentRead[sensor] = millis();
  }
  return true;
}

/**
 * Checks to see if Battery ACS712 current sensor is connected (and hopefully functionning)
 * 
 * @return true if Battery ACS712 current sensor is ok
 */
bool BatteryCurrentSensorCheck(void)
{
  bool readStatus = BatteryChargeCurrentRead(true);

  DisplayClear();
  DisplayPrint(0, 0, F("Charge sensor Test"));

  if (readStatus)
  {
    DebugPrintln("Charge Sensor , Value: " + String(g_BatteryChargeCurrent, 3), DBG_INFO, true);
    DisplayPrint(2, 2, F("Charge OK: "));
    DisplayPrint(7, 3, String(g_BatteryChargeCurrent, 0) + F(" mA"));
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Battery charge Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2, F("Charge ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to test and read Battery Charge current
 * 
 * * @param Now true to force an immediate read
 * * @return true if charge current could be read
 */
bool BatteryChargeCurrentRead(const bool Now)
{
  static unsigned long LastBatteryChargeCurrentRead = 0;

  if ((millis() - LastBatteryChargeCurrentRead > BATTERY_CHARGE_READ_INTERVAL) || Now)
  {
    int raw1 = ProtectedAnalogRead(PIN_ESP_AMP_CHARGE);
    int raw2 = ProtectedAnalogRead(PIN_ESP_AMP_CHARGE);
    int raw3 = ProtectedAnalogRead(PIN_ESP_AMP_CHARGE);
    int raw = (raw1 + raw2 + raw3) / 3;

    //  DebugPrintln("Raw Charge current value: " + String(raw), DBG_VERBOSE, true);

    if (raw > CHARGE_CURRENT_CHECK_THRESHOLD)
    {
      int voltage = map(raw + CHARGE_CURRENT_OFFSET, 0, 4095, 0, 3300);
      //    DebugPrintln(" Charge current voltage value: " + String(voltage), DBG_VERBOSE, true);

      float current = float(voltage - CHARGE_CURRENT_ZERO_VOLTAGE) / CHARGE_CURRENT_MV_PER_AMP;

      if ((current < CHARGE_CURRENT_DEADBAND) && (current > -CHARGE_CURRENT_DEADBAND))
      {
        current = 0;
      }

      g_BatteryChargeCurrent = current * 1000; //  to convert to mA
      LastBatteryChargeCurrentRead = millis();

      //    DebugPrintln("Charge current value: " + String(g_BatteryChargeCurrent,3), DBG_VERBOSE, true);
      return true;
    }
    else
    {
      g_BatteryChargeCurrent = UNKNOWN_FLOAT;
      return false;
    }
  }
  return true;
}
