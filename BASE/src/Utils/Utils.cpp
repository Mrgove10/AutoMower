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
    break; /**<10, Intrusion tested to reset CPU*/
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
    case ERROR_PERIMETER_CURRENT_TOO_LOW:
      return String(F("Perimeter current LOW: No signal (cut wire or other)"));
    case ERROR_PERIMETER_CURRENT_TOO_HIGH:
      return String(F("Perimeter current HIGH: abnormal"));
    case ERROR_TEMPERATURE_TOO_HIGH:
      return String(F("Base temperature too HIGH: No signal"));
    case ERROR_NO_MOWER_DATA:
      return String(F("Base received no mower communications"));

      //States-related Error conditions

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
  float checkedValue = parameterValue;

  if (parameterCode == "PerimeterPowerLevel")
  {
    checkedValue = min(100.0f, checkedValue);
    checkedValue = max(0.0f, checkedValue);
    g_PerimeterPowerLevel = checkedValue;
  }
  else if (parameterCode == "Dummy")
  {
//    g_Dummy = parameterValue;
  }
  else // Paramater Code not configured or found
  {
    found = false;
  }

  if (found)
  {
    LogPrintln("Parameter " + parameterCode + " updated to " + String(checkedValue, 3), TAG_PARAM, DBG_INFO);
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
  int taskcount = 2;
  String taskName[taskcount] = {"Current", PERIMETER_SEND_TASK_NAME};
  TaskHandle_t xHandle[taskcount] = {xTaskGetCurrentTaskHandle(), g_PerimeterSendTaskHandle};
  eTaskState state;
  unsigned int minMem;
  unsigned long startMillis = millis();

  if (task != "*")
  {
    taskcount = 1;
    
    if (task == PERIMETER_SEND_TASK_NAME)
    {
      xHandle[0] = g_PerimeterSendTaskHandle;
    }
    // else if (task == ANA_READ_TASK_NAME)
    // {
    //   xHandle[0] = g_AnaReadTaskHandle;
    // }
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

  DebugPrintln(" (" + String(millis()-startMillis) + "ms, heap: " + String(esp_get_free_heap_size()) +", temp:" + String(temperatureRead(), 1) + " deg)", DBG_VERBOSE, true, true);
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

bool MyIdleHook_0( void )
{
/* This hook function does nothing but increment a counter. */
 g_TotalIdleCycleCount[0] ++;
 return true;
}

bool MyIdleHook_1( void )
{
/* This hook function does nothing but increment a counter. */
 g_TotalIdleCycleCount[1] ++;
 return true;
}

