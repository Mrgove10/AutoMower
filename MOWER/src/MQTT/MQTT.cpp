#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "EEPROM/EEPROM.h"
#include "Fan/Fan.h"
#include "Temperature/Temperature.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include "MowerMoves/MowerMoves.h"
#include "StartupChecks.h"
#include "Display/Display.h"
#include "PerimeterTsk/PerimeterTsk.h"
#include "GyroAccel/GyroAccel.h"

void MQTTSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.subscribe(MQTT_MOWER_COMMAND_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_MOWER_COMMAND_CHANNEL + String("=") + String(SubStatus));
  
  SubStatus = MQTTclient.subscribe(MQTT_BASE_PERIMETER_STATUS_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_BASE_PERIMETER_STATUS_CHANNEL + String("=") + String(SubStatus));

  SubStatus = MQTTclient.subscribe(MQTT_BASE_RAIN_STATUS_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_BASE_RAIN_STATUS_CHANNEL + String("=") + String(SubStatus));
};

void MQTTUnSubscribe()
{
  boolean SubStatus;
  SubStatus = MQTTclient.unsubscribe(MQTT_MOWER_COMMAND_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_MOWER_COMMAND_CHANNEL + String("=") + String(SubStatus));

  SubStatus = MQTTclient.unsubscribe(MQTT_BASE_PERIMETER_STATUS_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_BASE_PERIMETER_STATUS_CHANNEL + String("=") + String(SubStatus));

  SubStatus = MQTTclient.unsubscribe(MQTT_BASE_RAIN_STATUS_CHANNEL);
  DebugPrintln(String("UnSubscribeStatus ") + MQTT_BASE_RAIN_STATUS_CHANNEL + String("=") + String(SubStatus));
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
// MQTT_MOWER_COMMAND_CHANNEL
//
//-------------------------------------------------------------------------

  if (String(topic) == String(MQTT_MOWER_COMMAND_CHANNEL))
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
      FanStop(FAN_1_RED);
      FanStop(FAN_2_BLUE);
      MowerStop();
      CutMotorStop();

      g_otaFlag = true;

      EEPROMSave(true);
      SerialAndTelnet.handle(); // Refresh Telnet Session
      delay(500);
      WiFi.disconnect();
      delay(500);
      ESP.restart();
    }

    else if (Command == "RESTART")
    {
      LogPrintln("Request for AutoMower RESTART", TAG_RESET, DBG_INFO);
      FanStop(FAN_1_RED);
      FanStop(FAN_2_BLUE);
      MowerStop();
      CutMotorStop();
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
          g_mowingMode = Val2;
          g_ZoneMowDuration = MOWER_MOWING_MOWING_SESSION_DURATION * 1000;
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
        if (Val2 >= MAXMOWERZONES)
        {
          DebugPrintln("Mower zone not defined !", DBG_ERROR, true);
        }
        else if (g_CurrentState != MowerState::docked)
        {
          DebugPrintln("Mower needs to be at base first !", DBG_ERROR, true);
        }
        else
        {
          g_CurrentState = MowerState::leaving_base;
          g_TargetNowingZone = int(Val2);
        }
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

    else if (Command == "CALIBRATE")
    {
      if (Val1Str == "PERIMETER")
      {
        PerimeterRawValuesCalibration(PERIMETER_RAW_SAMPLES);
        DebugPrintln("Calibration offset changed to " + String(g_PerimeterOffset), DBG_INFO, true);
      }
      else if (Val1Str == "GYRO")
      {
        if (g_CurrentState == MowerState::idle || g_CurrentState == MowerState::docked)
        {
          GyroErrorCalibration(GYRO_CALIBRATION_SAMPLES);
        }
        else
        {
          DebugPrintln("Gyro calibration can only be done in Idle or docked states", DBG_ERROR, true);
        }
      }
      // else if (Val1Str == "ACCEL")
      // {
      //   AccelErrorCalibration(ACCEL_CALIBRATION_SAMPLES);
      // }
      else
      {
        DebugPrintln("Calibration value unspecified", DBG_ERROR, true);
      }
    }

    else if (Command == "RESET_CHARGE_DURATION")
    {
      g_totalChargingTime = 0;
      g_BatteryChargingStartTime = millis();
      EEPROMSave(true); // Update EEPROM
      LogPrintln("Charge duration counter reset", TAG_VALUE, DBG_INFO);
    }

    else if (Command == "RESET_PARTIAL_MOWING")
    {
      g_partialMowingTime = 0;
      EEPROMSave(true); // Update EEPROM
      LogPrintln("Partial mowing time counter reset", TAG_VALUE, DBG_INFO);
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
      if (g_CurrentState != MowerState::idle)
      {
        DebugPrintln("Mower must be in idle mode for manual moves !", DBG_ERROR, true);
      }
      else
      {
        MowerForward(int(Val1));
        delay(int(Val2 * 1000));
        MowerStop();
      }
    }

    else if (Command == "TEST_REVERSE")
    {
      if (g_CurrentState != MowerState::idle)
      {
        DebugPrintln("Mower must be in idle mode for manual moves !", DBG_ERROR, true);
      }
      else
      {
        MowerReverse(int(Val1), int(Val2 * 1000));
      }
    }

    else if (Command == "TEST_TURN")
    {
      if (g_CurrentState != MowerState::idle)
      {
        DebugPrintln("Mower must be in idle mode for manual moves !", DBG_ERROR, true);
      }
      else
      {
        MowerTurn(int(Val1), bool(Val2));
      }
    }

    else if (Command == "TEST_ARC_FORWARD")
    {
      MowerArc(MOTION_MOTOR_FORWARD, int(Val1), int(Val2));
    }

#ifdef MQTT_GRAPH_DEBUG
    else if (Command == "START_MQTT_GRAPH_DEBUG")
    {
      g_MQTTGraphDebug = true;
    }
    else if (Command == "STOP_MQTT_GRAPH_DEBUG")
    {
      g_MQTTGraphDebug = false;
    }
    else if (Command == "START_MQTT_GRAPH_RAW_DEBUG")
    {
      g_MQTTGraphRawDebug = true;
    }
    else if (Command == "STOP_MQTT_GRAPH_RAW_DEBUG")
    {
      g_MQTTGraphRawDebug = false;
    }
#endif

#ifdef MQTT_PID_GRAPH_DEBUG
    else if (Command == "START_MQTT_PID_GRAPH_DEBUG")
    {
      g_MQTTPIDGraphDebug = true;
    }
    else if (Command == "STOP_MQTT_PID_GRAPH_DEBUG")
    {
      g_MQTTPIDGraphDebug = false;
    }
#endif

#ifdef MQTT_PITCH_ROLL_DEBUG
    else if (Command == "START_MQTT_PITCH_ROLL_DEBUG")
    {
      g_MQTTPitcRollDebug = true;
    }
    else if (Command == "STOP_MQTT_PITCH_ROLL_DEBUG")
    {
      g_MQTTPitcRollDebug = false;
    }
#endif

    else if (Command == "TUNE_OFFSET")
    {
      g_PerimeterOffset = g_PerimeterOffset + int(Val1); // only for testing purposes
    }

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
// MQTT_BASE_PERIMETER_STATUS_CHANNEL
//
//-------------------------------------------------------------------------

 else if (String(topic) == String(MQTT_BASE_PERIMETER_STATUS_CHANNEL))
  {
    FirebaseJson JSONDataPayload;
    FirebaseJsonData JSONData;

    JSONDataPayload.setJsonData(messageTemp);
    g_LastBaseStatusReceived = millis();

    // Extract Mower State value from JSON structure
    JSONDataPayload.get(JSONData, "PerimeterStatus");
    if (JSONData.success)
    {
      String State = JSONData.stringValue;
      DebugPrintln ("Received Base Perimeter State: " + State, DBG_DEBUG, true);
      g_PerimeterSignalStopped = (State.toInt() == 0);
      DebugPrintln ("Saved perimeter State g_PerimeterSignalStopped :" + String(g_PerimeterSignalStopped), DBG_VERBOSE, true);
    }
  }

//-------------------------------------------------------------------------
//
// MQTT_BASE_RAIN_STATUS_CHANNEL
//
//-------------------------------------------------------------------------

 else if (String(topic) == String(MQTT_BASE_RAIN_STATUS_CHANNEL))
  {
    FirebaseJson JSONDataPayload;
    FirebaseJsonData JSONData;

    JSONDataPayload.setJsonData(messageTemp);
    g_LastBaseStatusReceived = millis();

    // Extract Mower State value from JSON structure
    JSONDataPayload.get(JSONData, "RainStatus");
    if (JSONData.success)
    {
      String State = JSONData.stringValue;
      DebugPrintln ("Received Base Rain State: " + State, DBG_DEBUG, true);
      // TO DO
      // g_PerimeterSignalStopped = (State.toInt() == 0);
      // DebugPrintln ("Saved Rain State: " + String(g_PerimeterSignalStopped), DBG_VERBOSE, true);
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

    // Battery data
    JSONDataPayload.add("BatVolt", String(float(g_BatteryVoltage / 1000.0f), 3));
    JSONDataPayload.add("ChargeCur", String(g_BatteryChargeCurrent, 2));
    JSONDataPayload.add("BatSOC", String(g_BatterySOC,1));
    JSONDataPayload.add("BatCharging", String(g_BatteryIsCharging));
    
    // MotionMotor Data
    JSONDataPayload.add("DrvMotTemp", String(g_Temperature[TEMPERATURE_2_BLUE], 1));
    JSONDataPayload.add("DrvMotTempEr", String(g_TempErrorCount[TEMPERATURE_2_BLUE]));
    JSONDataPayload.add("RMotCur", String(g_MotorCurrent[MOTOR_CURRENT_RIGHT]));
    JSONDataPayload.add("RMotSpd", String(g_MotionMotorSpeed[MOTION_MOTOR_RIGHT] * g_MotionMotorDirection[MOTION_MOTOR_RIGHT]));
    JSONDataPayload.add("LMotCur", String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 2));
    JSONDataPayload.add("LMotSpd", String(g_MotionMotorSpeed[MOTION_MOTOR_LEFT] * g_MotionMotorDirection[MOTION_MOTOR_LEFT]));
    JSONDataPayload.add("DrvMotFan", String(g_FanOn[FAN_2_BLUE]));

    // Cut motor data
    JSONDataPayload.add("CMotTemp", String(g_Temperature[TEMPERATURE_1_RED], 1));
    JSONDataPayload.add("CMotTempEr", String(g_TempErrorCount[TEMPERATURE_1_RED]));
    JSONDataPayload.add("CMotCur", String(g_MotorCurrent[MOTOR_CURRENT_CUT], 2));
    JSONDataPayload.add("CMotSpd", String(float(g_CutMotorSpeed * g_CutMotorDirection), 2));
    JSONDataPayload.add("CMotAlm", String(g_CutMotorAlarm));
    JSONDataPayload.add("CMotFan", String(g_FanOn[FAN_1_RED]));

    // Sonar data
    JSONDataPayload.add("FSnrDist", String(g_SonarDistance[SONAR_FRONT]));
    JSONDataPayload.add("RSnrDist", String(g_SonarDistance[SONAR_RIGHT]));
    JSONDataPayload.add("LSnrDist", String(g_SonarDistance[SONAR_LEFT]));
    JSONDataPayload.add("FSnrErr", String(g_MaxSonarDistanceCount[SONAR_FRONT]));
    JSONDataPayload.add("RSnrErr", String(g_MaxSonarDistanceCount[SONAR_RIGHT]));
    JSONDataPayload.add("LSnrErr", String(g_MaxSonarDistanceCount[SONAR_LEFT]));
    JSONDataPayload.add("SnrLoop", String(g_SonarTskLoopCnt-LastSonarLoopCountSent));
    JSONDataPayload.add("LstSnr", String(g_LastSonarReadNum));

    // Compass & Gyro data
    JSONDataPayload.add("CompHead", String(g_CompassHeading));
    JSONDataPayload.add("MPUTemp", String(g_MPUTemperature));

    // GPS Data
    JSONDataPayload.add("GPSHead", String(g_GPSHeading, 1));
    JSONDataPayload.add("GPSSat", String(g_GPSSatellitesFix));
    JSONDataPayload.add("GPSHdop", String(g_GPSHdop, 2));
    JSONDataPayload.add("GPSSpd", String(g_GPSSpeed, 2));
    if (g_GPSLatitude != 0 && g_GPSLongitude != 0) 
    {
      JSONDataPayload.add("GPSLat", String(g_GPSLatitude, 9));
      JSONDataPayload.add("GPSLon", String(g_GPSLongitude, 9));
      JSONDataPayload.add("GPSAlt", String(g_GPSAltitude, 2));
    }
    else 
    {
      JSONDataPayload.add("GPSLat", String(UNKNOWN_FLOAT, 9));
      JSONDataPayload.add("GPSLon", String(UNKNOWN_FLOAT, 9));
      JSONDataPayload.add("GPSAlt", String(UNKNOWN_FLOAT, 2));
    }

    // Mowing Statistics data
    JSONDataPayload.add("Obstcl", String(g_totalObstacleDectections));
    JSONDataPayload.add("MowTim", String(int(g_totalMowingTime/60000)));
    JSONDataPayload.add("PartMowTim", String(int(g_partialMowingTime/60000)));
    JSONDataPayload.add("ChargTim", String(int(g_totalChargingTime/60000)));

    // Perimeter data
    JSONDataPayload.add("Mag", String(g_PerimeterMagnitude));
    JSONDataPayload.add("SMag", String(g_PerimeterSmoothMagnitude));
    JSONDataPayload.add("InOut", String(g_isInsidePerimeter));

    // Mower Angle data
    JSONDataPayload.add("Pitch", String(g_pitchAngle, 1));
    JSONDataPayload.add("Roll", String(g_rollAngle, 1));

    // ESP System data
    JSONDataPayload.add("Heap", String(esp_get_free_heap_size()));
    JSONDataPayload.add("CPUTemp", String(temperatureRead(), 1));
    JSONDataPayload.add("RSSI", String(WiFi.RSSI()));
    JSONDataPayload.add("MQTTErr", String(g_MQTTErrorCount));
    JSONDataPayload.add("CPU0IdleCnt", String(float(g_TotalIdleCycleCount[0]) / float(millis() - LastTelemetryDataSent), 3));
    JSONDataPayload.add("CPU1IdleCnt", String(float(g_TotalIdleCycleCount[1]) / float(millis() - LastTelemetryDataSent), 3));
    JSONDataPayload.add("LongLoopCnt", String(float(g_PrimProcTskLongLoopCnt) * 1000.0 / float(millis() - LastTelemetryDataSent), 3));  // in counts per second


    JSONDataPayload.toString(JSONDataPayloadStr, false);
    JSONDataPayloadStr.toCharArray(MQTTpayload, JSONDataPayloadStr.length() + 1);

    DebugPrintln(MQTTpayload + String(JSONDataPayloadStr.length()) + "=> " + String(publ) + " in " + String(millis() - StartSend) + " ms", DBG_VERBOSE, true);

    if (JSONDataPayloadStr.length() < MQTT_MAX_PAYLOAD)
    {
      publ = MQTTclient.publish(MQTT_TELEMETRY_CHANNEL, MQTTpayload);
      LastTelemetryDataSent = millis();
      if (publ == 1)
      {
        g_MaxSonarDistanceCount[SONAR_FRONT] = 0;
        g_MaxSonarDistanceCount[SONAR_LEFT] = 0;
        g_MaxSonarDistanceCount[SONAR_RIGHT] = 0;
        LastSonarLoopCountSent = g_SonarTskLoopCnt;
        g_TempErrorCount[TEMPERATURE_1_RED] = 0;
        g_TempErrorCount[TEMPERATURE_2_BLUE] = 0;
        g_TotalIdleCycleCount[0] = 0;
        g_TotalIdleCycleCount[1] = 0;
        g_PrimProcTskLongLoopCnt = 0;
        g_MQTTErrorCount = 0;
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
* Send Base station a command to start sleeping on MQTT channel
 */
void BaseSleepingStartSend(void)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  JSONNotePayload.clear();

  JSONNotePayload.add("Command", "STATE_CHANGE");
  JSONNotePayload.add("Val1", "SLEEPING");

  JSONNotePayload.toString(JSONNotePayloadStr, false);
  JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

  bool result = MQTTclient.publish(MQTT_BASE_COMMAND_CHANNEL, MQTTpayload);
  if (result != 1)
  {
    g_MQTTErrorCount = g_MQTTErrorCount + 1;
  }

  MQTTclient.loop();

  DebugPrintln("Sending to :[" + String(MQTT_BASE_COMMAND_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
}

/**
 * Send Base station a command to start sending perimeter signal on MQTT channel
 */
void BaseSendingStartSend(void)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  JSONNotePayload.clear();

  JSONNotePayload.add("Command", "STATE_CHANGE");
  JSONNotePayload.add("Val1", "SENDING");

  JSONNotePayload.toString(JSONNotePayloadStr, false);
  JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

  bool result = MQTTclient.publish(MQTT_BASE_COMMAND_CHANNEL, MQTTpayload);
  if (result != 1)
  {
    g_MQTTErrorCount = g_MQTTErrorCount + 1;
  }

  MQTTclient.loop();

  DebugPrintln("Sending to :[" + String(MQTT_BASE_COMMAND_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
}
