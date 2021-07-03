/*
    This file contains the definition of all global variables
*/
#include "myGlobals_definition.h"
#include "pin_definitions.h"
#include "i2c_definitions.h"
#include "Utils/Utils.h"

WiFiClient espClient;

/************************* MQTT *********************************/

PubSubClient MQTTclient(espClient);

int MQTTErrorCount = 0;
char MQTTpayload[MQTT_MAX_PAYLOAD];

/************************* JSON *********************************/

FirebaseJson JSONDataPayload;
String JSONDataPayloadStr;

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

/************************* LCD variables *********************************/

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);   // Uses Defaut Address

/************************* Keypad variables *********************************/

const uint8_t KeyMasks[KEYPAD_MAX_KEYS] = {0X2, 0X1, 0X8, 0x4};
const int KeyPins[KEYPAD_MAX_KEYS] = {PIN_MCP_KEYPAD_1 - 8, PIN_MCP_KEYPAD_2 - 8, PIN_MCP_KEYPAD_3 - 8, PIN_MCP_KEYPAD_4 - 8};  // GPIO B
bool KeyPressed[KEYPAD_MAX_KEYS] = {false, false, false, false};

/************************* MCP23017 I2C IO Extender variables *********************************/
#include <Adafruit_MCP23017.h>

Adafruit_MCP23017 IOExtend;

/************************* I2C HMC5883L Compasss Sensor variables *********************************/
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified Compass = Adafruit_HMC5883_Unified(COMPASS_ID);

float CompassHeading = 0;
float CompassHeadingCorrected = 0;
float CompassXField = 0;
float CompassYField = 0;

/************************* UART NEO-N8M GPS variables *********************************/
#include <TinyGPS++.h>

TinyGPSPlus GPS; // The TinyGPS++ object
//HardwareSerial Serial2;

float GPSHeading;                        // in Degrees
int GPSSatellitesFix = 0;
double GPSHdop = UNKNOWN_FLOAT;
double GPSSpeed = UNKNOWN_FLOAT;
double GPSAltitude = UNKNOWN_FLOAT;
double GPSLatitude = UNKNOWN_FLOAT;
double GPSLongitude = UNKNOWN_FLOAT;

/************************* DS18D20 temperature sensor variables *********************************/
#include <DallasTemperature.h>

OneWire TemperatureOneWire(PIN_ESP_TEMP);

DallasTemperature TemperatureSensors(&TemperatureOneWire);

// If you do not know your temperature sensor device addresses :
// 1- Uncomment the TEMPERATURE_SENSOR_ADDRESS_UNKNOWN definition below
// 2- Compile and run the programm
// 3- Get the device addresses shown in the startup log and replace the addresses below
// 4- Re-comment the definition below

// #define TEMPERATURE_SENSOR_ADDRESS_UNKNOWN

DeviceAddress temp_1_RedSensor = {0x28, 0xC9, 0xD0, 0x95, 0xF0, 0x01, 0x3C, 0x7D};
DeviceAddress temp_2_BlueSensor = {0x28, 0xD7, 0x3C, 0x95, 0xF0, 0x01, 0x3C, 0xCE};

int TempErrorCount[TEMPERATURE_COUNT] = { 0, 0};
float Temperature[TEMPERATURE_COUNT] = { 0, 0};

/************************* ACS712 Battery Charge current sensor variables *********************************/
float BatteryChargeCurrent = 0;

/************************* INA219 I2C Curent sensor variables *********************************/
#include <Adafruit_INA219.h>

Adafruit_INA219 MotorCurrentSensor[MOTOR_CURRENT_COUNT] = {Adafruit_INA219(MOTOR_RIGHT_INA219_I2C_ADDRESS), 
                                                           Adafruit_INA219(MOTOR_LEFT_INA219_I2C_ADDRESS), 
                                                           Adafruit_INA219(MOTOR_CUT_INA219_I2C_ADDRESS) };

float MotorCurrent[MOTOR_CURRENT_COUNT] = {0, 0, 0};

/************************* Voltage variables *********************************/

float BatteryVotlage = 0;
int BatteryStatus = BATTERY_VOLTAGE_OK;

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>

NewPing sonar[SONAR_COUNT] = {                                           // Sensor object array.
    NewPing(PIN_ESP_SONAR_CENTER, PIN_ESP_SONAR_CENTER, SONAR_MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
    NewPing(PIN_ESP_SONAR_LEFT, PIN_ESP_SONAR_LEFT, SONAR_MAX_DISTANCE),
    NewPing(PIN_ESP_SONAR_RIGHT, PIN_ESP_SONAR_RIGHT, SONAR_MAX_DISTANCE)};

int SonarDistance[SONAR_COUNT] = {0, 0, 0};      // in cm

/************************* Bumper variables *********************************/

bool LeftBumperTriggered = false;
bool RightBumperTriggered = false;

/************************* Tilt variables *********************************/

bool HorizontalTiltTriggered = false;
bool VerticalTiltTriggered = false;

/************************* Fan variables *********************************/

const int FanPin[FAN_COUNT] = {PIN_MCP_FAN_1, PIN_MCP_FAN_2};
bool FanOn[FAN_COUNT] = {false, false};

/************************* Program variables *********************************/

byte TestVal1 = 0;
byte TestVal2 = 0;
byte TestVal3 = 0;
int TestVal4 = 0;
