#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerDisplay/MowerDisplay.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "StartupChecks.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include "Keypad/Keypad.h"
#include "StartupChecks.h"
#include "Rain/Rain.h"

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
    menuDisplay(int(g_CurrentState));
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
        case 3:
            /* Action submenu - No display */
            break;
        case 4:
            DisplayClear();
            DisplayPrint(0,1, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut:" + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            menuDisplay(-1);
            break;
        default:
            break;
        }
    }
    else
    {
      // Display State and other state related informations
      DisplayPrint(3,1,g_StatesString[int(g_CurrentState)]);
      DisplayPrint(1,2, "Error # " + String(g_CurrentErrorCode), true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    DebugPrintln("Mower Acknowledgement requested (local)", DBG_ERROR, true);
    g_CurrentState = MowerState::idle;
    g_CurrentErrorCode = ERROR_NO_ERROR;
    delay(250);  // to ensure key is released 
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    inSubmenu = true;
    submenuNum = 2;
    internalRefresh = true;
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
    menuDisplay(int(g_CurrentState));
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
            headerDisplay(g_StatesString[int(g_CurrentState)], true);
            DisplayPrint(0, 1, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut: " + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            // Display currents
            DisplayPrint(0, 2, "R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) +
                               " L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) + 
                               " Ch:" + String(g_BatteryChargeCurrent, 0), true);
            menuDisplay(-1);
            break;
        default:
            break;
        }
    }
    else
    {
      // Display State and other state related informations
      DisplayPrint(5,1,g_StatesString[int(g_CurrentState)]);
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
    g_CurrentState = MowerState::mowing;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    g_CurrentState = MowerState::going_to_base;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    StartupChecks();
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

/**
 * Display the mowing state screen
 * @param refresh boolean to force full screen update
 * */
void mowingDisplay(bool refresh)
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
    menuDisplay(int(g_CurrentState));
  }

  if (millis() - lastRefresh > DISPLAY_MOWING_REFRESH_INTERVAL || refresh || internalRefresh)
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
            DisplayClear();
            headerDisplay(g_StatesString[int(g_CurrentState)], true);
            DisplayPrint(0, 1, "SL:" + String(g_SonarDistance[SONAR_LEFT]) +
                               " SF:" + String(g_SonarDistance[SONAR_FRONT]) + 
                               " SR:" + String(g_SonarDistance[SONAR_RIGHT]), true);
            DisplayPrint(0, 1, "Rain:" + String(isRaining(true)), true);
            menuDisplay(-1);
            break;
        case 4:
            DisplayClear();
            headerDisplay(g_StatesString[int(g_CurrentState)], true);
            DisplayPrint(0, 1, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut: " + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            DisplayPrint(0, 2, "R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + " ", true);
            DisplayPrint(6, 2, "L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) + " ", true);
            DisplayPrint(12, 2, "C:" + String(g_MotorCurrent[MOTOR_CURRENT_CUT], 0) + " ", true);
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
      DisplayPrint(5,1,g_StatesString[int(g_CurrentState)]);
      if (g_isInsidePerimeter)
      {
        DisplayPrint(0,2,"IN",true);
      }
      else
      {
        DisplayPrint(0,2,"OUT",true);
      }
      DisplayPrint(5,2,"Mag:" + String(g_PerimeterMagnitude) + " Smag:" + String(g_PerimeterSmoothMagnitude),true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    g_CurrentState = MowerState::idle;
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    g_CurrentState = MowerState::going_to_base;
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

/**
 * Display the going to base state screen
 * @param refresh boolean to force full screen update
 * */
void toBaseDisplay(bool refresh)
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
    menuDisplay(int(g_CurrentState));
  }

  if (millis() - lastRefresh > DISPLAY_TO_BASE_REFRESH_INTERVAL || refresh || internalRefresh)
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
            headerDisplay(g_StatesString[int(g_CurrentState)], true);
            // Display currents
            DisplayPrint(0, 1, "R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) +
                               " L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) + 
                               " Ch:" + String(g_BatteryChargeCurrent, 0), true);
            // Display PID Parameters
            DisplayPrint(0, 2, "P:" + String(g_ParamPerimeterTrackPIDKp, 4) +
                               " I:" + String(g_ParamPerimeterTrackPIDKi, 4) + 
                               " D:" + String(g_ParamPerimeterTrackPIDKd, 4), true);
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
      DisplayPrint(5,1,g_StatesString[int(g_CurrentState)]);
      if (g_isInsidePerimeter)
      {
        DisplayPrint(0,2,"IN",true);
      }
      else
      {
        DisplayPrint(0,2,"OUT",true);
      }
      DisplayPrint(5,2,"CL:" + String(g_WheelPerimeterTrackingCorrection[MOTOR_CURRENT_LEFT]) + " CR:" + String(g_WheelPerimeterTrackingCorrection[MOTOR_CURRENT_RIGHT]),true);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    g_CurrentState = MowerState::idle;
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

/**
 * Display the test state screen
 * @param refresh boolean to force full screen update
 * */
void testDisplay(bool refresh)
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
    menuDisplay(int(g_CurrentState));
  }

  if ( millis() - lastRefresh > DISPLAY_TEST_REFRESH_INTERVAL || refresh || internalRefresh)
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
            headerDisplay(g_StatesString[int(g_CurrentState)], true);
            DisplayPrint(0,2, "Mot:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + " Cut: " + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
            menuDisplay(-1);
            break;
        default:
            break;
        }
    }
    else
    {
      // Display State and other state related informations
      DisplayPrint(3,1,g_StatesString[int(g_CurrentState)]);
    }
    lastRefresh = millis();
    internalRefresh = false;
  }

  // Manage keys pressed
  KeypadRead();
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_1])
  {
    StartupChecks();
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_2])
  {
    MotionMotorTest(MOTION_MOTOR_RIGHT);
    MotionMotorTest(MOTION_MOTOR_LEFT);
  }
  if (!inSubmenu && g_KeyPressed[KEYPAD_KEY_3])
  {
    CutMotorTest();
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
  // 15 Charging indication 
  // 17-19 Battery SOC

  String chargingChar = "*";
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > DISPLAY_REFRESH_INTERVAL || now)
  {
    title.trim();   // Remove unnecessary spaces

    // Display title
    DisplayPrint(6,0,title.substring(0,8),true);

    // Display charging indicator
    if (!g_BatteryIsCharging)
    {
        chargingChar = " ";
    }
    DisplayPrint(15,0,chargingChar,true);

    // Display Battery SOC
    if (g_BatterySOC < 99.4f)
    {
        DisplayPrint(17,0,String(g_BatterySOC,0),true);
    }
    else
    {
        DisplayPrint(17,0,String(g_BatterySOC,0) + "%",true);
    }
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
