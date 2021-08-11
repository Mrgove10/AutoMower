#include <rom/rtc.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"

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
    if ((g_debugLevelStr == " " && timeStr == "") || append)
    {
      xSemaphoreTake(g_MySerialSemaphore, portMAX_DELAY);
      MySERIAL.print(message);
      xSemaphoreGive(g_MySerialSemaphore);
    }
    else
    {
      xSemaphoreTake(g_MySerialSemaphore, portMAX_DELAY);
      MySERIAL.print(timeStr + "-" + g_debugLevelStr + "-" + message);
      xSemaphoreGive(g_MySerialSemaphore);
    }
    SerialAndTelnet.handle();
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
    return String("No Error");

//General Error conditions

  case ERROR_BATTERY_CRITICAL:
    return String("Battery level below CRITICAL threshold");
  case ERROR_VERTICAL_TILT_ACTIVATED:
    return String("Vertical tilt sensor triggered");
  case ERROR_HORIZONTAL_TILT_ACTIVATED:
    return String("Horizontal tilt sensor triggered");

//States-related Error conditions

  case ERROR_MOWING_NO_START_BUMPER_ACTIVE:
    return String("Mowing cannot start: bumper activated");
  case ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE:
    return String("Mowing cannot start: object too close");
  case ERROR_MOWING_NO_START_TILT_ACTIVE:
    return String("Mowing cannot start: tilt sensor activated");
  case ERROR_MOWING_NO_START_NO_PERIMETER_SIGNAL:
    return String("Mowing cannot start: no perimeter signal");
  case ERROR_WIRE_SEARCH_NO_START_BUMPER_ACTIVE:
    return String("Wire search cannot start: bumper activated");
  case ERROR_WIRE_SEARCH_NO_START_OBJECT_TOO_CLOSE:
    return String("Wire search cannot start: object too close");
  case ERROR_WIRE_SEARCH_NO_START_TILT_ACTIVE:
    return String("Wire search cannot start: tilt sensor activated");
  case ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL:
    return String("Wire search cannot start: no perimeter signal");
  case ERROR_WIRE_SEARCH_PHASE_1_FAILLED:
    return String("Wire search phase 1 (reversing) failled to get inside perimeter");
  case ERROR_WIRE_SEARCH_PHASE_2_FAILLED:
    return String("Wire search phase 2 (forward) failled to get outside perimeter");
  case ERROR_WIRE_SEARCH_PHASE_3_FAILLED:
    return String("Wire search phase 3 (turn) failled to get inside perimeter");
  case ERROR_WIRE_SEARCH_PHASE_4_FAILLED:
    return String("Wire search phase 4 (after turn) failled to get inside perimeter");

// Undefined errors

  case ERROR_UNDEFINED:
    return String("ERROR UNDEFINED");
  default:
    return String("ERROR UNDEFINED: Please update ErrorString() function");
  }
}