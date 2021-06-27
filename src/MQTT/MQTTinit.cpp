#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "MQTT/MQTTReconnect.h"
#include "MQTT/MQTTCallback.h"

void MQTTInit(void)
{
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);

  MQTTclient.setBufferSize(512);

  MQTTclient.setCallback(MQTTCallback);

  MQTTReconnect();
}
