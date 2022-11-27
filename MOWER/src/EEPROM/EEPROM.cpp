#include "myGlobals_definition.h"
#include "EEPROM/EEPROM.h"
#include "Utils/Utils.h"

/**
 * Sets up EEPROM environment : reads EEPROM into EEPROM RAM image if checksum is valid or initialises EEPROM if checksum invalid
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

    // Stored calibrations
    g_PerimeterOffset = g_EEPROMLoad.Load.Data.PerimeterSignalOffset;
    g_GyroErrorX = g_EEPROMLoad.Load.Data.GyroErrorX;
    g_GyroErrorY = g_EEPROMLoad.Load.Data.GyroErrorY;
    g_GyroErrorZ = g_EEPROMLoad.Load.Data.GyroErrorZ;
    g_AccelErrorX = g_EEPROMLoad.Load.Data.AccelErrorX;
    g_AccelErrorY = g_EEPROMLoad.Load.Data.AccelErrorY;
    g_MPUCalibrationTemperature = g_EEPROMLoad.Load.Data.MPUCalibrationTemp;
    g_CompassMagXOffset = g_EEPROMLoad.Load.Data.CompassMagXOffset;
    g_CompassMagYOffset = g_EEPROMLoad.Load.Data.CompassMagYOffset;
    g_CompassMagZOffset = g_EEPROMLoad.Load.Data.CompassMagZOffset;
    g_CompassMagXScale = g_EEPROMLoad.Load.Data.CompassMagXScale;
    g_CompassMagYScale = g_EEPROMLoad.Load.Data.CompassMagYScale;
    g_CompassMagZScale = g_EEPROMLoad.Load.Data.CompassMagZScale;

    DebugPrintln("EEPROM value for g_GyroErrorX: " + String(g_GyroErrorX, 5), DBG_INFO);
    DebugPrintln("EEPROM value for g_GyroErrorY: " + String(g_GyroErrorY, 5), DBG_INFO);
    DebugPrintln("EEPROM value for g_GyroErrorZ: " + String(g_GyroErrorZ, 5), DBG_INFO);
    DebugPrintln("EEPROM value for g_AccelErrorX: " + String(g_AccelErrorX, 5), DBG_INFO);
    DebugPrintln("EEPROM value for g_AccelErrorY: " + String(g_AccelErrorY, 5), DBG_INFO);
    DebugPrintln("EEPROM value for g_MPUCalibrationTemperature: " + String(g_MPUCalibrationTemperature, 2), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagXOffset: " + String(g_CompassMagXOffset), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagYOffset: " + String(g_CompassMagYOffset), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagZOffset: " + String(g_CompassMagZOffset), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagXScale: " + String(g_CompassMagXScale, 2), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagYScale: " + String(g_CompassMagYScale, 2), DBG_INFO);
    DebugPrintln("EEPROM value for g_CompassMagZScale: " + String(g_CompassMagZScale, 2), DBG_INFO);

    // Stored parameters
    g_ParamPerimeterTrackPIDKp = g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKp;
    g_ParamPerimeterTrackPIDKi = g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKi;
    g_ParamPerimeterTrackPIDKd = g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKd;
    g_PerimeterTrackSetpoint = g_EEPROMLoad.Load.Data.PerimeterTrackSetpoint;
    g_PerimeterSignalLostThreshold = g_EEPROMLoad.Load.Data.PerimeterSignalLostThreshold;
    g_PerimeterSignalLowTrackThreshold = g_EEPROMLoad.Load.Data.PerimeterSignalLowTrackThreshold;
    g_DockRepositioningEnabled = g_EEPROMLoad.Load.Data.DockRepositioningEnabled;

    // Stored Statistics
    g_totalObstacleDetections = g_EEPROMLoad.Load.Data.totalObstacleDetections;
    g_totalMowingTime = g_EEPROMLoad.Load.Data.totalMowingTime;
    g_partialMowingTime = g_EEPROMLoad.Load.Data.partialMowingTime;
    g_operationTime = g_EEPROMLoad.Load.Data.operationTime;
    g_totalChargingTime = g_EEPROMLoad.Load.Data.chargingTime;
    g_totalCurrentUsed = g_EEPROMLoad.Load.Data.totalCurrentUsed;

    // OTA request
    g_otaFlag = g_EEPROMLoad.Load.Data.OTARequested;

    DebugPrintln("EEPROM value for g_totalObstacleDetections: " + String(g_totalObstacleDetections), DBG_INFO);
    DebugPrintln("EEPROM value for g_totalMowingTime: " + String(g_totalMowingTime), DBG_INFO);
    DebugPrintln("EEPROM value for g_partialMowingTime: " + String(g_partialMowingTime), DBG_INFO);
    DebugPrintln("EEPROM value for g_operationTime: " + String(g_operationTime), DBG_INFO);
    DebugPrintln("EEPROM value for g_totalChargingTime: " + String(g_totalChargingTime), DBG_INFO);
    DebugPrintln("EEPROM value for g_totalCurrentUsed: " + String(g_totalCurrentUsed), DBG_INFO);
    DebugPrintln("EEPROM value for g_otaFlag: " + String(g_otaFlag), DBG_INFO);
    DebugPrintln("Repositioning enabled: " + String(g_DockRepositioningEnabled), DBG_INFO);
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
void EEPROMSave(boolean immediately)
{
  if (immediately || millis() - g_LastEepromWriteTime > EEPROM_WRITE_FREQUENCY)
  {
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.year = myTime.year();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.month = myTime.month();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.day = myTime.day();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.hour = myTime.hour();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.minute = myTime.minute();
    g_EEPROMLoad.Load.Data.LastEepromSaveTime.second = myTime.second();

    // Stored calibrations
    g_EEPROMLoad.Load.Data.PerimeterSignalOffset = g_PerimeterOffset;
    g_EEPROMLoad.Load.Data.GyroErrorX = g_GyroErrorX;
    g_EEPROMLoad.Load.Data.GyroErrorY = g_GyroErrorY;
    g_EEPROMLoad.Load.Data.GyroErrorZ = g_GyroErrorZ;
    g_EEPROMLoad.Load.Data.AccelErrorX = g_AccelErrorX;
    g_EEPROMLoad.Load.Data.AccelErrorY = g_AccelErrorY;
    g_EEPROMLoad.Load.Data.MPUCalibrationTemp = g_MPUCalibrationTemperature;
    g_EEPROMLoad.Load.Data.CompassMagXOffset = g_CompassMagXOffset;
    g_EEPROMLoad.Load.Data.CompassMagYOffset = g_CompassMagYOffset;
    g_EEPROMLoad.Load.Data.CompassMagZOffset = g_CompassMagZOffset;
    g_EEPROMLoad.Load.Data.CompassMagXScale = g_CompassMagXScale;
    g_EEPROMLoad.Load.Data.CompassMagYScale = g_CompassMagYScale;
    g_EEPROMLoad.Load.Data.CompassMagZScale = g_CompassMagZScale;

    // Stored parameters
    g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKp = g_ParamPerimeterTrackPIDKp;
    g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKi = g_ParamPerimeterTrackPIDKi;
    g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKd = g_ParamPerimeterTrackPIDKd;
    g_EEPROMLoad.Load.Data.PerimeterTrackSetpoint = g_PerimeterTrackSetpoint;
    g_EEPROMLoad.Load.Data.PerimeterSignalLostThreshold = g_PerimeterSignalLostThreshold;
    g_EEPROMLoad.Load.Data.PerimeterSignalLowTrackThreshold = g_PerimeterSignalLowTrackThreshold;
    g_EEPROMLoad.Load.Data.DockRepositioningEnabled = g_DockRepositioningEnabled;

    // Stored Statistics
    g_EEPROMLoad.Load.Data.totalObstacleDetections = g_totalObstacleDetections;
    g_EEPROMLoad.Load.Data.totalMowingTime = g_totalMowingTime;
    g_EEPROMLoad.Load.Data.partialMowingTime = g_partialMowingTime;
    g_EEPROMLoad.Load.Data.operationTime = g_operationTime;
    g_EEPROMLoad.Load.Data.chargingTime = g_totalChargingTime;
    g_EEPROMLoad.Load.Data.totalCurrentUsed = g_totalCurrentUsed;

    // OTA request
    g_EEPROMLoad.Load.Data.OTARequested = g_otaFlag;

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

  // Stored calibrations
  g_EEPROMLoad.Load.Data.PerimeterSignalOffset = 0;
  g_EEPROMLoad.Load.Data.GyroErrorX = 0;
  g_EEPROMLoad.Load.Data.GyroErrorY = 0;
  g_EEPROMLoad.Load.Data.GyroErrorZ = 0;
  g_EEPROMLoad.Load.Data.AccelErrorX = 0;
  g_EEPROMLoad.Load.Data.AccelErrorY = 0;
  g_EEPROMLoad.Load.Data.MPUCalibrationTemp = 0;
  g_EEPROMLoad.Load.Data.CompassMagXOffset = 0;
  g_EEPROMLoad.Load.Data.CompassMagYOffset = 0;
  g_EEPROMLoad.Load.Data.CompassMagZOffset = 0;
  g_EEPROMLoad.Load.Data.CompassMagXScale = 0;
  g_EEPROMLoad.Load.Data.CompassMagYScale = 0;
  g_EEPROMLoad.Load.Data.CompassMagZScale = 0;

  // Stored parameters
  g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKp = 0;
  g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKi = 0;
  g_EEPROMLoad.Load.Data.ParamPerimeterTrackPIDKd = 0;
  g_EEPROMLoad.Load.Data.PerimeterTrackSetpoint = 0;
  g_EEPROMLoad.Load.Data.PerimeterSignalLostThreshold = 0;
  g_EEPROMLoad.Load.Data.PerimeterSignalLowTrackThreshold = 0;
  g_EEPROMLoad.Load.Data.DockRepositioningEnabled = false;

  // Stored Statistics
  g_EEPROMLoad.Load.Data.totalObstacleDetections = 0;
  g_EEPROMLoad.Load.Data.totalMowingTime = 0;
  g_EEPROMLoad.Load.Data.partialMowingTime = 0;
  g_EEPROMLoad.Load.Data.operationTime = 0;
  g_EEPROMLoad.Load.Data.chargingTime = 0;
  g_EEPROMLoad.Load.Data.totalCurrentUsed = 0;
  
  // OTA request
  g_EEPROMLoad.Load.Data.OTARequested = false;

  for (int i = 0; i < EEPROM_SPARE_SIZE - 1; i++)
  {
    g_EEPROMLoad.Load.Sparebuffer[i] = 0x00;
  }
}
