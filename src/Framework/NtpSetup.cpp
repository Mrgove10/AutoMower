/* Time init procedure */
#include "myGlobals_definition.h"
#include "Framework/Utils.h"

void NtpSetup(void)
{

  setDebug(INFO, MySERIAL);

  setInterval(NTPRefresh); // in seconds - Default is 10 minutes
                           // myTime.setLocation("Europe/Paris");
  myTime.setPosix(POSIXTZ);

  waitForSync();

  DebugPrint("Local Time is: " + myTime.dateTime(), DBG_ALWAYS, false);
  DebugPrintln(" DST:" + String(myTime.isDST()));
}