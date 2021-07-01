#include <Arduino.h>

/************************* MQTT *********************************/
#include <PubSubClient.h>

extern PubSubClient MQTTclient;
extern int MQTTErrorCount;

#define MQTT_TELEMETRY_SEND_INTERVAL 30000          // in ms
#define MQTT_MAX_PAYLOAD 512

extern char MQTTpayload[MQTT_MAX_PAYLOAD];

/************************* JSON *********************************/
#include <FirebaseJson.h>

extern FirebaseJson JSONDataPayload;
extern String JSONDataPayloadStr;

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

/************************* LCD variables *********************************/
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

/************************* MCP23017 I2C IO Extender variables *********************************/
#include <Adafruit_MCP23017.h>

extern Adafruit_MCP23017 IOExtend;

/************************* DS18D20 temperature sensor variables *********************************/
#include <DallasTemperature.h>
extern OneWire TemperatureOneWire;
extern DallasTemperature TemperatureSensors;
extern DeviceAddress temp_1_RedSensor;
extern DeviceAddress temp_2_BlueSensor;

extern int Temp1ErrorCount;
extern int Temp2ErrorCount;

/************************* ACS712 Battery Charge current sensor variables *********************************/
extern float BatteryChargeCurrent;

/************************* INA219 I2C Curent sensor variables *********************************/
#include <Adafruit_INA219.h>

#define MOTOR_CURRENT_COUNT 3         // Number of motor current sensors.

extern Adafruit_INA219 MotorCurrentSensor[MOTOR_CURRENT_COUNT];

#define MOTOR_CURRENT_RIGHT 0
#define MOTOR_CURRENT_LEFT 1
#define MOTOR_CURRENT_CUT 2

extern float MotorCurrent[MOTOR_CURRENT_COUNT];

/************************* Voltage variables *********************************/

#define VOLTAGE_RANGE_MAX 17000     // in mV

#define BATTERY_VOLTAGE_OK 0                // if above VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_MEDIUM 1            // if above BATTERY_VOLTAGE_MEDIUM_THRESHOLD and below VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_LOW 2               // if above BATTERY_VOLTAGE_LOW_THRESHOLD  and below BATTERY_VOLTAGE_MEDIUM_THRESHOLD
#define BATTERY_VOLTAGE_CRITICAL 3           // if below BATTERY_VOLTAGE_LOW_THRESHOLD

extern float BatteryVotlage;

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>
#include <NewPing.h>

#define SONAR_COUNT 3         // Number of sensors.
#define SONAR_MAX_DISTANCE 200      // Maximum distance (in cm) to ping.

#define SONAR_FRONT 1
#define SONAR_LEFT 2
#define SONAR_RIGHT 3

extern NewPing sonar[SONAR_COUNT];

/************************* Bumper variables *********************************/

extern bool LeftBumperTriggered;
extern bool RightBumperTriggered;

/************************* Tilt variables *********************************/

extern bool HorizontalTiltTriggered;
extern bool VerticalTiltTriggered;

/************************* Test sequence variables *********************************/
#define TEST_SEQ_STEP_WAIT 1000
#define TEST_SEQ_STEP_ERROR_WAIT 2000

/************************* Program variables *********************************/

#define UNKNOWN_FLOAT -999.99F
#define UNKNOWN_INT -999

extern byte TestVal1;
extern byte TestVal2;
extern byte TestVal3;
extern int TestVal4;
