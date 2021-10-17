#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "EEPROM/EEPROM.h"
#include "Fan/Fan.h"
#include "Temperature/Temperature.h"
#include "StartupChecks.h"
#include "Display/Display.h"

void MQTTSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.subscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
};

void MQTTUnSubscribe()
{
  boolean SubStatus = MQTTclient.unsubscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
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
      LogPrintln("Request for OTA update", TAG_OTA, DBG_INFO);
      g_otaFlag = true;
      OTAHandle();
    }

    else if (Command == "RESTART")
    {
      LogPrintln("Request for AutoMower RESTART", TAG_RESET, DBG_INFO);
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
      LogPrintln("Request for AutoMower Test", TAG_CHECK, DBG_INFO);
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
      LogPrintln("Request for Perimeter On", TAG_CHECK, DBG_INFO);
      g_enableSender = true;
    }

    else if (Command == "PERIMETER_OFF")
    {
      LogPrintln("Request for Perimeter Off", TAG_CHECK, DBG_INFO);
      g_enableSender = false;
    }

    else if (Command == "STATE_CHANGE")
    {
      DebugPrintln("State Change to " + Val1Str + " requested", DBG_INFO, true);

      if (Val1Str == "ACKNOWLEDGE")
      {
        DebugPrintln("Mower Acknowledgement requested (Remote)", DBG_ERROR, true);
        g_CurrentState = MowerState::idle;
        g_CurrentErrorCode = ERROR_NO_ERROR;
      }
      else if (Val1Str == "IDLE")
      {
        g_CurrentState = MowerState::idle;
      }
      else if (Val1Str == "DOCKED")
      {
        g_CurrentState = MowerState::docked;
      }
      else if (Val1Str == "MOWING")
      {
        if (g_CurrentState == MowerState::error)
        {
          DebugPrintln("Mower Acknowledgement required first !", DBG_ERROR, true);
        }
        else
        {
          g_CurrentState = MowerState::mowing;
//          g_mowingMode = Val2;
        }
      }
      else if (Val1Str == "TO_BASE")
      {
        if (g_CurrentState == MowerState::error)
        {
          DebugPrintln("Mower Acknowledgement required first !", DBG_ERROR, true);
        }
        else
        {
          g_CurrentState = MowerState::going_to_base;
        }
      }
      else if (Val1Str == "FROM_BASE")
      {
        g_CurrentState = MowerState::leaving_base;
      }
      else if (Val1Str == "ERROR") // only for testing purposes
      {
        g_CurrentState = MowerState::error;
      }
      else if (Val1Str == "TEST")
      {
        g_CurrentState = MowerState::test;
      }
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
    JSONDataPayload.add("State", String ((int) g_CurrentState));
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
    
    // // Mowing Statistics data
    // JSONDataPayload.add("Obstcl", String(g_totalObstacleDectections));
    // JSONDataPayload.add("MowTim", String(int(g_totalMowingTime/60000)));


    // ESP System data
    JSONDataPayload.add("Heap", String(esp_get_free_heap_size()));
    JSONDataPayload.add("Tasks", String(uxTaskGetNumberOfTasks()));
    JSONDataPayload.add("CPUTemp", String(temperatureRead(), 1));
    JSONDataPayload.add("RSSI", String(WiFi.RSSI()));

    JSONDataPayload.toString(JSONDataPayloadStr, false);
    JSONDataPayloadStr.toCharArray(MQTTpayload, JSONDataPayloadStr.length() + 1);

    DebugPrintln(MQTTpayload + String(JSONDataPayloadStr.length()) + "=> " + String(publ) + " in " + String(millis() - StartSend) + " ms", DBG_VERBOSE, true);

    if (JSONDataPayloadStr.length() < MQTT_MAX_PAYLOAD)
    {
      publ = MQTTclient.publish(MQTT_TELEMETRY_CHANNEL, MQTTpayload);
      LastTelemetryDataSent = millis();
      if (publ == 1)
      {
        g_TempErrorCount[TEMPERATURE_1_RED] = 0;
      }
    }
    else
    {
      LogPrintln("MQTT payload larger than buffer !!!!!!", TAG_ERROR, DBG_ERROR);
    }
    LastTelemetryDataSent = millis();
  }
}
