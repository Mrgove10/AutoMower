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
 * Mower reverse and turn function
 * @param Angle to turn in degrees (positive is right turn, negative is left turn)
 * @param Duration of reverse (in ms)
 * @param OnSpot turn with action of both wheels
 * 
 */
void MowerReserseAndTurn(const int Angle, const int Duration, const bool OnSpot = false);

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

#endif