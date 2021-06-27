#include <WiFi.h>
#include "myGlobals_definition.h"
#include "Credentials_definitions.h"
#include "Framework/Utils.h"


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  DebugPrintln("");
  DebugPrint("Connecting to ");
  DebugPrintln(WiFissid[0]);

  WiFi.begin(WiFissid[0], WiFipassword[0]);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 15)  {
    delay(500);
    count = count + 1;
    DebugPrint(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WiFissid[1], WiFipassword[1]);
    count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 15)  {
      delay(500);
      count = count + 1;
      DebugPrint("-");
    }
  }

  DebugPrintln("");
  DebugPrintln("WiFi connected");
//  DebugPrint("IP address: ");
//  DebugPrintln(String(WiFi.localIP()));
}
