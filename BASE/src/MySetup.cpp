#include <rom/rtc.h>
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "mySetup.h"
#include "Wifi/setup_wifi.h"
#include "NTP/NtpSetup.h"
#include "Telnet/InitTelnet.h"
#include "Utils/Utils.h"
#include "OTA/OTA.h"
#include "MQTT/MQTT.h"
#include "EEPROM/EEPROM.h"
#include "Display/Display.h"
#include "IOExtender/IOExtender.h"
#include "Keypad/Keypad.h"
#include "Temperature/Temperature.h"
#include "Fan/Fan.h"
//#include "AnaReadTsk/AnaReadTsk.h"
#include "PerimeterSendTsk/PerimeterSendTsk.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "StartupChecks.h"

void MySetup(void)
{
  g_MySerialSemaphore = xSemaphoreCreateMutex();
  g_I2CSemaphore = xSemaphoreCreateMutex(); // I2C resource protection semaphore

  Serial.begin(SERIAL_BAUD);
  delay(100);
  Serial.println();

    if (esp_register_freertos_idle_hook_for_cpu(&MyIdleHook_0, (UBaseType_t) 0) != ESP_OK)
  {
    DebugPrintln(" Idle Hook 0 not created ! ");
  }

  if (esp_register_freertos_idle_hook_for_cpu(&MyIdleHook_1, (UBaseType_t) 1) != ESP_OK)
  {
    DebugPrintln(" Idle Hook 1 not created ! ");
  }

#ifdef STOP_RESTART_TO_CAPTURE_CRASH_DUMP
// For testing ONLY, if reset is not a power-on, delay indefinitely to be able to "catch" reset cause.
// NOT TO BE USED IN NORMAL OPERATION AS MOWER WILL NOT RESET OUTPUTS AND MOTORS WILL KEEP RUNNING UNTIL 
// THE MOWER IS POWERED OFF OR A RESET IS PERFORMED ON ESP32 BOARD

  if (rtc_get_reset_reason(0) != 1)
  {
    while (true)
    {
      delay(100);
    }
  }
#endif

  DisplaySetup();

  IOExtendSetup();

  KeypadSetup();

  // Delay to ensure all serial.prints have finished before Telnet initialisation
  delay(500);

  // Setup Telnet Debug Management
  InitTelnet();

  DebugPrintln("");
  DebugPrintln(" Program Start.............");
  DebugPrint("Sketch compiled: ");
  DebugPrint(__DATE__);
  DebugPrint(" ");
  DebugPrintln(__TIME__);
  
  DebugPrintln("Free sketch space: " + String(ESP.getFreeSketchSpace()));
  String Resetreason = "Reset core 1:" + String(char_reset_reason(0)) + " core 2:" + String(char_reset_reason(1)) + " - Sketch compiled: " + String(__DATE__) + " " + String(__TIME__);
  DebugPrintln(Resetreason, true);
  DebugPrintln("Serial Baud:" + String(Serial.baudRate()));
  DebugPrintln("Running on Core:" + String(xPortGetCoreID()));
  DebugPrintln("Chip temperature:" + String(temperatureRead(), 1));
  DebugPrintln("Free Heap:" + String(esp_get_free_heap_size()));
  DebugPrintln("Tasks:" + String(uxTaskGetNumberOfTasks()));

  SerialAndTelnet.handle();

  EEPROMSetup();

  setup_wifi();

  OTASetup();

  NtpSetup();

  IPAddress ip = WiFi.localIP();
  char outBuf[18];
  sprintf(outBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  DebugPrint("SSID:" + String(WiFi.SSID()));
  DebugPrint(" IP:" + String(outBuf));
  DebugPrintln(" RSSI:" + String(WiFi.RSSI()) + " dBm");

  Resetreason = Resetreason + " IP :" + String(outBuf) + " Wifi " + String(WiFi.SSID());

  MQTTInit();

  SerialAndTelnet.handle();

  // Send reboot event to logger
  LogPrintln(Resetreason, TAG_RESET, DBG_WARNING);

  PerimeterLoadCurrentSensorSetup();

  TemperatureSensorSetup();

  FanSetup();

  // Start other RTOS tasks
//  AnaReadLoopTaskCreate();
  
  InitPerimeterSendPins();
  PerimeterSendLoopTaskCreate();

  // Set default trace level
  g_debugLevel = DBG_VERBOSE;

  SerialAndTelnet.handle();

  delay(1000); // temporary

  DebugPrintln("");
  DebugPrintln("End of Setup---------", DBG_VERBOSE, true);
  DebugPrintln("");

  bool startupChecksOk = StartupChecks(true);

  DebugPrintln("");
  DebugPrintln("End of Startupchecks - Status:" + String(startupChecksOk), DBG_VERBOSE, true);
  DebugPrintln("");

  SerialAndTelnet.handle();

  DisplayClear();
}