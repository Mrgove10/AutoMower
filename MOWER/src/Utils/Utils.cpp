#include <rom/rtc.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "EEPROM/EEPROM.h"

// ESP32 Reset reason codes

//char * char_reset_reason(byte reason)
String char_reset_reason(byte reason)
{
  switch (rtc_get_reset_reason(reason))
  {
  case 1:
    return "POWERON_RESET";
    break; /**<1,  Vbat power on reset*/
  case 3:
    return "SW_RESET";
    break; /**<3,  Software reset digital core*/
  case 4:
    return "OWDT_RESET";
    break; /**<4,  Legacy watch dog reset digital core*/
  case 5:
    return "DEEPSLEEP_RESET";
    break; /**<5,  Deep Sleep reset digital core*/
  case 6:
    return "SDIO_RESET";
    break; /**<6,  Reset by SLC module, reset digital core*/
  case 7:
    return "TG0WDT_SYS_RESET";
    break; /**<7,  Timer Group0 Watch dog reset digital core*/
  case 8:
    return "TG1WDT_SYS_RESET";
    break; /**<8,  Timer Group1 Watch dog reset digital core*/
  case 9:
    return "RTCWDT_SYS_RESET";
    break; /**<9,  RTC Watch dog Reset digital core*/
  case 10:
    return "INTRUSION_RESET";
    break; /**<10, Instrusion tested to reset CPU*/
  case 11:
    return "TGWDT_CPU_RESET";
    break; /**<11, Time Group reset CPU*/
  case 12:
    return "SW_CPU_RESET";
    break; /**<12, Software reset CPU*/
  case 13:
    return "RTCWDT_CPU_RESET";
    break; /**<13, RTC Watch dog Reset CPU*/
  case 14:
    return "EXT_CPU_RESET";
    break; /**<14, for APP CPU, reseted by PRO CPU*/
  case 15:
    return "RTCWDT_BROWN_OUT_RESET";
    break; /**<15, Reset when the vdd voltage is not stable*/
  case 16:
    return "RTCWDT_RTC_RESET";
    break; /**<16, RTC Watch dog reset digital core and rtc module*/
  default:
    return "NO_MEAN";
  }
}

/**
 * return a character depending on debug level
 * 
 * @param level     int        Debug message level.
 * @return String based on level
 */
String DebugLevelChar(const int level)
{
  switch (level)
  {
  case DBG_ALWAYS:
    return DBG_ALWAYS_TEXT;
    break;

  case DBG_ERROR:
    return DBG_ERROR_TEXT;
    break;

  case DBG_WARNING:
    return DBG_WARNING_TEXT;
    break;

  case DBG_INFO:
    return DBG_INFO_TEXT;
    break;

  case DBG_DEBUG:
    return DBG_DEBUG_TEXT;
    break;

  case DBG_VERBOSE:
    return DBG_VERBOSE_TEXT;
    break;

  default:
    return "";
  }
}

void DebugPrint(const String message, const int level, const boolean time, const boolean append)
{
  String timeStr = "";
  String g_debugLevelStr = "";

  if (time && !append)
  {
    timeStr = myTime.dateTime("H:i:s");
  }

  g_debugLevelStr = String(DebugLevelChar(level));

  /*
  Serial.print ("Level:"+String(Level));
  Serial.print (" | g_debugLevel:"+String(g_debugLevel));
  Serial.print (" | g_debugLevelStr:<"+g_debugLevelStr+">");
  Serial.println (" | timeStr:<"+timeStr+">");
*/
  if (level <= g_debugLevel)
  {
    xSemaphoreTake(g_MySerialSemaphore, portMAX_DELAY);
    if ((g_debugLevelStr == " " && timeStr == "") || append)
    {
      MySERIAL.print(message);
    }
    else
    {
      MySERIAL.print(timeStr + "-" + g_debugLevelStr + "-" + message);
    }
//    SerialAndTelnet.handle();
    xSemaphoreGive(g_MySerialSemaphore);
  }
}

void DebugPrintln(const String message, const int level, const boolean time, const boolean append)
{
  DebugPrint(message + String("\r\n"), level, time, append);
}

void LogPrintln(const String message, const String tags, const int level)
{
  String timeStr = myTime.dateTime("H:i:s");
  String messageStr = timeStr + "-" + DebugLevelChar(level) + "-" + message;
  DebugPrintln(messageStr, false);
  MQTTSendLogMessage(MQTT_LOG_CHANNEL, messageStr.c_str(), tags.c_str(), level);
}

/**
 * Return a string containing a test description of the error
 *  * 
 * @param errorCode Error message number
 * @return String error description
 */
String ErrorString(const int errorCode)
{
  switch (errorCode)
  {
    case ERROR_NO_ERROR:
      return String(F("No Error"));

      //General Error conditions
    case ERROR_BATTERY_CRITICAL:
      return String(F("Battery level below CRITICAL threshold"));
    case ERROR_VERTICAL_TILT_ACTIVATED:
      return String(F("Vertical tilt sensor triggered"));
    case ERROR_HORIZONTAL_TILT_ACTIVATED:
      return String(F("Horizontal tilt sensor triggered"));
    case ERROR_NO_PERIMETER_SIGNAL:
      return String(F("Perimeter wire signal lost or stopped"));
    case ERROR_SONAR_NOT_UPDATING:
      return String(F("Sonar values or task not updating"));
    case ERROR_UNDEFINED_STEP_ACTION:
      return String(F("Undefined zone step action"));
    case ERROR_PITCH_TO_HIGH:
      return String(F("Pitch too high"));
    case ERROR_ROLL_TO_HIGH:
      return String(F("Roll too high"));
    
      //States-related Error conditions
    case ERROR_MOWING_NO_START_BUMPER_ACTIVE:
      return String(F("Mowing cannot start: bumper activated"));
    case ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE:
      return String(F("Mowing cannot start: object too close"));
    case ERROR_MOWING_NO_START_TILT_ACTIVE:
      return String(F("Mowing cannot start: tilt sensor activated"));
    case ERROR_MOWING_NO_START_NO_PERIMETER_SIGNAL:
      return String(F("Mowing cannot start: no perimeter signal"));
    case ERROR_MOWING_CONSECUTIVE_OBSTACLES:
      return String(F("Mowing Stopped: too many successive obstacles"));
    case ERROR_MOWING_OUTSIDE_TOO_LONG:
      return String(F("Mowing Stopped: Out of perimeter for too long"));
    case ERROR_MOWING_CUT_MOTOR_OVERCURRENT:
      return String(F("Mowing Stopped: Cut motor in over-current"));
    case ERROR_WIRE_SEARCH_NO_START_BUMPER_ACTIVE:
      return String(F("Wire search cannot start: bumper activated"));
    case ERROR_WIRE_SEARCH_NO_START_OBJECT_TOO_CLOSE:
      return String(F("Wire search cannot start: object too close"));
    case ERROR_WIRE_SEARCH_NO_START_TILT_ACTIVE:
      return String(F("Wire search cannot start: tilt sensor activated"));
    case ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL:
      return String(F("Wire search cannot start: no perimeter signal"));
    case ERROR_WIRE_SEARCH_PHASE_1_FAILED:
      return String(F("Wire search PH1 (bckwd) failed to get inside perimeter"));
    case ERROR_WIRE_SEARCH_PHASE_2_FAILED:
      return String(F("Wire search PH2 (fwd) failed to get outside perimeter"));
    case ERROR_WIRE_SEARCH_CONSECUTIVE_OBSTACLES:
      return String(F("Wire search PH2 (fwd) failed: too many successive obstacles"));
    case ERROR_WIRE_SEARCH_PHASE_3_FAILED:
      return String(F("Wire search PH3 (turn) failed to get inside perimeter"));
    case ERROR_FOLLOW_WIRE_NO_START_BUMPER_ACTIVE:
      return String(F("Wire tracking cannot start: bumper activated"));
    case ERROR_FOLLOW_WIRE_NO_START_OBJECT_TOO_CLOSE:
      return String(F("Wire tracking cannot start: object too close"));
    case ERROR_FOLLOW_WIRE_NO_START_TILT_ACTIVE:
      return String(F("Wire tracking cannot start: tilt sensor activated"));
    case ERROR_FOLLOW_WIRE_NO_START_NO_PERIMETER_SIGNAL:
      return String(F("Wire tracking cannot start: no perimeter signal"));
    case ERROR_FOLLOW_WIRE_CONSECUTIVE_OBSTACLES:
      return String(F("Wire tracking Stopped: too many successive obstacles"));
    case ERROR_FOLLOW_WIRE_OUTSIDE_TOO_LONG:
      return String(F("Wire tracking Stopped: outside perimeter for too long"));
    case ERROR_LEAVING_NO_START_NO_PERIMETER_SIGNAL:
      return String(F("Leaving Base cannot start: no perimeter signal"));
    case ERROR_DOCKED_REPOSITION_FAILED:
      return String(F("Repositioning on Base failed"));

      // Undefined errors
    case ERROR_UNDEFINED:
      return String(F("ERROR UNDEFINED"));
    default:
      return String(F("ERROR UNDEFINED: Please update ErrorString() function"));
  }
}

/**
 * Set the value of a parameter and save to EEPROM
 *  * 
 * @param parameterCode Code of paramater as a string
 * @param parameterValue Value of paramater as a float
 * @return true is parameter was updated anf false if not (ParmaterCode invalid)
 */
bool ParameterChangeValue(const String parameterCode, const float parameterValue)
{
  bool found = true;

  if (parameterCode == "PerimTtrkngKp")
  {
    g_ParamPerimeterTrackPIDKp = parameterValue;
  }
  else if (parameterCode == "PerimTtrkngKi")
  {
    g_ParamPerimeterTrackPIDKi = parameterValue;
  }
  else if (parameterCode == "PerimTtrkngKd")
  {
    g_ParamPerimeterTrackPIDKd = parameterValue;
  }
  else if (parameterCode == "PerimTtrkSetPt")
  {
    g_PerimeterTrackSetpoint = parameterValue;
  }
  else if (parameterCode == "PerimLostThresld")
  {
    g_PerimeterSignalLostThreshold = parameterValue;
  }
  else if (parameterCode == "PerimTtrkLowThresld")
  {
    g_PerimeterSignalLowTrackThreshold = parameterValue;
  }
  else if (parameterCode == "DockRepositioningEnabled")
  {
    g_DockRepositioningEnabled = (parameterValue == 1);
    if (g_DockRepositioningEnabled)
    {
      LogPrintln("Dock repositioning ENABLED", TAG_VALUE, DBG_INFO);
    }
    else
    {
      LogPrintln("Dock repositioning DISABLED", TAG_VALUE, DBG_INFO);
    }
  }
  else // Paramater Code not configured or found
  {
    found = false;
  }

  if (found)
  {
    LogPrintln("Parameter " + parameterCode + " updated to " + String(parameterValue, 3), TAG_PARAM, DBG_INFO);
    EEPROMSave(true);
  }
  else
  {
    DebugPrintln("Parameter " + parameterCode + " not found or configured, check code or update ParameterChangeValue() function !", DBG_ERROR, true);
  }
  return found;
}

/**
 * Display RTOS Tasks status information 
 * 
 * @param task Name of task to display status, or "*" for all application tasks
 */
void DisplayTaskStatus(const String task)
{
  int taskcount = 5;
  String taskName[taskcount] = {"Current", FAST_ANA_READ_TASK_NAME, PERIMETER_TASK_NAME, SONAR_READ_TASK_NAME, ANA_READ_TASK_NAME};
  TaskHandle_t xHandle[taskcount] = {xTaskGetCurrentTaskHandle() , g_FastAnaReadTaskHandle, g_PerimeterProcTaskHandle, g_SonarReadTaskHandle, g_AnaReadTaskHandle};
  eTaskState state;
  unsigned int minMem;
  unsigned long startMillis = millis();

  if (task != "*")
  {
    taskcount = 1;
    taskName[0] = task;

    if (task == FAST_ANA_READ_TASK_NAME)
    {
      xHandle[0] = g_FastAnaReadTaskHandle;
    }
    else if (task == PERIMETER_TASK_NAME)
    {
      xHandle[0] = g_PerimeterProcTaskHandle;
    }
    else if (task == SONAR_READ_TASK_NAME)
    {
      xHandle[0] = g_SonarReadTaskHandle;
    }
    else if (task == ANA_READ_TASK_NAME)
    {
      xHandle[0] = g_AnaReadTaskHandle;
    }
    else
    {
      xHandle[0] = xTaskGetCurrentTaskHandle();
    }
  }

  for (int i = 0; i < taskcount; i++)
  {
    state = eTaskGetState(xHandle[i]);
    minMem = uxTaskGetStackHighWaterMark(xHandle[i]);
    DebugPrintln("Task " + taskName[i] + " | State: " + String(taskStateStr(state)) + " | Min stack: " + String(minMem), DBG_VERBOSE, true);
  }

  if (task == "*")
  {
    DebugPrint("Total Tasks: " + String(uxTaskGetNumberOfTasks()), DBG_VERBOSE, true);
  }

  DebugPrintln(" (" + String(millis()-startMillis) + "ms, Heap: " + String(esp_get_free_heap_size()) 
               + ", Temp:" + String(temperatureRead(), 1) + " deg"
               + ", Freq:" + String(getCpuFrequencyMhz()) + " MHz)", DBG_VERBOSE, true, true);
}

/**
 * Display RTOS Tasks state as human readable string 
 * 
 * @param state state of task to display readable status
 */

String taskStateStr(const eTaskState state)
{
  switch (state)
  {
    case eReady:
      return "Ready";
    case eRunning:
      return "Running";
    case eBlocked:
      return "Blocked";
    case eDeleted:
      return "Deleted";
    case eSuspended:
      return "Suspended";
    default:
      return "Unknown";
  }
}

/**
 * Set ESP frequency. Possible values are 240, 160, 80 and 40 MHz 
 * 
 * @param frequency CPU frequency to set (in MHz)
 */
void setCPUFreq(const int frequency)
{
  setCpuFrequencyMhz(frequency);
  DebugPrintln("CPU Frequency set to " + String(getCpuFrequencyMhz()) + "  MHz", DBG_INFO, true);
}

bool MyIdleHook_0( void )
{
/* This hook function does nothing but increment a counter. */
 g_IdleCycleCount[0] ++;
 g_TotalIdleCycleCount[0] ++;
 return true;
}

bool MyIdleHook_1( void )
{
/* This hook function does nothing but increment a counter. */
 g_IdleCycleCount[1]++;
 g_TotalIdleCycleCount[1] ++;
 return true;
}

/**
 * Protected I2C write to I2C device with address 
 * 
 * @param i2cAddr I2C address
 * @param i2cAddr I2C address
 * @param d byte value to write
 * 
 */
void I2C_write_AddrDev_AddrReg_Byte(byte i2cAddr, byte regaddr, byte d)
{
  // Ensure exclusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

   Wire.beginTransmission(i2cAddr);
   Wire.write(regaddr);
   Wire.write(d);
   Wire.endTransmission();

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);
}
