#include <WiFi.h>
#include "myGlobals_definition.h"
#include "Credentials_definitions.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  DebugPrintln("");
  DebugPrint("Connecting to ");
  DebugPrintln(WiFissid[0]);

  DisplayClear();
  DisplayPrint(0, 0, F("Wifi connection ..."));
  DisplayPrint(1, 1, WiFissid[0]);

  WiFi.begin(WiFissid[0], WiFipassword[0]);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 30)
  {
    delay(500);
    count = count + 1;
    DebugPrint(".");
    DisplayPrint(2 + count, 2, F("."));
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    DisplayPrint(1, 1, WiFissid[1]);

    WiFi.begin(WiFissid[1], WiFipassword[1]);
    count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 30)
    {
      delay(500);
      count = count + 1;
      DebugPrint("-");
      DisplayPrint(2 + count, 2, F("-"));
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    ESP.restart();
  }
  else 
  {
    DebugPrintln("");
    DebugPrintln("WiFi connected");
    DisplayPrint(8, 2, F("Connected"));
    char outBuf[18];
    IPAddress ip = WiFi.localIP();
    sprintf(outBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    DisplayPrint(2, 3, String(outBuf));

    delay(TEST_SEQ_STEP_WAIT);

    //  DebugPrint("IP address: ");
    //  DebugPrintln(String(WiFi.localIP()));
  }
}
