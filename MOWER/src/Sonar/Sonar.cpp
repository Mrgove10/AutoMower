#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Sonar/Sonar.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Sonar sensor setup function
 * 
 */
void SonarSensorSetup(void)
{
  DebugPrintln("Sonar Sensor Setup start", DBG_VERBOSE, true);

  g_SonarReadEnabled = true; // By default, sonar readings are enabled
}

/**
 * Checks to see if Sonar reading are available (sensor connected and functionning and reading task operational)
 * @param sensor int Sonar to check
 * @return true if sensor check is ok
 */
bool SonarSensorCheck(int sensor)
{
  bool sensorCheck = false;
  unsigned int Distance = UNKNOWN_INT;

  g_SonarReadEnabled = true; // Start sonar readings

  DebugPrintln("SonarSensorCheck start #" + String(sensor + 1), DBG_VERBOSE, true);
  if (sensor == 0)
  {
    DisplayClear();
    DisplayPrint(0, 0, F("Sonar Tests"));
  }

  //  Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);
  // Distance = sonar[sensor].convert_cm(sonar[sensor].ping_median(SONAR_READ_ITERATIONS));
  // sensorCheck = (Distance != 0 && Distance != UNKNOWN_INT);
  sensorCheck = (g_SonarDistance[sensor] != 0 && g_SonarDistance[sensor] != UNKNOWN_INT);

  // DebugPrintln(g_sensorStr[sensor] + " Distance " + String(g_SonarDistance[sensor]), DBG_INFO, true);

  if (sensorCheck)
  {
    DebugPrintln(g_sensorStr[sensor] + " Sonar sensor Ok : " + String(g_SonarDistance[sensor]), DBG_INFO, true);
    DisplayPrint(2, sensor + 1, g_sensorStr[sensor]);
    DisplayPrint(8, sensor + 1, "OK " + String(g_SonarDistance[sensor]) + " cm");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(g_sensorStr[sensor] + " No sensor echo", TAG_CHECK, DBG_WARNING);
    DisplayPrint(2, sensor + 1, g_sensorStr[sensor]);
    DisplayPrint(8, sensor + 1, F("NO ECHO"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }

  // g_SonarReadEnabled = false;       // Stop sonar readings
}

/**
 * Function to read distance
 * @param sensor int functional sensor to read distance from
 * @param Now optional bool true if immediate read
 * @return float sensor distance
 */
int SonarRead(const int sensor, const bool Now)
{
  static unsigned long LastSonarRead[SONAR_COUNT] = {0, 0, 0};

  if ((millis() - LastSonarRead[sensor] > SONAR_READ_INTERVAL) || Now)
  {
    unsigned int Distance = UNKNOWN_INT;
    //    Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);
    Distance = sonar[sensor].convert_cm(sonar[sensor].ping_median(SONAR_READ_ITERATIONS));

    // DebugPrintln("Sonar " + g_sensorStr[sensor] + " value: " + String(Distance) + " cm", DBG_VERBOSE, true);

    LastSonarRead[sensor] = millis();
    if (Distance == 0 || Distance >= SONAR_MAX_DISTANCE)
    {
      // g_SonarDistance[sensor] = SONAR_MAX_DISTANCE;
      g_MaxSonarDistanceCount[sensor] = g_MaxSonarDistanceCount[sensor] + 1;
    }
    else
    {
      g_SonarDistance[sensor] = Distance;
    }
  }
  g_SonarTskLoopCnt = g_SonarTskLoopCnt + 1;
  return g_SonarDistance[sensor];
}

/**
 * Sonar Read task main loop
 * @param dummyParameter is unused but required
 */
void SonarReadLoopTask(void *dummyParameter)
{
  static bool SetupDone = false;

  for (;;)
  {
    //------------------------------------------------------------------
    // Task Setup (done only on 1st call)
    //------------------------------------------------------------------

    if (!SetupDone)
    {
      DebugPrintln("Sonar read Task Started on core " + String(xPortGetCoreID()), DBG_VERBOSE, true);

      SonarSensorSetup(); // Setup Sensors

      g_SonarTskLoopCnt = 0;

      SetupDone = true;
      DebugPrintln("Sonar read Task setup complete", DBG_VERBOSE, true);
    }

    //------------------------------------------------------------------
    // Task Loop (done on each loop if read is not suspended)
    //------------------------------------------------------------------

    if (g_SonarReadEnabled)
    {
      for (int sonar = 0; sonar < SONAR_COUNT; sonar++)
      {
        g_LastSonarReadNum = sonar;
        SonarRead(sonar, true); // Read sonar value with no wait
        delay(5);
      }
      delay(SONAR_READ_TASK_LOOP_TIME);
    }
    else
    {
      for (int sonar = 0; sonar < SONAR_COUNT; sonar++)
      {
        g_SonarDistance[sonar] = UNKNOWN_INT; // set sonar value to unknown
      }
      delay(SONAR_READ_TASK_WAIT_ON_IDLE);
    }
  }
}

/**
 * Sonar Read task creation function
 */
void SonarReadLoopTaskCreate(void)
{
  BaseType_t xReturned;
  xReturned = xTaskCreatePinnedToCore(
      SonarReadLoopTask,          /* Task function. */
      SONAR_READ_TASK_NAME,       /* String with name of task. */
      SONAR_READ_TASK_STACK_SIZE, /* Stack size in bytes. */
      NULL,                       /* Parameter passed as input of the task */
      SONAR_READ_TASK_PRIORITY,   /* Priority of the task. */
      &g_SonarReadTaskHandle,     /* Task handle. */
      SONAR_READ_TASK_ESP_CORE);

  if (xReturned == pdPASS)
  {
    DebugPrintln("Sonar read Task created on Core " + String(SONAR_READ_TASK_ESP_CORE), DBG_VERBOSE, true);
  }
  else
  {
    DebugPrintln("Sonar read Task creation failled (" + String(xReturned) + ")", DBG_ERROR, true);
    //errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
    //errQUEUE_BLOCKED						( -4 )
    //errQUEUE_YIELD							( -5 )
  }
}

/**
 * Sonar Read task suspension function
 */
void SonarReadLoopTaskSuspend(void)
{
  vTaskSuspend(g_SonarReadTaskHandle);
  DebugPrintln("Sonar read Task suspended", DBG_INFO, true);
}

/**
 * Sonar Read task resume from suspension function
 */
void SonarReadLoopTaskResume(void)
{
  vTaskResume(g_SonarReadTaskHandle);
  DebugPrintln("Sonar read Task resumed", DBG_INFO, true);
}

/**
 * Sonar Read task delete
 */
void SonarReadLoopTaskDelete(void)
{
  vTaskDelete(g_SonarReadTaskHandle);
  DebugPrintln("Sonar read Task deleted", DBG_INFO, true);
}

/**
 * Sonar Read task monitoring function to check if task is running
 *
 * @param task boolean indicating if check is to be performed on task counter
 * @param distance boolean indicating if check is to be performed on distance values
 * @return boolean indicating if task appears not to be running (false) or is running (true)
 */
bool SonarReadLoopTaskMonitor(bool task, bool distance)
{
  static unsigned int LastLoopcnt = 0;
  static int LastDistance[SONAR_COUNT] = {0, 0, 0};
  static unsigned long LastTaskCheck = 0;
  static bool lastcheckOk = true;

  if (millis() - LastTaskCheck > SONAR_READ_TASK_MONITORING_INTERVAL)
  {
    LastTaskCheck = millis();

    if (task && g_SonarReadEnabled && g_SonarTskLoopCnt == LastLoopcnt)
    {
      if (lastcheckOk) {
        LogPrintln("Sonar Task not running on g_SonarTskLoopCnt (Enabled=" + String (g_SonarReadEnabled) + ", TskLoopCnt=" + String (g_SonarTskLoopCnt) + ")", TAG_MOWING, DBG_ERROR);
        DisplayTaskStatus(SONAR_READ_TASK_NAME);
      }
      lastcheckOk = false;
      return false;
    }
    
    if (distance && 
        g_SonarReadEnabled && 
        g_SonarDistance[SONAR_FRONT] == LastDistance[SONAR_FRONT] &&
        g_SonarDistance[SONAR_LEFT] == LastDistance[SONAR_LEFT] &&
        g_SonarDistance[SONAR_RIGHT] == LastDistance[SONAR_RIGHT])
    {
      if (lastcheckOk) 
      {
        LogPrintln("Sonar Task not running on sonar distance (Enabled = " + String (g_SonarReadEnabled) + ")", TAG_MOWING, DBG_ERROR);
        DisplayTaskStatus(SONAR_READ_TASK_NAME);
      }
      lastcheckOk = false;
      return false;
    }
    else
    {
      LastLoopcnt = g_SonarTskLoopCnt;
      LastDistance[SONAR_FRONT] = g_SonarDistance[SONAR_FRONT];
      LastDistance[SONAR_LEFT] = g_SonarDistance[SONAR_LEFT];
      LastDistance[SONAR_RIGHT] = g_SonarDistance[SONAR_RIGHT];
      lastcheckOk = true;
      return true;
    }
  }
  else
  {
    return true;
  }
}
