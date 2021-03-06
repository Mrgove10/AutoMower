// MCP 23017 Extender pins
#define PIN_MCP_SPARE_1 0
#define PIN_MCP_BATTERY_CHARGE_RELAY 1
// #define PIN_MCP_SPARE_2 1
#define PIN_MCP_MOTOR_CUT_LN1 2
#define PIN_MCP_MOTOR_CUT_LN2 3
#define PIN_MCP_MOTOR_LEFT_LN1 4
#define PIN_MCP_MOTOR_LEFT_LN2 5
#define PIN_MCP_MOTOR_RIGHT_LN1 6
#define PIN_MCP_MOTOR_RIGHT_LN2 7
#define PIN_MCP_KEYPAD_1 8
#define PIN_MCP_KEYPAD_2 9
#define PIN_MCP_KEYPAD_3 10
#define PIN_MCP_KEYPAD_4 11
#define PIN_MCP_FAN_1 12
#define PIN_MCP_FAN_2 13
#define PIN_MCP_MOTOR_CUT_HIGH_AMP 14
#define PIN_MCP_SPARE_3 15

// ESP pins
#define PIN_ESP_TILT_HORIZONTAL 4
#define PIN_ESP_TILT_VERTICAL 5

//#define PIN_ESP_MOTOR_CUT_RELAY 12    // replaced by
#define PIN_ESP_MOTOR_CUT_PWM_REVERSE 12

#define PIN_ESP_SONAR_LEFT 13

#define PIN_ESP_BUMPER_LEFT 14
#define PIN_ESP_BUMPER_RIGHT 15

#define PIN_ESP_GPS_RX 16
#define PIN_ESP_GPS_TX 17

#define PIN_ESP_SONAR_CENTER 18
#define PIN_ESP_SONAR_RIGHT 19

#define PIN_ESP_I2C_SDA 21
#define PIN_ESP_I2C_SCL 22

#define PIN_ESP_TEMP 23

#define PIN_ESP_MOTOR_CUT_PWM_FORWARD 25
#define PIN_ESP_MOTOR_LEFT_PWM 26
#define PIN_ESP_MOTOR_RIGHT_PWM 27

#define PIN_ESP_BUZZER 32
#define PIN_ESP_SPARE_2 33

#define PIN_ESP_RAIN 34
#define PIN_ESP_AMP_CHARGE 35
#define PIN_ESP_BAT_VOLT 36
#define PIN_ESP_PERIMETER 39 // This pin is to be read by I2S Driver. Refer to I2S_ADC_CHANNEL definition
