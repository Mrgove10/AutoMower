/*
    This file contains the definition of all global variables
*/
#include "myGlobals_definition.h"
#include "Utils/Utils.h"

WiFiClient espClient;

/************************* MQTT *********************************/

PubSubClient MQTTclient(espClient);

int MQTTErrorCount = 0;

/************************* Debug management using TelnetSpy *********************************/

TelnetSpy SerialAndTelnet;
WiFiServer tcpServer(TCP_PORT);

int debugLevel = DBG_VERBOSE;

/************************* OTA *********************************/

bool otaFlag = false;
unsigned long OTAelapsed = 0;

/************************* EEPROM Management *********************************/

EEPROMLoadStruct EEPROMLoad;

bool EEPROMValid = false;
bool EEPROMUpdate = false;

unsigned long LastEepromWriteTime = 0;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
Timezone myTime;


/************************* Program variables *********************************/

byte TestVal1 = 0;
byte TestVal2 = 0;
byte TestVal3 = 0;
int TestVal4 = 0;
