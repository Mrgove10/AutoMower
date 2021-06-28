#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "mySetup.h"
#include "Framework/setup_wifi.h"
#include "Framework/NtpSetup.h"
#include "Framework/InitTelnet.h"
#include "Framework/setup_wifi.h"
#include "Framework/Utils.h"
#include "Framework/OTA.h"
#include "MQTT/MQTTInit.h"

void MySetup(void)
{

  Serial.begin(SERIAL_BAUD);

  // Setup pins

  //  adcAttachPin(BatteryPin);
  //  adcAttachPin(MainBatteryPin);

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
  DebugPrintln(Resetreason,true);
  DebugPrintln("Serial Baud:" + String(Serial.baudRate()));

  setup_wifi();

  OTASetup();

  NtpSetup();

  IPAddress ip = WiFi.localIP();
  char outBuf[18];
  sprintf(outBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  DebugPrint("SSID:" + String(WiFi.SSID()));
  DebugPrint(" IP:" + String(outBuf));
  DebugPrintln(" RSSI:" + String(WiFi.RSSI()) + " dBm");

  MQTTInit();

  LogPrintln(Resetreason, TAG_RESET, DBG_WARNING);
}