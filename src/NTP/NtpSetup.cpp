/* Time init procedure */
#include "myGlobals_definition.h"
#include "Utils/Utils.h"

void NtpSetup(void)
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Time update ..."));

  setDebug(INFO, MySERIAL);
  setInterval(NTP_REFRESH); // in seconds - Default is 10 minutes
                           // myTime.setLocation("Europe/Paris");
  myTime.setPosix(POSIXTZ);

  waitForSync();

  lcd.setCursor(2, 1);
  lcd.print(myTime.dateTime("H:i:s"));
  delay(TEST_SEQ_STEP_WAIT);

  DebugPrint("Local Time is: " + myTime.dateTime(), DBG_ALWAYS, false);
  DebugPrintln(" DST:" + String(myTime.isDST()));
}