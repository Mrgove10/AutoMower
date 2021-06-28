#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "MQTT/MQTTReconnect.h"
#include "MQTT/MQTTCallback.h"

void MQTTInit(void)
{
  MQTTclient.setServer(MQTT_SERVER, MQTT_PORT);

  MQTTclient.setBufferSize(MQTT_MAX_PAYLOAD);

  MQTTclient.setCallback(MQTTCallback);

  MQTTReconnect();
}
