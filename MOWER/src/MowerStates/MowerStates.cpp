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
#include "Utils/Utils.h"
#include "Display/Display.h"

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

    // if (PreviousState == MowerState::mowing)
    // {
    MowerStop();
    CutMotorStop(true);
    // }
  }

  // Waiting for input ?
  // Send telemetry
}

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

/**
 * Mower in mowing mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerMowing(const bool StateChange, const MowerState PreviousState)
{
  //--------------------------------
  // Actions to take when entering the state
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
    
    //--------------------------------
    // Activate Sonar reading
    //--------------------------------

    g_SonarReadEnabled = true;          // activate Sonar readings
    delay(SONAR_READ_ACTIVATION_DELAY); //wait for task to take 1st readings

//    SonarRead(SONAR_FRONT, true);
//    SonarRead(SONAR_LEFT, true);
//    SonarRead(SONAR_RIGHT, true);

    //--------------------------------
    // Check if mowing conditions are met
    //--------------------------------

    // Bumpers not activated
    if (BumperRead(BUMPER_RIGHT) && BumperRead(BUMPER_LEFT))
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_NO_START_BUMPER_ACTIVE;
      return;
    }
    // Obstacles detected
    if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_TURN ||
        g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_TURN ||
        g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_TURN
        )
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_NO_START_OBJECT_TOO_CLOSE;
      return;
    }
    // Tilt activated
    if (TiltRead(TILT_HORIZONTAL) || TiltRead(TILT_VERTICAL))
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_NO_START_TILT_ACTIVE;
      return;
    }
    // No perimeter signal
    if (false)          // TO DO
    {
      g_CurrentState = MowerState::error;
      g_CurrentErrorCode = ERROR_MOWING_NO_START_NO_PERIMETER_SIGNAL;
      return;
    }

    //--------------------------------
    // Start mowing (code below only executed if no error detected)
    //--------------------------------

    // Sound starting beep to notify environment
    if (PreviousState == MowerState::idle)
    {
      // TO DO
    }

    MowerForward(MOWER_MOVES_SPEED_SLOW);
    // CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);

    g_MowingLoopCnt = 0;

  }

  g_MowingLoopCnt = g_MowingLoopCnt + 1;

  //--------------------------------
  // Check tilt sensors and take immediate action
  //--------------------------------

  if (TiltReadAndAct())
  {
    return;
  }

  //--------------------------------
  // Sonar environement sensing for approaching objects
  //--------------------------------

  // SonarRead(SONAR_FRONT, true);
  // SonarRead(SONAR_LEFT, true);
  // SonarRead(SONAR_RIGHT, true);

  if ((g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_SLOWING)
      && g_MotionMotorSpeed[0] != MOWER_MOVES_SPEED_SLOW)
  {
    DebugPrintln("Approaching object : Slowing down ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    MowerForward(MOWER_MOVES_SPEED_SLOW);
  }
  else if (abs(g_PerimeterMagnitudeAvg) > PERIMETER_APPROACHING_THRESHOLD && g_MotionMotorSpeed[0] != MOWER_MOVES_SPEED_SLOW)
  {
    DebugPrintln("Approaching perimeter : Slowing down ! (" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);
    MowerForward(MOWER_MOVES_SPEED_SLOW);
  }
  else
  {
    DebugPrintln("No approaching object or perimeter : normal speed", DBG_VERBOSE, true);
    MowerSpeed(MOWER_MOWING_TRAVEL_SPEED);
  }

  //--------------------------------
  // Bumper Collision detection
  //--------------------------------

  if (BumperRead(BUMPER_RIGHT) || BumperRead(BUMPER_LEFT))
  {
    DebugPrintln("Bumper collision detected ! ", DBG_DEBUG, true);
    MowerStop();
    CutMotorStop(true);

    // SonarRead(SONAR_LEFT, true);
    if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on left side
    {
      DebugPrintln("Turning left", DBG_VERBOSE, true);
      MowerReserseAndTurn(-90, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn left 90 degrees
    }
    else
    {
      // SonarRead(SONAR_RIGHT, true);
      if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN) // check if it's clear on right side
      {
        DebugPrintln("Reversing and turning right", DBG_VERBOSE, true);
        MowerReserseAndTurn(90, MOWER_MOVES_REVERSE_FOR_TURN_DURATION, true); // reverse and turn right 90 degrees
      }
      else
      {
        DebugPrintln("Reversing and going back", DBG_VERBOSE, true);
        MowerReserseAndTurn(135, MOWER_MOVES_REVERSE_FOR_TURN_DURATION * 2, true); // reverse and turn right 135 degrees....and hope for the best !
      }
    }
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Perimeter wire dection
  //--------------------------------

  if (!g_isInsidePerimeter)
  {
    DebugPrintln("Outside Perimeter cable (mag:" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    // SonarRead(SONAR_LEFT, true);

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
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Front Sonar Collision detection
  //--------------------------------

  // SonarRead(SONAR_FRONT, true);

  if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {
    DebugPrintln("Font sonar proximity ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    // SonarRead(SONAR_LEFT, true);

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
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Left Sonar Collision detection
  //--------------------------------

  // SonarRead(SONAR_LEFT, true);

  if (g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {
    DebugPrintln("Left sonar proximity ! (" + String(g_SonarDistance[SONAR_LEFT]) + "cm)", DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    // SonarRead(SONAR_RIGHT, true);

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
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Right Sonar Collision dection
  //--------------------------------

  // SonarRead(SONAR_RIGHT, true);

  if (g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {
    DebugPrintln("Right sonar proximity ! (" + String(g_SonarDistance[SONAR_RIGHT]) + "cm)", DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    // SonarRead(SONAR_LEFT, true);

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
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  // TO DO
}

/**
 * Mower returning to base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerGoingToBase(const bool StateChange, const MowerState PreviousState)
{
  //--------------------------------
  // Actions to take when entering the state
  //--------------------------------

  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Mower going back to base", TAG_TO_BASE, DBG_INFO);

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;
    
    // Activate Sonar reading
    g_SonarReadEnabled = true;          // activate Sonar readings
    delay(SONAR_READ_ACTIVATION_DELAY); //wait for task to take 1st readings

    // Sound starting beep to notify environment
    if (PreviousState == MowerState::idle)
    {
      // TO DO
    }

    // just in case, stop cut motor
    CutMotorStop();
  }

  // find target with compas

  //--------------------------------
  // Find perimeter wire
  //--------------------------------

  if (!MowerFindWire(BACK_TO_BASE_HEADING, BACK_TO_BASE_CLOCKWISE))
  {
    return;
  }
  
  g_CurrentState = MowerState::idle;

  //--------------------------------
  // Follow perim to base
  //--------------------------------
  // TO DO
}

/**
 * Mower leaving base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerLeavingBase(const bool StateChange, const MowerState PreviousState)
{
  // go backward for 50 cm
  //  uTurn();
  //go forward
  g_CurrentState = MowerState::mowing;
}

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

/**
 * Mower finds perimeter wire according to heading given and, when found, orient right or left
 * @param heading integer indicating the heading to follows to try to find the wire
 * @param clockwise boolean indicating the direction in which the mower should face if it finds the wire
 * @return Success boolean depending on whether it found the wire or not
 */
bool MowerFindWire(const int heading, const bool clockwise)
{
// ************************
// TO DO - HEADING !!!!!
// ************************

  DebugPrintln("");
  LogPrintln("Mowing Started searching for wire", TAG_SEARCH, DBG_INFO);

  //--------------------------------------------------------------
  // Check if pre-requisistes are met:
  // no bumper, tilt or sonar restrictions (same as starting mowing)
  //--------------------------------------------------------------

  // Bumpers not activated
  if (BumperRead(BUMPER_RIGHT) && BumperRead(BUMPER_LEFT))
  {
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_NO_START_BUMPER_ACTIVE;
    return false;
  }
  // Obstacles detected
  if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_TURN ||
      g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_TURN ||
      g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_TURN
      )
  {
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_NO_START_OBJECT_TOO_CLOSE;
    return false;
  }
  // Tilt activated
  if (TiltRead(TILT_HORIZONTAL) || TiltRead(TILT_VERTICAL))
  {
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_NO_START_TILT_ACTIVE;
    return false;
  }
  // No perimeter signal
  if (false)          // TO DO
  {
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_NO_START_NO_PERIMETER_SIGNAL;
    return false;
  }

  //--------------------------------------------------------------
  // First, if outside perimeter, mower reverses to try and come back in the perimeter
  // If search "timeout" is reached, procedure (and mower) stops and raises an error
  //--------------------------------------------------------------

  unsigned long searchStartTime = millis();

  DebugPrintln("Phase 1 - Reversing to find wire", DBG_DEBUG, true);

  while (!g_isInsidePerimeter && 
          millis() - searchStartTime < PERIMETER_SEARCH_REVERSE_MAX_TIME &&
          g_CurrentState == MowerState::going_to_base)
  {
    // Reverse Mower
    MowerReverse(PERIMETER_SEARCH_REVERSE_SPEED, PERIMETER_SEARCH_REVERSE_TIME);
  }
  MowerStop();

  // Check if inside, if not trigger error
  if (!g_isInsidePerimeter) 
  {
    DebugPrintln("Phase 1 failled: not inside perimeter", DBG_DEBUG, true);

    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_1_FAILLED;
    return false;
  }

  //--------------------------------------------------------------
  // Second, once in the perimeter, it goes forward to find the wire (and stops if it finds obstacles) and goes over the wire (stops when outside)
  // If search "timeout" is reached, procedure (and mower) stops and raises an error
  //--------------------------------------------------------------

  DebugPrintln("Phase 2 - Forward to find wire", DBG_DEBUG, true);
  searchStartTime = millis();
  while (g_isInsidePerimeter && 
         millis() - searchStartTime < PERIMETER_SEARCH_FORWARD_MAX_TIME_1 &&
         g_CurrentState == MowerState::going_to_base)
  {
    // Go Forward
// TO DO => use a common procedure

    if (abs(g_PerimeterMagnitudeAvg) > PERIMETER_APPROACHING_THRESHOLD && g_MotionMotorSpeed[0] != MOWER_MOVES_SPEED_SLOW)
    {
      DebugPrintln("Approaching perimeter : Slowing down ! (" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);
      MowerForward(MOWER_MOVES_SPEED_SLOW);
    }
    else
    {
      DebugPrintln("No perimeter approaching : normal speed", DBG_VERBOSE, true);
      MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
    }
    delay(PERIMETER_SEARCH_FORWARD_TIME);

    // TO DO obstacle detection / avoidance !
  }
  MowerStop();

  // Check if outside, if not trigger error
  if (g_isInsidePerimeter) 
  {
    DebugPrintln("Phase 2 failled: not outside perimeter", DBG_DEBUG, true);
    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_2_FAILLED;
    return false;
  }
  //--------------------------------------------------------------
  // Third, it turns in direction requested (clockwise or counter-clockwise)
  // By slowing down before crossing the wire, the mower should have not gone very far over
  // and doing an "on the spot" turn, we can hope to be just inside the perimeter
  //--------------------------------------------------------------

  int turnIncrement = 0;

  if(clockwise)
  {
    DebugPrintln("Phase 3 - Turn clockwise", DBG_DEBUG, true);
    turnIncrement = PERIMETER_SEARCH_ANGLE_INCREMENT;
    // MowerTurn(PERIMETER_SEARCH_CLOCKWISE_TURN_ANGLE,true);
  }
  else
  {
    DebugPrintln("Phase 3 - Turn counter clockwise", DBG_DEBUG, true);
    turnIncrement = - PERIMETER_SEARCH_ANGLE_INCREMENT;
    // MowerTurn(PERIMETER_SEARCH_COUNTER_CLOCKWISE_TURN_ANGLE,true);
  }

  // searchStartTime = millis();
  int turnCount = 0;
  while (!g_isInsidePerimeter && 
          // millis() - searchStartTime < PERIMETER_SEARCH_TURN_MAX_TIME &&
          turnCount < PERIMETER_SEARCH_TURN_MAX_ITERATIONS &&
          g_CurrentState == MowerState::going_to_base)
  {
    // Turn on the spot by steps
    MowerTurn(turnIncrement, true);
    delay(PERIMETER_TIMER_PERIOD / 1000);  // wait to make sure that Perimeter signal read task can refresh g_isInsidePerimeter status
    turnCount = turnCount + 1;
  }
//  MowerStop();


//TO DO decide if not reaching inside is a faillure or not

  // Check if inside, if not trigger error
  if (!g_isInsidePerimeter) 
  {
    DebugPrintln("Phase 3 failled: not inside perimeter", DBG_DEBUG, true);

    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_3_FAILLED;
    return false;
  }

  //--------------------------------------------------------------
  // Fourth, is to drive forward to come back inside the wire
  // If not inside within a "timeout" is reached, procedure (and mower) stops and raises an error
  //--------------------------------------------------------------

  DebugPrintln("Phase 4 - Forward to find wire", DBG_DEBUG, true);
  searchStartTime = millis();
  while (!g_isInsidePerimeter && 
          millis() - searchStartTime < PERIMETER_SEARCH_FORWARD_MAX_TIME_2 &&
          g_CurrentState == MowerState::going_to_base)
  {
    // Go Forward
    MowerForward(PERIMETER_SEARCH_FORWARD_SPEED);
    delay(PERIMETER_SEARCH_FORWARD_TIME);
  }
  MowerStop();

  // Check if inside, if not trigger error
  if (!g_isInsidePerimeter) 
  {
    DebugPrintln("Phase 4 failled: not inside perimeter", DBG_DEBUG, true);

    g_CurrentState = MowerState::error;
    g_CurrentErrorCode = ERROR_WIRE_SEARCH_PHASE_4_FAILLED;
    return false;
  }

  //--------------------------------------------------------------
  // Finally, hopefully, mower should be "on" the wire (or close to it inside the perimeter), ready to follow the wire in correct direction !! Procedure stops and returns success
  //--------------------------------------------------------------

  return true;
}
