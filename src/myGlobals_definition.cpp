/*
    This file contains the definition of all global variables
*/
#include "myGlobals_definition.h"
#include "Framework/Utils.h"

WiFiClient espClient;

/************************* MQTT *********************************/

PubSubClient MQTTclient(espClient);

/************************* Debug management using TelnetSpy *********************************/

TelnetSpy SerialAndTelnet;
const int tcpPort = 1000;
WiFiServer tcpServer(tcpPort);

int debugLevel = DBG_VERBOSE;

/************************* OTA *********************************/

const int OTAPort = 8266;
const unsigned long OTATimeout = 180000;

bool otaFlag = false;
unsigned long OTAelapsed = 0;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
Timezone myTime;
const int NTPRefresh = 60 * 60;
