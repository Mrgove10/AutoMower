#include <Arduino.h>

/************************* MQTT *********************************/
#include <PubSubClient.h>

extern PubSubClient MQTTclient;
extern int MQTTErrorCount;

/************************* Debug management using TelnetSpy *********************************/
#include <TelnetSpy.h>
#define MySERIAL SerialAndTelnet
#define TCP_PORT 1000
extern WiFiServer tcpServer;
extern TelnetSpy SerialAndTelnet;

extern int debugLevel;

/************************* OTA *********************************/
#include <ArduinoOTA.h>

#define OTA_PORT 3232
#define OTA_TIMEOUT 180000

extern bool otaFlag;
extern unsigned long OTAelapsed;

/************************* EEPROM Management *********************************/

#include "EEPROM.h"
#include "EEPROM/EEPROM_Struct.h"

extern EEPROMLoadStruct EEPROMLoad;
extern bool EEPROMValid;
extern bool EEPROMUpdate;

extern unsigned long LastEepromWriteTime;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
#include <ezTime.h>
extern Timezone myTime;
#define NTP_REFRESH 3600
#define POSIXTZ "CET-1CEST,M3.5.0,M10.5.0/3"

/************************* Bumper variables *********************************/

extern bool LeftBumpertriggered;
extern bool RightBumpertriggered;

/************************* Program variables *********************************/


extern byte TestVal1;
extern byte TestVal2;
extern byte TestVal3;
extern int TestVal4;
