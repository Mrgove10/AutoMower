#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerMoves/MowerMoves.h"
#include "MotionMotor/MotionMotor.h"
#include "MowerStates/MowerStates.h"
#include "CutMotor/CutMotor.h"
#include "Sonar/Sonar.h"
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

    // if no obstacles detected and all conditions ok, start motors
    if (!BumperRead(BUMPER_RIGHT) &&
        !BumperRead(BUMPER_LEFT) &&
        // need to add condition on Tilt sensors
        g_SonarDistance[SONAR_FRONT] > SONAR_MIN_DISTANCE_FOR_TURN &&
        g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN &&
        g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN)
    {
      MowerForward(MOWER_MOVES_SPEED_SLOW);
      //    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
    }
  }

  //--------------------------------
  // Sonar environement sensing for approaching objects
  //--------------------------------

  // SonarRead(SONAR_FRONT, true);
  // SonarRead(SONAR_LEFT, true);
  // SonarRead(SONAR_RIGHT, true);

  if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_SLOWING)
  {
    DebugPrintln("Approaching object : Slowing down ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    MowerForward(MOWER_MOVES_SPEED_SLOW);
  }
  else
  {
    DebugPrintln("No approaching object : normal speed", DBG_VERBOSE, true);
    MowerSpeed(MOWER_MOWING_TRAVEL_SPEED);
  }

  if (abs(g_PerimeterMagnitude) > 50)
  {
    DebugPrintln("Approaching perimeter : Slowing down ! (" + String(g_PerimeterMagnitude) + ")", DBG_DEBUG, true);
    MowerForward(MOWER_MOVES_SPEED_SLOW);
  }
  else
  {
    DebugPrintln("No perimeter approaching : normal speed", DBG_VERBOSE, true);
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
  // stop motors
  // find target with compas
  // go forward
  // detect perim
  // follow perim to base
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
    g_SonarReadEnabled = false;

    DebugPrintln("");
    LogPrintln("Mower stopped on Error #" + String(g_CurrentErrorCode) + "-" + ErrorString(g_CurrentErrorCode), TAG_ERROR, DBG_ERROR);
  }
  // disable sensors
  // send notification to phone
  // send telemetry
  // sound SOS beep
  // wait for user action
}
