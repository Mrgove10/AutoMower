#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerMoves/MowerMoves.h"
#include "MotionMotor/MotionMotor.h"
#include "MowerStates/MowerStates.h"
#include "CutMotor/CutMotor.h"
#include "Sonar/Sonar.h"
#include "Tilt/Tilt.h"
#include "Bumper/Bumper.h"
#include "Rain/Rain.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

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
    LogPrintln("Mower Idle", TAG_MOWING, DBG_INFO);

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;

#ifdef MQTT_PID_GRAPH_DEBUG
    g_MQTTPIDGraphDebug = false;  // for testing
#endif

    // if (PreviousState == MowerState::mowing)
    // {
    MowerStop();
    CutMotorStop(true);
    // }
  }

  // Waiting for input ?
  // Send telemetry
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
  // wait for GO
  // send telemetry
  // if battery is egnoth
  // GO
  // else
  // send error bat low
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
  //--------------------------------
  // Actions to take when entering the mowing state
  //--------------------------------

  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Mowing Started", TAG_MOWING, DBG_INFO);

    //change Telemetry frequency
    //Initialise Mowing start time
    //
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

    MowerForward(MOWER_MOVES_SPEED_SLOW);
    // CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);

    g_MowingLoopCnt = 0;

  }
  g_MowingLoopCnt = g_MowingLoopCnt + 1;

  // Ongoing Mowing routine is as follows:
  // Check for tilt,
  // Check enviroment to slow down for approaching obstacles,
  // Check for obstacle detection and react (trigger error if too many),
  // Stop when conditions met (TO DO)

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
    DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
    return;
  }

  //--------------------------------
  // Is Battery level above threshold ? 
  //--------------------------------

  if (g_BatteryVotlage < BATTERY_VOLTAGE_RETURN_TO_BASE_THRESHOLD)
  {
    DebugPrintln("Battery low: returning to base (" + String(g_BatteryVotlage) + " mv)", DBG_INFO, true);
    MowerStop();
    CutMotorStop();
    g_CurrentState = MowerState::going_to_base;
    return;
  }

  //--------------------------------
  // Is it raining ? 
  //--------------------------------

  if (isRaining())
  {
    DebugPrintln("Raining : returning to base", DBG_INFO, true);
    MowerStop();
    CutMotorStop();
    g_CurrentState = MowerState::going_to_base;
    return;
  }

  //--------------------------------
  // Environment sensing for approaching objects
  //--------------------------------
  if (!MowerSlowDownApproachingObstables(MOWER_MOWING_TRAVEL_SPEED - MOWER_MOVES_SPEED_SLOW,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         PERIMETER_APPROACHING_THRESHOLD))
  {
    MowerSpeed(MOWER_MOWING_TRAVEL_SPEED);
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
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_CONSECUTIVE_OBSTACLES;
      return;
    }
    else
    {
      // Start mowing again
      MowerForward(MOWER_MOWING_TRAVEL_SPEED);
      CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
    }
  }

  // TO DO When to stop mowing and go back to base
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

  // TO DO conditions to end wire tracking

//  g_CurrentState = MowerState::idle;
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
    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);
  }

  // go backward for 50 cm
  //  uTurn();
  //go forward
  // g_CurrentState = MowerState::mowing;
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
                        ERROR_NO_ERROR,                              // no error here as 1st phase is a reverse motion
                        ERROR_NO_ERROR,                              // no error here as 1st phase is a reverse motion
                        ERROR_NO_ERROR,                              // no error here as 1st phase is a reverse motion
                        ERROR_NO_ERROR,                              // no error here as 1st phase is a reverse motion
                        ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL,
                        true))
    {
      return false;
    }

    // Initialise Phase
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
        return false;        // continue search
      }

      // Back inside => the phase ends
      if (g_isInsidePerimeter)
      {
        MowerStop();
        *phase = PERIMETER_SEARCH_PHASE_2;
        DebugPrintln("Phase 2 - Forward to find wire", DBG_DEBUG, true);
        searchStartTime = millis();
        MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
        return false;        // continue search
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
        DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
        g_CurrentState = MowerState::error;
        g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
        return false;
      }

      // Still inside and moving towards perimeter 
      if (g_isInsidePerimeter && millis() - searchStartTime < PERIMETER_SEARCH_FORWARD_MAX_TIME_1)
      {
        // Environment sensing for approaching objects
        if (!MowerSlowDownApproachingObstables(PERIMETER_SEARCH_FORWARD_SPEED - MOWER_MOVES_SPEED_SLOW,
                                                SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                                SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                                SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                                PERIMETER_APPROACHING_THRESHOLD))
        {
          MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
          return false;        // continue search
        }

        // Obstacle Collision detection
        if (OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          false,                        //  no Perimeter detection (and action)
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
        return false;        // continue search
      }

      // Reached outside (wire found): => the phase ends
      if (!g_isInsidePerimeter)
      {
        MowerStop();
        *phase = PERIMETER_SEARCH_PHASE_3;
        if(clockwise)
        {
          DebugPrintln("Phase 3 - Turn clockwise", DBG_DEBUG, true);
          turnIncrement = PERIMETER_SEARCH_ANGLE_INCREMENT;
        }
        else
        {
          DebugPrintln("Phase 3 - Turn counter clockwise", DBG_DEBUG, true);
          turnIncrement = - PERIMETER_SEARCH_ANGLE_INCREMENT;
        }
        turnCount = 0;
        return false;        // continue search
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
        MowerTurn(turnIncrement, true);
        delay(PERIMETER_TIMER_PERIOD * 2 / 1000);  // wait to make sure that Perimeter signal read task can refresh g_isInsidePerimeter status
        turnCount = turnCount + 1;

        return false;        // continue search
      }

      // Back inside and signal is strong enough => the phase ends
      if (g_isInsidePerimeter && !g_PerimeterSignalLowForTracking)
      {
        MowerStop();
        *phase = PERIMETER_SEARCH_FINISHED;
        return true;        // end search
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
  
  if (*reset)
  {
    DebugPrintln("Starting to follow wire....", DBG_DEBUG, true);

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

    DebugPrintln("Kp:" + String(g_ParamPerimeterTrackPIDKp) + " Ki:" + String(g_ParamPerimeterTrackPIDKi) + " Kd:" + String(g_ParamPerimeterTrackPIDKd), DBG_DEBUG, true);
    DebugPrintln("Smag:" + String(g_PerimeterSmoothMagnitude), DBG_DEBUG, true);
    DebugPrintln("");
    *reset = false;

    // Initialize the variables the PID is linked to
    g_PIDSetpoint = g_PerimeterTrackSetpoint;
    g_PIDInput = g_PerimeterMagnitudeAvg;

    // Turn the PID on
    g_PerimeterTrackPID.SetMode(AUTOMATIC);
    g_PerimeterTrackPID.SetTunings(g_ParamPerimeterTrackPIDKp, g_ParamPerimeterTrackPIDKi , g_ParamPerimeterTrackPIDKd, P_ON_E);
    g_PerimeterTrackPID.SetOutputLimits(-70, 70);   // in %
    g_PerimeterTrackPID.SetControllerDirection(DIRECT);
    g_PerimeterTrackPID.SetSampleTime(PERIMETER_TRACKING_PID_INTERVAL);

#ifdef MQTT_PID_GRAPH_DEBUG
    g_MQTTPIDGraphDebug = true;  // for testing
#endif

    // Cancel any outstanding wheel speed corrections
    MotionMotorsTrackingAdjustSpeed(0, 0);

    // Move forward
    MowerForward(BACK_TO_BASE_SPEED);
  }

  //--------------------------------
  // Check tilt sensors and take immediate action
  //--------------------------------
  if (CheckTiltReadAndAct())
  {
    return false;
  }

  //--------------------------------
  // Environment sensing for approaching objects
  //--------------------------------
  bool SlowedDown = false;
  if (clockwise)
  {
    SlowedDown = MowerSlowDownApproachingObstables(BACK_TO_BASE_SPEED - MOWER_MOVES_SPEED_NORMAL,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         0,                                                     // no left detection
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         0);                                                    // no perimeter detection
  }
  else
  {
    SlowedDown = MowerSlowDownApproachingObstables(BACK_TO_BASE_SPEED - MOWER_MOVES_SPEED_NORMAL,
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         SONAR_MIN_DISTANCE_FOR_SLOWING, 
                                         0,                                                     // no right detection
                                         0);                                                    // no perimeter detection
  } 

  if (!SlowedDown)                // No approaching objects : resume normal speed
  {
    MowerSpeed(BACK_TO_BASE_SPEED); 
  }

  //--------------------------------
  // Obstacle Collision detection
  //--------------------------------

  int colisionDetected = OBSTACLE_DETECTED_NONE;

  if (clockwise)
  {
    colisionDetected = OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                      SONAR_MIN_DISTANCE_FOR_STOP,
                                                      0,                            // no left detection
                                                      SONAR_MIN_DISTANCE_FOR_STOP,
                                                      false,                       // no perimeter detection
                                                      false);                       // no action (for the moment ???????????????)
  }
  else
  {
    colisionDetected = OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                      SONAR_MIN_DISTANCE_FOR_STOP,
                                                      SONAR_MIN_DISTANCE_FOR_STOP,
                                                      0,                            // no right detection
                                                      false,                       // no perimeter detection
                                                      false);                       // no action (for the moment ???????????????)
  }

  // For the moment, to keep things simple, we will conider that their should be no object on the wire path (time will tell is this is a reasonable assumption).
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
  // Is Perimeter signal strong enough ? 
  //--------------------------------

  if (g_PerimeterSignalLowForTracking)
  {
    DebugPrintln("Perimeter signal too low (" + String(g_PerimeterSmoothMagnitudeTracking) + ")", DBG_ERROR, true);
// for the moment, do nothing
// TO DO .... 
// stop mower
// either trigger a wire search or declare an error
    // return false;
  }

  //--------------------------------
  // Is Perimeter signal Lost or stopped? 
  //--------------------------------

  if (g_PerimeterSignalLost || g_PerimeterSignalStopped)
  {
    DebugPrintln("Perimeter signal Lost (" + String(g_PerimeterSmoothMagnitude) + ") or stopped", DBG_ERROR, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_NO_PERIMETER_SIGNAL;
    return false;
  }

  //--------------------------------
  // Use PID to apply direction orders to motors
  //--------------------------------

  if (millis() - lastPIDUpdate > PERIMETER_TRACKING_PID_INTERVAL)
  {
    // update PID with new input
    // g_PIDInput = g_PerimeterMagnitudeAvgPID;
    g_PIDInput = g_PerimeterMagnitudeAvg;

    // Update PID settings "on the fly" too enable easier tuning
    g_PIDSetpoint = g_PerimeterTrackSetpoint;
    g_PerimeterTrackPID.SetTunings(g_ParamPerimeterTrackPIDKp, g_ParamPerimeterTrackPIDKi , g_ParamPerimeterTrackPIDKd, P_ON_E);

    DebugPrintln("PID P " + String(g_PerimeterTrackPID.GetKp()) + " " + String(g_ParamPerimeterTrackPIDKp), DBG_VERBOSE, true);

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

    if (g_PIDOutput != 0)
    {
      if (g_PIDOutput > 0)     // Mag is negative and PID output positive, so we are inside and need to move towards the outside
      {
        if (clockwise)          // we need to go towards the left
        {
          MotionMotorsTrackingAdjustSpeed(- g_PIDOutput, 0);
        }
        else                    // we need to go towards the right
        {
          MotionMotorsTrackingAdjustSpeed(0,- g_PIDOutput);
        }
      }
      else      // Mag is positive and PID output negative, so we are outside and need to move towards the inside
      {
        if (clockwise)          // we need to go towards the right
        {
          MotionMotorsTrackingAdjustSpeed(0, g_PIDOutput);
        }
        else                    // we need to go towards the left
        {
          MotionMotorsTrackingAdjustSpeed(g_PIDOutput, 0);
        }
      }
    }
    else
    {
      MotionMotorsTrackingAdjustSpeed(0, 0);
    }

#ifdef MQTT_PID_GRAPH_DEBUG
//  send debug information through MQTT
    if(g_MQTTPIDGraphDebug)
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
  
// TO DO
// when to stop ????? When charging has started ??
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
 * @brief Function to check pre-conditions before performing tasks. Pre-condition checks can be performed on Tilt sensors (both), bumpers (both), Sonar sensors (Front, left or right) and perimeter wire active
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
 * @param ErrorMode (optional) , if true, changes mower mode to Error (default is false).
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

  if (Perimeter != ERROR_NO_ERROR && (false))       // TO DO
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
 * Obstacle detection can be any combination of bumper (both), Sonars (Front, Left or right) and perimeter wire, but function returns at first detection.
 * Detections are performed in the following order: 1-Bumper, 2- Perimeter, 3-Front sonar, 4-Left sonar, 5-Right sonar.
 * If the action enable parameter is not set, the function only returns if a detection occured or not and action to take if left to caller to decide.
 * At the end of the funtion, if the action enable is set, the mower is stopped and the cutting motor is stopped. Caller to restart the motors as required.
 * 
 * @param Bumper as optional boolean : bumper active triggers detection/action. 0 disbales the check. Default is 0
 * @param Front as optional int: sonar measured front distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Left as optional int: sonar measured left distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Right as optional int: sonar measured right distance to trigger detection/action. 0 disbales the check. Default is 0
 * @param Perimeter as optional boolean: outside perimeter wire to trigger detection/action. 0 disables the check. Absolute value is used to perform the check (applies to both inside and outside perimeter wire).  Default is 0.
 * @param ActionMode (optional) , if true, changes action is performed if condition is detected (default is false).
 * @return integer indicating which detection was detected.
 */
int CheckObstacleAndAct(const bool Bumper, const int Front, const int Left, const int Right, const bool Perimeter, const bool ActionMode)
{
  //--------------------------------
  // Bumper Collision detection
  //--------------------------------
  // Reaction to bumper collision is as follows:
  // Stop motion and cutting, 
  // if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  // if not possible, if possible to turn right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  // if not possible, reverse further and turn to  be ready to go in "opposite direction"

  if (!Bumper && (BumperRead(BUMPER_RIGHT) || BumperRead(BUMPER_LEFT)))
  {
    DebugPrintln("Bumper collision detected ! ", DBG_DEBUG, true);
    if (ActionMode)
    { 
      // Stop motion and stop cutting motor
      MowerStop();
      CutMotorStop(true);

      if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on left side
      {
        DebugPrintln("Turning left", DBG_VERBOSE, true);
        MowerReserseAndTurn(random(-90, -45), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn left 90 degrees
      }
      else
      {
        if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on right side
        {
          DebugPrintln("Reversing and turning right", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(45, 90), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn right 90 degrees
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
  // Perimeter wire dection
  //--------------------------------
  // Reaction to being outside perimeter wire is as follows:
  // Stop motion (no need to stop cutting as this a normal event), 
  // if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  // if not possible, if possible to turn Right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  // if not possible, reverse further and turn to be ready to go in "opposite direction"

  if (Perimeter && !g_isInsidePerimeter)
  {
    DebugPrintln("Outside Perimeter cable (mag:" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);

    if (ActionMode)
    { 
      MowerStop();

      if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on left side
      {
        DebugPrintln("Reversing and turning left", DBG_VERBOSE, true);
        MowerReserseAndTurn(random(-90, -45), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn left 90 degrees
      }
      else
      {
        // SonarRead(SONAR_RIGHT, true);

        if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on right side
        {
          DebugPrintln("Reversing and turning right", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(45, 90), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn right 90 degrees
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
  // Stop motion and cutting, 
  // if possible to turn left, reverse and turn left by a "small" angle (~ less than 90 degrees)
  // if not possible, if possible to turn right, reverse and turn right by a "small" angle (~ less than 90 degrees)
  // if not possible, reverse further and turn to  be ready to go in "opposite direction"

  if (Front != 0 && (g_SonarDistance[SONAR_FRONT] < Front))
  {
    DebugPrintln("Font sonar proximity ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    { 
      // Stop motion and stop cutting motor
      MowerStop();
      CutMotorStop(false);

      if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on left side
      {
        DebugPrintln("Reversing and turning left", DBG_VERBOSE, true);
        MowerReserseAndTurn(random(-90, -45), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn left 90 degrees
      }
      else
      {
        if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on right side
        {
          DebugPrintln("Reversing and turning right", DBG_VERBOSE, true);
          MowerReserseAndTurn(random(45, 90), MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn right 90 degrees
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
  // Stop motion and cutting, 
  // if possible to turn right, turn mower to the right by a "small" angle (~ less than 60 degrees)
  // if not possible, reverse and turn to be ready to go in "opposite direction"

  if (Left != 0 && (g_SonarDistance[SONAR_LEFT] < Left))
  {
    DebugPrintln("Left sonar proximity ! (" + String(g_SonarDistance[SONAR_LEFT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    { 
      MowerStop();
      CutMotorStop(false);

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
  // Right Sonar Collision dection
  //--------------------------------
  // Reaction to Right sonar detection is as follows:
  // Stop motion and cutting, 
  // if possible to turn Left, turn mower to the left by a "small" angle (~ less than 60 degrees)
  // if not possible, reverse and turn to be ready to go in "opposite direction"

  if (Right != 0 && (g_SonarDistance[SONAR_RIGHT] < Right))
  {
    DebugPrintln("Right sonar proximity ! (" + String(g_SonarDistance[SONAR_RIGHT]) + "cm)", DBG_DEBUG, true);
    if (ActionMode)
    { 
      MowerStop();
      CutMotorStop(false);

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

  // reset successive detections counter
  g_successiveObstacleDectections = 0;
  return OBSTACLE_DETECTED_NONE;
}
