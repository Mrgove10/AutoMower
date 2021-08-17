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
SemaphoreHandle_t g_MySerialSemaphore;

/************************* OTA *********************************/

bool g_otaFlag = false;
unsigned long g_OTAelapsed = 0;

/************************* Timer ISR *********************************/

// hw_timer_t * g_FastTimer = NULL;
// hw_timer_t * g_SlowTimer = NULL;

// portMUX_TYPE g_FastTimerMux = portMUX_INITIALIZER_UNLOCKED;
// portMUX_TYPE g_SlowTimerMux = portMUX_INITIALIZER_UNLOCKED;

// SemaphoreHandle_t g_FastTimerSemaphore;

// volatile unsigned long g_FastTimerCount = 0;
// volatile int g_SlowTimerCount = 0;

/************************* Perimeter signal code *********************************/

// Based on Ardumower project :http://grauonline.de/alexwww/ardumower/filter/filter.html
// "pseudonoise4_pw" signal
// if using reconstructed sender signal, use this
int8_t g_sigcode_norm[PERIMETER_SIGNAL_CODE_LENGTH] = {1, 1, -1, -1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, -1};
// "pseudonoise4_pw" signal (differential)
// if using the coil differential signal, use this
int8_t g_sigcode_diff[PERIMETER_SIGNAL_CODE_LENGTH] = {1, 0, -1, 0, 1, -1, 1, -1, 0, 1, -1, 1, 0, -1, 0, 1, -1, 0, 1, -1, 0, 1, 0, -1};

// pseudonoise5_nrz  signal
// if using reconstructed sender signal, use this
// int8_t g_sigcode_norm[PERIMETER_SIGNAL_CODE_LENGTH] = {1, 1, 1, -1, -1, -1, 1, 1, -1, 1, 1, 1, -1, 1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1}; // pseudonoise5_nrz
// "pseudonoise5_nrz" signal (differential)
// if using the coil differential signal, use this
// int8_t g_sigcode_diff[PERIMETER_SIGNAL_CODE_LENGTH] = {1, 0, 0, -1,  0,  0, 1, 0, -1, 1, 0, 0, -1, 1, -1, 1, -1,  0,  0,  0, 1, -1,  0, 1, -1, 0, 0, -1,  0, 1, 0};

/************************* High speed Analog Read task *********************************/

SemaphoreHandle_t g_ADCinUse;           // to protect access to ADC between I2S driver and other analogRead calls
SemaphoreHandle_t g_RawValuesSemaphore; // to protect access to shared global variables used in Perimter data Processing task

QueueHandle_t g_I2SQueueHandle;       // Queue used by I2S driver to notify for availability of new samples in a full DMA buffer
TaskHandle_t g_FastAnaReadTaskHandle; // High speed analog read RTOS task handle

uint16_t g_raw[PERIMETER_RAW_SAMPLES]; // Circular Buffer containing last samples read from I2S DMA buffers
int g_rawWritePtr = 0;                 // Pointer to last value written to g_raw circular buffer

unsigned int g_FastAnaReadTimeout = 0; // Counter of I2S read time outs, indication incorrect situation
unsigned int g_inQueueMax = 0;         // Max I2S notification queue waiting events (should be 0)
unsigned int g_inQueue = 0;            // Accumulated I2S notification queue waiting events (should be 0)

/************************* Analog Read loop task *********************************/

// TaskHandle_t g_AnaReadTask;
// portMUX_TYPE g_AnaReadMux = portMUX_INITIALIZER_UNLOCKED;

// volatile int g_readAnaBuffer[ANA_READ_BUFFER_SIZE];
// volatile int g_readAnaBufferPtr = 0;
// volatile int g_timerCallCounter = 0;
// volatile unsigned long g_AnalogReadMicrosTotal = 0;

// volatile long g_MissedReadings = 0;
// volatile float g_rate = 0;
// volatile long g_Triggers = 0;

/************************* Perimeter data processing task *********************************/
hw_timer_t *g_PerimeterTimerhandle = NULL; // Perimeter processing task timer based trigger ISR handle

QueueHandle_t g_PerimeterTimerQueue; // Queue red by Perimeter processing task

TaskHandle_t g_PerimeterProcTaskHandle; // Perimeter processing task RTOS task handle

unsigned int g_PerimeterQueuefull = 0;  // Assumulated count of full Perimeter queue events
unsigned int g_inPerimeterQueueMax = 0; // Max Perimeter queue waiting events (should be 0)
unsigned int g_inPerimeterQueue = 0;    // Accumulated Perimeter queue waiting events (should be 0)

// Values comming as output of Perimeter processing made available to other tasks through global variables
int16_t g_PerimeterRawMax = 0;
int16_t g_PerimeterRawMin = SCHAR_MAX;
int16_t g_PerimeterRawAvg = 0;
bool g_isInsidePerimeter = false;
unsigned long g_lastIsInsidePerimeterTime = 0;
bool g_PerimetersignalTimedOut = false;
int g_PerimeterMagnitude = 0;
int g_PerimeterMagnitudeAvg = 0;
int g_PerimeterMagnitudeAvgPID=0;

int g_PerimeterSmoothMagnitude = 0;
float g_PerimeterFilterQuality = 0;
int16_t g_PerimeterOffset = 0;                   // (Saved to EEPROM)
int g_signalCounter = 0;

uint16_t g_RawCopy[PERIMETER_RAW_SAMPLES]; //  Copy of circular Buffer containing last samples read from I2S DMA buffers
int g_rawWritePtrCopy;                     // Pointer to last value written to g_RawCopy circular buffer copy
int8_t g_PerimeterSamplesForMatchedFilter[I2S_DMA_BUFFER_LENGTH];

#ifdef MQTT_GRAPH_DEBUG
bool g_MQTTGraphDebug = false;
bool g_MQTTGraphRawDebug = false;
#endif
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

/************************* Sonar Read task *********************************/

TaskHandle_t g_SonarReadTaskHandle; // Sonar Read task RTOS task handle

bool g_SonarReadEnabled = false;          // Global variable to suspend sonar sensor reading

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>

NewPing sonar[SONAR_COUNT] = {                                               // Sensor object array.
    NewPing(PIN_ESP_SONAR_CENTER, PIN_ESP_SONAR_CENTER, SONAR_MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
    NewPing(PIN_ESP_SONAR_LEFT, PIN_ESP_SONAR_LEFT, SONAR_MAX_DISTANCE),
    NewPing(PIN_ESP_SONAR_RIGHT, PIN_ESP_SONAR_RIGHT, SONAR_MAX_DISTANCE)};

String g_sensorStr[SONAR_COUNT] = {"Front", "Left", "Right"};

int g_SonarDistance[SONAR_COUNT] = {0, 0, 0}; // in cm

/************************* Bumper variables *********************************/

String g_bumperStr[BUMPER_COUNT] = {"Left", "Right"};
int g_bumperPin[BUMPER_COUNT] = {PIN_ESP_BUMPER_LEFT, PIN_ESP_BUMPER_RIGHT};

portMUX_TYPE g_BumperMux[BUMPER_COUNT] = {portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED};

volatile bool g_BumperTriggered[BUMPER_COUNT] = {false, false};

/************************* Tilt variables *********************************/

String g_tiltStr[TILT_COUNT] = {"Horizontal", "Vertical"};
int g_tiltPin[TILT_COUNT] = {PIN_ESP_TILT_HORIZONTAL, PIN_ESP_TILT_VERTICAL};

portMUX_TYPE g_TiltMux[TILT_COUNT] = {portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED};

volatile bool g_TiltTriggered[TILT_COUNT] = {false, false};

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

float g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_COUNT] = {0, 0};         // from perimeter tracking PID control

/************************* Mower Moves variables *********************************/

// Move count variables
int g_successiveObstacleDectections = 0;       // successive obstacle detections (to trigger appropriate reaction)

// Perimeter tracking function

#include <PID_v1.h>

//Define Variables used by PID library
double g_PIDSetpoint, g_PIDInput, g_PIDOutput;

  //Specify the links and initial tuning parameters
PID g_PerimeterTrackPID(&g_PIDInput, &g_PIDOutput, &g_PIDSetpoint, 0, 0, 0, DIRECT);

#ifdef MQTT_PID_GRAPH_DEBUG
bool g_MQTTPIDGraphDebug = true;
#endif

double g_PerimeterTrackSetpoint = 0;             // Setpoint for PID wire tracking
double g_ParamPerimeterTrackPIDKp = 0;           // Kp PID Parameter for wire tracking
double g_ParamPerimeterTrackPIDKi = 0;           // Ki PID Parameter for wire tracking
double g_ParamPerimeterTrackPIDKd = 0;           // Kd PID Parameter for wire tracking

/************************* CUT Motor variables *********************************/

bool g_CutMotorOn = false;
int g_CutMotorDirection = CUT_MOTOR_STOPPED;
int g_CutMotorSpeed = 0;
bool g_CutMotorAlarm = false;

/************************* Error variables *********************************/

int g_CurrentErrorCode = ERROR_NO_ERROR; // Current Error code

/************************* Mower operation statistics *********************************/

long g_totalObstacleDectections = 0;         // Total number of obstacle detections   (Saved to EEPROM)


/************************* Program variables *********************************/

MowerState g_CurrentState = MowerState::idle;
MowerState g_PreviousState = MowerState::idle;

/************************* Mower State variables *********************************/

// Mowing mode
int g_MowingLoopCnt = 0;         // number of loops since mowing started