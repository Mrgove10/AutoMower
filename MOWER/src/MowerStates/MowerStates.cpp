#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerMoves/MowerMoves.h"
#include "MotionMotor/MotionMotor.h"
#include "MowerStates/MowerStates.h"
#include "MowerDisplay/MowerDisplay.h"
#include "CutMotor/CutMotor.h"
#include "Sonar/Sonar.h"
#include "Battery/Battery.h"
#include "Tilt/Tilt.h"
#include "Bumper/Bumper.h"
#include "Rain/Rain.h"
#include "Battery/Battery.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "MQTT/MQTT.h"

//---------------------------------------------------------------------------------------------------------------------------
//
// Idle Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower in idle state
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerIdle(const bool StateChange, const MowerState PreviousState)
{
  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Mower Idle", TAG_STATES, DBG_INFO);

    // Change display with refresh
    idleDisplay(true);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;

    // Close Battery charge relay to be able to detect base station
    BatteryChargeRelayClose();

#ifdef MQTT_PID_GRAPH_DEBUG
    g_MQTTPIDGraphDebug = false; // for testing
#endif

    // if (PreviousState == MowerState::mowing)
    // {
    MowerStop();
    CutMotorStop(true);
    // }
  }

  // Update display
  idleDisplay();

  // If charge current is detected, mower is assumed to be on base and changes status to docked
  if (g_BatteryChargeCurrent > MOWER_AT_BASE_CURRENT)
  {
    LogPrintln("Mower on charge", TAG_STATES, DBG_INFO);
    g_CurrentState = MowerState::docked;
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Docked Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower docked
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerDocked(const bool StateChange, const MowerState PreviousState)
{
  if (StateChange)
  {
    LogPrintln("Mower Docked", TAG_STATES, DBG_INFO);

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_SLOW;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

  // Change display with refresh
    dockedDisplay(true);
  }

  // Update display 
    dockedDisplay();
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Mowing Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower in mowing mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerMowing(const bool StateChange, const MowerState PreviousState)
{
  static unsigned long mowingStartTime = 0;
  static unsigned long lastCutDirectionChange = 0;
  static int bladeDirection = CUT_MOTOR_FORWARD;
  static int outsideCount = 0;

  // Varibales to manage spiral mowing
  static unsigned long stepDuration = MOWER_MOWING_SPIRAL_START_CIRCLE_DURATION * MOWER_MOWING_SPIRAL_CIRCLES_PER_STEP;
  static int spiralStep = 0;      // curren step;
  static int rightSpeed = 0;      // right motor speed
  static int leftSpeed = 0;       // left motor speed
  static bool isSpiral = false;   // indicates if mowing mode si of spiral type
  static unsigned long lastSpiralSpeedAdjustment = 0;  // time of last spiral speed change

  //--------------------------------
  // Actions to take when entering the mowing state
  //--------------------------------

  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Mowing Started", TAG_MOWING, DBG_INFO);

    // Change display with refresh
    mowingDisplay(true);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_FAST;

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    //--------------------------------
    // Activate Sonar reading
    //--------------------------------

    g_SonarReadEnabled = true;          // activate Sonar readings
    delay(SONAR_READ_ACTIVATION_DELAY); //wait for task to take 1st readings

    //--------------------------------
    // Check if mowing conditions are met
    //--------------------------------

    if (!CheckPreConditions(ERROR_MOWING_NO_START_TILT_ACTIVE,
                            ERROR_MOWING_NO_START_BUMPER_ACTIVE,
                            ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_MOWING_NO_START_NO_PERIMETER_SIGNAL,
                            true))
    {
      return;
    }

    //--------------------------------
    // Start mowing (code below only executed if no error detected)
    //--------------------------------

    // Sound starting beep to notify environment
    if (PreviousState == MowerState::idle)
    {
      // TO DO Beep
    }

    //Initialise Mowing start time
    mowingStartTime = millis();

    // Determine random cutting motor rotation direction
    // if (millis() % 2 == 0) 
    // {
      bladeDirection = CUT_MOTOR_FORWARD;
    // }
    // else
    // {
    //   bladeDirection = CUT_MOTOR_REVERSE;
    // }
    lastCutDirectionChange = millis();
  
    // Start Mowing

    CutMotorStart(bladeDirection, MOWER_MOWING_CUTTING_SPEED);
    // Give time for cut motor to start
    delay(MOWER_MOWING_CUT_START_WAIT);

    // Initialise speed according to mowing mode
    switch (g_mowingMode)
    {
      case MOWER_MOWING_MODE_RANDOM:
        rightSpeed = MOWER_MOWING_TRAVEL_SPEED;
        leftSpeed = MOWER_MOWING_TRAVEL_SPEED;
        isSpiral = false;
        break;
      case MOWER_MOWING_MODE_SPIRAL_CLOCKWISE:
        rightSpeed = MOTION_MOTOR_MIN_SPEED;
        leftSpeed = MOWER_MOWING_TRAVEL_SPEED;
        isSpiral = true;
        stepDuration = MOWER_MOWING_SPIRAL_START_CIRCLE_DURATION * MOWER_MOWING_SPIRAL_CIRCLES_PER_STEP;
        spiralStep = 0;
        lastSpiralSpeedAdjustment = millis();
        break;
      case MOWER_MOWING_MODE_SPIRAL_COUNTER_CLOCKWISE:
        rightSpeed = MOWER_MOWING_TRAVEL_SPEED;
        leftSpeed = MOTION_MOTOR_MIN_SPEED;
        isSpiral = true;
        stepDuration = MOWER_MOWING_SPIRAL_START_CIRCLE_DURATION * MOWER_MOWING_SPIRAL_CIRCLES_PER_STEP;
        spiralStep = 0;
        lastSpiralSpeedAdjustment = millis();
        break;
      default:
        break;
    }

    // Start mower forward
    MowerArc(MOTION_MOTOR_FORWARD, leftSpeed, rightSpeed);
//    MowerForward(MOWER_MOVES_SPEED_NORMAL);

    g_MowingLoopCnt = 0;
    outsideCount = 0;
  }

  //--------------------------------
  // Mowing loop starts here
  //--------------------------------

  // Ongoing Mowing routine is as follows:
  //    Check for tilt,
  //    Check for lost or stopped perimeter signal,
  //    Check for battery level,
  //    Check for rain,
  //    Check enviroment to slow down for approaching obstacles,
  //    Check for obstacle detection and react (trigger error if too many),
  //    Stop when conditions met (TO DO)

  g_MowingLoopCnt = g_MowingLoopCnt + 1;

  //--------------------------------
  // Check tilt sensors and take immediate action
  //--------------------------------

  if (CheckTiltReadAndAct())
  {
    return;
  }

  //--------------------------------
  // Is Perimeter signal Lost or stopped ?
  //--------------------------------

  if (g_PerimeterSignalLost || g_PerimeterSignalStopped)
  {
    MowerStop();
    CutMotorStop();
    DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
    g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
    return;
  }

  //--------------------------------
  // Is mower outside for too long ?
  //--------------------------------
  if (!g_isInsidePerimeter)
  {
    outsideCount = outsideCount + 1;
  }
  else
  {
    outsideCount = max(0, outsideCount - 2);
  }
  if (outsideCount > MOWER_MOWING_MAX_CONSECUTVE_OUTSIDE)
  {
    MowerStop();
    CutMotorStop();
    DebugPrintln("Mower outside Perimeter for too long (" + String(outsideCount) + ")", DBG_ERROR, true);
    g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_MOWING_OUTSIDE_TOO_LONG;
    return;
  }

  //--------------------------------
  // Is Battery level above threshold ?
  //--------------------------------

  if (g_BatteryVoltage < BATTERY_VOLTAGE_RETURN_TO_BASE_THRESHOLD)
  {
    DebugPrintln("Battery low: returning to base (" + String(g_BatteryVoltage) + " mv)", DBG_INFO, true);
    MowerStop();
    CutMotorStop();
    g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
    g_CurrentState = MowerState::going_to_base;
    return;
  }

  //--------------------------------
  // Is it raining ?
  //--------------------------------

  // if (isRaining())
  // {
  //   DebugPrintln("Raining : returning to base", DBG_INFO, true);
  //   MowerStop();
  //   CutMotorStop();
  //   g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
  //   g_CurrentState = MowerState::going_to_base;
  //   return;
  // }

  //--------------------------------
  // Is it time for a cut direction change?
  //--------------------------------
  if (millis() - lastCutDirectionChange > MOWER_MOWING_CUT_DIRECTION_CHANGE_INTERVAL)
  {
    // determine new cut motor rotation direction
    if (g_CutMotorDirection == CUT_MOTOR_FORWARD)
    {
      bladeDirection = CUT_MOTOR_REVERSE;
    }
    if (g_CutMotorDirection == CUT_MOTOR_REVERSE)
    {
      bladeDirection = CUT_MOTOR_FORWARD;
    }

    DebugPrintln("Changing cut motor rotation direction to " + String(bladeDirection), DBG_INFO, true);
    DisplayPrint(0,2, F("Cut direct. change"));

    // Stop blades and mower
    CutMotorStop(true);
    MowerStop();

    // Wait for blade to stop
    delay(MOWER_MOWING_CUTTING_DIRECTION_WAIT_TIME);

    // Start cut motor with new direction
    CutMotorStart(bladeDirection, MOWER_MOWING_CUTTING_SPEED);

    // Give time for cut motor to start
    delay(MOWER_MOWING_CUT_START_WAIT);

    MowerArc(MOTION_MOTOR_FORWARD, leftSpeed, rightSpeed);

//    MowerForward(MOWER_MOWING_TRAVEL_SPEED);

    // Memorise time of direction change
    lastCutDirectionChange = millis();
    DisplayPrint(0,2, F("                  "));
  }

  //--------------------------------
  // Environment sensing for approaching objects
  //--------------------------------
  if (!MowerSlowDownApproachingObstables(15,
  // if (!MowerSlowDownApproachingObstables(MOWER_MOWING_TRAVEL_SPEED - MOWER_MOVES_SPEED_SLOW,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING,
                                         PERIMETER_APPROACHING_THRESHOLD))
  {
    if (!g_CutMotorOn)
    {
      CutMotorStart(bladeDirection, MOWER_MOWING_CUTTING_SPEED);
      // Give time for cut motor to start
      delay(MOWER_MOWING_CUT_START_WAIT);
    }
    MowerArc(MOTION_MOTOR_FORWARD, leftSpeed, rightSpeed);
    // MowerSpeed(MOWER_MOWING_TRAVEL_SPEED);
  }

  //--------------------------------
  // Obstacle Collision detection
  //--------------------------------

  // TO DO : if outside, mower gets "locked-out"!!

  if (OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                    SONAR_MIN_DISTANCE_FOR_STOP,
                                                    SONAR_MIN_DISTANCE_FOR_STOP,
                                                    SONAR_MIN_DISTANCE_FOR_STOP,
                                                    true,
                                                    true))
  {
    // Check if number of consecutive obstacle detection is above threshold and put mower in Error mode
    if (g_successiveObstacleDectections > MOWER_MOWING_MAX_CONSECUTVE_OBSTACLES)
    {
      g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_CONSECUTIVE_OBSTACLES;
      return;
    }
    else
    {
      // In case of obstacle or perimeter reached, spiral mowing mode ends
      if (isSpiral)
      {
        rightSpeed = MOWER_MOWING_TRAVEL_SPEED;
        leftSpeed = MOWER_MOWING_TRAVEL_SPEED;
        isSpiral = false;
        g_mowingMode = MOWER_MOWING_MODE_RANDOM;
      }

      // Start mowing again if inside perimeter
      if (g_isInsidePerimeter)
      { 
        if (!g_CutMotorOn)
        {
          CutMotorStart(bladeDirection, MOWER_MOWING_CUTTING_SPEED);
          // Give time for cut motor to start
          delay(MOWER_MOWING_CUT_START_WAIT);
        }
        MowerForward(MOWER_MOWING_TRAVEL_SPEED);
      }
    }
  }

// If spiral mowing mode and circle time is elapsed, update speeds
  if (isSpiral && (millis() - lastSpiralSpeedAdjustment > stepDuration))
  {
    switch (g_mowingMode)
    {
      case MOWER_MOWING_MODE_SPIRAL_CLOCKWISE:
        rightSpeed = rightSpeed + MOWER_MOWING_SPIRAL_SPEED_INCREMENT;
        break;
      case MOWER_MOWING_MODE_SPIRAL_COUNTER_CLOCKWISE:
        leftSpeed = leftSpeed + MOWER_MOWING_SPIRAL_SPEED_INCREMENT;
        break;
      default:
        break;
    }
    stepDuration = stepDuration + (g_spiralStepTimeIncrement[spiralStep] * MOWER_MOWING_SPIRAL_CIRCLES_PER_STEP);
    spiralStep = spiralStep + 1;

    // If last step is reached, stop spiral mow
    if (spiralStep == MOWER_MOWING_SPIRAL_MAX_STEP) 
    { 
      isSpiral = false;
      g_mowingMode = MOWER_MOWING_MODE_RANDOM;
      rightSpeed = MOWER_MOWING_SPIRAL_MAX_SPEED;
      leftSpeed = MOWER_MOWING_SPIRAL_MAX_SPEED;
    }
    
    DebugPrintln("Spiral mowing: step " + String(spiralStep) + ", changing motor speeds to Left:" + String(leftSpeed) + "%, Right:" + String(rightSpeed) + "%", DBG_INFO, true);

    //Memorise last change
    lastSpiralSpeedAdjustment = millis();

    // Update motor speeds
    MowerArc(MOTION_MOTOR_FORWARD, leftSpeed, rightSpeed);
  }

  g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
  mowingStartTime = millis();

  // Update display
  mowingDisplay();
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Returning to base Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower returning to base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerGoingToBase(const bool StateChange, const MowerState PreviousState)
{
  bool PIDReset = false;
  bool FindWireReset = false;
  static bool FindWire = false;
  static bool FollowWire = false;
  static int FindWirePhase = 0;

  //--------------------------------
  // Actions to take when entering the Return to Base state
  //--------------------------------

  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Mower going back to base", TAG_TO_BASE, DBG_INFO);

    // Change display with refresh
    toBaseDisplay(true);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_FAST;

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    // Activate Sonar reading
    g_SonarReadEnabled = true;          // activate Sonar readings
    delay(SONAR_READ_ACTIVATION_DELAY); //wait for task to take 1st readings

    // Sound starting beep to notify environment
    if (PreviousState == MowerState::idle)
    {
      // TO DO Beep
    }

    // just in case, stop cut motor
    CutMotorStop();

    // Close Battery charge relay to be able to detect arrival on base station
    BatteryChargeRelayClose();

    // Ensure wire finding function is called on first call
    FindWire = true;
    FindWireReset = true;
    FindWirePhase = PERIMETER_SEARCH_PHASE_1;

    // Ensure wire tracking PID is reset on first call
    FollowWire = false;
    PIDReset = true;
  }

  // Overall procedure is as follows:
  // Check for tilt,
  // Execute wire find procedure (does not runs as a loop and has it's own pre-condition and obstacle detection checks),
  // Execute follow wire procedure (does not runs as a loop and has it's own pre-condition and obstacle detection checks),
  // Decide on actions if obstacles are detected,
  // Stop when conditions met (TO DO)

  //--------------------------------
  // Check tilt sensors and take immediate action
  //--------------------------------

  if (CheckTiltReadAndAct())
  {
    return;
  }

  // find target with compass TO DO

  //--------------------------------
  // Find perimeter wire
  //--------------------------------
  if (FindWire)
  {
    if (MowerFindWire(FindWireReset, &FindWirePhase, BACK_TO_BASE_HEADING, BACK_TO_BASE_CLOCKWISE))
    {
      // Stop wire finding
      FindWire = false;
      FollowWire = true;
      PIDReset = true;
    }
  }

  //--------------------------------
  // Follow perim to base
  //--------------------------------
  if (FollowWire)
  {
    if (!MowerFollowWire(&PIDReset, BACK_TO_BASE_HEADING, BACK_TO_BASE_CLOCKWISE))
    {
      //      g_CurrentState = MowerState::idle;
      FollowWire = false;
    };
  }


  // Update display
  toBaseDisplay();

  // Check if base reached - Current is flowing
  BatteryChargeCurrentRead(true);
  if (g_BatteryChargeCurrent > MOWER_AT_BASE_CURRENT)
  {
    MowerStop();
    LogPrintln("Mower arrived at base", TAG_TO_BASE, DBG_INFO);

    g_CurrentState = MowerState::docked;
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Leaving base Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower leaving base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerLeavingBase(const bool StateChange, const MowerState PreviousState)
{
  if (StateChange)
  {
    LogPrintln("Mower leaving Base", TAG_STATES, DBG_INFO);

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    // Open Battery charge relay to reduce energy consumption of keeping relay closed
    BatteryChargeRelayOpen();

  //  Reset battery charge current to 0
    g_BatteryChargeCurrent = 0;
    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_FAST;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    // Change display with refresh
    LeavingBaseDisplay(true);
  }

  // Reverse out of base
  int exitAngle = random(-180, -270);

  MowerReserseAndTurn(exitAngle, LEAVING_BASE_REVERSE_DURATION, true); // reverse and turn random angle

  LeavingBaseDisplay();

  // for the moment, mow from the spot
  g_CurrentState = MowerState::mowing;
}

//---------------------------------------------------------------------------------------------------------------------------
//
// ERROR Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower in error
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerInError(const bool StateChange, const MowerState PreviousState)
{
  if (StateChange)
  {
    // STOP all motors
    MowerStop();
    CutMotorStop(true);

    // Change display with refresh
    errorDisplay(true);

    // Change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    // Suspend Sonar readings
    //    g_SonarReadEnabled = false;

    DebugPrintln("");
    LogPrintln("Mower stopped on Error #" + String(g_CurrentErrorCode) + "-" + ErrorString(g_CurrentErrorCode), TAG_ERROR, DBG_ERROR);

    // disable other sensors ?
    // send notification to phone ?
    // send telemetry ?

    DebugPrintln("Mower Acknowledgement required!", DBG_ERROR, true);
  }
  else
  {
    // Update display
    errorDisplay();

    // wait for user action (keypad action)
    // sound SOS beep
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Helper functions
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Mower finds perimeter wire according to heading given and, when found, orient right or left
 * @param reset boolean indicating the wire finding function needs to be reset
 * @param phase pointer to integer used to indicate the search phase (input and output)
 * @param heading integer indicating the heading to follows to try to find the wire
 * @param clockwise boolean indicating the direction in which the mower should face if it finds the wire
 * @return Success boolean depending on whether it found the wire (true) or not (false)
 */
bool MowerFindWire(const bool reset, int *phase, const int heading, const bool clockwise)
{
  static unsigned long searchStartTime = millis();
  static int turnIncrement = 0;
  static int turnCount = 0;
  // ************************
  // TO DO - HEADING to base!!!!!
  // ************************
  if (reset)
  {
    DebugPrintln("");
    LogPrintln("Mower started searching for wire", TAG_SEARCH, DBG_INFO);

    //--------------------------------
    // Check if pre-requisistes conditions are met
    //--------------------------------

    if (!CheckPreConditions(ERROR_WIRE_SEARCH_NO_START_TILT_ACTIVE,
                            ERROR_NO_ERROR, // no error here as 1st phase is a reverse motion
                            ERROR_NO_ERROR, // no error here as 1st phase is a reverse motion
                            ERROR_NO_ERROR, // no error here as 1st phase is a reverse motion
                            ERROR_NO_ERROR, // no error here as 1st phase is a reverse motion
                            ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL,
                            true))
    {
      return false;
    }

    // Initialise Phases
    *phase = PERIMETER_SEARCH_PHASE_1;
    DebugPrintln("Phase 1 - Reversing to find wire", DBG_DEBUG, true);
    searchStartTime = millis();
    turnIncrement = 0;
    turnCount = 0;
  }

  switch (*phase)
  {
    //--------------------------------------------------------------
    // First, if outside perimeter, mower reverses to try and come back in the perimeter
    // If search "timeout" is reached, procedure (and mower) stops and raises an error
    //--------------------------------------------------------------

  case PERIMETER_SEARCH_PHASE_1:
  {
    // Not inside and still time
    if (!g_isInsidePerimeter && millis() - searchStartTime < PERIMETER_SEARCH_REVERSE_MAX_TIME)
    {
      // Reverse Mower
      MowerReverse(PERIMETER_SEARCH_REVERSE_SPEED, PERIMETER_SEARCH_REVERSE_TIME);
      return false; // continue search
    }

    // Back inside => the phase ends
    if (g_isInsidePerimeter)
    {
      MowerStop();
      *phase = PERIMETER_SEARCH_PHASE_2;
      DebugPrintln("Phase 2 - Forward to find wire", DBG_DEBUG, true);
      searchStartTime = millis();
      MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
      return false; // continue search
    }

    // Time is up ... raise error and stop search
    if (millis() - searchStartTime > PERIMETER_SEARCH_REVERSE_MAX_TIME)
    {
      DebugPrintln("Phase 1 failled: not inside perimeter", DBG_DEBUG, true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_1_FAILLED;
      return false;
    }

    // Should never come Here !
    DebugPrintln("Phase 1 : Code should not be here !", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_UNDEFINED;
    return false;
  }

    //--------------------------------------------------------------
    // Second, once in the perimeter, it goes forward to find the wire (and stops if it finds obstacles) and goes over the wire (stops when outside)
    // If search "timeout" is reached, procedure (and mower) stops and raises an error
    //--------------------------------------------------------------

  case PERIMETER_SEARCH_PHASE_2:
  {
    // Is Perimeter signal Lost or stopped ?
    if (g_PerimeterSignalLost || g_PerimeterSignalStopped)
    {
      MowerStop();
      DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
      return false;
    }

    // Still inside and moving towards perimeter
    if (g_isInsidePerimeter && millis() - searchStartTime < PERIMETER_SEARCH_FORWARD_MAX_TIME_1)
    {
      // Environment sensing for approaching objects
      // if (!MowerSlowDownApproachingObstables(PERIMETER_SEARCH_FORWARD_SPEED - MOWER_MOVES_SPEED_SLOW,
      if (!MowerSlowDownApproachingObstables(10,
                                             SONAR_MIN_DISTANCE_FOR_SLOWING,
                                             SONAR_MIN_DISTANCE_FOR_SLOWING,
                                             SONAR_MIN_DISTANCE_FOR_SLOWING,
                                             PERIMETER_APPROACHING_THRESHOLD))
      {
        MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
        return false; // continue search
      }

      // Obstacle Collision detection
      if (OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                        SONAR_MIN_DISTANCE_FOR_STOP,
                                                        SONAR_MIN_DISTANCE_FOR_STOP,
                                                        SONAR_MIN_DISTANCE_FOR_STOP,
                                                        false, //  no Perimeter detection (and action)
                                                        true))
      {
        // Check if number of consecutive obstacle detection is above threshold and put mower in Error mode
        if (g_successiveObstacleDectections > PERIMETER_SEARCH_MAX_CONSECUTVE_OBSTACLES)
        {
          g_CurrentState = MowerState::error;
          g_CurrentErrorCode = ERROR_WIRE_SEARCH_CONSECUTIVE_OBSTACLES;
          return false;
        }
        else
        {
          // Start searching again
          // TO DO - HEADING to base!!!!!
          MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
        }
      }

      DebugPrintln("Phase 2 : keep going....", DBG_VERBOSE, true);
      return false; // continue search
    }

    // Reached outside (wire found): => the phase ends
    if (!g_isInsidePerimeter && g_PerimeterSmoothMagnitudeTracking > PERIMETER_MIN_MAGNITUDE_FOR_OUT_DETECTION)
    {
      MowerStop();
      *phase = PERIMETER_SEARCH_PHASE_3;
      if (clockwise)
      {
        DebugPrintln("Phase 3 - Turn clockwise", DBG_DEBUG, true);
        turnIncrement = PERIMETER_SEARCH_ANGLE_INCREMENT;
      }
      else
      {
        DebugPrintln("Phase 3 - Turn counter clockwise", DBG_DEBUG, true);
        turnIncrement = -PERIMETER_SEARCH_ANGLE_INCREMENT;
      }
      turnCount = 0;
      return false; // continue search
    }
    else    // we consider that we are not really outside as the signal magniture is too low and we keep going...
    {
      DebugPrintln("Phase 2 : keep going (outside, but perimeter signal too weak)....", DBG_VERBOSE, true);
      return false; // continue search
    }

    // Time is up ... raise error and stop search
    if (millis() - searchStartTime > PERIMETER_SEARCH_FORWARD_MAX_TIME_1)
    {
      MowerStop();
      DebugPrintln("Phase 2 failled: not outside perimeter", DBG_DEBUG, true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_2_FAILLED;
      return false;
    }

    // Should never come Here !
    MowerStop();
    DebugPrintln("Phase 2 : Code should not be here ! (Phase:" + String(*phase) + ") " + String(millis() - searchStartTime) + " ms", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_UNDEFINED;
    return false;
  }

    //--------------------------------------------------------------
    // Third, it turns in direction requested (clockwise or counter-clockwise)
    // By slowing down before crossing the wire, the mower should have not gone very far over
    // and doing an "on the spot" turn, we can hope to be just inside the perimeter
    //--------------------------------------------------------------

  case PERIMETER_SEARCH_PHASE_3:
  {
    // Is Perimeter signal Lost or stopped ?
    if (g_PerimeterSignalLost || g_PerimeterSignalStopped)
    {
      MowerStop();
      DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
      return false;
    }

    // Not inside and still turn steps available
    if (!g_isInsidePerimeter && turnCount < PERIMETER_SEARCH_TURN_MAX_ITERATIONS)
    {
      //--------------------------------
      // Check tilt sensors and take immediate action
      //--------------------------------
      if (CheckTiltReadAndAct())
      {
        return false;
      }

      // Turn on the spot by steps
      DebugPrintln("Phase 3 turn: " + String(turnCount), DBG_DEBUG, true);
      MowerTurn(turnIncrement, true);
      delay(PERIMETER_TIMER_PERIOD * 2 / 1000); // wait to make sure that Perimeter signal read task can refresh g_isInsidePerimeter status
      turnCount = turnCount + 1;

      return false; // continue search
    }

    // Back inside and signal is strong enough => the phase ends
    // if (g_isInsidePerimeter && !g_PerimeterSignalLowForTracking)
    if (g_isInsidePerimeter)
    {
      MowerStop();
      *phase = PERIMETER_SEARCH_FINISHED;
      return true; // end search
    }

    // Max Number of turns reached ... raise error and stop search
    if (turnCount >= PERIMETER_SEARCH_TURN_MAX_ITERATIONS)
    {
      DebugPrintln("Phase 3 failled: not inside perimeter", DBG_DEBUG, true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_3_FAILLED;
      return false;
    }
    // Should never come Here !
    DebugPrintln("Phase 3 : Code should not be here ! (Phase:" + String(*phase) + ")", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_UNDEFINED;
    return false;
  }

  default:
  {
    MowerStop();
    DebugPrintln("Phase Undefined - should not be here !!!  (Phase:" + String(*phase) + ")", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_UNDEFINED;
    return false;
  }
  }
}

/**
 * @brief Mower follows perimeter wire back to charging station or to mowing zone
 * @param reset boolean indicating the wire trakcing function and PID needs to be reset
 * @param heading integer indicating the heading to follow to get to charging station
 * @param clockwise boolean indicating the direction in which the mower is following the wire
 * @return Success boolean depending on whether it found the wire (true) or not (false)
 */
bool MowerFollowWire(bool *reset, const int heading, const bool clockwise)
{
  static unsigned long lastPIDUpdate = 0;
  int speedAdjustment = 0;
  static int outsideCount = 0;

  if (*reset)
  {
    DebugPrintln("");
    LogPrintln("Mower started to follow wire", TAG_SEARCH, DBG_INFO);

    //--------------------------------
    // Check if pre-requisistes conditions are met
    //--------------------------------

    if (!CheckPreConditions(ERROR_FOLLOW_WIRE_NO_START_TILT_ACTIVE,
                            ERROR_FOLLOW_WIRE_NO_START_BUMPER_ACTIVE,
                            ERROR_FOLLOW_WIRE_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_FOLLOW_WIRE_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_FOLLOW_WIRE_NO_START_OBJECT_TOO_CLOSE,
                            ERROR_FOLLOW_WIRE_NO_START_NO_PERIMETER_SIGNAL,
                            true))
    {
      return false;
    }

    DebugPrintln("Kp:" + String(g_ParamPerimeterTrackPIDKp,4) + " Ki:" + String(g_ParamPerimeterTrackPIDKi,4) + " Kd:" + String(g_ParamPerimeterTrackPIDKd,4), DBG_DEBUG, true);
    DebugPrintln("Smag:" + String(g_PerimeterSmoothMagnitude), DBG_DEBUG, true);
    DebugPrintln("");

    // Acknowledge PID init
    *reset = false;

    // Initialize the variables the PID is linked to
    g_PIDSetpoint = g_PerimeterTrackSetpoint;
    g_PIDInput = g_PerimeterMagnitudeAvg;

    // Turn the PID on
    g_PerimeterTrackPID.SetMode(AUTOMATIC);
    g_PerimeterTrackPID.SetTunings(g_ParamPerimeterTrackPIDKp, g_ParamPerimeterTrackPIDKi, g_ParamPerimeterTrackPIDKd, P_ON_E);
    g_PerimeterTrackPID.SetOutputLimits(-50, 50); // in %
    g_PerimeterTrackPID.SetControllerDirection(DIRECT);
    g_PerimeterTrackPID.SetSampleTime(PERIMETER_TRACKING_PID_INTERVAL);

#ifdef MQTT_PID_GRAPH_DEBUG
    // g_MQTTPIDGraphDebug = true; // for testing
#endif

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    // Move forward
    MowerForward(BACK_TO_BASE_SPEED);

    outsideCount = 0;
  }

  //--------------------------------
  // Check tilt sensors and take immediate action
  //--------------------------------
  if (CheckTiltReadAndAct())      // Function stops motors if required
  {
    return false;
  }

  //--------------------------------
  // Environment sensing for approaching objects depending on direction
  //--------------------------------
  bool SlowedDown = false;
  if (clockwise)
  {
    SlowedDown = MowerSlowDownApproachingObstables(5,
                                                   SONAR_MIN_DISTANCE_FOR_SLOWING/2,
                                                   0, // no left detection
                                                   SONAR_MIN_DISTANCE_FOR_SLOWING/2,
                                                   200); // no perimeter detection
  }
  else
  {
    SlowedDown = MowerSlowDownApproachingObstables(5,
                                                   SONAR_MIN_DISTANCE_FOR_SLOWING/2,
                                                   SONAR_MIN_DISTANCE_FOR_SLOWING/2,
                                                   0,  // no right detection
                                                   200); // no perimeter detection
  }

  if (!SlowedDown) // No approaching objects : resume normal speed
  {
    MowerSpeed(BACK_TO_BASE_SPEED);
    speedAdjustment = 0;
  }
  else
  {
    speedAdjustment = 5;
  }

  //--------------------------------
  // Obstacle Collision detection depending on direction
  //--------------------------------

  // TO DO decide if this can remain depengin on docking strategy

  int colisionDetected = OBSTACLE_DETECTED_NONE;

  if (clockwise)
  {
    colisionDetected = OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                                    //  SONAR_MIN_DISTANCE_FOR_STOP,
                                                                     0, // no Front detection
                                                                     0, // no left detection
                                                                    //  SONAR_MIN_DISTANCE_FOR_STOP,
                                                                    0,
                                                                     false,  // no perimeter detection
                                                                     false); // no action (for the moment ???????????????)
  }
  else
  {
    colisionDetected = OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                                    //  SONAR_MIN_DISTANCE_FOR_STOP,
                                                                     0, // no Front detection
                                                                    //  SONAR_MIN_DISTANCE_FOR_STOP,
                                                                     0,
                                                                     0,      // no right detection
                                                                     false,  // no perimeter detection
                                                                     false); // no action (for the moment ???????????????)
  }

  // For the moment, to keep things simple, we will conider that there should be no object on the wire path (time will tell is this is a reasonable assumption).
  // To achieve this, we simply set the FOLLOW_WIRE_MAX_CONSECUTVE_OBSTACLES definition to 0.
  // So if an obstacle is detected, we declare an error and stop the wire following function.

  if (colisionDetected != OBSTACLE_DETECTED_NONE)
  {
    // Check if number of consecutive obstacle detection is above threshold and put mower in Error mode
    if (g_successiveObstacleDectections > FOLLOW_WIRE_MAX_CONSECUTVE_OBSTACLES)
    {
      MowerStop();
      CutMotorStop(true);
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_FOLLOW_WIRE_CONSECUTIVE_OBSTACLES;
      return false;
    }
    else
    {
      // This will be the place to insert any code if we decide to implement obstacle avoidance strategy in case of obstacle during wire tracking function.
    }
  }

  //--------------------------------
  // Is Perimeter signal Lost or stopped?
  //--------------------------------

  if (g_PerimeterSignalLost || g_PerimeterSignalStopped)
  {
    MowerStop();
    DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
    return false;
  }

  //--------------------------------
  // Is Perimeter signal strong enough ?
  //--------------------------------

  if (g_PerimeterSignalLowForTracking)
  {
    DebugPrintln("Perimeter signal too low for tracking (" + String(g_PerimeterSmoothMagnitudeTracking) + ")", DBG_VERBOSE, true);
    // for the moment, do nothing
    // TO DO ....
    // stop mower
    // either trigger a wire search or declare an error
    // return false;
  }

  //--------------------------------
  // Is mower outside for too long ?
  //--------------------------------
  if (!g_isInsidePerimeter)
  {
    outsideCount = outsideCount + 1;
  }
  else
  {
    // outsideCount = max(0, outsideCount - 2);
    outsideCount = 0;
  }
  if (outsideCount > PERIMETER_TRACKING_MAX_CONSECUTVE_OUTSIDE)
  {
    MowerStop();
    CutMotorStop();
    DebugPrintln("Mower outside Perimeter for too long (" + String(outsideCount) + ")", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_FOLLOW_WIRE_OUTSIDE_TOO_LONG;
    return false;
  }

  //--------------------------------
  // Use PID to correct mower alignment over peimeter wire by adjusting orders to motors
  //--------------------------------

  if (millis() - lastPIDUpdate > PERIMETER_TRACKING_PID_INTERVAL)
  {
    // update PID with new input
    // g_PIDInput = g_PerimeterMagnitudeAvgPID;
    g_PIDInput = g_PerimeterMagnitudeAvg;

    // Update PID settings "on the fly" too enable easier tuning (parameters are modified through MQTT topics)
    g_PIDSetpoint = g_PerimeterTrackSetpoint;
    g_PerimeterTrackPID.SetTunings(g_ParamPerimeterTrackPIDKp, g_ParamPerimeterTrackPIDKi, g_ParamPerimeterTrackPIDKd, P_ON_E);

//    DebugPrintln("PID P " + String(g_PerimeterTrackPID.GetKp()) + " " + String(g_ParamPerimeterTrackPIDKp), DBG_VERBOSE, true);

    bool PIDReturn = g_PerimeterTrackPID.Compute();
    if (!PIDReturn)
    {
      DebugPrintln("PID Returned false...", DBG_DEBUG, true);
    }
    lastPIDUpdate = millis();

    // convert PID output into motor commands
    //
    // Assuming we have a 0 setpoint (assuming that when straight above wire mag = 0) :
    // If Mag value goes negative, we are going towards the inside and PID controller will give a positive output value
    //    If we aim to follow the wire in a clockwise direction, we should turn the mower to the left (by slowing down the left wheel)
    //    If we aim to follow the wire in a anticlockwise direction, we should turn the mower to the right (by slowing down the right wheel)
    // If Mag value goes positive, we are going towards the ouside and PID controller will give a negative output value
    //    If we aim to follow the wire in a clockwise direction, we should turn the mower to the right (by slowing down the right wheel)
    //    If we aim to follow the wire in a anticlockwise direction, we should turn the mower to the left (by slowing down the left wheel)
    //
    // Action on wheels to generate turn is by reducing the speed of the inner wheel because if motor is allready at full speed, it is not posible to increase speed of outter wheel

    DebugPrintln("\t\t\tPID Returned ..." + String(g_PIDOutput), DBG_DEBUG, true);

    if (g_PIDOutput != 0)
    {
      if (g_PIDOutput > 0) // Mag is negative and PID output positive, so we are inside and need to move towards the outside
      {
        if (clockwise) // we need to go towards the left
        {
          MotionMotorsTrackingAdjustSpeed(-g_PIDOutput-speedAdjustment, -speedAdjustment);
        }
        else // we need to go towards the right
        {
          MotionMotorsTrackingAdjustSpeed(-speedAdjustment, -g_PIDOutput-speedAdjustment);
        }
      }
      else // Mag is positive and PID output negative, so we are outside and need to move towards the inside
      {
        if (clockwise) // we need to go towards the right
        {
          MotionMotorsTrackingAdjustSpeed(-speedAdjustment, g_PIDOutput-speedAdjustment);
        }
        else // we need to go towards the left
        {
          MotionMotorsTrackingAdjustSpeed(g_PIDOutput-speedAdjustment, -speedAdjustment);
        }
      }
    }
    else
    {
      MotionMotorsTrackingAdjustSpeed(0, 0);
    }

#ifdef MQTT_PID_GRAPH_DEBUG
    //  send debug information through MQTT
    if (g_MQTTPIDGraphDebug)
    {
      String JsonPayload = "";
      FirebaseJson JSONDBGPayload;
      String JSONDBGPayloadStr;
      char MQTTpayload[MQTT_MAX_PAYLOAD];

      JSONDBGPayload.clear();
      JSONDBGPayload.add("S", g_PIDSetpoint);
      JSONDBGPayload.add("I", g_PIDInput);
      JSONDBGPayload.add("O", g_PIDOutput);
      JSONDBGPayload.add("CL", g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_LEFT]);
      JSONDBGPayload.add("CR", g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_RIGHT]);
      JSONDBGPayload.add("ML", g_MotionMotorSpeed[MOTION_MOTOR_LEFT]);
      JSONDBGPayload.add("MR", g_MotionMotorSpeed[MOTION_MOTOR_RIGHT]);

      JSONDBGPayload.toString(JSONDBGPayloadStr, false);
      JSONDBGPayloadStr.toCharArray(MQTTpayload, JSONDBGPayloadStr.length() + 1);
      bool result = MQTTclient.publish(MQTT_PID_DEBUG_CHANNEL, MQTTpayload);
      if (result != 1)
      {
        g_MQTTErrorCount = g_MQTTErrorCount + 1;
      }
      MQTTclient.loop();
      //    DebugPrintln("Sending to :[" + String(MQTT_PID_DEBUG_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
    }
#endif
  }
  return true;
}

/**
 * Enables the perimeter tracking adjustment of the speed for both motors
 * @param leftMotorAjustment adjustment to apply to left Motor (in %)
 * @param rightMotorAjustment adjustment to apply to right Motor (in %)
 */
void MotionMotorsTrackingAdjustSpeed(const int leftMotorAjustment, const int rightMotorAjustment)
{
  g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_LEFT] = leftMotorAjustment;
  MotionMotorSetSpeed(MOTION_MOTOR_LEFT, BACK_TO_BASE_SPEED); // function will apply correction and set new speed
  g_WheelPerimeterTrackingCorrection[MOTION_MOTOR_RIGHT] = rightMotorAjustment;
  MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, BACK_TO_BASE_SPEED); // function will apply correction and set new speed
}

/**
 * @brief Function to check pre-conditions before performing tasks of a given state. Pre-condition checks can be performed on Tilt sensors (both), bumpers (both), Sonar sensors (Front, left or right) and perimeter wire active
 * If a precondition is not met and if the error enable parameter is set, an error is triggered (error number passed as parameter) and the mower is set into ERROR mode.
 * If the error enable parameter is not set, the function only returns if pre-conditions are net or not and the mower state is not changed (left to caller to decide).
 * To disable a pre-condition check, the ERROR_NO_ERROR error number should be passed as parameter value.
 * 
 * @param Tilt (optional) check tilt sensors (both) and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param Bumper (optional) check bumper sensors (both) and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param Front (optional) check front sonar and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param Left (optional) check left sonar and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param Right (optional) check right sonar and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param Perimeter (optional) check perimeter wire active and trigger ERROR Mode if ErrorMode is true (ERROR_NO_ERROR to disable, by default)
 * @param ErrorMode (optional) if true, changes mower mode to Error (default is false).
 * @return boolean indicating if preconditions are met (true) or not (false).
 */
bool CheckPreConditions(const int Tilt, const int Bumper, const int Front, const int Left, const int Right, const int Perimeter, const bool ErrorMode)
{
  // Tilt checks (activated or not)

  if (Tilt != ERROR_NO_ERROR && (TiltRead(TILT_HORIZONTAL) || TiltRead(TILT_VERTICAL)))
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Tilt;
    }
    return false;
  }

  // Bumper checks (activated or not)

  if (Bumper != ERROR_NO_ERROR && (BumperRead(BUMPER_RIGHT) || BumperRead(BUMPER_LEFT)))
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Bumper;
    }
    return false;
  }

  // Front sonar (close obstacle or not)

  if (Front != ERROR_NO_ERROR && (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_PRECONDITION))
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Front;
    }
    return false;
  }

  // Left sonar (close obstacle or not)

  if (Left != ERROR_NO_ERROR && (g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_PRECONDITION))
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Left;
    }
    return false;
  }

  // Right sonar (close obstacle or not)

  if (Right != ERROR_NO_ERROR && (g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_PRECONDITION))
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Right;
    }
    return false;
  }

  // Perimeter active

  // TO DO

  if (Perimeter != ERROR_NO_ERROR && (false)) // TO DO
  {
    if (ErrorMode)
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = Perimeter;
    }
    return false;
  }

  return true;
}

/**
 * @brief Function to check if an obstacle is detected and performs a pre-defined action (if activated).
 * Obstacle detection can be any combination of bumper (both), Sonars (Front, Left or right) and perimeter wire, but function returns to caller at first detection.
 * Detections are performed in the following order: 1-Bumper, 2- Perimeter, 3-Front sonar, 4-Left sonar, 5-Right sonar.
 * If the action enable parameter is not set, the function only returns if a detection occured or not and action to take is left to caller to decide.
 * At the end of the funtion, if the action enable is set, the mower is stopped and the cutting motor is stopped. Caller to restart the motors as required.
 * 
 * @param Bumper as optional boolean : bumper active triggers detection/action. 0 disbales the check. Default is 0
 * @param Front as optional int: sonar measured front distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Left as optional int: sonar measured left distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Right as optional int: sonar measured right distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Perimeter as optional boolean: outside perimeter wire to trigger detection/action. 0 disables the check. Absolute value is used to perform the check (applies to both inside and outside perimeter wire).  Default is 0.
 * @param ActionMode as optional boolean: if true, changes action is performed if condition is detected (default is false).
 * @return integer indicating which detection was detected.
 */
int CheckObstacleAndAct(const bool Bumper, const int Front, const int Left, const int Right, const bool Perimeter, const bool ActionMode)
{
  int firstSide = 0;  // first side to check distance before acting on obstacle detection 
  int secondSide = 0; // Second side to check distance before acting on obstacle detection
  String SideStr[SONAR_COUNT] = {"", "Left", "Right"};
  int firstSideAngle = 0; // Angle to turn for first direction
  int secondSideAngle = 0; // Angle to turn for second direction

  // Determine random order for sides to check
  if (millis() % 2 == 0)
  {
    firstSide = SONAR_RIGHT;
    firstSideAngle = random(45, 200);
    secondSide = SONAR_LEFT;
    secondSideAngle = random(-200, -45);
  }
  else
  {
    firstSide = SONAR_LEFT;
    firstSideAngle = random(-200, -45);
    secondSide = SONAR_RIGHT;
    secondSideAngle = random(45, 200);
  }

  //--------------------------------
  // Bumper Collision detection
  //--------------------------------
  // Reaction to bumper collision is as follows:
  //    Stop motion and cutting,
  //    if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  //    if not possible, if possible to turn right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  //    if not possible, reverse further and turn to  be ready to go in "opposite direction"

  if (!Bumper && (BumperRead(BUMPER_RIGHT) || BumperRead(BUMPER_LEFT)))
  {
    DebugPrintln("Bumper collision detected ! ", DBG_DEBUG, true);
    if (ActionMode)
    {
      // Stop motion and stop cutting motor
      MowerStop();
      CutMotorStop(true);

      if (g_SonarDistance[firstSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on first side
      {
        DebugPrintln("Turning " + SideStr[firstSide] , DBG_VERBOSE, true);
        MowerReserseAndTurn(firstSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
      }
      else
      {
        if (g_SonarDistance[secondSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on second side
        {
          DebugPrintln("Reversing and turning " + SideStr[secondSide], DBG_VERBOSE, true);
          MowerReserseAndTurn(secondSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
        }
        else
        {
          DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(135, 225), MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn right 135 degrees....and hope for the best !
        }
      }
    }

    // count as an obstable detection
    g_totalObstacleDectections = g_totalObstacleDectections + 1;
    g_successiveObstacleDectections = g_successiveObstacleDectections + 1;

    return OBSTACLE_DETECTED_BUMPER;
  }

  //--------------------------------
  // Perimeter wire detection
  //--------------------------------
  // Reaction to being outside perimeter wire is as follows:
  //    Stop motion (no need to stop cutting as this a normal event),
  //    if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  //    if not possible, if possible to turn Right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  //    if not possible, reverse further and turn to be ready to go in "opposite direction"

  if (Perimeter && !g_isInsidePerimeter && g_PerimeterSmoothMagnitudeTracking > PERIMETER_MIN_MAGNITUDE_FOR_OUT_DETECTION)
  {
    DebugPrintln("Outside Perimeter cable (mag:" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);

    if (ActionMode)
    {
      MowerStop();

      if (g_SonarDistance[firstSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on first side
      {
        DebugPrintln("Turning " + SideStr[firstSide] , DBG_VERBOSE, true);
        MowerReserseAndTurn(firstSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
      }
      else
      {
        if (g_SonarDistance[secondSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on second side
        {
          DebugPrintln("Reversing and turning " + SideStr[secondSide], DBG_VERBOSE, true);
          MowerReserseAndTurn(secondSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
        }
        else
        {
          DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(135, 225), MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn right 135 degrees....and hope for the best !
        }
      }
    }
    return OBSTACLE_DETECTED_PERIMETER;
  }

  //--------------------------------
  // Front Sonar Collision detection
  //--------------------------------
  // Reaction to Front sonar detection is as follows:
  //    Stop motion and cutting,
  //    if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  //    if not possible, if possible to turn right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  //    if not possible, reverse further and turn to  be ready to go in "opposite direction"

  if (Front != 0 && (g_SonarDistance[SONAR_FRONT] < Front))
  {
    DebugPrintln("Font sonar proximity ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    {
      // Stop motion and stop cutting motor
      MowerStop();
      CutMotorStop(false);

      if (g_SonarDistance[firstSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on first side
      {
        DebugPrintln("Turning " + SideStr[firstSide] , DBG_VERBOSE, true);
        MowerReserseAndTurn(firstSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
      }
      else
      {
        if (g_SonarDistance[secondSide] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on second side
        {
          DebugPrintln("Reversing and turning " + SideStr[secondSide], DBG_VERBOSE, true);
          MowerReserseAndTurn(secondSideAngle, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn random angle
        }
        else
        {
          DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(135, 225), MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn right 135 degrees....and hope for the best !
        }
      }
    }

    // count as an obstable detection
    g_totalObstacleDectections = g_totalObstacleDectections + 1;
    g_successiveObstacleDectections = g_successiveObstacleDectections + 1;

    return OBSTACLE_DETECTED_FRONT;
  }

  //--------------------------------
  // Left Sonar Collision detection
  //--------------------------------
  // Reaction to Left sonar detection is as follows:
  //    Stop motion and cutting,
  //    if possible to turn right, turn mower to the right by a "small" angle (~ less than 60 degrees)
  //    if not possible, reverse and turn to be ready to go in "opposite direction"

  if (Left != 0 && (g_SonarDistance[SONAR_LEFT] < Left))
  {
    DebugPrintln("Left sonar proximity ! (" + String(g_SonarDistance[SONAR_LEFT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    {
      MowerStop();
      // CutMotorStop(false);

      if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on right side
      {
        DebugPrintln("Turning right", DBG_VERBOSE, true);
        MowerTurn(random(20, 60), true); // turn right 30 degrees
      }
      else
      {
        DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
        MowerReserseAndTurn(random(135, 225), MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn right 135 degrees....and hope for the best !
      }
    }

    // count as an obstable detection
    g_totalObstacleDectections = g_totalObstacleDectections + 1;
    g_successiveObstacleDectections = g_successiveObstacleDectections + 1;
    return OBSTACLE_DETECTED_LEFT;
  }

  //--------------------------------
  // Right Sonar Collision detection
  //--------------------------------
  // Reaction to Right sonar detection is as follows:
  //    Stop motion and cutting,
  //    if possible to turn Left, turn mower to the left by a "small" angle (~ less than 60 degrees)
  //    if not possible, reverse and turn to be ready to go in "opposite direction"

  if (Right != 0 && (g_SonarDistance[SONAR_RIGHT] < Right))
  {
    DebugPrintln("Right sonar proximity ! (" + String(g_SonarDistance[SONAR_RIGHT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    {
      MowerStop();
      // CutMotorStop(false);

      if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on left side
      {
        DebugPrintln("Turning left", DBG_VERBOSE, true);
        MowerTurn(random(-60, -20), true); // turn left 30 degrees
      }
      else
      {
        DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
        MowerReserseAndTurn(random(-225, -135), MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn left 135 degrees....and hope for the best !
      }
    }

    // count as an obstable detection
    g_totalObstacleDectections = g_totalObstacleDectections + 1;
    g_successiveObstacleDectections = g_successiveObstacleDectections + 1;
    return OBSTACLE_DETECTED_RIGHT;
  }

  // reset successive detections counter (we only get to here if no obstacle has been detected)
  g_successiveObstacleDectections = 0;
  return OBSTACLE_DETECTED_NONE;
}
