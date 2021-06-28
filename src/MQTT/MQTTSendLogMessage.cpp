#include <FirebaseJson.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Framework/Utils.h"

FirebaseJson JSONNotePayload;

void MQTTSendLogMessage(const char* MQTTTopic, const char* Message, const char* Tag, const int Level)
{
  String JSONNotePayloadStr;
  char MQTTpayload[MQTT_MAX_PAYLOAD];

  JSONNotePayload.clear();

  JSONNotePayload.add("Message", Message);
  JSONNotePayload.add("Tags", Tag);  
  JSONNotePayload.add("Level", Level);  
   
  JSONNotePayload.toString(JSONNotePayloadStr, false);
  JSONNotePayloadStr.toCharArray(MQTTpayload, JSONNotePayloadStr.length()+1);
  
  bool result = MQTTclient.publish(MQTTTopic,MQTTpayload);
  if (result != 1) {MQTTErrorCount = MQTTErrorCount + 1;}

  MQTTclient.loop();

  DebugPrint("Sending to :[" + String(MQTTTopic) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true); 

}
