#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Framework/Utils.h"

void MQTTSubscribe()
{
  bool SubStatus;
  SubStatus = MQTTclient.subscribe(MQTT_COMMAND_CHANNEL);
  DebugPrintln(String("SubStatus ") + MQTT_COMMAND_CHANNEL + String("=") + String(SubStatus));
};