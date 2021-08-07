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
#ifdef LCD2004_DISPLAY
  if (lcd.begin(COLUMS, ROWS, LCD_5x8DOTS) != 1) //colums - 20, rows - 4, pixels - 5x8, SDA - D2, SCL - D1
  {
    DebugPrintln(F("LCD is not connected or lcd pins declaration is wrong."), DBG_ERROR, true);
    delay(2000);
  }
#endif
#ifdef OLEDSSD1306_DISPLAY
  oled.init();
  oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  oled.setBrightness(OLED_BRIGHTNESS);
#endif

  DebugPrintln("Display setup Done", DBG_VERBOSE, true);

  DisplayClear();
  DisplayPrint(0, 0, F("AutoMower"));
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
#ifdef LCD2004_DISPLAY
  lcd.clear();
#endif
#ifdef OLEDSSD1306_DISPLAY
  oled.clear();
#endif
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
#ifdef LCD2004_DISPLAY
  lcd.setCursor(X, Y);
  lcd.print(Text);
#endif
#ifdef OLEDSSD1306_DISPLAY
  if (OverWrite)
  {
    int length = oled.getStringWidth(Text) * OLED_PIXEL_PER_COLUMN;
    oled.setColor(BLACK);
    oled.fillRect(X * OLED_PIXEL_PER_COLUMN, Y * OLED_PIXEL_PER_LINE, length, OLED_PIXEL_PER_LINE);
    oled.display();
    oled.setColor(WHITE);
  }
  oled.drawString(X * OLED_PIXEL_PER_COLUMN, Y * OLED_PIXEL_PER_LINE, Text);
  oled.display();
#endif
};
