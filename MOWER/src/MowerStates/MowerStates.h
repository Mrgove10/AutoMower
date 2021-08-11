#ifndef states_h
#define states_h

/**
 * Mower in idle state
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerIdle(const bool StateChange, const MowerState PreviousState);

/**
 * Mower docked
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerDocked(const bool StateChange, const MowerState PreviousState);

/**
 * Mower in mowing mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerMowing(const bool StateChange, const MowerState PreviousState);

/**
 * Mower returning to base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerGoingToBase(const bool StateChange, const MowerState PreviousState);

/**
 * Mower leaving base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerLeavingBase(const bool StateChange, const MowerState PreviousState);

/**
 * Mower in error
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState MowerState indicating previous state
 */
void MowerInError(const bool StateChange, const MowerState PreviousState);

/**
 * Mower finds perimeter wire according to heading given and, when found, orient right or left
 * @param heading integer indicating the heading to follows to try to find the wire
 * @param clockwise boolean indicating the direction in which the mower should face if it finds the wire
 * @return Success boolean depending on whether it found the wire or not
 */
bool MowerFindWire(const int heading, const bool clockwise);

/**
 * @brief Mower follows perimeter wire back to charging station
 * @param reset boolean indicating the wire trakcing function and PID needs to be reset
 * @param heading integer indicating the heading to follow to get to charging station
 * @param clockwise boolean indicating the direction in which the mower is following the wire
 * @return Success boolean depending on whether it found the wire or not
 */
bool MowerFollowWire(const bool reset, const int heading, const bool clockwise);

#endif