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
 * @param reset boolean indicating the wire finding function needs to be reset
 * @param phase pointer to integer used to indicate the search phase (input and output)
 * @param heading integer indicating the heading to follows to try to find the wire
 * @param clockwise boolean indicating the direction in which the mower should face if it finds the wire
 * @return Success boolean depending on whether it found the wire (true) or not (false)
 */
bool MowerFindWire(const bool reset, int *phase, const int heading, const bool clockwise);

/**
 * @brief Mower follows perimeter wire back to charging station
 * @param reset boolean indicating the wire tracking function and PID needs to be reset
 * @param heading integer indicating the heading to follow to get to charging station
 * @param clockwise boolean indicating the direction in which the mower is following the wire
 * @return Success boolean depending on whether it found the wire (true) or not (false)
 */
bool MowerFollowWire(bool *reset, const int heading, const bool clockwise);

/**
 * Enables the perimeter tracking adjustment of the speed for both motors
 * @param leftMotorAdjustment adjustment to apply to left Motor (in %)
 * @param rightMotorAdjustment adjustment to apply to right Motor (in %)
 */
void MotionMotorsTrackingAdjustSpeed(const int leftMotorAdjustment, const int rightMotorAdjustment);

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
bool CheckPreConditions(const int Tilt = ERROR_NO_ERROR, const int Bumper = ERROR_NO_ERROR, const int Front = ERROR_NO_ERROR, const int Left = ERROR_NO_ERROR, const int Right = ERROR_NO_ERROR, const int Perimeter = ERROR_NO_ERROR, const bool ErrorMode = false);

/**
 * @brief Function to check if an obstacle is detected and performs a pre-defined action (if activated). Obstacle detection can any combination of bumper (both), Sonars (Front, Left or right) and perimeter wire
 * If the action enable parameter is not set, the function only returns if detection occurred or not and action to take if left to caller to decide.
 * At the end of the function, if the action enable is set, the mower is stopped and the cutting motor is stopped. Caller to restart the motors as required.
 * 
 * @param Bumper as optional boolean : bumper active triggers detection/action. 0 disables the check. Default is 0
 * @param Front as optional int: sonar measured front distance to trigger detection/action. 0 disables the check. Default is 0
 * @param Left as optional int: sonar measured left distance to trigger detection/action. 0 disables the check. Default is 0
 * @param Right as optional int: sonar measured right distance to trigger detection/action. 0 disables the check. Default is 0
 * @param Perimeter as optional boolean: outside perimeter wire to trigger detection/action. 0 disables the check. Absolute value is used to perform the check (applies to both inside and outside perimeter wire).  Default is 0.
 * @param ActionMode (optional) , if true, changes action is performed if condition is detected (default is false).
 * @param MotorOverCurrent as optional int: over current on any of the motion motors to trigger detection/action. 0 disables the check. Default is 0
 * 
 * @return integer indicating which detection was detected.
 */
int CheckObstacleAndAct(const bool Bumper = false, const int Front = 0, const int Left = 0, const int Right = 0, const bool Perimeter = false, const int MotorOverCurrent = 0, const bool ActionMode = false);

/**
 * @brief Function to reposition mower on docking station to ensure that charging point are well aligned and that charging current is normal.
 * This is performed by performing a small reverse and forward movement if the measured charging current is too low.
 * Caller to take necessary action if procedure failed (e.g. enter error state).
 * 
 * @param MinCurrent int with minimum charging current threshold under which the repositioning is performed
 * @param MaxAttempts int with maximum number of attempts to be performed
 *
 * @return boolean indicating if repositioning succeded (true) or not (false).
 */
bool RepositionOnDock(const int MinCurrent, int MaxAttempts);

#endif