#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "Temperature/Temperature.h"
#include "EEPROM/EEPROM.h"
//#include "Keypad/Keypad.h"
#include "Fan/Fan.h"
#include "Rain/Rain.h"
#include "PwrSupply/PwrSupply.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "IOExtender/IOExtender.h"
#include <pin_definitions.h>
#include "Display/Display.h"
#include "BaseStates/BaseStates.h"

void setup()
{
  MySetup();
}

void loop()
{
  static BaseState StateOnCall = BaseState::idle;

  static unsigned long LastDisplay = 0;
  static unsigned long LastTaskRefreshed = 0;

  // Common routine mower tasks

  // FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  // if (millis() - LastDisplay > 1000)
  // {
  //   DisplayClear();
  //   String FanStr = "Fan ";
  //   if (g_FanOn[FAN_1_RED])
  //   {
  //     FanStr = FanStr + "ON";
  //   }
  //   else
  //   {
  //     FanStr = FanStr + "OFF";
  //   }

  //   DisplayPrint(0, 0, "Temp:" + String(g_Temperature[TEMPERATURE_1_RED], 1), true);
  //   DisplayPrint(10, 0, FanStr, true);

  //   String RainStr = "Rain ";
  //   if (isRaining())
  //   {
  //     RainStr = RainStr + "Yes";
  //   }
  //   else
  //   {
  //     RainStr = RainStr + "No";
  //   }

  //   DisplayPrint(0, 1, RainStr + " (" + String(g_RainLevel, 2) + ")", true);

  //   PwrSupplyVoltageRead();

  //   DisplayPrint(0, 2, "Supply:" + String(g_PwrSupplyVoltage / 1000.0f , 2) + "V", true);

  //   PerimeterLoadCurrentRead();

  //   String SenderStsStr = "";
  //   if (g_enableSender)
  //   {
  //     SenderStsStr = SenderStsStr + "ON";
  //   }
  //   else
  //   {
  //     SenderStsStr = SenderStsStr + "OFF";
  //   }

  //   DisplayPrint(0, 3, SenderStsStr + " " + String(g_PerimeterCurrent, 0) + "mA " + String(g_PerimeterVoltage, 1) + "V " + String(g_PerimeterPower, 2) + "W", true);

  //   LastDisplay = millis();
  // }

  if ((millis() - LastTaskRefreshed > 60000))
  {
   DisplayTaskStatus("*");
    LastTaskRefreshed = millis();
  }

  bool stateChange = g_BaseCurrentState != StateOnCall;

  if (stateChange)
  {
    g_BasePreviousState = g_BaseCurrentState;
  }

  StateOnCall = g_BaseCurrentState;

  switch (g_BaseCurrentState)
  {
  case BaseState::idle:
    BaseIdle(stateChange, g_BasePreviousState);
    break;

  case BaseState::sleeping:
    BaseSleeping(stateChange, g_BasePreviousState);
    break;

  case BaseState::sending:
    BaseSending(stateChange, g_BasePreviousState);
    break;

  case BaseState::error:
    BaseInError(stateChange, g_BasePreviousState);
    break;

  default:
    break;
  }

  // Display Mower Data

//  DisplayMowerData();

  // Routine system operating tasks

  EEPROMSave(false); // Update EEPROM

  MQTTReconnect(); // Check MQTT Status and reconnect

  MQTTSendTelemetry(); // Send Mower Telemetry

  MQTTclient.loop(); // Update MQTT

  SerialAndTelnet.handle(); // Refresh Telnet Session

  events(); // eztime refresh
  
  DisplayDimming(DISPLAY_BACKLIGHT_OFF_DELAY); // Dimm screen if timeout is reached 

  delay(50);
}