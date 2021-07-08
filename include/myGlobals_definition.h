#include <Arduino.h>
#include "Environment_definitions.h"

/************************* MQTT *********************************/
#include <PubSubClient.h>

extern PubSubClient MQTTclient;
extern int MQTTErrorCount;

#define MQTT_TELEMETRY_SEND_INTERVAL 30000 // in ms
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

/************************* Display variables *********************************/

#ifdef LCD2004_DISPLAY
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#endif

#ifdef OLEDSSD1306_DISPLAY

#include <SSD1306.h> 
extern SSD1306Wire oled;

#endif

#define COLUMS 20
#define ROWS 4
#define OLED_PIXEL_PER_LINE 16 // to reproduce LCD "form factor" on OLED display : 64/4
#define OLED_PIXEL_PER_COLUMN 7 // to reproduce LCD "form factor" on OLED display : 128/20 => 6.4 rounded up to 7
#define OLED_BRIGHTNESS 200

/************************* Keypad variables *********************************/

#define KEYPAD_READ_INTERVAL 100 // in ms
#define KEYPAD_MAX_KEYS 4
#define KEYPAD_GPIO 1 // GPIO B

extern const uint8_t KeyMasks[KEYPAD_MAX_KEYS];
extern const int KeyPins[KEYPAD_MAX_KEYS];
extern bool KeyPressed[KEYPAD_MAX_KEYS];

/************************* MCP23017 I2C IO Extender variables *********************************/
#include <Adafruit_MCP23017.h>

extern Adafruit_MCP23017 IOExtend;

/************************* I2C HMC5883L Compasss Sensor variables *********************************/
#include <Adafruit_HMC5883_U.h>

extern Adafruit_HMC5883_Unified Compass;

//#define COMPASS_PRESENT
#define COMPASS_ID 12345
#define COMPASS_PRECISION 1
#define COMPASS_X_HEADING_CORRECTION -3.5f
#define COMPASS_Y_HEADING_CORRECTION -6.5f
#define COMPASS_READ_INTERVAL 4000

extern float CompassHeading;          // in Degrees
extern float CompassHeadingCorrected; // in Degrees
extern float CompassXField;
extern float CompassYField;

/************************* UART NEO-N8M GPS variables *********************************/
#include <TinyGPS++.h>

#define GPS_BAUD 115200UL
#define GPS_READ_INTERVAL 500
#define GPS_UART Serial2
#define GPS_CHARS_TO_DETECT 20

extern TinyGPSPlus GPS; // The TinyGPS++ object
extern HardwareSerial Serial2;

extern float GPSHeading; // in Degrees
extern int GPSSatellitesFix;
extern double GPSHdop;
extern double GPSSpeed;
extern double GPSAltitude;
extern double GPSLatitude;
extern double GPSLongitude;

/************************* DS18D20 temperature sensor variables *********************************/
#include <DallasTemperature.h>
extern OneWire TemperatureOneWire;
extern DallasTemperature TemperatureSensors;
extern DeviceAddress temp_1_RedSensor;
extern DeviceAddress temp_2_BlueSensor;

#define TEMPERATURE_COUNT 2 // Number of temperature sensors
#define TEMPERATURE_1_RED 0
#define TEMPERATURE_2_BLUE 1
#define TEMPERATURE_READ_INTERVAL 5000 // in ms

extern int TempErrorCount[TEMPERATURE_COUNT];
extern float Temperature[TEMPERATURE_COUNT];

/************************* ACS712 Battery Charge current sensor variables *********************************/
extern float BatteryChargeCurrent;

#define BATTERY_CHARGE_READ_INTERVAL 2000   // in ms
#define CHARGE_CURRENT_CHECK_THRESHOLD 1000 // in raw AnalogRead points
#define CHARGE_CURRENT_OFFSET 140           // in raw AnalogRead
#define CHARGE_CURRENT_MV_PER_AMP 100.0F    // From ACS712-20A datasheet
#define CHARGE_CURRENT_ZERO_VOLTAGE 2500    // in mv
#define CHARGE_CURRENT_DEADBAND 250         // in mA

/************************* INA219 I2C Curent sensor variables *********************************/
#include <Adafruit_INA219.h>

#define MOTOR_CURRENT_COUNT 3 // Number of motor current sensors.

extern Adafruit_INA219 MotorCurrentSensor[MOTOR_CURRENT_COUNT];

#define MOTOR_CURRENT_RIGHT 0
#define MOTOR_CURRENT_LEFT 1
#define MOTOR_CURRENT_CUT 2
#define MOTOR_CURRENT_READ_INTERVAL 500 // in ms
extern float MotorCurrent[MOTOR_CURRENT_COUNT];

/************************* Voltage variables *********************************/

#define VOLTAGE_RANGE_MAX 17000 // in mV

#define BATTERY_VOLTAGE_OK 0               // if above VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_MEDIUM 1           // if above BATTERY_VOLTAGE_MEDIUM_THRESHOLD and below VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_LOW 2              // if above BATTERY_VOLTAGE_LOW_THRESHOLD  and below BATTERY_VOLTAGE_MEDIUM_THRESHOLD
#define BATTERY_VOLTAGE_CRITICAL 3         // if below BATTERY_VOLTAGE_LOW_THRESHOLD
#define BATTERY_VOLTAGE_READ_INTERVAL 5000 // in ms

extern float BatteryVotlage;
extern int BatteryStatus;

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>
#include <NewPing.h>

#define SONAR_COUNT 3            // Number of sensors.
#define SONAR_MAX_DISTANCE 200   // Maximum distance (in cm) to ping.
#define SONAR_READ_INTERVAL 1000 // in ms

#define SONAR_FRONT 0
#define SONAR_LEFT 1
#define SONAR_RIGHT 2

extern NewPing sonar[SONAR_COUNT];
extern int SonarDistance[SONAR_COUNT]; // in cm

/************************* Bumper variables *********************************/

extern bool LeftBumperTriggered;
extern bool RightBumperTriggered;

/************************* Tilt variables *********************************/

extern bool HorizontalTiltTriggered;
extern bool VerticalTiltTriggered;

/************************* Fan variables *********************************/

#define FAN_COUNT 2 // Number of Fans
#define FAN_1_RED 0
#define FAN_2_BLUE 1
#define FAN_UPDATE_INTERVAL 15000 // in ms
#define FAN_START_THRESHOLD 27.5f // in deg C
#define FAN_STOP_THRESHOLD 26.0f  // in deg C
#define FAN_TEST_DURATION 5000    // in ms

extern const int FanPin[FAN_COUNT];
extern bool FanOn[FAN_COUNT];

/************************* Motion Motor variables *********************************/

#define MOTION_MOTOR_COUNT 2 // Number of motors
#define MOTION_MOTOR_RIGHT 0
#define MOTION_MOTOR_LEFT 1

#define MOTION_MOTOR_PWM_FREQUENCY 5000
#define MOTION_MOTOR_PWM_RESOLUTION 12
#define MOTION_MOTOR_PWM_CHANNEL_RIGHT 0
#define MOTION_MOTOR_PWM_CHANNEL_LEFT 1

#define MOTION_MOTOR_RIGHT 0
#define MOTION_MOTOR_LEFT 1

#define MOTION_MOTOR_STOPPED 0
#define MOTION_MOTOR_FORWARD 1
#define MOTION_MOTOR_REVERSE -1
#define MOTION_MOTOR_MIN_SPEED 4096 / 4

extern const int MotionMotorIn1Pin[MOTION_MOTOR_COUNT];
extern const int MotionMotorIn2Pin[MOTION_MOTOR_COUNT];
extern const int MotionMotorPWMChannel[MOTION_MOTOR_COUNT];
extern bool MotionMotorOn[MOTION_MOTOR_COUNT];
extern int MotionMotorDirection[MOTION_MOTOR_COUNT];
extern int MotionMotorSpeed[MOTION_MOTOR_COUNT];
extern String MotionMotorStr[MOTION_MOTOR_COUNT];

/************************* CUT Motor variables *********************************/

#define CUT_MOTOR_PWM_FREQUENCY 5000
#define CUT_MOTOR_PWM_RESOLUTION 12
#define CUT_MOTOR_PWM_CHANNEL_FORWARD 2
#define CUT_MOTOR_PWM_CHANNEL_REVERSE 3

#define CUT_MOTOR_STOPPED 0
#define CUT_MOTOR_FORWARD 1
#define CUT_MOTOR_REVERSE -1
#define CUT_MOTOR_MIN_SPEED 4096 / 10

#define CUT_MOTOR_FAST_STOP_INVERSE_SPEED 4096      // full inverse speed
#define CUT_MOTOR_FAST_STOP_INVERSE_DURATION  750 // in ms

#define CUT_MOTOR_CHECK_INTERVAL 2000   // in ms

extern bool CutMotorOn;
extern int CutMotorDirection;
extern int CutMotorSpeed;
extern bool CutMotorAlarm;

/************************* Test sequence variables *********************************/
#define TEST_SEQ_STEP_WAIT 1000
#define TEST_SEQ_STEP_ERROR_WAIT 1000

/************************* Program variables *********************************/

#define UNKNOWN_FLOAT -999.99F
#define UNKNOWN_INT -999

extern byte TestVal1;
extern byte TestVal2;
extern byte TestVal3;
extern int TestVal4;
