#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "BaseDisplay/BaseDisplay.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "Keypad/Keypad.h"
#include "StartupChecks.h"
#include "Rain/Rain.h"

//---------------------------------------------------------------------------------------------------------------------------
//
// ERROR Mode
//
//---------------------------------------------------------------------------------------------------------------------------


/**
 * Display the ERROR state screen
 * @param refresh boolean to force full screen update
 * */
void errorDisplay(bool refresh)
{
  static bool inSubmenu = false;
  static int submenuNum = 0;
  static unsigned long lastRefresh = 0;
  static bool internalRefresh = false;

  if ((refresh || internalRefresh) && !inSubmenu)
  { 
    // Clear screen and display header and menu bar
    DisplayClear();
    headerDisplay("", true);
    menuDisplay(int(g_BaseCurrentState));
  }

  if (millis() - lastRefresh > DISPLAY_ERROR_REFRESH_INTERVAL || refresh || internalRefresh)
  {
    String errorStr = ErrorString(g_CurrentErrorCode);

    if (inSubmenu)
    {
        switch (submenuNum)
        {
        case 1:
            /* Action submenu - No display */
            break;
        case 2:
            /* Action submenu - No display */
            break;
        case 3:
            DisplayClear();
            DisplayPrint(0,0, errorStr.substring(0,min(int(errorStr.length()),20)), true);
            if (errorStr.length() >= 20)
            {
              DisplayPrint(0,1, errorStr.substring(20,min(int(errorStr.length()),40)), true);
            }
            if (errorStr.length() >= 40)
            {
              DisplayPrint(0,2, errorStr.substring(40,min(int(errorStr.length()),60)), true);
            }
            menuDisplay(-1);
            break;
        case 4:
            DisplayClear();
            DisplayPrint(0,1, "Temp:" + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            menuDisplay(-1);
            break;
        default:
            break;
        }
    }
    else
    {
      // Display State and other state related informations
      DisplayPrint(3,1,g_StatesString[int(g_BaseCurrentState)]);
      DisplayPrint(1,2, "Error # " + String(g_CurrentErrorCode), true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    // Do nothing
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    DebugPrintln("Base Acknowledgement requested (local)", DBG_ERROR, true);
    g_BaseCurrentState = BaseState::idle;
    g_CurrentErrorCode = ERROR_NO_ERROR;
    delay(400);  // to ensure key is released 
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    inSubmenu = true;
    submenuNum = 3;
    internalRefresh = true;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_4])
  {
    inSubmenu = true;
    submenuNum = 4;
    internalRefresh = true;
  }

  // leaving submenu using "1" key
  if (inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    inSubmenu = false;
    submenuNum = 0;
    internalRefresh = true;
    delay(400);  // to ensure key is released 
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Idle Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Display the idle state screen
 * @param refresh boolean to force full screen update
 * */
void idleDisplay(bool refresh)
{
  static bool inSubmenu = false;
  static int submenuNum = 0;
  static unsigned long lastRefresh = 0;
  static bool internalRefresh = false;

  if ((refresh || internalRefresh) && !inSubmenu)
  { 
    // Clear screen and display header and menu bar
    DisplayClear();
    headerDisplay("", true);
    menuDisplay(int(g_BaseCurrentState));
  }

  if (millis() - lastRefresh > DISPLAY_IDLE_REFRESH_INTERVAL || refresh || internalRefresh)
  {
    if (inSubmenu)
    {
        switch (submenuNum)
        {
        case 1:
            /* Action submenu - No display */
            break;
        case 2:
            /* Action submenu - No display */
            break;
        case 3:
            /* Action submenu - No display */
            break;
        case 4:
            DisplayClear();
            headerDisplay(g_StatesString[int(g_BaseCurrentState)], true);
            // DisplayPrint(0, 1, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut: " + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            // DisplayPrint(0, 1, "Pit:" + String(g_pitchAngle, 1) + " Rol: " + String(g_rollAngle, 1) + "    ", true);
            // // Display currents
            // DisplayPrint(0, 2, "L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) +
            //                    " R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + 
            //                    " Ch:" + String(g_BatteryChargeCurrent, 0), true);
            menuDisplay(-1);
            break;
        default:
            break;
        }
    }
    else
    {
      headerDisplay("", true);
      // Display State and other state related informations
      DisplayPrint(5,1,g_StatesString[int(g_BaseCurrentState)]);
      String timeStr = myTime.dateTime("H:i:s");
      DisplayPrint(5,2,timeStr,true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    g_BaseCurrentState = BaseState::sending;
    delay(400);  // to ensure key is released 
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    g_BaseCurrentState = BaseState::sleeping;
    delay(400);  // to ensure key is released 
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    StartupChecks(true);
    internalRefresh = true;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_4])
  {
    inSubmenu = true;
    submenuNum = 4;
    internalRefresh = true;
  }

  // leaving submenu using "1" key
  if (inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    inSubmenu = false;
    submenuNum = 0;
    internalRefresh = true;
    delay(400);  // to ensure key is released 
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Sending Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Display the Sending state screen
 * @param refresh boolean to force full screen update
 * */
void sendingDisplay(bool refresh)
{
  static bool inSubmenu = false;
  static int submenuNum = 0;
  static unsigned long lastRefresh = 0;
  static bool internalRefresh = false;

  if ((refresh || internalRefresh) && !inSubmenu)
  { 
    // Clear screen and display header and menu bar
    DisplayClear();
    headerDisplay("", true);
    menuDisplay(int(g_BaseCurrentState));
  }

  if (millis() - lastRefresh > DISPLAY_SENDING_REFRESH_INTERVAL || refresh || internalRefresh)
  {
    if (inSubmenu)
    {
      String RainStr = "Rain ";

      switch (submenuNum)
      {
      case 1:
          /* Action submenu - No display */
          break;
      case 2:
          /* Action submenu - No display */
          break;
      case 3:
          DisplayClear();
          headerDisplay(g_StatesString[int(g_BaseCurrentState)], true);
          DisplayPrint(0, 1, "Pwr" + String(g_PwrSupplyVoltage / 1000.0f , 2) + "V", true);

          if (isRaining())
          {
            RainStr = RainStr + "Yes";
          }
          else
          {
            RainStr = RainStr + "No";
          }
          DisplayPrint(0, 2, RainStr + " (" + String(g_RainLevel, 2) + ")", true);
          menuDisplay(-1);
          break;
      case 4:
          /* No display */
          break;
      default:
          break;
      }
    }
    else
    {
      // Display State and other state related informations
      headerDisplay("", true);
      // Display IN/OUT Perimeter wire
      DisplayPrint(5,1,g_StatesString[int(g_BaseCurrentState)]);
      if (g_enableSender)
      {
        DisplayPrint(0,2,"ON",true);
      }
      else
      {
        DisplayPrint(0,2,"OFF",true);
      }

      DisplayPrint(5,2,"Cur:" + String(g_PerimeterCurrent) + " mA   ",true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    // Do nothing
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    g_BaseCurrentState = BaseState::sleeping;
    delay(400);  // to ensure key is released 
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    inSubmenu = true;
    submenuNum = 3;
    internalRefresh = true;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_4])
  {
    inSubmenu = true;
    submenuNum = 4;
    internalRefresh = true;
  }

  // leaving submenu using "1" key
  if (inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    inSubmenu = false;
    submenuNum = 0;
    internalRefresh = true;
    delay(400);  // to ensure key is released 
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Sleeping Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Display the sleeping state screen
 * @param refresh boolean to force full screen update
 * */
void sleepingDisplay(bool refresh)
{
  static bool inSubmenu = false;
  static int submenuNum = 0;
  static unsigned long lastRefresh = 0;
  static bool internalRefresh = false;
  static unsigned long refreshInterval = DISPLAY_SLEEPING_REFRESH_INTERVAL;

  if ((refresh || internalRefresh) && !inSubmenu)
  { 
    // Clear screen and display header and menu bar
    DisplayClear();
    headerDisplay("", true);
    menuDisplay(int(g_BaseCurrentState));
  }

  if (g_FanOn[FAN_1_RED] || g_MowerChargeCurrent > 0) 
  {
    refreshInterval = DISPLAY_SLEEPING_REFRESH_INTERVAL;
  }
  else 
  {
    refreshInterval = DISPLAY_SLEEPING_REFRESH_INTERVAL * 20;
  }

  if (millis() - lastRefresh > refreshInterval || refresh || internalRefresh)
  {
    if (inSubmenu)
    {
      switch (submenuNum)
      {
      case 1:
          /* Action submenu - No display */
          break;
      case 2:
          /* Action submenu - No display */
          break;
      case 3:
          /* Action submenu - No display */
          break;
      case 4:
          DisplayClear();
          headerDisplay(g_StatesString[int(g_BaseCurrentState)], true);
          // Display temperatures
          // DisplayPrint(0,2, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut: " + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
          menuDisplay(-1);
          break;
      default:
          break;
      }
    }
    else
    {
      // Display State and other state related informations
      headerDisplay("", true);
      DisplayPrint(5,1,g_StatesString[int(g_BaseCurrentState)]);
      String Line;

      if (g_MowerBatterySOC != UNKNOWN_FLOAT)
      {
        Line = "Bat: " + String(g_MowerBatterySOC,0) + " %";
      }

      if (g_MowerChargeCurrent > 0)
      {
        Line = Line + " | " + String(g_MowerChargeCurrent,0) + " mA";
      }

      Line = Line + "                 ";   // add spaces to errase end of line

      DisplayPrint(0,2,Line.substring(0,20),true);
    }

    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    g_BaseCurrentState = BaseState::sending;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    // Do nothing
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    // Do nothing
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_4])
  {
    inSubmenu = true;
    submenuNum = 4;
    internalRefresh = true;
  }

  // leaving submenu using "1" key
  if (inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    inSubmenu = false;
    submenuNum = 0;
    internalRefresh = true;
    delay(400);  // to ensure key is released 
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Helper functions
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Display menu bar on last line of display
 * @param title screen title to display on 1st line
 * @param now boolean to force update
 * */
void headerDisplay(String title, bool now)
{
  // Screen Header is structured as follows :
  // 0-4 space left for displaying communication animation
  // 6-13 Title
  // 14 Sending indication 
  // 15-19 Temperature

  const String SendingChar = "~";
  String SendTxt = "   ";
  const String FanChar[2] = {"+", "x"};
  static unsigned long lastUpdate = 0;
  static int FanIdx = 0;
  static int SendIdx = 0;

  if (millis() - lastUpdate > DISPLAY_REFRESH_INTERVAL || now)
  {
    title.trim();   // Remove unnecessary spaces

    // Display charging animation
    if (g_enableSender)
    {
      for (int i=0; i < SendIdx; i++)
      {
        SendTxt = SendTxt + SendingChar;
      }
      SendIdx = SendIdx + 1;
      if (SendIdx == 3)
      {
        SendIdx = 0;
      }
    }

    DisplayPrint(0,0,SendTxt,true);

    // Display title
    DisplayPrint(6,0,title.substring(0,8),true);

    // Display fan indicator
    if (g_FanOn[FAN_1_RED])
    {
      DisplayPrint(13,0,FanChar[FanIdx],true);
      FanIdx = FanIdx + 1;
      if (FanIdx == 2)
      {
        FanIdx = 0;
      }
    }
    else
    {
      DisplayPrint(13,0," ",true);
    }
    
    // Display Temperature
    DisplayPrint(15,0,String(g_Temperature[TEMPERATURE_1_RED],1),true);
    lastUpdate = millis();
  }
}

/**
 * Display menu bar on last line of display
 * @param state Mower state as an int or -1 for return menu
 */
void menuDisplay(int state)
{
  String line = g_menuString[state];

  if (state == -1)
  {
    line = MENU_RETURN_MENU;
  }

  // Display menu bar on last line of display
  DisplayPrint(0,3,line,true);
}
