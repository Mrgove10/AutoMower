#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerMoves/MowerMoves.h"
#include "MotionMotor/MotionMotor.h"
#include "MowerStates/MowerStates.h"
#include "CutMotor/CutMotor.h"
#include "Sonar/Sonar.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Mower in idle state
 */
void MowerIdle(const bool StateChange)
{
  // Waiting for input ?
  // Send telemetry
}


/**
 * Mower docked
 */
void MowerDocked(const bool StateChange)
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
 */
void MowerMowing(const bool StateChange)
{
  SonarRead(SONAR_FRONT, true);
  SonarRead(SONAR_LEFT, true);
  SonarRead(SONAR_RIGHT, true);

  if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_SLOWING ||
      g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_SLOWING)
  {
    DebugPrintln("Approaching object : Slowing down ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    MowerForward(MOWER_MOVES_SPEED_SLOW);
  }
  else
  {
    DebugPrintln("No approaching object : normal speed", DBG_DEBUG, true);
    MowerSpeed(MOWER_MOWING_TRAVEL_SPEED);
  }

  //--------------------------------
  // Bumper Collision dection
  //--------------------------------

  if (g_RightBumperTriggered || g_LeftBumperTriggered)
  {

    DebugPrintln("Bumper collision detected ! ", DBG_DEBUG, true);
    MowerStop();
    CutMotorStop(true);

    SonarRead(SONAR_LEFT, true);
    if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN)        // check if it's clear on left side
    {
      DebugPrintln("Turning left", DBG_DEBUG, true);
      MowerReserseAndTurn(-90,MOWER_MOVES_REVERSE_FOR_TURN_DURATION,true);    // reverse and turn left 90 degrees
    }
    else
    {
      SonarRead(SONAR_RIGHT, true);
      if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN)          // check if it's clear on right side
      {
        DebugPrintln("Reversing and turning right", DBG_DEBUG, true);
        MowerReserseAndTurn(90,MOWER_MOVES_REVERSE_FOR_TURN_DURATION,true);    // reverse and turn right 90 degrees
      }
      else
      {
        DebugPrintln("Reversing and going back", DBG_DEBUG, true);
        MowerReserseAndTurn(135,MOWER_MOVES_REVERSE_FOR_TURN_DURATION*2,true);    // reverse and turn right 135 degrees....and hope for the best !
      }
    }
    
    g_RightBumperTriggered = false;
    g_LeftBumperTriggered = false;

    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }  

  //--------------------------------
  // Front Sonar Collision dection
  //--------------------------------

  SonarRead(SONAR_FRONT, true);

  if (g_SonarDistance[SONAR_FRONT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {

    DebugPrintln("Font sonar proximity ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    SonarRead(SONAR_LEFT, true);

    if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN)        // check if it's clear on left side
    {
      DebugPrintln("Reversing and turning left", DBG_DEBUG, true);

      MowerReserseAndTurn(-90,MOWER_MOVES_REVERSE_FOR_TURN_DURATION,true);    // reverse and turn left 90 degrees
    }
    else
    {
      SonarRead(SONAR_RIGHT, true);

      if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN)          // check if it's clear on right side
      {
        DebugPrintln("Reversing and turning right", DBG_DEBUG, true);
        MowerReserseAndTurn(90,MOWER_MOVES_REVERSE_FOR_TURN_DURATION,true);    // reverse and turn right 90 degrees
      }
      else
      {
        DebugPrintln("Reversing and going back", DBG_DEBUG, true);
        MowerReserseAndTurn(135,MOWER_MOVES_REVERSE_FOR_TURN_DURATION*2,true);    // reverse and turn right 135 degrees....and hope for the best !
      }
    }
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Left Sonar Collision dection
  //--------------------------------

  SonarRead(SONAR_LEFT, true);

  if (g_SonarDistance[SONAR_LEFT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {
    DebugPrintln("Left sonar proximity ! (" + String(g_SonarDistance[SONAR_LEFT]) + "cm)" , DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    SonarRead(SONAR_RIGHT, true);

    if (g_SonarDistance[SONAR_RIGHT] > SONAR_MIN_DISTANCE_FOR_TURN)        // check if it's clear on right side
    {
      DebugPrintln("Turning right", DBG_DEBUG, true);
      MowerTurn(45,true);    // turn right 30 degrees
    }
    else
    {
      DebugPrintln("Reversing and going back", DBG_DEBUG, true);
      MowerReserseAndTurn(135,MOWER_MOVES_REVERSE_FOR_TURN_DURATION*2,true);    // reverse and turn right 135 degrees....and hope for the best !
    }
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Right Sonar Collision dection
  //--------------------------------

  SonarRead(SONAR_RIGHT, true);

  if (g_SonarDistance[SONAR_RIGHT] < SONAR_MIN_DISTANCE_FOR_STOP)
  {
    DebugPrintln("Right sonar proximity ! (" + String(g_SonarDistance[SONAR_RIGHT]) + "cm)" , DBG_DEBUG, true);

    MowerStop();
    CutMotorStop();

    SonarRead(SONAR_LEFT, true);

    if (g_SonarDistance[SONAR_LEFT] > SONAR_MIN_DISTANCE_FOR_TURN)        // check if it's clear on left side
    {
      DebugPrintln("Turning left", DBG_DEBUG, true);
      MowerTurn(-45,true);    // turn left 30 degrees
    }
    else
    {
     DebugPrintln("Reversing and going back", DBG_DEBUG, true);
     MowerReserseAndTurn(-135,MOWER_MOVES_REVERSE_FOR_TURN_DURATION*2,true);    // reverse and turn left 135 degrees....and hope for the best !
    }
    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
  }

  //--------------------------------
  // Perimeter wire dection
  //--------------------------------

    // TO DO

  //--------------------------------
  // Actions to take when entering the state
  //--------------------------------

  if (StateChange) 
  {
    DebugPrintln("");
    LogPrintln("Mowing Started", TAG_MOWING, DBG_INFO);

    MowerForward(MOWER_MOWING_TRAVEL_SPEED);
    CutMotorStart(MOWER_MOWING_CUTTING_DIRECTION, MOWER_MOWING_CUTTING_SPEED);
    //change Telemetry frequency
    //Initialise Mowing start time
    //
  }

}

/**
 * Mower returning to base
 * 
 */
void MowerGoingToBase(const bool StateChange)
{
  // stop motors
  // find target with compas
  // go forward
  // detect perim
  // follow perim to base
}

/**
 * Mower leaving base
 * 
 */
void MowerLeavingBase(const bool StateChange)
{
  // go backward for 50 cm
//  uTurn();
  //go forward
  g_CurrentState = MowerState::mowing;
}

/**
 * Mower in error
 * 
 */
void MowerInError(const bool StateChange)
{
  // STOP all motors
  // disable sensors
  // send notification to phone
  // send telemetry
  // wait for user action
}
