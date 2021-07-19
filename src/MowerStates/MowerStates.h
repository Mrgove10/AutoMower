#ifndef states_h
#define states_h

/**
 * Mower in idle state
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerIdle(const bool StateChange);

/**
 * Mower docked
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerDocked(const bool StateChange);

/**
 * Mower in mowing mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerMowing(const bool StateChange);

/**
 * Mower returning to base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerGoingToBase(const bool StateChange);

/**
 * Mower leaving base
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerLeavingBase(const bool StateChange);

/**
 * Mower in error
 * @param StateChange boolean indicating this is the first call to this state after a state change
 */
void MowerInError(const bool StateChange);

#endif