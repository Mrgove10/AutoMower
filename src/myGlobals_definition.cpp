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

int g_MQTTErrorCount = 0;

/************************* JSON *********************************/

FirebaseJson JSONDataPayload;
String JSONDataPayloadStr;

/************************* Debug management using TelnetSpy *********************************/

TelnetSpy SerialAndTelnet;
WiFiServer tcpServer(TCP_PORT);

int g_debugLevel = DBG_VERBOSE;

/************************* OTA *********************************/

bool g_otaFlag = false;
unsigned long g_OTAelapsed = 0;

/************************* EEPROM Management *********************************/

EEPROMLoadStruct g_EEPROMLoad;

bool g_EEPROMValid = false;
bool g_EEPROMUpdate = false;

unsigned long g_LastEepromWriteTime = 0;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
Timezone myTime;

/************************* Display variables *********************************/

#ifdef OLEDSSD1306_DISPLAY
SSD1306Wire oled(OLEDSSD1306_DISPLAY_I2C_ADDRESS, PIN_ESP_I2C_SDA, PIN_ESP_I2C_SCL);
#endif

#ifdef LCD2004_DISPLAY
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE); // Uses Defaut Address
#endif

/************************* Keypad variables *********************************/

//const int g_KeyPins[KEYPAD_MAX_KEYS] = {PIN_MCP_KEYPAD_1 - 8, PIN_MCP_KEYPAD_2 - 8, PIN_MCP_KEYPAD_3 - 8, PIN_MCP_KEYPAD_4 - 8}; // GPIO B
bool g_KeyPressed[KEYPAD_MAX_KEYS] = {false, false, false, false};

/************************* MCP23017 I2C IO Extender variables *********************************/
#include <Adafruit_MCP23017.h>

Adafruit_MCP23017 IOExtend;

/************************* I2C HMC5883L Compasss Sensor variables *********************************/
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified Compass = Adafruit_HMC5883_Unified(COMPASS_ID);

float g_CompassHeading = 0;
float g_CompassHeadingCorrected = 0;
float g_CompassXField = 0;
float g_CompassYField = 0;

/************************* UART NEO-N8M GPS variables *********************************/
#include <TinyGPS++.h>

TinyGPSPlus GPS; // The TinyGPS++ object
//HardwareSerial Serial2;

float g_GPSHeading; // in Degrees
int g_GPSSatellitesFix = 0;
double g_GPSHdop = UNKNOWN_FLOAT;
double g_GPSSpeed = UNKNOWN_FLOAT;
double g_GPSAltitude = UNKNOWN_FLOAT;
double g_GPSLatitude = UNKNOWN_FLOAT;
double g_GPSLongitude = UNKNOWN_FLOAT;

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

int g_TempErrorCount[TEMPERATURE_COUNT] = {0, 0};
float g_Temperature[TEMPERATURE_COUNT] = {0, 0};

/************************* ACS712 Battery Charge current sensor variables *********************************/
float g_BatteryChargeCurrent = 0;

/************************* INA219 I2C Curent sensor variables *********************************/
#include <Adafruit_INA219.h>

Adafruit_INA219 MotorCurrentSensor[MOTOR_CURRENT_COUNT] = {Adafruit_INA219(MOTOR_RIGHT_INA219_I2C_ADDRESS),
                                                           Adafruit_INA219(MOTOR_LEFT_INA219_I2C_ADDRESS),
                                                           Adafruit_INA219(MOTOR_CUT_INA219_I2C_ADDRESS)};

float g_MotorCurrent[MOTOR_CURRENT_COUNT] = {0, 0, 0};

/************************* Voltage variables *********************************/

float g_BatteryVotlage = 0;
int g_BatteryStatus = BATTERY_VOLTAGE_OK;

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>

NewPing sonar[SONAR_COUNT] = {                                               // Sensor object array.
    NewPing(PIN_ESP_SONAR_CENTER, PIN_ESP_SONAR_CENTER, SONAR_MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
    NewPing(PIN_ESP_SONAR_LEFT, PIN_ESP_SONAR_LEFT, SONAR_MAX_DISTANCE),
    NewPing(PIN_ESP_SONAR_RIGHT, PIN_ESP_SONAR_RIGHT, SONAR_MAX_DISTANCE)};

int g_SonarDistance[SONAR_COUNT] = {0, 0, 0}; // in cm

/************************* Bumper variables *********************************/

bool g_LeftBumperTriggered = false;
bool g_RightBumperTriggered = false;

/************************* Tilt variables *********************************/

bool g_HorizontalTiltTriggered = false;
bool g_VerticalTiltTriggered = false;

/************************* Fan variables *********************************/

const int g_FanPin[FAN_COUNT] = {PIN_MCP_FAN_1, PIN_MCP_FAN_2};
bool g_FanOn[FAN_COUNT] = {false, false};

/************************* Motion Motor variables *********************************/

const int g_MotionMotorIn1Pin[MOTION_MOTOR_COUNT] = {PIN_MCP_MOTOR_RIGHT_LN1, PIN_MCP_MOTOR_LEFT_LN1};
const int g_MotionMotorIn2Pin[MOTION_MOTOR_COUNT] = {PIN_MCP_MOTOR_RIGHT_LN2, PIN_MCP_MOTOR_LEFT_LN2};
const int g_MotionMotorPWMChannel[MOTION_MOTOR_COUNT] = {MOTION_MOTOR_PWM_CHANNEL_RIGHT, MOTION_MOTOR_PWM_CHANNEL_LEFT};

bool g_MotionMotorOn[MOTION_MOTOR_COUNT] = {false, false};
int g_MotionMotorDirection[MOTION_MOTOR_COUNT] = {MOTION_MOTOR_STOPPED, MOTION_MOTOR_STOPPED};
int g_MotionMotorSpeed[MOTION_MOTOR_COUNT] = {0, 0};

String g_MotionMotorStr[MOTION_MOTOR_COUNT] = {"Right", "Left"};

/************************* CUT Motor variables *********************************/

bool g_CutMotorOn = false;
int g_CutMotorDirection = CUT_MOTOR_STOPPED;
int g_CutMotorSpeed = 0;
bool g_CutMotorAlarm = false;

/************************* Program variables *********************************/

byte TestVal1 = 0;
byte TestVal2 = 0;
byte TestVal3 = 0;
int TestVal4 = 0;
