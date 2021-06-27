#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Framework/Utils.h"
#include "Framework/OTA.h"

#include "MQTT/MQTTCallback.h"
#include "MQTT/MQTTSubscribe.h"
#include "MQTT/MQTTUnSubscribe.h"

void MQTTCallback(char* topic, byte* message, unsigned int length) {

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
  
  for (unsigned int i = 0; i < length; i++) {
    DebugPrint(String((char)message[i]));
    messageTemp += (char)message[i];
  }
  DebugPrintln("");

  if (String(topic) == String(MQTT_COMMAND_CHANNEL)) {

    if (String(messageTemp) == "OTA" &&
        String(messageTemp) != lastCommand &&
        lastCommand != ""){
      DebugPrintln("                        OTA Requested", DBG_INFO, true);
      lastCommand = String(messageTemp);
      otaFlag = true;
      OTAHandle();
    }  
    
    else if(String(messageTemp) == lastCommand ){
      DebugPrintln(" ...... Nothing (same command) !!!!", DBG_DEBUG, true);
    }
    
    else {
      DebugPrintln( " ...... MQTTcommand not recognised !!!!", DBG_WARNING, true);
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