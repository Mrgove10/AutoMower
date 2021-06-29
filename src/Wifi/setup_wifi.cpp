#include <WiFi.h>
#include "myGlobals_definition.h"
#include "Credentials_definitions.h"
#include "Utils/Utils.h"
#include "LCD/LCD.h"

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  DebugPrintln("");
  DebugPrint("Connecting to ");
  DebugPrintln(WiFissid[0]);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Wifi connection ..."));
  lcd.setCursor(1, 1);
  lcd.print(WiFissid[0]);
  lcd.setCursor(1, 2);

  WiFi.begin(WiFissid[0], WiFipassword[0]);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 15)
  {
    delay(500);
    count = count + 1;
    DebugPrint(".");
    lcd.print(F("."));
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.setCursor(1, 1);
    lcd.print(WiFissid[1]);
    lcd.setCursor(1, 2);

    WiFi.begin(WiFissid[1], WiFipassword[1]);
    count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 15)
    {
      delay(500);
      count = count + 1;
      DebugPrint("-");
      lcd.print(F("-"));
    }
  }

  DebugPrintln("");
  DebugPrintln("WiFi connected");
  lcd.setCursor(8, 2);
  lcd.print(F("Connected"));
  char outBuf[18];
  IPAddress ip = WiFi.localIP();
  sprintf(outBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  lcd.setCursor(2, 3);
  lcd.print(outBuf);

  delay(TEST_SEQ_STEP_WAIT);

  //  DebugPrint("IP address: ");
  //  DebugPrintln(String(WiFi.localIP()));
}
