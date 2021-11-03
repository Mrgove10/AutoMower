#include <Arduino.h>
#include "Environment_definitions.h"
#include "MowerStates.h"
#include "BaseStates.h"
#include <esp_freertos_hooks.h>


/************************* MQTT *********************************/

#include <PubSubClient.h>

#define MQTT_TELEMETRY_SEND_INTERVAL_SLOW 2 * 60 * 1000 // in ms
#define MQTT_TELEMETRY_SEND_INTERVAL 20 * 1000 // in ms
#define MQTT_TELEMETRY_SEND_INTERVAL_FAST 15 * 1000 // in ms
#define MQTT_PERIMETER_STATUS_SEND_INTERVAL 45 * 1000 // in ms
#define MQTT_RAIN_STATUS_SEND_INTERVAL 60 * 1000 // in ms
#define MQTT_MAX_PAYLOAD 1024
#define MQTT_MOWER_TELEMETRY_TIMEOUT 3 * 60 * 1000 // in ms

extern PubSubClient MQTTclient;
extern int g_MQTTErrorCount;
extern unsigned long g_MQTTSendInterval;
extern unsigned long g_LastMowerTelemetryReceived;

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

extern int g_debugLevel;
extern SemaphoreHandle_t g_MySerialSemaphore;

/************************* OTA *********************************/

#include <ArduinoOTA.h>

#define OTA_PORT 3232
#define OTA_TIMEOUT 180000UL

extern bool g_otaFlag;
//extern unsigned long g_OTAelapsed;

/************************* Perimeter signal code *********************************/

// Based on Ardumower project :http://grauonline.de/alexwww/ardumower/filter/filter.html
// "pseudonoise4_pw" signal
#define PERIMETER_SIGNAL_CODE_LENGTH 24
// pseudonoise5_nrz  signal
// #define PERIMETER_SIGNAL_CODE_LENGTH 31

extern const int8_t g_sigcode[PERIMETER_SIGNAL_CODE_LENGTH];

// /************************* Perimeter data processing task *********************************/

// #define TIMER_PRESCALER 80                // timer counts every microseconds
// #define PERIMETER_TIMER_PERIOD 100 * 1000 // in microseconds
// #define PERIMETER_TIMER_NUMBER 0          // Timer used
// #define PERIMETER_QUEUE_LEN 5             // Queue length. Not one to enable some latency to processing task

// #define PERIMETER_TASK_ESP_CORE 1          // Core assigned to task
// #define PERIMETER_TASK_PRIORITY 1          // Priority assigned to task
// #define PERIMETER_TASK_STACK_SIZE 8000    // Stack assigned to task (in bytes)
// #define PERIMETER_TASK_NAME "PerimProcTsk" // Task name

// #define PERIMETER_TASK_PROCESSING_TRIGGER 1     // for perimeter data processing
// #define PERIMETER_TASK_PROCESSING_CALIBRATION 2 // for calibration offset determination

// #define PERIMETER_USE_DIFFERENTIAL_SIGNAL true
// #define PERIMETER_SWAP_COIL_POLARITY false
// // #define PERIMETER_IN_OUT_DETECTION_THRESHOLD 1000
// #define PERIMETER_IN_OUT_DETECTION_THRESHOLD 600
// #define PERIMETER_APPROACHING_THRESHOLD 500
// // #define PERIMETER_APPROACHING_THRESHOLD PERIMETER_IN_OUT_DETECTION_THRESHOLD/ 2

// extern hw_timer_t *g_PerimeterTimerhandle; // Perimeter processing task timer based trigger ISR handle

// extern QueueHandle_t g_PerimeterTimerQueue; // Queue red by Perimeter processing task

// extern TaskHandle_t g_PerimeterProcTaskHandle; // Perimeter processing task RTOS task handle

// extern unsigned int g_PerimeterQueuefull;  // Assumulated count of full Perimeter queue events
// extern unsigned int g_inPerimeterQueueMax; // Max Perimeter queue waiting events (should be 0)
// extern unsigned int g_inPerimeterQueue;    // Accumulated Perimeter queue waiting events (should be 0)

// // Values comming as output of Perimeter processing made available to other tasks through global variables
// extern int16_t g_PerimeterRawMax;
// extern int16_t g_PerimeterRawMin;
// extern int16_t g_PerimeterRawAvg;
// extern bool g_isInsidePerimeter;
// extern unsigned long g_lastIsInsidePerimeterTime;
// extern bool g_PerimetersignalTimedOut;
// extern int g_PerimeterMagnitude;
// extern int g_PerimeterMagnitudeAvg;
// // extern int g_PerimeterMagnitudeAvgPID;
// extern int g_PerimeterSmoothMagnitude;
// extern int g_PerimeterSmoothMagnitudeTracking;
// extern float g_PerimeterFilterQuality;
// extern int16_t g_PerimeterOffset; // (Saved to EEPROM)
// extern int g_signalCounter;

// extern bool g_PerimeterSignalStopped;          // This boolean indicates that the sender has notified that the wire is cut or stopped
// extern bool g_PerimeterSignalLost;             // This boolean indicates that the perimeter signal is either too weak (meaning that the perimeter wire is probably cut or the sender is stopped)
// extern int16_t g_PerimeterSignalLostThreshold; // Threshold under which g_PerimeterSignalLost is true (Dynamic parameter Saved to EEPROM)

// extern bool g_PerimeterSignalLowForTracking;       // This boolean indicates that the perimeter signal is too weak while wire tracking meaning that the mower is no longuer "over" the wire
// extern int16_t g_PerimeterSignalLowTrackThreshold; // Threshold under which g_PerimeterSignalLowForTracking is true (Dynamic parameter Saved to EEPROM)


/************************* Perimeter signal send task *********************************/

#define PERIMETER_SEND_TASK_ESP_CORE 1        // Core assigned to task
#define PERIMETER_SEND_TASK_PRIORITY 1        // Priority assigned to task
#define PERIMETER_SEND_TASK_STACK_SIZE 5000   // Stack assigned to task (in bytes)
#define PERIMETER_SEND_TASK_NAME "PerimSendTsk" // Task name

#define PERIMETER_SEND_TIMER_NUMBER 0
#define PERIMETER_SEND_TIMER_PRESCALER 80
#define PERIMETER_SEND_TIMER_INTERVAL 1*104  // in microseconds, 1000 = 1Hz

#define PERIMETER_SEND_PWM_FREQUENCY 5000
#define PERIMETER_SEND_PWM_RESOLUTION 12
#define PERIMETER_SEND_POINTS 4096
#define PERIMETER_SEND_PWM_CHANNEL 0

//#define PERIMETER_SEND_TASK_LOOP_WAIT 50 // In ms

extern TaskHandle_t g_PerimeterSendTaskHandle; // Perimeter signal send task RTOS task handle

# define PERIMETER_SEND_QUEUE_LEN 10
extern QueueHandle_t g_PerimeterSendQueue; // Perimeter signal send task RTOS queue
extern SemaphoreHandle_t g_PerimeterSendTimerSemaphore;
extern hw_timer_t * g_PerimeterSendTimerhandle;
extern portMUX_TYPE g_PerimeterSendTimerMux;
extern portMUX_TYPE g_PerimeterSendLoopMux;
extern volatile unsigned long g_PerimeterSendTimerCount;
extern volatile int g_PerimeterSendQfull;

extern volatile boolean g_enableSender;
extern volatile int g_SentStep;
extern float g_PerimeterPowerLevel;       // Stored in EEPROM

/************************* Analog Read task *********************************/

#define ANA_READ_TASK_ESP_CORE 1        // Core assigned to task
#define ANA_READ_TASK_PRIORITY 1        // Priority assigned to task
#define ANA_READ_TASK_STACK_SIZE 5000   // Stack assigned to task (in bytes)
#define ANA_READ_TASK_NAME "AnaReadTsk" // Task name

#define ANA_READ_TASK_LOOP_WAIT 50 // In ms

extern TaskHandle_t g_AnaReadTaskHandle; // Analog Read task RTOS task handle
extern SemaphoreHandle_t g_I2CSemaphore; // I2C resource protection semaphore

/************************* EEPROM Management *********************************/

#include "EEPROM.h"
#include "EEPROM_Struct.h"

extern EEPROMLoadStruct g_EEPROMLoad;
extern bool g_EEPROMValid;
extern bool g_EEPROMUpdate;

extern unsigned long g_LastEepromWriteTime;

/************************* Eztime *********************************/

// do not place before EEprom definition section or causes conflict !!
#include <ezTime.h>
extern Timezone myTime;
#define NTP_REFRESH 3600
#define POSIXTZ "CET-1CEST,M3.5.0,M10.5.0/3"

/************************* Display variables *********************************/

// The type of screen connected is defined int he Environment_definitions.h file

#ifdef DISPLAY_LCD2004
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#endif

#ifdef DISPLAY_OLEDSSD1306
#include <SSD1306.h>
extern SSD1306Wire oled;
#define OLED_PIXEL_PER_LINE 16  // to reproduce LCD "form factor" on OLED display : 64/4
#define OLED_PIXEL_PER_COLUMN 7 // to reproduce LCD "form factor" on OLED display : 128/20 => 6.4 rounded up to 7
#define OLED_BRIGHTNESS_NORMAL 255
#define OLED_BRIGHTNESS_LOW 1
#endif

#define DISPLAY_COLUMS 20
#define DISPLAY_ROWS 4

#define DISPLAY_BACKLIGHT_OFF_DELAY 2 * 60 * 1000   // Used to trigger screen backlight switch off, in ms
extern unsigned long g_LastDisplayUpdate;             // Used to trigger screen backlight switch off

/************************* Keypad variables *********************************/

#define KEYPAD_READ_INTERVAL 250 // in ms
#define KEYPAD_MAX_KEYS 4
#define KEYPAD_GPIO 1 // GPIO B

#define KEYPAD_KEY_1 0
#define KEYPAD_KEY_2 1
#define KEYPAD_KEY_3 2
#define KEYPAD_KEY_4 3

//extern const int g_KeyPins[KEYPAD_MAX_KEYS];
extern bool g_KeyPressed[KEYPAD_MAX_KEYS];

/************************* MCP23017 I2C IO Extender variables *********************************/

#include <Adafruit_MCP23017.h>

extern Adafruit_MCP23017 IOExtend;

/************************* Rain Sensor variables *********************************/

#define RAIN_SENSOR_CHECK_THRESHOLD 0
#define RAIN_SENSOR_RAINING_THRESHOLD 1.0f // this may have to be placed in a parameter
#define RAIN_READ_INTERVAL 30000         // in ms

extern float g_RainLevel;
extern bool g_IsRainning;

/************************* DS18D20 temperature sensor variables *********************************/

#include <DallasTemperature.h>
extern OneWire TemperatureOneWire;
extern DallasTemperature TemperatureSensors;
extern DeviceAddress temp_1_RedSensor;
extern DeviceAddress temp_2_BlueSensor;

#define TEMPERATURE_COUNT 1 // Number of temperature sensors
#define TEMPERATURE_1_RED 0
#define TEMPERATURE_READ_INTERVAL 5000 // in ms
#define BASE_TEMPERATURE_TOO_HIGH_THRESHOLD 40.0f // in celcius

extern int g_TempErrorCount[TEMPERATURE_COUNT];
extern float g_Temperature[TEMPERATURE_COUNT];

/************************* INA219 I2C Perimeter current sensor variables *********************************/

#include <Adafruit_INA219.h>

#define PERIMETER_CURRENT_READ_INTERVAL 2000               // in ms
#define PERIMETER_CHECK_INTERVAL 15000                    // in ms
#define PERIMETER_CURRENT_CURRENT_MIN 1                    // in mA, used to filter out very low sensor readings

#define BASE_PERIMETER_CURRENT_TOO_LOW_THRESHOLD 300 // in mA, at 100% Powerlevel
#define BASE_PERIMETER_CURRENT_TOO_HIGH_THRESHOLD 800 // in mA,

extern Adafruit_INA219 PerimeterCurrentSensor;

extern float g_PerimeterCurrent;
extern float g_PerimeterVoltage;
extern float g_PerimeterPower;

/************************* Voltage variables *********************************/

#define VOLTAGE_RANGE_MAX 17000 // in mV

// #define VOLTAGE_DETECTION_THRESHOLD 9
// #define BATTERY_0_PERCENT_VOLTAGE 10900                // in mV
 #define PWR_SUPPLY_VOLTAGE_LOW_THRESHOLD 11150            // in mV
// #define BATTERY_VOLTAGE_RETURN_TO_BASE_THRESHOLD 11100 // in mV
 #define PWR_SUPPLY_VOLTAGE_MEDIUM_THRESHOLD 11400         // in mV
 #define PWR_SUPPLY_VOLTAGE_NORMAL_THRESHOLD 12000         // in mV
// #define BATTERY_VOLTAGE_TO_START_CHARGE 12350          // in mv
// #define BATTERY_VOLTAGE_FULL_THRESHOLD 12400           // in mV

#define PWR_SUPPLY_VOLTAGE_OK 0               // if above BATTERY_VOLTAGE_NORMAL_THRESHOLD
#define PWR_SUPPLY_VOLTAGE_MEDIUM 1           // if between BATTERY_VOLTAGE_MEDIUM_THRESHOLD and BATTERY_VOLTAGE_NORMAL_THRESHOLD
#define PWR_SUPPLY_VOLTAGE_LOW 2              // if between BATTERY_VOLTAGE_LOW_THRESHOLD and BATTERY_VOLTAGE_MEDIUM_THRESHOLD
#define PWR_SUPPLY_VOLTAGE_CRITICAL 3         // if below BATTERY_VOLTAGE_LOW_THRESHOLD
#define PWR_SUPPLY_VOLTAGE_READ_INTERVAL 10000 // in ms

extern float g_PwrSupplyVoltage;          // Power Supply voltage in mV
extern int g_PwrSupplyStatus;

/************************* Fan variables *********************************/

#define FAN_COUNT 1                                     // Number of Fans
#define FAN_1_RED 0                                     // Cut motor Fan
#define FAN_UPDATE_INTERVAL 15000                       // in ms
#define FAN_1_START_THRESHOLD 35.0f                     // in deg C
#define FAN_1_STOP_THRESHOLD FAN_1_START_THRESHOLD - 4.0f // in deg C
#define FAN_TEST_DURATION 3000                          // in ms

extern const int g_FanPin[FAN_COUNT];
extern bool g_FanOn[FAN_COUNT];

/************************* Error variables *********************************/
// Please update ErrorString() function in Utils to include textual description to new error message

#define ERROR_NO_ERROR 0

//General Error conditions
#define ERROR_PERIMETER_CURRENT_TOO_LOW 001
#define ERROR_PERIMETER_CURRENT_TOO_HIGH 002
#define ERROR_TEMPERATURE_TOO_HIGH 003
#define ERROR_NO_MOWER_DATA 004

//States-related Error conditions

#define ERROR_UNDEFINED 999

extern int g_CurrentErrorCode; // Current Error code

/************************* Mower Menu definitions *********************************/

#define MENU_IDLE_TXT "Idle"
#define MENU_DETAILS_TXT "Dtls"
#define MENU_RETURN_MENU "Back|    |    |    "

#define DISPLAY_REFRESH_INTERVAL 2000                   // in ms
#define DISPLAY_IDLE_REFRESH_INTERVAL 1000              // in ms
#define DISPLAY_SENDING_REFRESH_INTERVAL 1000             // in ms
#define DISPLAY_ERROR_REFRESH_INTERVAL 5000             // in ms
#define DISPLAY_SLEEPING_REFRESH_INTERVAL 1000            // in ms

#define STATES_COUNT 4      // number of states in States enum (messy but did not find how to easly derive automatically number of elements from enum)

extern String g_menuString[STATES_COUNT];  // contains the text to display @ bottom of screen to act as a menu
extern String g_StatesString[STATES_COUNT]; // contains the text description of a state

/************************* Base operation statistics *********************************/

extern unsigned long g_totalBaseOnTime; // Total time spent with perimeter On, in minutes (Saved to EEPROM)
extern unsigned long g_totalBaseRainTime; // Total time spent with Rainning On, in minutes (Saved to EEPROM)

/************************* Test sequence variables *********************************/

#define TEST_SEQ_STEP_WAIT 750
#define TEST_SEQ_STEP_ERROR_WAIT 1000

/************************* Program variables *********************************/

#define UNKNOWN_FLOAT -999.99F
#define UNKNOWN_INT -999

extern BaseState g_BaseCurrentState;
extern BaseState g_BasePreviousState;

extern MowerState g_MowerCurrentState;
extern float g_MowerChargeCurrent;

#define BASE_DATA_DISPLAY_INTERVAL 2000 // in ms

/************************* Program debugging *********************************/

// For testing ONLY, if reset is not following a power-on, delay indefinately to be able to "catch" reset cause on the serial monitor.
// NOT TO BE USED IN NORMAL OPERATION AS MOWER WILL NOT RESET OUTPUTS AND MOTORS WILL KEEP RUNNING UNTILL 
// THE MOWER IS POWERED OFF OR A RESET IS PERFORMED MANUALY ON ESP32 BOARD
// Folowing line needs to be commented out for function to be active

// #define STOP_RESTART_TO_CAPTURE_CRASH_DUMP true

extern uint32_t g_TotalIdleCycleCount[2];
