#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MowerMoves/MowerMoves.h"
#include "MotionMotor/MotionMotor.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Mower mouvement stop function
 */
void MowerStop()
{
  DebugPrintln("Mower Stop", DBG_VERBOSE, true);
  MotionMotorStop(MOTION_MOTOR_RIGHT);
  MotionMotorStop(MOTION_MOTOR_LEFT);
}

/**
 * Mower forward move
 * @param Speed to travel
 */
void MowerForward(const int Speed)
{
  DebugPrintln("Mower Forward at " + String(Speed) + "%", DBG_VERBOSE, true);
  MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_FORWARD, Speed);
  MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_FORWARD, Speed);
}

/**
 * Sets/changes Mower speed
 * @param Speed to travel
 */
void MowerSpeed(const int Speed)
{
  DebugPrintln("Mower speed at " + String(Speed) + "%", DBG_VERBOSE, true);
  MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, Speed);
  MotionMotorSetSpeed(MOTION_MOTOR_LEFT, Speed);
}


/**
 * Mower reverse move
 * @param Speed to reverse
 * @param Duration of reverse (in ms)
 */
void MowerReverse(const int Speed, const int Duration)
{
  DebugPrintln("Mower Reverse at " + String(Speed) + "%", DBG_VERBOSE, true);
  MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_REVERSE, Speed);
  MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_REVERSE, Speed);
  delay(Duration);
  MowerStop();
}

/**
 * Mower turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param OnSpot turn with action of both wheels
 * 
 */
void MowerTurn(const int Angle, const bool OnSpot)
{
  int LimitedAngle = min(Angle,360);
  LimitedAngle = max(LimitedAngle,-360);
  float turnDuration = float(abs(LimitedAngle) / (MOWER_MOVES_TURN_ANGLE_RATIO));
  DebugPrintln("Mower turn of " + String(Angle) + " Deg => " + String(turnDuration,0) + " ms", DBG_VERBOSE, true);

  if (LimitedAngle < 0)         // Left turn
  {
    MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_FORWARD, MOWER_MOVES_TURN_SPEED);
    if (OnSpot)
    {
      MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_REVERSE, MOWER_MOVES_TURN_SPEED);
    }
    delay(turnDuration);
    MotionMotorStop(MOTION_MOTOR_RIGHT);
    MotionMotorStop(MOTION_MOTOR_LEFT);
  }
  else
  {
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_FORWARD, MOWER_MOVES_TURN_SPEED);
    if (OnSpot)
    {
      MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_REVERSE, MOWER_MOVES_TURN_SPEED);
    }
    delay(turnDuration);
    MotionMotorStop(MOTION_MOTOR_RIGHT);
    MotionMotorStop(MOTION_MOTOR_LEFT);
  }
}

/**
 * Mower reverse and turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param Duration of reverse (in ms)
 * @param OnSpot turn with action of both wheels
 * 
 */
void MowerReserseAndTurn(const int Angle, const int Duration, const bool OnSpot)
{
  MowerReverse(MOWER_MOVES_SPEED_SLOW, MOWER_MOVES_REVERSE_FOR_TURN_DURATION);
  MowerTurn(Angle,OnSpot);
}

/*
void getMeUnstuck()
{
  // stop motor
  // go back 10 cm
  // turn right or left (by certain angle)
  turn(15, true);
  // go forward
}
*/
