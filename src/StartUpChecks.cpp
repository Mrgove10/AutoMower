#include "Environment_definitions.h"
#include "StartupChecks.h"
#include "Utils/Utils.h"
#include "Rain/Rain.h"
#include "Bumper/Bumper.h"

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