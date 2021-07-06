#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "LCD/LCD.h"
#include "Utils/Utils.h"

/**
 * I2C LCD  Setup function
 * 
 */
void LCDSetup(void)
{

//  while (lcd.begin(COLUMS, ROWS, LCD_5x8DOTS) != 1) //colums - 20, rows - 4, pixels - 5x8, SDA - D2, SCL - D1
  if (lcd.begin(COLUMS, ROWS, LCD_5x8DOTS) != 1) //colums - 20, rows - 4, pixels - 5x8, SDA - D2, SCL - D1
  {
    DebugPrintln(F("LCD is not connected or lcd pins declaration is wrong."), DBG_ERROR, true);
    delay(2000);
  }
  
  DebugPrintln("LCD setup Done", DBG_VERBOSE, true);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("AutoMower")); 
  lcd.setCursor(6, 1);
  lcd.print(F("by Richards...")); 
  lcd.setCursor(0, 3);
  lcd.print(__DATE__);
  lcd.print(F(" "));
  lcd.print(__TIME__);
  delay(2000);

}