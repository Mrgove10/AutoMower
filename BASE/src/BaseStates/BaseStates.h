#ifndef states_h
#define states_h

/**
 * Base in idle state
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseIdle(const bool StateChange, const BaseState PreviousState);

/**
 * Base sleeping
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseSleeping(const bool StateChange, const BaseState PreviousState);

/**
 * Base sending perimeter signal mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseSending(const bool StateChange, const BaseState PreviousState);

/**
 * Base in error
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseInError(const bool StateChange, const BaseState PreviousState);

#endif