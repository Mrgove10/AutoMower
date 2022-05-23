#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Display/Display.h"
#include "Utils/Utils.h"

/**
 * Display Setup function
 * 
 */
void DisplaySetup(void)
{
#ifdef DISPLAY_LCD2004
  if (lcd.begin(DISPLAY_COLUMNS, DISPLAY_ROWS, LCD_5x8DOTS) != 1) //columns - 20, rows - 4, pixels - 5x8, SDA - D2, SCL - D1
  {
    DebugPrintln(F("LCD is not connected or lcd pins declaration is wrong."), DBG_ERROR, true);
  }
#endif
#ifdef DISPLAY_OLEDSSD1306
  oled.init();
  oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  oled.setBrightness(OLED_BRIGHTNESS_NORMAL);
#endif

  DebugPrintln("Display setup Done", DBG_VERBOSE, true);

  DisplayClear();
  DisplayPrint(0, 0, F("AutoMower Base"));
  DisplayPrint(6, 1, F("by Richards..."));
  DisplayPrint(0, 3, String(__DATE__) + " " + String(__TIME__));
  delay(2000);
}

/**
 * Display clear  function
 * 
 */
void DisplayClear(void)
{
  // Ensure exclusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

#ifdef DISPLAY_LCD2004
  lcd.clear();
  lcd.backlight();
#endif
#ifdef DISPLAY_OLEDSSD1306
  oled.clear();
  oled.setBrightness(OLED_BRIGHTNESS_NORMAL);
#endif

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  // Memorise last screen action
  g_LastDisplayUpdate = millis();
}

/**
 * Display text function
 * @param X column where to display text (0 is first column)
 * @param Y line where to display text (0 is first line)
 * @param Text to display
 * @param Overwrite optional bool to indicate if text is to be overwritten without a full screen clear
 *  */
void DisplayPrint(int X, int Y, String Text, const bool OverWrite)
{
  // Ensure exclusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

#ifdef DISPLAY_LCD2004
  lcd.backlight();
  lcd.setCursor(X, Y);
  lcd.print(Text);
#endif
#ifdef DISPLAY_OLEDSSD1306
  if (OverWrite)
  {
    int length = oled.getStringWidth(Text) * OLED_PIXEL_PER_COLUMN;
    oled.setColor(BLACK);
    oled.fillRect(X * OLED_PIXEL_PER_COLUMN, Y * OLED_PIXEL_PER_LINE, length, OLED_PIXEL_PER_LINE);
    oled.display();
    oled.setColor(WHITE);
  }
  oled.drawString(X * OLED_PIXEL_PER_COLUMN, Y * OLED_PIXEL_PER_LINE, Text);
  oled.setBrightness(OLED_BRIGHTNESS_NORMAL);
  oled.display();
#endif

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);
  
  // Memorise last screen action
  g_LastDisplayUpdate = millis();
};

/**
 * Display backlight dimming function (energy and screen saver)
 * 
 * @param timeout timeout to be used to dimm screen when has not been updated for the timeout period
 */
void DisplayDimming(const unsigned long timeout)
{
  if (millis() - g_LastDisplayUpdate > timeout && g_LastDisplayUpdate != 0)
  {
    // Ensure exclusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

#ifdef DISPLAY_LCD2004
    lcd.noBacklight();
#endif
#ifdef DISPLAY_OLEDSSD1306
    oled.setBrightness(OLED_BRIGHTNESS_LOW);
#endif

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

    DebugPrintln("Display dimming Done", DBG_VERBOSE, true);

  // Clear last screen action
  g_LastDisplayUpdate = 0;
  }
}