/* Time init procedure */
#include "myGlobals_definition.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

void NtpSetup(void)
{
  DisplayClear();
  DisplayPrint(0, 0, F("Time update ..."));

  setDebug(INFO, MySERIAL);
  setInterval(NTP_REFRESH); // in seconds - Default is 10 minutes
                            // myTime.setLocation("Europe/Paris");
  myTime.setPosix(POSIXTZ);

  waitForSync(30);  // wait for 30 seconds max

  DisplayPrint(8, 1, myTime.dateTime("H:i:s"));
  delay(TEST_SEQ_STEP_WAIT);

  DebugPrint("Local Time is: " + myTime.dateTime(), DBG_ALWAYS, false);
  DebugPrintln(" DST:" + String(myTime.isDST()));
}