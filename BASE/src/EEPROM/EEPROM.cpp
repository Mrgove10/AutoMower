#include "myGlobals_definition.h"
#include "EEPROM/EEPROM.h"
#include "Utils/Utils.h"

/**
 * Sets up EEPROM environement : reads EEPROM into EEPROM RAM image if checksum is valid or initialises EEPROM if checksum invalid
 * 
 */
void EEPROMSetup(void)
{
  DebugPrintln("EEPROMSetup", DBG_VERBOSE);

  int count = 0;

  EEPROM.begin(EEPROM_SIZE);

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    DebugPrintln("EEPROM setup problem", DBG_ERROR);
  }
  else
  {
    DebugPrintln("Get EEPROM value: (size=" + String(EEPROM_SIZE) + " bytes) @ Base Address " + String(EEPROM_BASE_ADDRESS), DBG_INFO);

    for (int i = 0; i < EEPROM_SIZE; i++)
    {
      //      Serial.print(i); Serial.print(" "); Serial.println(EEPROM.readByte(EEPROMBaseAdd+i));

      g_EEPROMLoad.LoadBuffer[i] = EEPROM.read(EEPROM_BASE_ADDRESS + i);

      if (false)
      {
        DebugPrint("[" + String(i) + "]x", DBG_VERBOSE, true);
        if (g_EEPROMLoad.LoadBuffer[i] <= 0X0F)
        {
          DebugPrint("0");
        }
        DebugPrint(String(g_EEPROMLoad.LoadBuffer[i], HEX));
        DebugPrint(" ");
        count = count + 1;
        if (count > 15)
        {
          DebugPrintln("");
          count = 0;
        }
      }
    }
  }

  // calculate checksum
  byte calculatedChecksum = 0;

  for (int i = 0; i < EEPROM_SIZE - 1; i++)
  {
    calculatedChecksum = calculatedChecksum + g_EEPROMLoad.LoadBuffer[i];
  }
  calculatedChecksum = (calculatedChecksum & 0x3F) + 0x20;

  if (calculatedChecksum != g_EEPROMLoad.LoadBuffer[EEPROM_SIZE - 1])
  {
    DebugPrintln("EEPROM Checksum = 0x" + String(g_EEPROMLoad.LoadBuffer[EEPROM_SIZE - 1], HEX) + " invalid (should be 0x" + String(calculatedChecksum, HEX) + "): initialising EEPROM !", DBG_ERROR);
    EEPROMInitialise();
    EEPROMWrite();
  }
  else
  {
    DebugPrintln("EEPROM Checksum ok (0x" + String(g_EEPROMLoad.LoadBuffer[EEPROM_SIZE - 1], HEX) + ")", DBG_INFO);

    char buf[128];
    sprintf(buf, "Last EEPROM update Time: %02d/%02d/%04d %02d:%02d:%02d ",
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.day,
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.month,
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.year,
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.hour,
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.minute,
            g_EEPROMLoad.Load.Data.LastEepromSaveTime.second);
    DebugPrintln(String(buf), DBG_INFO);

    g_EEPROMValid = true;

    // Stored States
    g_BaseCurrentState = g_EEPROMLoad.Load.Data.BaseCurrentState;

    // Stored parameters
    g_PerimeterPowerLevel = g_EEPROMLoad.Load.Data.PerimeterPowerLevel;

    // Stored Statistics
    g_totalBaseOnTime = g_EEPROMLoad.Load.Data.totalBaseOnTime;
    g_totalBaseRainTime = g_EEPROMLoad.Load.Data.totalBaseRainTime;

    DebugPrintln("EEPROM value for g_BaseCurrentState: " + String((int) g_BaseCurrentState, 0), DBG_INFO);
    DebugPrintln("EEPROM value for g_PerimeterPowerLevel: " + String(g_PerimeterPowerLevel, 0), DBG_INFO);
    DebugPrintln("EEPROM value for g_totalBaseOnTime: " + String(g_totalBaseOnTime, 0), DBG_INFO);
    DebugPrintln("EEPROM value for g_totalBaseRainTime: " + String(g_totalBaseRainTime, 0), DBG_INFO);
  }
}

/**
 * Flushes content of EEPROM RAM image into EEPROM and commits
 * 
 */
void EEPROMWrite(void)
{
  byte calculatedChecksum = 0;
  int count = 0;

  for (int i = 0; i < EEPROM_SIZE - 1; i++)
  {
    calculatedChecksum = calculatedChecksum + g_EEPROMLoad.LoadBuffer[i];
  }
  calculatedChecksum = (calculatedChecksum & 0x3F) + 0x20;

  g_EEPROMLoad.LoadBuffer[EEPROM_SIZE - 1] = calculatedChecksum;

  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    if (false)
    {
      DebugPrint("[" + String(i) + "]x");
      if (g_EEPROMLoad.LoadBuffer[i] <= 0X0F)
      {
        DebugPrint("0");
      }
      DebugPrint(String(g_EEPROMLoad.LoadBuffer[i], HEX));
      DebugPrint(" ");
      count = count + 1;
      if (count > 15)
      {
        DebugPrintln("");
        count = 0;
      }
    }

    EEPROM.write(EEPROM_BASE_ADDRESS + i, g_EEPROMLoad.LoadBuffer[i]);
    //    Serial.print(i); Serial.print(" "); Serial.print(g_EEPROMLoad.LoadBuffer[i]); Serial.print(" "); Serial.println(EEPROM.readByte(EEPROMBaseAdd+i));
  }

  EEPROM.commit();

  g_LastEepromWriteTime = millis();

  DebugPrintln("EEPROM updated, CheckSum=0x" + String(g_EEPROMLoad.LoadBuffer[EEPROM_SIZE - 1], HEX), DBG_INFO, true);
}

/**
 * Updates EEPROM RAM image before saving to EEPROM (triggers EPPROMWrite)
 * 
 */
void EEPROMSave(boolean immediatly)
{
  if (immediatly || millis() - g_LastEepromWriteTime > EEPROM_WRITE_FREQUENCY)
  {
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.year = myTime.year();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.month = myTime.month();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.day = myTime.day();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.hour = myTime.hour();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.minute = myTime.minute();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.second = myTime.second();

    // Stored States
    g_EEPROMLoad.Load.Data.BaseCurrentState = g_BaseCurrentState;

    // Stored parameters
    g_EEPROMLoad.Load.Data.PerimeterPowerLevel = g_PerimeterPowerLevel;

    // Stored Statistics
    g_EEPROMLoad.Load.Data.totalBaseOnTime = g_totalBaseOnTime;
    g_EEPROMLoad.Load.Data.totalBaseRainTime = g_totalBaseRainTime;

    EEPROMWrite();
  }
}

/**
 * Initialises and RESETs the EEPROM to default initial values (ALL DATA LOST)
 * 
 */
void EEPROMInitialise(void)
{
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.year = myTime.year();
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.month = myTime.month();
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.day = myTime.day();
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.hour = myTime.hour();
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.minute = myTime.minute();
  g_EEPROMLoad.Load.Data.LastEepromSaveTime.second = myTime.second();

  // Stored States
  g_EEPROMLoad.Load.Data.BaseCurrentState = BaseState::sleeping;

  // Stored parameters
  g_EEPROMLoad.Load.Data.PerimeterPowerLevel = 100;       // in %

  // Stored Statistics
  g_EEPROMLoad.Load.Data.totalBaseOnTime = 0;
  g_EEPROMLoad.Load.Data.totalBaseRainTime = 0;

  for (int i = 0; i < EEPROM_SPARE_SIZE - 1; i++)
  {
    g_EEPROMLoad.Load.Sparebuffer[i] = 0x00;
  }
}
