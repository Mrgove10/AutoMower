#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "StartupChecks.h"
#include "Utils/Utils.h"
#include "Rain/Rain.h"
#include "Bumper/Bumper.h"
#include "Tilt/Tilt.h"


/**
 * Runs all Mower checks on Startup
 * 
 * @return true if all checks pass
 */
bool StartupChecks(void)
{  
  bool allChecks = true;

  DebugPrintln("");
  DebugPrintln("Starting AutoMower startup checks.....", DBG_INFO, true);
  DebugPrintln("");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Startup Checks..."));
  delay(TEST_SEQ_STEP_WAIT);

  if (!TiltSensorCheck(TILT_HORIZONTAL)) {allChecks = false;};
  if (!TiltSensorCheck(TILT_VERTICAL)) {allChecks = false;};

  if (!BumperSensorCheck(BUMPER_LEFT)) {allChecks = false;};
  if (!BumperSensorCheck(BUMPER_RIGHT)) {allChecks = false;};
 
  if (!RainSensorCheck()) {allChecks = false;};

   // insert here all other startup checks

  if (allChecks) 
  {
      LogPrintln("All checks Ok", TAG_CHECK, DBG_ALWAYS);
  }
  DebugPrintln("");

  return allChecks;
}