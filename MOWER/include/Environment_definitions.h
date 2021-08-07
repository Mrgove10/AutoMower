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
#define LCD2004_DISPLAY             // for I2C 2004 4*20 type LCD
//#define OLEDSSD1306_DISPLAY         // for I2C SSD1306 64*128 Oled display

/************************* I2C HMC5883L Compasss Sensor variables *********************************/

#define COMPASS_DECLINATION_ANGLE 0.03961897f

// -------------------- Network  ------------------------
#define ESPHOSTNAME "MyMower"

// -------------------- Tags ------------------------
#define TAG_RESET "RESET"
#define TAG_MOWING "MOWING"
#define TAG_OTA "OTA"
#define TAG_CHECK "CHECK"
#define TAG_ERROR "ERROR"
#define TAG_VALUE "VALUE"

// -------------------- Serial communications ------------------------
#define SERIAL_BAUD 115200
#define TELNET_BAUD 115200
