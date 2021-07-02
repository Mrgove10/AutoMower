#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "Temperature/Temperature.h"

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

    JSONNotePayload.add("Message", Message);
    JSONNotePayload.add("Tags", Tag);
    JSONNotePayload.add("Level", Level);

    JSONNotePayload.toString(JSONNotePayloadStr, false);
    JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length() + 1);

    bool result = MQTTclient.publish(MQTTTopic, MQTTpayload);
    if (result != 1)
    {
      MQTTErrorCount = MQTTErrorCount + 1;
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

        if (String(messageTemp) == "OTA" &&
            String(messageTemp) != lastCommand)
        {
            LogPrintln("Request for OTA update", TAG_OTA, DBG_INFO);
            lastCommand = String(messageTemp);
            otaFlag = true;
            OTAHandle();
        }

        else if (String(messageTemp) == lastCommand)
        {
            DebugPrintln(" ...... Nothing (same command) !!!!", DBG_DEBUG, true);
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
    /*
  if (String(topic) == String(MQTTLinkyChan)) {
      String Payload_String;
      FirebaseJson JSONDataPayload;
      FirebaseJsonData JSONData;
      
      String JSONDataPayloadStr;

      JSONDataPayload.setJsonData(messageTemp);
//      JSONDataPayload.toString(JSONDataPayloadStr,false);
      JSONDataPayload.get(JSONData, "URMS1");
      String Value = JSONData.stringValue;
      URMS1 = Value.toFloat();
      MySERIAL.print("URMS1: "); MySERIAL.println(URMS1);
      if (ResetPRMS) {
        PRMSAvg = 0;
        NbvalPRMS = 0;
        ResetPRMS = false;
      }
      PRMSAvg =  PRMSAvg * double (NbvalPRMS) / double(NbvalPRMS + 1) + double (URMS1) / double (NbvalPRMS + 1);
      NbvalPRMS = NbvalPRMS + 1; 
      MySERIAL.print("PRMS: "); MySERIAL.println(PRMSAvg,3);
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
            MQTTErrorCount = MQTTErrorCount + 1;
            delay(5000);
        }
    }
}

void MQTTInit(void)
{
    lcd.setCursor(0, 2);
    lcd.print(F("Server link ..."));

    MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);

    MQTTclient.setBufferSize(MQTT_MAX_PAYLOAD);

    MQTTclient.setCallback(MQTTCallback);

    MQTTReconnect();

    lcd.setCursor(2, 3);

    if (MQTTclient.connect(ESPHOSTNAME))
    {
      lcd.print(F("Connected"));
    }
    else
    {
      lcd.print(F("FAILED"));
      delay(TEST_SEQ_STEP_ERROR_WAIT);
    }
    delay(TEST_SEQ_STEP_WAIT);
}

void MQTTSendTelemetry()
{
  static unsigned long LastTelemetryDataSent = 0;

  if ((millis() - LastTelemetryDataSent > MQTT_TELEMETRY_SEND_INTERVAL)) 
  {
    unsigned long StartSend = millis();

    byte publ = 0;

    JSONDataPayload.clear();

    JSONDataPayload.add("BatteryVoltage",           String(float(BatteryVotlage/1000.0f),2));
    JSONDataPayload.add("BatteryChargeCurrent",     String(BatteryChargeCurrent,2));

    JSONDataPayload.add("DriveMotorTemperature",    String(Temperature[TEMPERATURE_1_RED],1));
    JSONDataPayload.add("RightMotorCurrent",        String(MotorCurrent[MOTOR_CURRENT_RIGHT],2));
    JSONDataPayload.add("LeftMotorCurrent",         String(MotorCurrent[MOTOR_CURRENT_LEFT],2));
    JSONDataPayload.add("DriveMotorFan",            String(FanOn[FAN_1_RED]));
    
    JSONDataPayload.add("CutMotorTemperature",      String(Temperature[TEMPERATURE_2_BLUE],1));
    JSONDataPayload.add("CutMotorCurrent",          String(MotorCurrent[MOTOR_CURRENT_CUT],2));
    JSONDataPayload.add("CutMotorFan",              String(FanOn[FAN_2_BLUE]));

    JSONDataPayload.add("FrontSonarDistance",       String(SonarDistance[SONAR_FRONT]));
    JSONDataPayload.add("RightSonarDistance",       String(SonarDistance[SONAR_RIGHT]));
    JSONDataPayload.add("LeftSonarDistance",        String(SonarDistance[SONAR_LEFT]));
    
    JSONDataPayload.toString(JSONDataPayloadStr, false);
    JSONDataPayloadStr.toCharArray(MQTTpayload, JSONDataPayloadStr.length()+1);

    DebugPrintln(MQTTpayload + String(JSONDataPayloadStr.length()) + "=> " + String(publ) + " in " + String(millis() - StartSend) + " ms", DBG_VERBOSE, true); 

    if (JSONDataPayloadStr.length() < MQTT_MAX_PAYLOAD)
    {
      publ = MQTTclient.publish(MQTT_TELEMETRY_CHANNEL, MQTTpayload);
      LastTelemetryDataSent = millis();
    }
    else 
    {
      LogPrintln("MQTT payload larger than buffer !!!!!!", TAG_ERROR, DBG_ERROR);
    }
    LastTelemetryDataSent = millis();
  }
}
