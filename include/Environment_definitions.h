/*
    This file contains the decalration of all global variables or Macros
*/
// -------------------- MQTT ------------------------
#define MQTT_COMMAND_CHANNEL "AutoMower/Command"
#define MQTT_LOG_CHANNEL "AutoMower/Log"
#define MQTT_TELEMETRY_CHANNEL "AutoMower/Telemetry"
#define MQTT_SERVER "192.168.1.3"
#define MQTT_PORT 1883

/************************* I2C HMC5883L Compasss Sensor variables *********************************/

#define COMPASS_DECLINATION_ANGLE 0.03961897f

// -------------------- Network  ------------------------
#define ESPHOSTNAME "MyMower"

// -------------------- Tags ------------------------
#define TAG_RESET "RESET"
#define TAG_OTA "OTA"
#define TAG_CHECK "CHECK"
#define TAG_ERROR "ERROR"

// -------------------- Serial communications ------------------------
#define SERIAL_BAUD 115200
#define TELNET_BAUD 115200


