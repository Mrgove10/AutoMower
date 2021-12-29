#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "EEPROM/EEPROM.h"
#include "Fan/Fan.h"
#include "Temperature/Temperature.h"
#include "PerimeterSendTsk/PerimeterSendTsk.h"
#include "StartupChecks.h"
#include "Display/Display.h"

void MQTTSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.subscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
  SubStatus = MQTTclient.subscribe(MQTT_MOWER_TELEMETRY_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_MOWER_TELEMETRY_CHANNEL + String("=") + String(SubStatus));
};

void MQTTUnSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.unsubscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
  SubStatus = MQTTclient.unsubscribe(MQTT_MOWER_TELEMETRY_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_MOWER_TELEMETRY_CHANNEL + String("=") + String(SubStatus));
};

FirebaseJson JSONNotePayload;

void MQTTSendLogMessage(const char *MQTTTopic, const char *Message, const char *Tag, const int Level)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  JSONNotePayload.clear();

  JSONNotePayload.add("Sender", ESPHOSTNAME);
  JSONNotePayload.add("Message", Message);
  JSONNotePayload.add("Tags", Tag);
  JSONNotePayload.add("Level", Level);

  JSONNotePayload.toString(JSONNotePayloadStr, false);
  JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

  bool result = MQTTclient.publish(MQTTTopic, MQTTpayload);
  if (result != 1)
  {
    g_MQTTErrorCount = g_MQTTErrorCount + 1;
  }

  MQTTclient.loop();

  DebugPrintln("Sending to :[" + String(MQTTTopic) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
}

void MQTTCallback(char *topic, byte *message, unsigned int length)
{
  static String lastCommand;

  DebugPrintln("Start of MQTTCallback...", DBG_VERBOSE, true);

  /*
  display.clear();
  display.drawCircle(0,0,3);
  display.display();
  delay(150);
  display.drawCircle(0,0,6);
  display.display();
  delay(150);
  display.drawCircle(0,0,9);
  display.display();
  delay(300);
  display.clear();
  display.display();
*/

  String messageTemp;

  DebugPrint("Message arrived on topic: ", DBG_INFO, true);
  DebugPrint(topic);
  DebugPrint(". Message: ");

  for (unsigned int i = 0; i < length; i++)
  {
    DebugPrint(String((char)message[i]));
    messageTemp += (char)message[i];
  }
  DebugPrintln("");

//-------------------------------------------------------------------------
//
// MQTT_COMMAND_CHANNEL
//
//-------------------------------------------------------------------------

  if (String(topic) == String(MQTT_COMMAND_CHANNEL))
  {
    FirebaseJson JSONDataPayload;
    FirebaseJsonData JSONData;
    String JSONDataPayloadStr;

    String Command = "";
    String Val1Str = "";
    float Val1 = UNKNOWN_FLOAT;
    String Val2Str = "";
    float Val2 = UNKNOWN_FLOAT;

    JSONDataPayload.setJsonData(messageTemp);

    // Extract Command value from JSON structure
    JSONDataPayload.get(JSONData, "Command");
    if (JSONData.success)
    {
      Command = JSONData.stringValue;
      DebugPrintln ("Command: " + Command, DBG_DEBUG, true);
    }

    // Extract Val1 value from JSON structure
    JSONDataPayload.get(JSONData, "Val1");
    if (JSONData.success) 
    {
      Val1Str = JSONData.stringValue;
      Val1 = Val1Str.toFloat();
      DebugPrintln(" Val1: <" + Val1Str + "> [" + String(Val1, 5) + "]", 0, false, true);
    }

    // Extract Val2 valu from JSON structure

    JSONDataPayload.get(JSONData, "Val2");
    if (JSONData.success) 
    {
      Val2Str = JSONData.stringValue;
      Val2 = Val2Str.toFloat();
      DebugPrintln("Val2: <" + Val2Str + "> [" + String(Val2, 5) + "]", 0, false, true);
    }

//-------------------------------------------------------------------------
//
// Permanent functions
//
//-------------------------------------------------------------------------

    if (Command == "OTA")
    {
      LogPrintln("Request for Base OTA update", TAG_OTA, DBG_INFO);
      g_otaFlag = true;
      OTAHandle();
    }

    else if (Command == "RESTART")
    {
      LogPrintln("Request for Base RESTART", TAG_RESET, DBG_INFO);
      FanStop(FAN_1_RED);
//      CutMotorStop();
      EEPROMSave(true);
      SerialAndTelnet.handle(); // Refresh Telnet Session
      delay(500);
      WiFi.disconnect();
      delay(2000);
      ESP.restart();
    }

    else if (Command == "TEST")
    {
      LogPrintln("Request for Base Test", TAG_CHECK, DBG_INFO);
      StartupChecks(true);
    }

    else if (Command == "DEBUG")
    {
      if (Val1Str == "VERBOSE")
      {
        g_debugLevel = DBG_VERBOSE;
        DebugPrintln("Debug level to VERBOSE", DBG_INFO, true);
      }
      else if (Val1Str == "DEBUG")
      {
        g_debugLevel = DBG_DEBUG;
        DebugPrintln("Debug level to DEBUG", DBG_INFO, true);
      }
      else if (Val1Str == "INFO")
      {
        g_debugLevel = DBG_INFO;
        DebugPrintln("Debug level to INFO", DBG_INFO, true);
      }
    }

    else if (Command == "PERIMETER_ON")
    {
      LogPrintln("MQTT Request for Perimeter On", TAG_CHECK, DBG_INFO);
      PerimeterSignalStart();
    }

    else if (Command == "PERIMETER_OFF")
    {
      LogPrintln("MQTT Request for Perimeter Off", TAG_CHECK, DBG_INFO);
      PerimeterSignalStop();
    }

    else if (Command == "STATE_CHANGE")
    {
      DebugPrintln("Base state Change to " + Val1Str + " requested", DBG_INFO, true);

      if (Val1Str == "ACKNOWLEDGE")
      {
        DebugPrintln("Base Acknowledgement requested (Remote)", DBG_ERROR, true);
        g_BaseCurrentState = BaseState::idle;
        g_CurrentErrorCode = ERROR_NO_ERROR;
        g_LastMowerTelemetryReceived = millis();
      }
      else if (Val1Str == "IDLE")
      {
        g_BaseCurrentState = BaseState::idle;
      }
      else if (Val1Str == "SENDING")
      {
        if (g_BaseCurrentState == BaseState::error)
        {
          DebugPrintln("Base Acknowledgement required first !", DBG_ERROR, true);
        }
        else
        {
          g_BaseCurrentState = BaseState::sending;
        }
      }
      else if (Val1Str == "SLEEPING")
      {
        if (g_BaseCurrentState == BaseState::error)
        {
          DebugPrintln("Base Acknowledgement required first !", DBG_ERROR, true);
        }
        else
        {
          g_BaseCurrentState = BaseState::sleeping;
        }
      }
      else if (Val1Str == "ERROR") // only for testing purposes
      {
        g_BaseCurrentState = BaseState::error;
      }
    }

    else if (Command == "RESET_RAIN_DURATION")
    {
      g_totalBaseRainDuration = 0;
      g_RainStartTime = 0;
      EEPROMSave(true); // Update EEPROM
      LogPrintln("Rain duration counter reset", TAG_VALUE, DBG_INFO);
    }

    else if (Command == "PARAMETER")
    {
      ParameterChangeValue(Val1Str, Val2); // Value changed and saved in EEPROM
    }

//-------------------------------------------------------------------------
//
// Test ONLY functions
//
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//
// End of Test ONLY functions
//
//-------------------------------------------------------------------------

    else
    {
      DebugPrintln(" ...... MQTTcommand not recognised !!!!", DBG_WARNING, true);
    }
  }

//-------------------------------------------------------------------------
//
// MQTT_MOWER_TELEMETRY_CHANNEL
//
//-------------------------------------------------------------------------

  else if (String(topic) == String(MQTT_MOWER_TELEMETRY_CHANNEL))
  {
    FirebaseJson JSONDataPayload;
    FirebaseJsonData JSONData;

    JSONDataPayload.setJsonData(messageTemp);
    g_LastMowerTelemetryReceived = millis();

    // Extract Mower State value from JSON structure
    JSONDataPayload.get(JSONData, "State");
    if (JSONData.success)
    {
      String State = JSONData.stringValue;
      DebugPrintln ("Received Mower State: " + State, DBG_DEBUG, true);
      switch (State.toInt())
      {
        case int(MowerState::idle):
          g_MowerCurrentState = MowerState::idle;
          break;
        case int(MowerState::docked):
          g_MowerCurrentState = MowerState::docked;
          break;
        case int(MowerState::mowing):
          g_MowerCurrentState = MowerState::mowing;
          break;
        case int(MowerState::going_to_base):
          g_MowerCurrentState = MowerState::going_to_base;
          break;
        case int(MowerState::leaving_base):
          g_MowerCurrentState = MowerState::leaving_base;
          break;
        case int(MowerState::error):
          g_MowerCurrentState = MowerState::error;
          break;
        case int(MowerState::test):
          g_MowerCurrentState = MowerState::test;
          break;
        default:
          DebugPrintln ("Mower State not recognised: [" + String(State.toInt()) + "]", DBG_ERROR, true);
          break; 
      }
      DebugPrintln ("Saved Mower State: " + String((int) g_MowerCurrentState), DBG_VERBOSE, true);
    }

    // Extract Battery Charge current value from JSON structure
    JSONDataPayload.get(JSONData, "ChargeCur");
    if (JSONData.success)
    {
      String ChargeCurrent = JSONData.stringValue;
      g_MowerChargeCurrent = ChargeCurrent.toFloat();
      DebugPrintln ("Saved Mower Charge current: " + String(g_MowerChargeCurrent, 2) + " mA", DBG_VERBOSE, true);
    }

    // Extract Battery SOC value from JSON structure
    JSONDataPayload.get(JSONData, "BatSOC");
    if (JSONData.success)
    {
      String BatterySOC = JSONData.stringValue;
      g_MowerBatterySOC = BatterySOC.toInt();
      DebugPrintln ("Saved Mower Battery SOC: " + String(g_MowerBatterySOC) + " %", DBG_VERBOSE, true);
    }
    else
    {
      g_MowerBatterySOC = UNKNOWN_INT;
    }

  }

  /* other channel */

  /*
  if (String(topic) == String(XXXXXXXXX)) {

// do something

  }
*/
  DebugPrintln("End of MQTTCallback...", DBG_VERBOSE, true);
}

void MQTTReconnect()
{
  // Loop until we're reconnected
  if (!MQTTclient.connected())
  {
    DebugPrint("Attempting MQTT connection...", DBG_ALWAYS, true);
    // Attempt to connect
    if (MQTTclient.connect(ESPHOSTNAME))
    {
      DebugPrintln("connected");
      // Subscribe
      MQTTSubscribe();
    }
    else
    {
      DebugPrint("failed, rc=", DBG_ERROR, true);
      DebugPrint(String(MQTTclient.state()));
      DebugPrintln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      g_MQTTErrorCount = g_MQTTErrorCount + 1;
      delay(5000);
    }
  }
}

void MQTTDisconnect(void)
{
  MQTTclient.disconnect();
  DebugPrintln("Disconnected from MQTT server", DBG_WARNING, true);
}

void MQTTInit(const bool Display)
{
  if (Display)
  {
    DisplayPrint(0, 2, F("Server link ..."));
  }

  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);
  MQTTclient.setBufferSize(MQTT_MAX_PAYLOAD);
  MQTTclient.setCallback(MQTTCallback);

  MQTTReconnect();

  bool status;
  IPAddress ip = WiFi.localIP();
  String Host = ESPHOSTNAME + ip[3];
  status = MQTTclient.connect(Host.c_str());

  if (Display)
  {
    if (status)
    {
      DisplayPrint(8, 3, F("Connected"));
    }
    else
    {
      DisplayPrint(2, 3, F("FAILED"));
      delay(TEST_SEQ_STEP_ERROR_WAIT);
    }
    delay(TEST_SEQ_STEP_WAIT);
  }
}

void MQTTSendTelemetry(const bool now)
{
  static unsigned long LastTelemetryDataSent = 0;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  if ((millis() - LastTelemetryDataSent > g_MQTTSendInterval) || now)
  {
    unsigned long StartSend = millis();

    byte publ = 0;

    JSONDataPayload.clear();

    // Status Data
    JSONDataPayload.add("State", String ((int) g_BaseCurrentState));
    JSONDataPayload.add("Error", String (g_CurrentErrorCode));
    JSONDataPayload.add("SuppVolt", String(float(g_PwrSupplyVoltage / 1000.0f), 3));

    // Perimeter data
    JSONDataPayload.add("PerimCur", String(g_PerimeterCurrent, 0));
    JSONDataPayload.add("PerimVolt", String(g_PerimeterVoltage, 3));
    JSONDataPayload.add("PerimLevel", String(g_PerimeterPowerLevel, 0));
    JSONDataPayload.add("PerimOn", String(g_enableSender));

    JSONDataPayload.add("BaseTemp", String(g_Temperature[TEMPERATURE_1_RED],1));
    JSONDataPayload.add("BaseTempErr", String(g_TempErrorCount[TEMPERATURE_1_RED]));
    JSONDataPayload.add("BaseFan", String(g_FanOn[FAN_1_RED]));
    JSONDataPayload.add("RainLvl", String(g_RainLevel, 2));
    JSONDataPayload.add("IsRainning", String(g_IsRainning));
    
    // Base Statistics data
    JSONDataPayload.add("OnTime", String(int(g_totalBaseOnTime/60000)));
    JSONDataPayload.add("RainDuration", String(int(g_totalBaseRainDuration/60000)));

    // ESP System data
    JSONDataPayload.add("Heap", String(esp_get_free_heap_size()));
    JSONDataPayload.add("Tasks", String(uxTaskGetNumberOfTasks()));
    JSONDataPayload.add("CPUTemp", String(temperatureRead(), 1));
    JSONDataPayload.add("RSSI", String(WiFi.RSSI()));
    JSONDataPayload.add("MQTTErr", String(g_MQTTErrorCount));
    JSONDataPayload.add("CPU0IdleCnt", String(float(g_TotalIdleCycleCount[0]) / float(millis() - LastTelemetryDataSent), 3));
    JSONDataPayload.add("CPU1IdleCnt", String(float(g_TotalIdleCycleCount[1]) / float(millis() - LastTelemetryDataSent), 3));

    JSONDataPayload.toString(JSONDataPayloadStr, false);
    JSONDataPayloadStr.toCharArray(MQTTpayload, JSONDataPayloadStr.length() + 1);

    DebugPrintln(MQTTpayload + String(JSONDataPayloadStr.length()) + "=> " + String(publ) + " in " + String(millis() - StartSend) + " ms", DBG_VERBOSE, true);

    if (JSONDataPayloadStr.length() < MQTT_MAX_PAYLOAD)
    {
      publ = MQTTclient.publish(MQTT_BASE_TELEMETRY_CHANNEL, MQTTpayload);
      LastTelemetryDataSent = millis();
      if (publ == 1)
      {
        g_TempErrorCount[TEMPERATURE_1_RED] = 0;
        g_MQTTErrorCount = 0;
        g_TotalIdleCycleCount[0] = 0;
        g_TotalIdleCycleCount[1] = 0;
      }
      else
      {
        g_MQTTErrorCount = g_MQTTErrorCount + 1;
      }
    }
    else
    {
      LogPrintln("MQTT payload larger than buffer !!!!!!", TAG_ERROR, DBG_ERROR);
    }
    LastTelemetryDataSent = millis();
  }
}


/**
 * Send Perimeter signal status on MQTT channel
 * @param now boolean indicating the sending is to be performed immediatly
 */
void PerimeterSignalStatusSend(const bool now = false)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];
  static unsigned long LastStatusDataSent = 0;

  if ((millis() - LastStatusDataSent > MQTT_PERIMETER_STATUS_SEND_INTERVAL) || now)
  {

    JSONNotePayload.clear();

    JSONNotePayload.add("PerimeterStatus", int(g_enableSender));
    JSONNotePayload.toString(JSONNotePayloadStr, false);
    JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

    bool result = MQTTclient.publish(MQTT_PERIMETER_STATUS_CHANNEL, MQTTpayload);
    if (result != 1)
    {
      g_MQTTErrorCount = g_MQTTErrorCount + 1;
    }

    MQTTclient.loop();

    DebugPrintln("Sending to :[" + String(MQTT_PERIMETER_STATUS_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
    LastStatusDataSent = millis();
  }
}

/**
 * Send rain status on MQTT channel
 * @param now boolean indicating the sending is to be performed immediatly
 */
void BaseRainStatusSend(const bool now = false)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];
  static unsigned long LastStatusDataSent = 0;

  if ((millis() - LastStatusDataSent > MQTT_RAIN_STATUS_SEND_INTERVAL) || now)
  {

    JSONNotePayload.clear();

    JSONNotePayload.add("RainStatus", int(g_IsRainning));
    JSONNotePayload.toString(JSONNotePayloadStr, false);
    JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

    bool result = MQTTclient.publish(MQTT_RAIN_STATUS_CHANNEL, MQTTpayload);
    if (result != 1)
    {
      g_MQTTErrorCount = g_MQTTErrorCount + 1;
    }

    MQTTclient.loop();

    DebugPrintln("Sending to :[" + String(MQTT_RAIN_STATUS_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
    LastStatusDataSent = millis();
  }
}

