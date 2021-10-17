#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "StartupChecks.h"
#include "Utils/Utils.h"
#include "Rain/Rain.h"
#include "Temperature/Temperature.h"
#include "Fan/Fan.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "PwrSupply/PwrSupply.h"
#include "Display/Display.h"

/**
 * Runs all Mower checks on Startup
 * @param true to indicate that all tests are to be performed of false to perform minimum tests (false by default)
 * @return true if all checks pass
 */
bool StartupChecks(const bool allTests)
{
  bool allChecks = true;

  DebugPrintln("");
  DebugPrintln("Starting AutoMower Base startup checks.....", DBG_INFO, true);
  DebugPrintln("");
  DisplayClear();
  DisplayPrint(0, 1, F("Startup Checks..."));
  delay(TEST_SEQ_STEP_WAIT);

  if (!PwrSupplyVoltageCheck())
  {
    allChecks = false;
  };
  DebugPrintln(" ");

  if (!RainSensorCheck())
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!TemperatureSensorCheck(TEMPERATURE_1_RED))
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!PerimeterLoadCurrentSensorCheck())
  {
    allChecks = false;
  };

  if (allTests)
  {
    DebugPrintln(" ");
    FanTest(FAN_1_RED);
  }
  DebugPrintln(" ");


  DebugPrintln(" ");

  // insert here all other startup checks

  if (allChecks)
  {
    LogPrintln("All checks Ok", TAG_CHECK, DBG_ALWAYS);
  }
  DebugPrintln("");

  DebugPrintln("");
  DebugPrintln("End of AutoMower Base startup checks.....", DBG_INFO, true);
  DebugPrintln("");


  SerialAndTelnet.handle();

  return allChecks;
}