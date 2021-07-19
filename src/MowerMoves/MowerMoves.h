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

#endif