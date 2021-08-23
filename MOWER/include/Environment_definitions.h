/*
    This file contains the decalration of all global variables or Macros
*/
// -------------------- MQTT ------------------------
#define MQTT_COMMAND_CHANNEL "AutoMower/Command"
#define MQTT_LOG_CHANNEL "AutoMower/Log"
#define MQTT_TELEMETRY_CHANNEL "AutoMower/Telemetry"
#define MQTT_SERVER "192.168.1.3"
#define MQTT_PORT 1883

// -------------------- Type of Display ------------------------
// Select type of screen connected by un-commenting the line bellow corresponding to the display used
#define DISPLAY_LCD2004 // for I2C 2004 4*20 type LCD
//#define DISPLAY_OLEDSSD1306         // for I2C SSD1306 64*128 Oled display

/************************* I2C HMC5883L Compasss Sensor variables *********************************/

#define COMPASS_DECLINATION_ANGLE 0.03961897f

// -------------------- Network  ------------------------
#define ESPHOSTNAME "MyMower"

// -------------------- Tags diplayed in Grafana ------------------------
#define TAG_RESET "RESET"
#define TAG_MOWING "MOWING"
#define TAG_OTA "OTA"
#define TAG_CHECK "CHECK"
#define TAG_ERROR "ERROR"
#define TAG_VALUE "VALUE"
#define TAG_SEARCH "WIRE_SEARCH"
#define TAG_TO_BASE "BACK2BASE"
#define TAG_PARAM "PARAMETER"

// -------------------- Serial communications ------------------------
#define SERIAL_BAUD 115200
#define TELNET_BAUD 115200
