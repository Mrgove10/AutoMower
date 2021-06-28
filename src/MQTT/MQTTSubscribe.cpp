#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Framework/Utils.h"

void MQTTSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.subscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("SubscribeStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
};