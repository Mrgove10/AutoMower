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
unsigned long g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL;

unsigned long g_LastMowerTelemetryReceived = 0;

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
//unsigned long g_OTAelapsed = 0;

/************************* Perimeter signal code *********************************/

// Based on Ardumower project :http://grauonline.de/alexwww/ardumower/filter/filter.html
// "pseudonoise4_pw" signal

const int8_t g_sigcode[PERIMETER_SIGNAL_CODE_LENGTH] = {1, 1, -1, -1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, -1}; // ORIGINAL
//const int8_t g_sigcode[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0};  //motor driver friendly signal
//const int8_t g_sigcode[] = {1, 1, 1, 0, 0, 0, -1, -1, -1, 0, 0, 0, 1, 1, 1, 0, 0, 0, -1, -1, -1, 0, 0, 0};
//const int8_t g_sigcode[] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1};
//const int8_t g_sigcode[] = {1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
//const int8_t g_sigcode[] = {1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1, -1, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1, -1};
//const int8_t g_sigcode[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//const int8_t g_sigcode[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

// const int8_t g_sigcode[] = {1, 1, 1, -1, -1, -1, 1, 1, -1, 1, 1, 1, -1, 1, -1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1}; // pseudonoise5_nrz

/************************* Perimeter data processing task *********************************/

// hw_timer_t *g_PerimeterTimerhandle = NULL; // Perimeter processing task timer based trigger ISR handle

// QueueHandle_t g_PerimeterTimerQueue; // Queue red by Perimeter processing task

// TaskHandle_t g_PerimeterProcTaskHandle; // Perimeter processing task RTOS task handle

// unsigned int g_PerimeterQueuefull = 0;  // Assumulated count of full Perimeter queue events
// unsigned int g_inPerimeterQueueMax = 0; // Max Perimeter queue waiting events (should be 0)
// unsigned int g_inPerimeterQueue = 0;    // Accumulated Perimeter queue waiting events (should be 0)

// // Values comming as output of Perimeter processing made available to other tasks through global variables
// int16_t g_PerimeterRawMax = 0;
// int16_t g_PerimeterRawMin = SCHAR_MAX;
// int16_t g_PerimeterRawAvg = 0;
// bool g_isInsidePerimeter = false;
// unsigned long g_lastIsInsidePerimeterTime = 0;
// bool g_PerimetersignalTimedOut = false;
// int g_PerimeterMagnitude = 0;
// int g_PerimeterMagnitudeAvg = 0;
// // int g_PerimeterMagnitudeAvgPID=0;

// int g_PerimeterSmoothMagnitude = 0;
// int g_PerimeterSmoothMagnitudeTracking = 0;

// float g_PerimeterFilterQuality = 0;
// int16_t g_PerimeterOffset = 0; // (Saved to EEPROM)
// int g_signalCounter = 0;

// bool g_PerimeterSignalStopped = false;      // This boolean indicates that the sender has notified that the wire is cut or stopped
// bool g_PerimeterSignalLost = true;          // This boolean indicates that the perimeter signal is either too weak (meaning that the perimeter wire is probably cut or the sender is stopped)
// int16_t g_PerimeterSignalLostThreshold = 0; // Threshold under which g_PerimeterSignalLost is true (Dynamic parameter Saved to EEPROM)

// bool g_PerimeterSignalLowForTracking = true;    // This boolean indicates that the perimeter signal is too weak while wire tracking meaning that the mower is no longuer "over" the wire
// int16_t g_PerimeterSignalLowTrackThreshold = 0; // Threshold under which g_PerimeterSignalLowForTracking is true (Dynamic parameter Saved to EEPROM)

/************************* Perimeter signal send task *********************************/

TaskHandle_t g_PerimeterSendTaskHandle; // Perimeter signal send task RTOS task handle

QueueHandle_t g_PerimeterSendQueue; // Perimeter signal send task RTOS queue
SemaphoreHandle_t g_PerimeterSendTimerSemaphore;
hw_timer_t * g_PerimeterSendTimerhandle = NULL;
portMUX_TYPE g_PerimeterSendTimerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE g_PerimeterSendLoopMux = portMUX_INITIALIZER_UNLOCKED;
volatile unsigned long g_PerimeterSendTimerCount = 0;
volatile int g_PerimeterSendQfull = 0;

volatile boolean g_enableSender = true;
volatile int g_SentStep = 0;

float g_PerimeterPowerLevel = 0;

/************************* Analog Read task *********************************/

TaskHandle_t g_AnaReadTaskHandle; // Sonar Read task RTOS task handle
SemaphoreHandle_t g_I2CSemaphore; // I2C resource protection semaphore

/************************* EEPROM Management *********************************/

EEPROMLoadStruct g_EEPROMLoad;

bool g_EEPROMValid = false;
bool g_EEPROMUpdate = false;

unsigned long g_LastEepromWriteTime = 0;

/************************* Eztime *********************************/
// do not place before EEprom definition section or causes conflict !!
Timezone myTime;

/************************* Display variables *********************************/

#ifdef DISPLAY_LCD2004
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE); // Uses Defaut Address
#endif

#ifdef DISPLAY_OLEDSSD1306
SSD1306Wire oled(DISPLAY_OLEDSSD1306_I2C_ADDRESS, PIN_ESP_I2C_SDA, PIN_ESP_I2C_SCL);
#endif

unsigned long g_LastDisplayUpdate = 0;             // Used to trigger screen backlight switch off

/************************* Keypad variables *********************************/

//const int g_KeyPins[KEYPAD_MAX_KEYS] = {PIN_MCP_KEYPAD_1 - 8, PIN_MCP_KEYPAD_2 - 8, PIN_MCP_KEYPAD_3 - 8, PIN_MCP_KEYPAD_4 - 8}; // GPIO B
bool g_KeyPressed[KEYPAD_MAX_KEYS] = {false, false, false, false};

/************************* MCP23017 I2C IO Extender variables *********************************/

#include <Adafruit_MCP23017.h>

Adafruit_MCP23017 IOExtend;

/************************* Rain Sensor variables *********************************/

float g_RainLevel = 0;
bool g_IsRainning = false;

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
DeviceAddress temp_1_RedSensor = {0x28, 0xff, 0x64, 0x1E, 0x0F, 0x91, 0x77, 0x48};

int g_TempErrorCount[TEMPERATURE_COUNT] = {0};
float g_Temperature[TEMPERATURE_COUNT] = {0};

/************************* INA219 I2C Battery Charge current sensor variables *********************************/

#include <Adafruit_INA219.h>

Adafruit_INA219 PerimeterCurrentSensor = Adafruit_INA219(PERIMETER_INA219_I2C_ADDRESS);

float g_PerimeterCurrent;
float g_PerimeterVoltage;
float g_PerimeterPower;

// float g_BatterySOC = 0;  // Indicates the battery state of charge in %
// bool g_BatteryRelayIsClosed;  // Indicates whether the battery relay is closed (true) or not (false)
// bool g_BatteryIsCharging = false;  // indicates whether the battery is charging or not

/************************* Voltage variables *********************************/

float g_PwrSupplyVoltage = 0;
int g_PwrSupplyStatus = PWR_SUPPLY_VOLTAGE_OK;

/************************* Fan variables *********************************/

const int g_FanPin[FAN_COUNT] = {PIN_MCP_FAN_1};
bool g_FanOn[FAN_COUNT] = {false};

/************************* Error variables *********************************/

int g_CurrentErrorCode = ERROR_NO_ERROR; // Current Error code

/************************* Mower Menu definitions *********************************/

// contains the text to display @ bottom of screen to act as a menu
String g_menuString[STATES_COUNT] = {"Send |Zzzz |Test | .... ",   // Idle
                                     "Send |     |     | .... ",   // Sleeping
                                     "     |Zzzz | ... |     ",   // Sending
                                     "     |Ack  |Text | .... "};  //Error

String g_StatesString[STATES_COUNT] = {"   IDLE   ",   // Idle
                                       " SLEEPING ",   // Sleeping
                                       " SENDING  ",   // Mowing
                                       "  ERROR   "};  //Error

/************************* Base operation statistics *********************************/

unsigned long g_totalBaseOnTime = 0; // Total time spent with perimeter On, in minutes (Saved to EEPROM)
unsigned long g_totalBaseRainTime = 0; // Total time spent with Rainning On, in minutes (Saved to EEPROM)

/************************* Program variables *********************************/

BaseState g_BaseCurrentState = BaseState::sleeping;
BaseState g_BasePreviousState = BaseState::sleeping;

MowerState g_MowerCurrentState = MowerState::idle;
float g_MowerChargeCurrent = 0;
float g_MowerBatterySOC = UNKNOWN_FLOAT;

/************************* Program debugging *********************************/

uint32_t g_TotalIdleCycleCount[2] = {0, 0};
