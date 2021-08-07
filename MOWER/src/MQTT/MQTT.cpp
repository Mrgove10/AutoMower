#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "Temperature/Temperature.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include "MowerMoves/MowerMoves.h"
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

    JSONDataPayload.setJsonData(messageTemp);
    JSONDataPayload.get(JSONData, "Command");
    String Command = JSONData.stringValue;

    JSONDataPayload.get(JSONData, "Val1");
    String Val1Str = JSONData.stringValue;
    float Val1 = Val1Str.toFloat();

    JSONDataPayload.get(JSONData, "Val2");
    String Val2Str = JSONData.stringValue;
    float Val2 = Val2Str.toFloat();

    if (Command == "OTA")
    {
      LogPrintln("Request for OTA update", TAG_OTA, DBG_INFO);
      g_otaFlag = true;
      OTAHandle();
    }

    else if (Command == "TEST")
    {
      LogPrintln("Request for AutoMower Test", TAG_CHECK, DBG_INFO);
      StartupChecks();
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

    else if (Command == "TEST_MOTOR")
    {
      MotionMotorTest(MOTION_MOTOR_RIGHT); //TEMPORAIRE
      MotionMotorTest(MOTION_MOTOR_LEFT);  //TEMPORAIRE
    }

    else if (Command == "TEST_CUTMOTOR")
    {
      CutMotorTest();
    }

    else if (Command == "TEST_STOP")
    {
      MowerStop();
    }

    else if (Command == "TEST_FORWARD")
    {
      MowerForward(int(Val1));
      delay(int(Val2 * 1000));
      MowerStop();
    }

    else if (Command == "TEST_REVERSE")
    {
      MowerReverse(int(Val1), int(Val2 * 1000));
    }

    else if (Command == "TEST_TURN")
    {
      MowerTurn(int(Val1), bool(Val2));
    }

    else if (Command == "STATE_CHANGE")
    {
      DebugPrintln("State Change to " + Val1Str, DBG_INFO, true);

      if (Val1Str == "IDLE")
      {
        g_CurrentState = MowerState::idle;
      }
      else if (Val1Str == "DOCKED")
      {
        g_CurrentState = MowerState::docked;
      }
      else if (Val1Str == "MOWING")
      {
        g_CurrentState = MowerState::mowing;
      }
      else if (Val1Str == "TO_BASE")
      {
        g_CurrentState = MowerState::going_to_base;
      }
      else if (Val1Str == "FROM_BASE")
      {
        g_CurrentState = MowerState::leaving_base;
      }
      else if (Val1Str == "ERROR")
      {
        g_CurrentState = MowerState::error;
      }
      else if (Val1Str == "TEST")
      {
        g_CurrentState = MowerState::test;
      }
    }

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
  DebugPrint("Disconnected from MQTT server", DBG_WARNING, true);
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
      DisplayPrint(2, 3, F("Connected"));
    }
    else
    {
      DisplayPrint(2, 3, F("FAILED"));
      delay(TEST_SEQ_STEP_ERROR_WAIT);
    }
    delay(TEST_SEQ_STEP_WAIT);
  }
}

void MQTTSendTelemetry()
{
  static unsigned long LastTelemetryDataSent = 0;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  if ((millis() - LastTelemetryDataSent > MQTT_TELEMETRY_SEND_INTERVAL))
  {
    unsigned long StartSend = millis();

    byte publ = 0;

    JSONDataPayload.clear();

    JSONDataPayload.add("BatVolt", String(float(g_BatteryVotlage / 1000.0f), 2));
    JSONDataPayload.add("ChargeCur", String(g_BatteryChargeCurrent, 2));

    JSONDataPayload.add("DrvMotTemp", String(g_Temperature[TEMPERATURE_2_BLUE], 1));
    JSONDataPayload.add("DrvMotTempEr", String(g_TempErrorCount[TEMPERATURE_2_BLUE]));
    JSONDataPayload.add("RMotCur", String(g_MotorCurrent[MOTOR_CURRENT_RIGHT]));
    JSONDataPayload.add("RMotSpd", String(g_MotionMotorSpeed[MOTION_MOTOR_RIGHT] * g_MotionMotorDirection[MOTION_MOTOR_RIGHT]));
    JSONDataPayload.add("LMotCur", String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 2));
    JSONDataPayload.add("LMotSpd", String(g_MotionMotorSpeed[MOTION_MOTOR_LEFT] * g_MotionMotorDirection[MOTION_MOTOR_LEFT]));
    JSONDataPayload.add("DrvMotFan", String(g_FanOn[FAN_2_BLUE]));

    JSONDataPayload.add("CMotTemp", String(g_Temperature[TEMPERATURE_1_RED], 1));
    JSONDataPayload.add("CMotTempEr", String(g_TempErrorCount[TEMPERATURE_1_RED]));
    JSONDataPayload.add("CMotCur", String(g_MotorCurrent[MOTOR_CURRENT_CUT], 2));
    JSONDataPayload.add("CMotSpd", String(float(g_CutMotorSpeed * g_CutMotorDirection * 100) / 4096, 2));
    JSONDataPayload.add("CMotAlm", String(g_CutMotorAlarm));
    JSONDataPayload.add("CMotFan", String(g_FanOn[FAN_1_RED]));

    JSONDataPayload.add("FSnrDist", String(g_SonarDistance[SONAR_FRONT]));
    JSONDataPayload.add("RSnrDist", String(g_SonarDistance[SONAR_RIGHT]));
    JSONDataPayload.add("LSnrDist", String(g_SonarDistance[SONAR_LEFT]));

    JSONDataPayload.add("CompHead", String(g_CompassHeading));

    JSONDataPayload.add("GPSHead", String(g_GPSHeading, 1));
    JSONDataPayload.add("GPSSat", String(g_GPSSatellitesFix));
    JSONDataPayload.add("g_GPSHdop", String(g_GPSHdop, 2));
    JSONDataPayload.add("GPSSpd", String(g_GPSSpeed, 2));
    JSONDataPayload.add("GPSAlt", String(g_GPSAltitude, 2));
    JSONDataPayload.add("GPSLat", String(g_GPSLatitude, 2));
    JSONDataPayload.add("GPSLon", String(g_GPSLongitude, 2));

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
        g_TempErrorCount[TEMPERATURE_2_BLUE] = 0;
      }
    }
    else
    {
      LogPrintln("MQTT payload larger than buffer !!!!!!", TAG_ERROR, DBG_ERROR);
    }
    LastTelemetryDataSent = millis();
  }
}
