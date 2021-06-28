#include <Arduino.h>

/************************* MQTT *********************************/
#include <PubSubClient.h>

extern PubSubClient MQTTclient;
extern int MQTTErrorCount;

/************************* Debug management using TelnetSpy *********************************/
#include <TelnetSpy.h>
#define MySERIAL SerialAndTelnet
extern const int tcpPort;
extern WiFiServer tcpServer;
extern TelnetSpy SerialAndTelnet;

extern int debugLevel;

/************************* OTA *********************************/
#include <ArduinoOTA.h>

extern const int OTAPort;
extern const unsigned long OTATimeout;

extern bool otaFlag;
extern unsigned long OTAelapsed;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
#include <ezTime.h>
extern Timezone myTime;
extern const int NTPRefresh;
#define POSIXTZ "CET-1CEST,M3.5.0,M10.5.0/3"
