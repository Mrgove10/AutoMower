
#include <Arduino.h>
#include <definitions.h>
#include <NewPing.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

// multiplexer
Adafruit_MCP23017 mcp;

// Ultrasonic sensors
NewPing sonar[SONAR_NUM] = {                                           // Sensor object array.
    NewPing(PIN_ESP_SONAR_CENTER, PIN_ESP_SONAR_CENTER, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
    NewPing(PIN_ESP_SONAR_LEFT, PIN_ESP_SONAR_LEFT, MAX_DISTANCE),
    NewPing(PIN_ESP_SONAR_RIGHT, PIN_ESP_SONAR_RIGHT, MAX_DISTANCE)};

// Temperature sensors
OneWire ds(PIN_ESP_TEMP);

// screen
LiquidCrystal_I2C lcd(0x38);

void multiplexSetup()
{
  mcp.begin();
  mcp.pinMode(0, INPUT);
  mcp.pullUp(0, HIGH); // turn on a 100K pullup internally
}

void lcdSetup()
{
  lcd.begin(SCREEN_COL, SCREEN_LINES); // columns, lines
  lcd.clear();
}

void setup()
{
  Serial.begin(115200);
  multiplexSetup();
}

void loop()
{
  for (uint8_t i = 0; i < SONAR_NUM; i++)
  {            // Loop through each sensor and display results.
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    Serial.print(i);
    Serial.print("=");
    Serial.print(sonar[i].ping_cm());
    Serial.print("cm ");
  }
}