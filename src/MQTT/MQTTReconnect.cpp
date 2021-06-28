#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "MQTT/MQTTReconnect.h"
#include "MQTT/MQTTSubscribe.h"
#include "Framework/Utils.h"

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
