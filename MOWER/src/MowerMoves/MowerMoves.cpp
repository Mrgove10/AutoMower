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
  
  // Wait before any movement is made - To limit mechanical stress
  delay(150);
}

/**
 * Mower forward move
 * @param Speed to travel
 * @param Soft boolean to trigger a soft change (default is false)
 */
void MowerForward(const int Speed, const bool Soft)
{
  DebugPrintln("Mower Forward at " + String(Speed) + "%", DBG_VERBOSE, true);
  if (Soft)
  {
    // Determine speed difference and number of steps
    int CurrentSpeed = (g_MotionMotorSpeed[MOTION_MOTOR_RIGHT] + g_MotionMotorSpeed[MOTION_MOTOR_LEFT]) / 2;    // In case 2 motors not at same speed, use average.
    int Steps = abs(Speed - CurrentSpeed);   // 1% per step
    // Determine increment for each step
    int stepIncrement;
    if (Speed > CurrentSpeed)
    {
      stepIncrement = 1;      // Speed increase
    }
    else if (Speed < CurrentSpeed)
    {
      stepIncrement = -1;     // Speed reduction
    }
    else
    {
      stepIncrement = 0;    // No speed change
    }

    // Apply current speed and direction
    MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_FORWARD, g_MotionMotorSpeed[MOTION_MOTOR_RIGHT]);
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_FORWARD, g_MotionMotorSpeed[MOTION_MOTOR_LEFT]);

    // Apply speed increment progressively
    if (stepIncrement != 0) 
    {
      for (int i = MOTION_MOTOR_MIN_SPEED - 1; i <= Steps; i = i + 1)
      {
        MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, CurrentSpeed + (i * stepIncrement));
        MotionMotorSetSpeed(MOTION_MOTOR_LEFT, CurrentSpeed + (i * stepIncrement));
        delayMicroseconds(500);
      }
    }
    // Set final speed (in case initial speed was different between motors)
    MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, Speed);
    MotionMotorSetSpeed(MOTION_MOTOR_LEFT, Speed);
  }  
  else
  {
    MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_FORWARD, Speed);
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_FORWARD, Speed);
  }
}

/**
 * Sets/changes Mower speed
 * @param Speed to travel
 */
void MowerSpeed(const int Speed)
{
  static int lastSpeed = 0;
  if (Speed != lastSpeed)
  {
    DebugPrintln("Mower speed at " + String(Speed) + "%", DBG_VERBOSE, true);
    lastSpeed = Speed;
  }
  MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, Speed);
  MotionMotorSetSpeed(MOTION_MOTOR_LEFT, Speed);
}

/**
 * Mower reverse move
 * @param Speed to reverse
 * @param Duration of reverse (in ms)
 * @param Soft boolean to trigger a soft change (default is false)
 */
void MowerReverse(const int Speed, const int Duration, const bool Soft)
{
  DebugPrintln("Mower Reverse at " + String(Speed) + "%", DBG_VERBOSE, true);
  if (Soft)
  {
    // Determine speed difference and number of steps
    int CurrentSpeed = 0;
    int Steps = abs(Speed - CurrentSpeed);   // 1% per step
    // Determine increment for each step
    int stepIncrement = 1;      // Speed increase

    // Apply current speed and direction
    MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_REVERSE, MOTION_MOTOR_MIN_SPEED - 1);
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_REVERSE, MOTION_MOTOR_MIN_SPEED - 1);

    // Apply speed increment progressively
    for (int i = MOTION_MOTOR_MIN_SPEED; i <= Steps; i = i + 1)
    {
      MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, CurrentSpeed + (i * stepIncrement));
      MotionMotorSetSpeed(MOTION_MOTOR_LEFT, CurrentSpeed + (i * stepIncrement));
      delayMicroseconds(500);
    }

    SerialAndTelnet.handle(); // Refresh Telnet Session

    delay(Duration);

    // Determine number of steps
    CurrentSpeed = Speed;
    Steps = CurrentSpeed;   // 1% per step
    // Determine increment for each step
    stepIncrement = -1;     // Speed reduction
    for (int i = 1; i <= Steps - MOTION_MOTOR_MIN_SPEED + 1; i = i + 1)
    {
      MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, CurrentSpeed + (i * stepIncrement));
      MotionMotorSetSpeed(MOTION_MOTOR_LEFT, CurrentSpeed + (i * stepIncrement));
      delayMicroseconds(500);
    }
    MowerStop();
  }  
  else
  {
    MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_REVERSE, Speed);
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_REVERSE, Speed);
    delay(Duration);
    MowerStop();
    // Wait before any movement is made - To limit mechanical stress
    delay(150);
  }
}

/**
 * Mower turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param OnSpot turn with action of both wheels
 * 
 */
void MowerTurn(const int Angle, const bool OnSpot)
{
  // Limit angle to [-360,+360] degrees
  int LimitedAngle = min(Angle, 360);
  LimitedAngle = max(LimitedAngle, -360);
  float turnDuration = float(abs(LimitedAngle) / (MOWER_MOVES_TURN_ANGLE_RATIO));
  DebugPrintln("Mower turn of " + String(Angle) + " Deg => " + String(turnDuration, 0) + " ms", DBG_VERBOSE, true);

  // Disable pitch and roll calculation
  g_MotionMotorTurnInProgress = true;

  if (LimitedAngle < 0) // Left turn
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
  else // Right turn
  {
    MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_FORWARD, MOWER_MOVES_TURN_SPEED);
    if (OnSpot)
    {
      MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_REVERSE, MOWER_MOVES_TURN_SPEED);
    }
    delay(turnDuration);
    MotionMotorStop(MOTION_MOTOR_LEFT);
    MotionMotorStop(MOTION_MOTOR_RIGHT);
  }
  
  // Enable pitch and roll calculation
  g_MotionMotorTurnInProgress = false;
}

/**
 * Mower reverse and turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param Duration of reverse (in ms)
 * @param OnSpot turn with action of both wheels, default is false
 * @param PitchComp compensate for pitch angle (increased reversing duration and angle), default is false
 * @param RollComp compensate for roll angle (increased angle), default is false
 * 
 */
void MowerReserseAndTurn(const int Angle, const int Duration, const bool OnSpot, const bool PitchComp, const bool RollComp)
{
  int correctedAngle = Angle;
  int correctedDuration = Duration;

  // Roll compensation on angle
  if (RollComp && abs(g_TCrollAngle) > ROLL_TURN_COMPENSATION_THRESHOLD)
  {
    int RollAngleCorrection = int(abs(g_TCrollAngle) * ROLL_TURN_COMPENSATION_FACTOR);
    DebugPrintln("Mower at an angle (Roll:" + String(g_TCrollAngle) + ") : turn angle corrected by " + String(RollAngleCorrection) + " degrees", DBG_DEBUG, true);
    if (Angle > 0)
    {
      correctedAngle = correctedAngle + RollAngleCorrection;
    }
    else
    {
      correctedAngle = correctedAngle - RollAngleCorrection;
    }
  }

  // Check if mower facing downwards, increase turning angle and duration to compensate for tilt angle
  if (PitchComp && g_TCpitchAngle < PITCH_TURN_COMPENSATION_THRESHOLD)
  {
    int PitchAngleCorrection = int(abs(g_TCpitchAngle) * PITCH_TURN_COMPENSATION_FACTOR);
    int PitchDurationCorrection = int(abs(g_TCpitchAngle) * PITCH_REVERSE_COMPENSATION_FACTOR);

    DebugPrintln("Mower facing downwards (Pitch:" + String(g_TCpitchAngle) + ") : turn angle and duration corrected by " + String(PitchAngleCorrection) + " degrees and " + String(PitchDurationCorrection) + " seconds", DBG_DEBUG, true);
    correctedDuration = correctedDuration + PitchDurationCorrection * 1000;
    if (Angle > 0)
    {
      correctedAngle = correctedAngle + PitchAngleCorrection;
    }
    else
    {
      correctedAngle = correctedAngle - PitchAngleCorrection;
    }
  }

  MowerReverse(MOWER_MOVES_REVERSE, correctedDuration, true);
  MowerTurn(correctedAngle, OnSpot);

  // Wait before any movement is made - To limit mechanical stress
  delay(150);
}

/**
 * Mower checks selected obstacle types and reduces speed if conditions are met
 * @param SpeedDelta as int: the speed reduction to be applied expressed as a positive value (in absolue %). If multiple conditions are selected, same speed reduction is applied.
 * @param Front as optional int: sonar measured front distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Left as optional int: sonar measured left distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Right as optional int: sonar measured right distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Perimeter as optional int: perimeter wire signal magnitude under which  mower needs to slow down. 0 disables the check. Absolute value is used to perform the check (applies to both inside and outside perimeter wire).  Default is 0.
 * @return boolean indicating if the function triggered a speed reduction
 */
bool MowerSlowDownApproachingObstables(const int SpeedDelta, const int Front, const int Left, const int Right, const int Perimeter)
{
  static unsigned long lastSpeedReduction = 0;
  bool SpeedReductiontiggered = false;
  static bool SpeedReductionInProgress = false;

  // To avoid a jerky mouvement, speed reduction is maintained at least for a set duration
  // if (millis() - lastSpeedReduction < OBSTACLE_APPROACH_LOW_SPEED_MIN_DURATION)
  // {
  //   return true;
  // }
  // else
  // {
  //   // SpeedReductionInProgress = false;
  // }

  // Check for objects in Front

  if (Front > 0 && g_SonarDistance[SONAR_FRONT] < Front)
  {
    DebugPrintln("Front approaching object: Slowing down ! (" + String(g_SonarDistance[SONAR_FRONT]) + "cm)", DBG_DEBUG, true);
    SpeedReductiontiggered = true;
  }

  // Check for objects on left side

  if (Left > 0 && g_SonarDistance[SONAR_LEFT] < Left)
  {
    DebugPrintln("Left approaching object: Slowing down ! (" + String(g_SonarDistance[SONAR_LEFT]) + "cm)", DBG_DEBUG, true);
    SpeedReductiontiggered = true;
  }

  // Check for objects on right side

  if (Right > 0 && g_SonarDistance[SONAR_RIGHT] < Right)
  {
    DebugPrintln("Right approaching object: Slowing down ! (" + String(g_SonarDistance[SONAR_RIGHT]) + "cm)", DBG_DEBUG, true);
    SpeedReductiontiggered = true;
  }

  // Check for Perimeter wire

  if (Perimeter > 0 && abs(g_PerimeterMagnitudeAvg) > Perimeter)
  {
    DebugPrintln("Approaching perimeter: Slowing down ! (" + String(g_PerimeterMagnitude) + ")", DBG_VERBOSE, true);
    SpeedReductiontiggered = true;
  }

  // If at least one of the conditions are met and if motor speed is higher that minimum threshold, reduce speed

  // Left Motor
  // if (SpeedReductiontiggered && g_MotionMotorSpeed[MOTION_MOTOR_LEFT] - SpeedDelta > MOTION_MOTOR_MIN_SPEED )
  if (SpeedReductiontiggered && !SpeedReductionInProgress)
  {
    DebugPrintln("Left motor speed reduced by " + String(SpeedDelta) + "%", DBG_VERBOSE, true);
    MotionMotorSetSpeed(MOTION_MOTOR_LEFT, - SpeedDelta, true);
  }

  // Right Motor
  // if (SpeedReductiontiggered && g_MotionMotorSpeed[MOTION_MOTOR_RIGHT] - SpeedDelta > MOTION_MOTOR_MIN_SPEED)
  if (SpeedReductiontiggered && !SpeedReductionInProgress)
  // if (SpeedReductiontiggered)
  {
    DebugPrintln("Right motor speed reduced by " + String(SpeedDelta) + "%", DBG_VERBOSE, true);
    MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, - SpeedDelta, true);
    SpeedReductionInProgress = true;
  }

  // keep track of when last speed reduction was triggered
  if (SpeedReductiontiggered)
  {
    lastSpeedReduction = millis();
  }
  else
  {
    SpeedReductionInProgress = false;
  }

  return SpeedReductiontiggered;
}

/**
 * Mower arc function : mower moves in given direction with motors running at a different speed, thus turning forming an arc : used for spiral mowing
 * @param direction forward (MOTION_MOTOR_FORWARD) or reverse (MOTION_MOTOR_REVERSE)
 * @param leftSpeed Left motor speed (in %)
 * @param rightSpeed Right motor speed (in %)
 */
void MowerArc(const int direction, const int leftSpeed, const int rightSpeed)
{
  if (direction == MOTION_MOTOR_FORWARD)
  {
    if (leftSpeed != rightSpeed)
    {
      DebugPrintln("Mower arc Forward (Left:" + String(leftSpeed) + "%, Right:" + String(rightSpeed) + "%)", DBG_VERBOSE, true);
    }
    else
    {
      DebugPrintln("Mower Forward @ " + String(leftSpeed) + "%", DBG_VERBOSE, true);
    }
  }
  else
  {
    if (leftSpeed != rightSpeed)
    {
      DebugPrintln("Mower arc Reverse (Left:" + String(leftSpeed) + "%, Right:" + String(rightSpeed) + "%)", DBG_VERBOSE, true);
    }
    else
    {
      DebugPrintln("Mower Reverse @ " + String(leftSpeed) + "%", DBG_VERBOSE, true);
    }
  }

  // Disable pitch and roll calculation is turning in progress
  g_MotionMotorTurnInProgress = (leftSpeed != rightSpeed);

  MotionMotorStart(MOTION_MOTOR_RIGHT, direction, rightSpeed);
  MotionMotorStart(MOTION_MOTOR_LEFT, direction, leftSpeed);
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
