#ifndef moves_h
#define moves_h

/**
 * Mower mouvement stop function
 */
void MowerStop();

/**
 * Mower forward move
 * @param Speed to travel
 */
void MowerForward(const int Speed);

/**
 * Sets/changes Mower speed
 * @param Speed to travel
 */
void MowerSpeed(const int Speed);

/**
 * Mower reverse move
 * @param Speed to reverse
 * @param Duration of reverse (in ms)
 */
void MowerReverse(const int Speed, const int Duration);

/**
 * Mower turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param OnSpot turn with action of both wheels
 * 
 */
void MowerTurn(const int Angle, const bool OnSpot = false);

/**
 * @brief Mower reverse and turn function.
 * Based on the PitchComp and RollComp parameters, this function will adjust the angle and the reverse duration to compensate for
 * the reduced efficiency of the reversing and turning actions, due to reduced speed (motor power and efficientcy) and slipping/spinning
 * of tires when the terrain is at an angle.
 * 
 * If the roll angle is higher than the ROLL_TURN_COMPENSATION_THRESHOLD fixed value, the turning angle is increased by multiplying the
 * angle (provided as a parameter by the calling function) by the ROLL_TURN_COMPENSATION_FACTOR fixed value.
 * Note: this function does not check the relevance of the turning direction (positive or negative value) with the roll angle
 * 
 * If the negative pitch angle (mower facing downwards) is lower than the PITCH_REVERSE_COMPENSATION_THRESHOLD fixed value, the reversing
 * duration is increased by multiplying the duration (provided as a parameter by the calling function) by the PITCH_REVERSE_COMPENSATION_FACTOR
 * fixed value. The turning angle is increased by multiplying the angle (provided as a parameter by the calling function) by the 
 * PITCH_TURN_COMPENSATION_FACTOR fixed value.
 * 
 * Note: when both a high roll (mower leaning to one side) and low pitch (mower facing downwards) are combined, the compensation factors are both
 * applied.
 * 
 * No pitch (under)correction applied if the pitch angle is high (mower facing upwards).
 * 
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param Duration of reverse (in ms)
 * @param OnSpot turn with action of both wheels, default is false
 * @param PitchComp compensate for pitch angle (increased reversing duration and angle), default is false
 * @param RollComp compensate for roll angle (increased angle), default is false
 * 
 */
void MowerReserseAndTurn(const int Angle, const int Duration, const bool OnSpot = false, const bool PitchComp = false, const bool RollComp = false);

/**
 * Mower checks selected obstacle types and reduces speed if conditions are met
 * @param SpeedDelta as int: the speed reduction to be applied (in absolue %). If multiple conditions are selected, same speed reduction is applied.
 * @param Front as optional int: sonar measured front distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Left as optional int: sonar measured left distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Right as optional int: sonar measured right distance under which mower needs to slow down. 0 disbales the check. Default is 0
 * @param Perimeter as optional int: perimeter wire signal magnitude under which  mower needs to slow down. 0 disables the check. Absolute value is used to perform the check (applies to both inside and outside perimeter wire).  Default is 0.
 * @return boolean indicating if the function triggered a speed reduction
 */
bool MowerSlowDownApproachingObstables(const int SpeedDelta, const int Front = 0, const int Left = 0, const int Right = 0, const int Perimeter = 0);

/**
 * Mower arc function : mower moves in given direction with motors running at a different speed, thus turning forming an arc : used for spiral mowing
 * @param direction forward (MOTION_MOTOR_FORWARD) or reverse (MOTION_MOTOR_REVERSE)
 * @param leftSpeed Left motor speed (in %)
 * @param rightSpeed Right motor speed (in %)
 */
void MowerArc(const int direction, const int leftSpeed, const int rightSpeed);

#endif