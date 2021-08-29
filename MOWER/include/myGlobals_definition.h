#include <Arduino.h>
#include "Environment_definitions.h"
#include "states.h"

/************************* MQTT *********************************/
#include <PubSubClient.h>

#define MQTT_TELEMETRY_SEND_INTERVAL_SLOW 2 * 60 * 1000 // in ms
#define MQTT_TELEMETRY_SEND_INTERVAL 30 * 1000 // in ms
#define MQTT_TELEMETRY_SEND_INTERVAL_FAST 10 * 1000 // in ms
#define MQTT_MAX_PAYLOAD 1024

extern PubSubClient MQTTclient;
extern int g_MQTTErrorCount;
extern unsigned long g_MQTTSendInterval;

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
#define OTA_TIMEOUT 180000

extern bool g_otaFlag;
//extern unsigned long g_OTAelapsed;

/************************* Timer ISR *********************************/

// extern hw_timer_t * g_FastTimer;
// extern hw_timer_t * g_SlowTimer;

// #define TIMER_FAST_NUMBER 0
// #define TIMER_SLOW_NUMBER 1

// #define TIMER_FAST_FREQUENCY 1000                 // 38647 Hz => 1 000 000 microseconds / 38 647 = 25.87522 microseconds
// #define TIMER_SLOW_FREQUENCY 1000 * 1000        // in microseconds => 1 second

// extern SemaphoreHandle_t g_FastTimerSemaphore;

// extern portMUX_TYPE g_FastTimerMux;
// extern portMUX_TYPE g_SlowTimerMux;

// extern volatile unsigned long g_FastTimerCount;
// extern volatile int g_SlowTimerCount;

/************************* Perimeter signal code *********************************/

// Based on Ardumower project :http://grauonline.de/alexwww/ardumower/filter/filter.html
// "pseudonoise4_pw" signal
#define PERIMETER_SIGNAL_CODE_LENGTH 24
// pseudonoise5_nrz  signal
// #define PERIMETER_SIGNAL_CODE_LENGTH 31

extern int8_t g_sigcode_norm[PERIMETER_SIGNAL_CODE_LENGTH];
extern int8_t g_sigcode_diff[PERIMETER_SIGNAL_CODE_LENGTH];

#define PERIMETER_SUBSAMPLE 4

/************************* High speed Analog Read task *********************************/
#define I2S_PORT I2S_NUM_0
#define I2S_READ_TIMEOUT 1             // in RTOS ticks
#define I2S_ADC_UNIT ADC_UNIT_1        // ADC Unit used
#define I2S_ADC_CHANNEL ADC1_CHANNEL_3 /*!< ADC1 channel 3 is GPIO39 */

// The principle is to capture in one buffer at least 2 Perimeter cable codes
// Additionnaly, the code is over-sampled 4 times as it is read 4 times faster than the sender sends the code.
// This oversamplin is needed as the coil detects the change of magnetic field cause by the chnage of direction(because the change of field)
// of the current in the perimeter wire and this field does not last very long.
// In practice, the sender sends a 1 or 0 or -1 at 9 kHz (the pulse is therefore 104 microseconds long) and the mower sensing function read the
// signal at ~38 kHz (4 times faster) and aims to capture 4 values for each "pulse" emited by the sender.
// For example, if the pulse sequence is 24 values long, the DMA buffer is 24 * 4 * 2 = 1 samples long.
// To avoid having to read the buffer too often (and effectively impacting cpu use) the DMA buffer is dimentioned to hold 2 complete sequences
// Therefore, the DMA buffer read frequency is reduced. For example, for a 24 pulse sequence, a complete sequence takes 24*104=2496 us to send and the read
// frequency for 2 sequences is 24*104*2=4992 us or approximately 200 times (1,000,000/4992) per second.
// To limit any loss of data dues to temporary slow DMA buffer read, 4 buffers are allocated.

#define I2S_SAMPLE_RATE 38400                                                       // I2S scanning rate in samples per second
#define I2S_DMA_BUFFERS 4                                                           // number of allocated I2S DMA buffers
#define I2S_DMA_BUFFER_LENGTH PERIMETER_SIGNAL_CODE_LENGTH *PERIMETER_SUBSAMPLE * 3 // in number of samples
#define PERIMETER_RAW_SAMPLES I2S_DMA_BUFFER_LENGTH * 3                             // We store more samples than just one DMA buffer to have more data to process

#define FAST_ANA_READ_TASK_ESP_CORE 1            // Core assigned to task
#define FAST_ANA_READ_TASK_PRIORITY 1            // Priority assigned to task
#define FAST_ANA_READ_TASK_STACK_SIZE 3000      // Stack assigned to task (in bytes)
#define FAST_ANA_READ_TASK_NAME "FastAnaReadTsk" // Task name

extern SemaphoreHandle_t g_ADCinUse;           // to protect access to ADC between I2S driver and other analogRead calls
extern SemaphoreHandle_t g_RawValuesSemaphore; // to protect access to shared global variables used in Perimter data Processing task

extern QueueHandle_t g_I2SQueueHandle; // Queue used by I2S driver to notify for availability of new samples in a full DMA buffer

extern TaskHandle_t g_FastAnaReadTaskHandle; // High speed analog read RTOS task handle

extern uint16_t g_raw[PERIMETER_RAW_SAMPLES]; // Circular Buffer containing last samples read from I2S DMA buffers
extern int g_rawWritePtr;                     // Pointer to last value written to g_raw circular buffer

extern unsigned int g_FastAnaReadTimeout; // Counter of I2S read time outs, indication incorrect situation
extern unsigned int g_inQueueMax;         // Max I2S notification queue waiting events (should be 0)
extern unsigned int g_inQueue;            // Accumulated I2S notification queue waiting events (should be 0)

// #define ANA_READ_TASK_ESP_CORE 1
// #define ANA_READ_TASK_SAMPLE_RATE 120000     // to be merged with TIMER_FAST_FREQUENCY
// #define ANA_READ_TASK_ADC_CHANNEL ADC1_CHANNEL_3
// #define ANA_READ_BUFFER_SIZE 512

// extern TaskHandle_t g_AnaReadTask;
// extern portMUX_TYPE g_AnaReadMux;

// extern volatile int g_readAnaBuffer[ANA_READ_BUFFER_SIZE];
// extern volatile int g_readAnaBufferPtr;
// extern volatile int g_timerCallCounter;
// extern volatile unsigned long g_AnalogReadMicrosTotal;

// extern volatile long g_MissedReadings;
// extern volatile float g_rate;
// extern volatile long g_Triggers;

/************************* Perimeter data processing task *********************************/

#define TIMER_PRESCALER 80                // timer counts every microseconds
#define PERIMETER_TIMER_PERIOD 100 * 1000 // in microseconds
#define PERIMETER_TIMER_NUMBER 0          // Timer used
#define PERIMETER_QUEUE_LEN 5             // Queue length. Not one to enable some latency to processing task

#define PERIMETER_TASK_ESP_CORE 1          // Core assigned to task
#define PERIMETER_TASK_PRIORITY 1          // Priority assigned to task
#define PERIMETER_TASK_STACK_SIZE 8000    // Stack assigned to task (in bytes)
#define PERIMETER_TASK_NAME "PerimProcTsk" // Task name

#define PERIMETER_TASK_PROCESSING_TRIGGER 1     // for perimeter data processing
#define PERIMETER_TASK_PROCESSING_CALIBRATION 2 // for calibration offset determination

#define PERIMETER_USE_DIFFERENTIAL_SIGNAL true
#define PERIMETER_SWAP_COIL_POLARITY false
// #define PERIMETER_IN_OUT_DETECTION_THRESHOLD 1000
#define PERIMETER_IN_OUT_DETECTION_THRESHOLD 400
#define PERIMETER_APPROACHING_THRESHOLD 300
// #define PERIMETER_APPROACHING_THRESHOLD PERIMETER_IN_OUT_DETECTION_THRESHOLD/ 2

extern hw_timer_t *g_PerimeterTimerhandle; // Perimeter processing task timer based trigger ISR handle

extern QueueHandle_t g_PerimeterTimerQueue; // Queue red by Perimeter processing task

extern TaskHandle_t g_PerimeterProcTaskHandle; // Perimeter processing task RTOS task handle

extern unsigned int g_PerimeterQueuefull;  // Assumulated count of full Perimeter queue events
extern unsigned int g_inPerimeterQueueMax; // Max Perimeter queue waiting events (should be 0)
extern unsigned int g_inPerimeterQueue;    // Accumulated Perimeter queue waiting events (should be 0)

// Values comming as output of Perimeter processing made available to other tasks through global variables
extern int16_t g_PerimeterRawMax;
extern int16_t g_PerimeterRawMin;
extern int16_t g_PerimeterRawAvg;
extern bool g_isInsidePerimeter;
extern unsigned long g_lastIsInsidePerimeterTime;
extern bool g_PerimetersignalTimedOut;
extern int g_PerimeterMagnitude;
extern int g_PerimeterMagnitudeAvg;
// extern int g_PerimeterMagnitudeAvgPID;
extern int g_PerimeterSmoothMagnitude;
extern int g_PerimeterSmoothMagnitudeTracking;
extern float g_PerimeterFilterQuality;
extern int16_t g_PerimeterOffset; // (Saved to EEPROM)
extern int g_signalCounter;

extern bool g_PerimeterSignalStopped;          // This boolean indicates that the sender has notified that the wire is cut or stopped
extern bool g_PerimeterSignalLost;             // This boolean indicates that the perimeter signal is either too weak (meaning that the perimeter wire is probably cut or the sender is stopped)
extern int16_t g_PerimeterSignalLostThreshold; // Threshold under which g_PerimeterSignalLost is true (Dynamic parameter Saved to EEPROM)

extern bool g_PerimeterSignalLowForTracking;       // This boolean indicates that the perimeter signal is too weak while wire tracking meaning that the mower is no longuer "over" the wire
extern int16_t g_PerimeterSignalLowTrackThreshold; // Threshold under which g_PerimeterSignalLowForTracking is true (Dynamic parameter Saved to EEPROM)

extern uint16_t g_RawCopy[PERIMETER_RAW_SAMPLES]; //  Copy of circular Buffer containing last samples read from I2S DMA buffers
extern int g_rawWritePtrCopy;                     // Pointer to last value written to g_RawCopy circular buffer copy
extern int8_t g_PerimeterSamplesForMatchedFilter[I2S_DMA_BUFFER_LENGTH];

/************************* Analog Read task *********************************/
#define ANA_READ_TASK_ESP_CORE 1        // Core assigned to task
#define ANA_READ_TASK_PRIORITY 1        // Priority assigned to task
#define ANA_READ_TASK_STACK_SIZE 5000   // Stack assigned to task (in bytes)
#define ANA_READ_TASK_NAME "AnaReadTsk" // Task name

#define ANA_READ_TASK_LOOP_WAIT 100 // In ms

extern TaskHandle_t g_AnaReadTaskHandle; // Analog Read task RTOS task handle
extern SemaphoreHandle_t g_I2CSemaphore; // I2C resource protection semaphore

/************************* EEPROM Management *********************************/

#include "EEPROM.h"
#include "EEPROM/EEPROM_Struct.h"

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
#define OLED_BRIGHTNESS_LOW 2
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
#define RAIN_SENSOR_CHECK_THRESHOLD 10
#define RAIN_SENSOR_RAINING_THRESHOLD 100 // this may have to be placed in a parameter
#define RAIN_READ_INTERVAL 30000         // in ms

/************************* I2C HMC5883L Compasss Sensor variables *********************************/
#include <Adafruit_HMC5883_U.h>

extern Adafruit_HMC5883_Unified Compass;

//#define COMPASS_PRESENT
#define COMPASS_ID 12345
#define COMPASS_PRECISION 1
#define COMPASS_X_HEADING_CORRECTION -3.5f
#define COMPASS_Y_HEADING_CORRECTION -6.5f
#define COMPASS_READ_INTERVAL 4000

extern float g_CompassHeading;          // in Degrees
extern float g_CompassHeadingCorrected; // in Degrees
extern float g_CompassXField;
extern float g_CompassYField;

/************************* UART NEO-N8M GPS variables *********************************/
#include <TinyGPS++.h>

#define GPS_BAUD 115200UL
#define GPS_READ_INTERVAL 500
#define GPS_UART Serial2
#define GPS_CHARS_TO_DETECT 20

extern TinyGPSPlus GPS; // The TinyGPS++ object
extern HardwareSerial Serial2;

extern float g_GPSHeading; // in Degrees
extern int g_GPSSatellitesFix;
extern double g_GPSHdop;
extern double g_GPSSpeed;
extern double g_GPSAltitude;
extern double g_GPSLatitude;
extern double g_GPSLongitude;

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

extern int g_TempErrorCount[TEMPERATURE_COUNT];
extern float g_Temperature[TEMPERATURE_COUNT];

/************************* INA219 I2C Battery Charge current sensor variables *********************************/
#include <Adafruit_INA219.h>

#define BATTERY_CHARGE_READ_INTERVAL 5000   // in ms
#define BATTERY_CHECK_INTERVAL 15000 // in ms
#define BATTERY_CHARGE_CURRENT_MIN 1    // in mA, used to filter out very low sensor readings
#define BATTERY_CHARGE_CURRENT_TO_STOP_CHARGE 300  // in mA, used to stop charging when charging current is low
#define BATTERY_CHARGE_CURRENT_CHARGING_THRESHOLD 50 // in mA, used to determine if battery is charging

extern Adafruit_INA219 BatteryChargeSensor;

extern float g_BatteryChargeCurrent;
extern float g_BatterySOC;  // Indicates the battery state of charge in %
extern bool g_BatteryRelayIsClosed;  // Indicates whether the battery relay is closed (true) or not (false)
extern bool g_BatteryIsCharging;  // Indicates whether the battery is charging (true) or not (false)

/************************* Voltage variables *********************************/

#define VOLTAGE_RANGE_MAX 17000 // in mV

// #define VOLTAGE_DETECTION_THRESHOLD 9
#define BATTERY_0_PERCENT_VOLTAGE 11000                // in mV
#define BATTERY_VOLTAGE_LOW_THRESHOLD 11250            // in mV
#define BATTERY_VOLTAGE_RETURN_TO_BASE_THRESHOLD 11300 // in mV
#define BATTERY_VOLTAGE_MEDIUM_THRESHOLD 11500         // in mV
#define BATTERY_VOLTAGE_NORMAL_THRESHOLD 12100         // in mV
#define BATTERY_VOLTAGE_TO_START_CHARGE 12300          // in mv
#define BATTERY_VOLTAGE_FULL_THRESHOLD 12500           // in mV
#define BATTERY_VOLTAGE_TO_STOP_CHARGE BATTERY_VOLTAGE_FULL_THRESHOLD  // in mv

#define BATTERY_VOLTAGE_OK 0               // if above BATTERY_VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_MEDIUM 1           // if between BATTERY_VOLTAGE_MEDIUM_THRESHOLD and BATTERY_VOLTAGE_NORMAL_THRESHOLD
#define BATTERY_VOLTAGE_LOW 2              // if between BATTERY_VOLTAGE_LOW_THRESHOLD and BATTERY_VOLTAGE_MEDIUM_THRESHOLD
#define BATTERY_VOLTAGE_CRITICAL 3         // if below BATTERY_VOLTAGE_LOW_THRESHOLD
#define BATTERY_VOLTAGE_READ_INTERVAL 5000 // in ms

extern float g_BatteryVoltage;          // Battery voltage in mV
extern int g_BatteryStatus;

/************************* INA219 I2C Curent sensor variables *********************************/

#define MOTOR_CURRENT_COUNT 3 // Number of motor current sensors.

extern Adafruit_INA219 MotorCurrentSensor[MOTOR_CURRENT_COUNT];

#define MOTOR_CURRENT_RIGHT 0
#define MOTOR_CURRENT_LEFT 1
#define MOTOR_CURRENT_CUT 2
#define MOTOR_CURRENT_READ_INTERVAL 500 // in ms
extern float g_MotorCurrent[MOTOR_CURRENT_COUNT];

/************************* Sonar Read task *********************************/
#define SONAR_READ_TASK_ESP_CORE 1          // Core assigned to task
#define SONAR_READ_TASK_PRIORITY 1          // Priority assigned to task
#define SONAR_READ_TASK_STACK_SIZE 3000     // Stack assigned to task (in bytes)
#define SONAR_READ_TASK_NAME "SonarReadTsk" // Task name

#define SONAR_READ_TASK_WAIT_ON_IDLE 500 // in ms
#define SONAR_READ_TASK_LOOP_TIME 150    // in ms

#define SONAR_READ_ACTIVATION_DELAY SONAR_READ_TASK_WAIT_ON_IDLE + SONAR_READ_TASK_LOOP_TIME * 2
extern TaskHandle_t g_SonarReadTaskHandle; // Sonar Read task RTOS task handle

extern bool g_SonarReadEnabled; // Global variable to suspend sonar sensor reading

/************************* HC-SR04 Sonar sensor variables *********************************/
#include <Wire.h>
#include <NewPing.h>

#define SONAR_COUNT 3           // Number of sensors.
#define SONAR_MAX_DISTANCE 200  // Maximum distance (in cm) to ping.
#define SONAR_READ_INTERVAL 300 // in ms
#define SONAR_READ_ITERATIONS 9

#define SONAR_FRONT 0
#define SONAR_LEFT 1
#define SONAR_RIGHT 2

extern NewPing sonar[SONAR_COUNT];

extern String g_sensorStr[SONAR_COUNT];

extern int g_SonarDistance[SONAR_COUNT]; // in cm

/************************* Bumper variables *********************************/

#define BUMPER_COUNT 2 // Number of sensors
#define BUMPER_LEFT 0
#define BUMPER_RIGHT 1

#define BUMPER_DEBOUNCE_TIMEOUT 100 // in ms

extern String g_bumperStr[BUMPER_COUNT];
extern int g_bumperPin[BUMPER_COUNT];

extern portMUX_TYPE g_BumperMux[BUMPER_COUNT];

extern volatile bool g_BumperTriggered[BUMPER_COUNT];

/************************* Tilt variables *********************************/

#define TILT_COUNT 2 // Number of sensors
#define TILT_HORIZONTAL 0
#define TILT_VERTICAL 1

#define TILT_DEBOUNCE_TIMEOUT 100 // in ms

extern String g_tiltStr[TILT_COUNT];
extern int g_tiltPin[TILT_COUNT];

extern portMUX_TYPE g_TiltMux[TILT_COUNT];

extern volatile bool g_TiltTriggered[TILT_COUNT];

/************************* Fan variables *********************************/

#define FAN_COUNT 2 // Number of Fans
#define FAN_1_RED 0
#define FAN_2_BLUE 1
#define FAN_UPDATE_INTERVAL 15000                     // in ms
#define FAN_START_THRESHOLD 29.5f                     // in deg C
#define FAN_STOP_THRESHOLD FAN_START_THRESHOLD - 1.5f // in deg C
#define FAN_TEST_DURATION 3000                        // in ms

extern const int g_FanPin[FAN_COUNT];
extern bool g_FanOn[FAN_COUNT];

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

#define MOTION_MOTOR_POINTS 4096  // depending on MOTION_MOTOR_PWM_RESOLUTION
#define MOTION_MOTOR_MIN_SPEED 35 // in %
#define MOTION_MOTOR_TEST_STEP_DURATION 2000

extern const int g_MotionMotorIn1Pin[MOTION_MOTOR_COUNT];
extern const int g_MotionMotorIn2Pin[MOTION_MOTOR_COUNT];
extern const int g_MotionMotorPWMChannel[MOTION_MOTOR_COUNT];
extern bool g_MotionMotorOn[MOTION_MOTOR_COUNT];
extern int g_MotionMotorDirection[MOTION_MOTOR_COUNT];
extern int g_MotionMotorSpeed[MOTION_MOTOR_COUNT];
extern String g_MotionMotorStr[MOTION_MOTOR_COUNT];

extern float g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_COUNT]; // from perimeter tracking PID control

/************************* Mower Moves variables *********************************/

#define MOWER_MOVES_SPEED_CRAWL 40  // in %
#define MOWER_MOVES_SPEED_SLOW 50   // in %
#define MOWER_MOVES_SPEED_NORMAL 80 // in %
#define MOWER_MOVES_SPEED_MAX 100   // in %

#define MOWER_MOVES_REVERSE 85 // in %

#define MOWER_MOVES_TURN_SPEED 80
#define MOWER_MOVES_TURN_ANGLE_RATIO 360.0f / 6000.0f // in Angle degrees per ms
#define MOWER_MOVES_REVERSE_FOR_TURN_DURATION 3500    // in ms

#define SONAR_MIN_DISTANCE_FOR_SLOWING 25      // in cm
#define SONAR_MIN_DISTANCE_FOR_TURN 40         // in cm
#define SONAR_MIN_DISTANCE_FOR_STOP 15         // in cm
#define SONAR_MIN_DISTANCE_FOR_PRECONDITION 20 // in cm

#define PERIMETER_MIN_MAGNITUDE_FOR_OUT_DETECTION 200 // to prevent spurious out of perimeter detections

// Mowing
#define MOWER_MOWING_TRAVEL_SPEED 100
#define MOWER_MOWING_MAX_CONSECUTVE_OBSTACLES 5
#define MOWER_MOWING_MAX_CONSECUTVE_OUTSIDE 30
#define MOWER_MOWING_CUT_DIRECTION_CHANGE_INTERVAL 20 * 60 * 1000  // in ms, how often the mower stops to change cut motor rotation direction
#define MOWER_MOWING_CUTTING_DIRECTION_WAIT_TIME 30 * 1000      // in ms, time to wait cut motor has stopped
#define MOWER_MOWING_CUT_START_WAIT 5000  // in ms, time to wait after starting cut motor before starting motion motors. Especailly susefull when battery in not @ 100%
extern unsigned long g_totalMowingTime; // Total time spent mowing, in minutes (Saved to EEPROM)

// Perimeter search function

#define PERIMETER_SEARCH_PHASE_1 1
#define PERIMETER_SEARCH_PHASE_2 2
#define PERIMETER_SEARCH_PHASE_3 3
#define PERIMETER_SEARCH_FINISHED 4

#define PERIMETER_SEARCH_REVERSE_MAX_TIME 4000    // in ms
#define PERIMETER_SEARCH_REVERSE_SPEED 90         // in %
#define PERIMETER_SEARCH_REVERSE_TIME 1000        // in %
#define PERIMETER_SEARCH_FORWARD_MAX_TIME_1 60000 // in ms
// #define PERIMETER_SEARCH_FORWARD_MAX_TIME_2 5000 // in ms
#define PERIMETER_SEARCH_FORWARD_SPEED 100 // in %
#define PERIMETER_SEARCH_FORWARD_TIME 500 // in ms
// #define PERIMETER_SEARCH_CLOCKWISE_TURN_ANGLE 135 // in deg
// #define PERIMETER_SEARCH_COUNTER_CLOCKWISE_TURN_ANGLE -135 // in deg
#define PERIMETER_SEARCH_ANGLE_INCREMENT 15 // in deg
// #define PERIMETER_SEARCH_TURN_MAX_TIME 10000 // in ms
#define PERIMETER_SEARCH_TURN_MAX_ITERATIONS 180 / PERIMETER_SEARCH_ANGLE_INCREMENT // in loop counts. It should not be necessary to turn more than 150 deg depending on wire approach angle
#define PERIMETER_SEARCH_MAX_CONSECUTVE_OBSTACLES 3
#define PERIMETER_SEARCH_AT_BASE_CURRENT 5

// Back to base function
#define BACK_TO_BASE_HEADING 0                 // in deg 0=North
#define BACK_TO_BASE_CLOCKWISE false           // in which direction to follow wire
#define BACK_TO_BASE_SPEED 100                 // in %
#define FOLLOW_WIRE_MAX_CONSECUTVE_OBSTACLES 0 // Setting to 0 will stop the wire tracking function on first obstacle

// Leaving Base base function
#define LEAVING_BASE_REVERSE_SPEED 80                 // in %
#define LEAVING_BASE_REVERSE_DURATION 10 * 1000       // in ms
//#define LEAVING_BASE_FORWARD_SPEED 100                 // in %

// Obstacle detection causes
#define OBSTACLE_DETECTED_NONE 0
#define OBSTACLE_DETECTED_BUMPER 1
#define OBSTACLE_DETECTED_FRONT 2
#define OBSTACLE_DETECTED_LEFT 3
#define OBSTACLE_DETECTED_RIGHT 4
#define OBSTACLE_DETECTED_PERIMETER 5

#define OBSTACLE_APPROACH_LOW_SPEED_MIN_DURATION 5000 // in ms

// Move count variables
extern int g_successiveObstacleDectections; // successive obstacle detections (to trigger appropriate reaction)

// Perimeter tracking function

#include <PID_v1.h>

//Define Variables used by PID library
extern double g_PIDSetpoint, g_PIDInput, g_PIDOutput;

//Specify the links and initial tuning parameters
extern PID g_PerimeterTrackPID;

#define PERIMETER_TRACKING_PID_INTERVAL 150 // in ms

extern double g_PerimeterTrackSetpoint;   // Setpoint for PID wire tracking (Saved to EEPROM)
extern double g_ParamPerimeterTrackPIDKp; // Kp PID Parameter for wire tracking (Saved to EEPROM)
extern double g_ParamPerimeterTrackPIDKi; // Ki PID Parameter for wire tracking (Saved to EEPROM)
extern double g_ParamPerimeterTrackPIDKd; // Kd PID Parameter for wire tracking (Saved to EEPROM)

/************************* CUT Motor variables *********************************/

#define CUT_MOTOR_PWM_FREQUENCY 5000
#define CUT_MOTOR_PWM_RESOLUTION 12
#define CUT_MOTOR_PWM_CHANNEL_FORWARD 2
#define CUT_MOTOR_PWM_CHANNEL_REVERSE 3

#define CUT_MOTOR_STOPPED 0
#define CUT_MOTOR_FORWARD 1
#define CUT_MOTOR_REVERSE -1
#define CUT_MOTOR_MIN_SPEED 10

#define CUT_MOTOR_FAST_STOP_INVERSE_SPEED 100   // full inverse speed
#define CUT_MOTOR_FAST_STOP_INVERSE_DURATION 750 // in ms

#define CUT_MOTOR_CHECK_INTERVAL 2000 // in ms

#define MOWER_MOWING_CUTTING_SPEED 100 // in %
#define MOWER_MOWING_CUTTING_DIRECTION CUT_MOTOR_FORWARD

extern bool g_CutMotorOn;
extern int g_CutMotorDirection;
extern int g_CutMotorSpeed;
extern bool g_CutMotorAlarm;

/************************* Error variables *********************************/
// Please update ErrorString() function in Utils to include textual description to new error message

#define ERROR_NO_ERROR 0

//General Error conditions
#define ERROR_BATTERY_CRITICAL 001
#define ERROR_VERTICAL_TILT_ACTIVATED 002
#define ERROR_HORIZONTAL_TILT_ACTIVATED 003
#define ERROR_NO_PERIMETER_SIGNAL 004

//States-related Error conditions
#define ERROR_MOWING_NO_START_BUMPER_ACTIVE 100
#define ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE 101
#define ERROR_MOWING_NO_START_TILT_ACTIVE 102
#define ERROR_MOWING_NO_START_NO_PERIMETER_SIGNAL 103
#define ERROR_MOWING_CONSECUTIVE_OBSTACLES 104
#define ERROR_MOWING_OUTSIDE_TOO_LONG 105

#define ERROR_WIRE_SEARCH_NO_START_BUMPER_ACTIVE 200
#define ERROR_WIRE_SEARCH_NO_START_OBJECT_TOO_CLOSE 201
#define ERROR_WIRE_SEARCH_NO_START_TILT_ACTIVE 202
#define ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL 203
#define ERROR_WIRE_SEARCH_PHASE_1_FAILLED 204
#define ERROR_WIRE_SEARCH_PHASE_2_FAILLED 205
#define ERROR_WIRE_SEARCH_CONSECUTIVE_OBSTACLES 206
#define ERROR_WIRE_SEARCH_PHASE_3_FAILLED 207

#define ERROR_FOLLOW_WIRE_NO_START_BUMPER_ACTIVE 210
#define ERROR_FOLLOW_WIRE_NO_START_OBJECT_TOO_CLOSE 211
#define ERROR_FOLLOW_WIRE_NO_START_TILT_ACTIVE 212
#define ERROR_FOLLOW_WIRE_NO_START_NO_PERIMETER_SIGNAL 213
#define ERROR_FOLLOW_WIRE_CONSECUTIVE_OBSTACLES 214

#define ERROR_UNDEFINED 999

extern int g_CurrentErrorCode; // Current Error code

/************************* Mower Menu definitions *********************************/

#define MENU_IDLE_TXT "Idle"
#define MENU_DETAILS_TXT "Dtls"
#define MENU_RETURN_MENU "Back|    |    |    "

#define DISPLAY_REFRESH_INTERVAL 2000                   // in ms
#define DISPLAY_IDLE_REFRESH_INTERVAL 10 * 60 * 1000    // in ms
#define DISPLAY_MOWING_REFRESH_INTERVAL 500            // in ms
#define DISPLAY_ERROR_REFRESH_INTERVAL 5000             // in ms
#define DISPLAY_TEST_REFRESH_INTERVAL 5000              // in ms
#define DISPLAY_TO_BASE_REFRESH_INTERVAL 500           // in ms
#define DISPLAY_DOCKED_REFRESH_INTERVAL 1000           // in ms
#define DISPLAY_LEAVING_BASE_REFRESH_INTERVAL 1000           // in ms

#define STATES_COUNT 7      // number of states in States enum (messy but did not find how to easly derive automatically number of elements from enum)

extern String g_menuString[STATES_COUNT];  // contains the text to display @ bottom of screen to act as a menu
extern String g_StatesString[STATES_COUNT]; // contains the text description of a state

/************************* Mower operation statistics *********************************/

extern long g_totalObstacleDectections; // Total number of obstacle detections   (Save to EEPROM)

/************************* Test sequence variables *********************************/
#define TEST_SEQ_STEP_WAIT 750
#define TEST_SEQ_STEP_ERROR_WAIT 1000

/************************* Program variables *********************************/

#define UNKNOWN_FLOAT -999.99F
#define UNKNOWN_INT -999

extern MowerState g_CurrentState;
extern MowerState g_PreviousState;

#define MOWER_DATA_DISPLAY_INTERVAL 2000 // in ms

/************************* Mower State variables *********************************/

// Mowing mode
extern int g_MowingLoopCnt; // number of loops since mowing started

/************************* Program debugging *********************************/

// For testing ONLY, if reset is not following a power-on, delay indefinately to be able to "catch" reset cause on the serial monitor.
// NOT TO BE USED IN NORMAL OPERATION AS MOWER WILL NOT RESET OUTPUTS AND MOTORS WILL KEEP RUNNING UNTILL 
// THE MOWER IS POWERED OFF OR A RESET IS PERFORMED MANUALY ON ESP32 BOARD
// Folowing line needs to be commented out for function to be active

// #define STOP_RESTART_TO_CAPTURE_CRASH_DUMP true

// Perimeter wire signal debugging

// Folowing line needs to be commented out for function to be active

#define MQTT_GRAPH_DEBUG true

#ifdef MQTT_GRAPH_DEBUG
extern bool g_MQTTGraphDebug;    // to start/stop the transmission of MQTT debug data
extern bool g_MQTTGraphRawDebug; // to start/stop the transmission of MQTT raw debug data
#define MQTT_DEBUG_CHANNEL "MQTTest/Data"
#define MQTT_DEBUG_RAW_CHANNEL "MQTTest/DataRaw"

#endif

// Perimeter tracking PID debugging
// Folowing line needs to be commented out for function to be active

#define MQTT_PID_GRAPH_DEBUG true       // to enable sending by MQTT of PID related data

#ifdef MQTT_PID_GRAPH_DEBUG
extern bool g_MQTTPIDGraphDebug; // to start/stop the transmission of MQTT debug data
#define MQTT_PID_DEBUG_CHANNEL "Automower/PIDDebug"
#endif
