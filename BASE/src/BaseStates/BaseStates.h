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

/**
 * Base temperature monitoring : Base is set in error mode and sending stopped if temperature above max threshold
 * @param Theshold temperature above which base station is stopped
 * @return boolean indicating if temperature is too high (true) or not (false)
 */
bool BaseTemperatureTooHighCheck(const float Threshold);

/**
 * Base Perimeter current monitoring : Base is set in error mode and sending stopped if perimeter current too low
 * @param Threshold current (in mA) under which base station is stopped
 * @return boolean indicating if current is too low (true) or not (false)
 */
bool PerimeterCurrentTooLowCheck(const float Threshold);

/**
 * Base Perimeter current monitoring : Base is set in error mode and sending stopped if perimeter current too high
 * @param Threshold current (in mA) over which base station is stopped
 * @return boolean indicating if current is too high (true) or not (false)
 */
bool PerimeterCurrentTooHighCheck(const float Threshold);

/**
 * Mower status received monitoring : Base is set in error mode if no data is received before timeout and base is set into Sleeping mode if mower is in Docked Mode
 * @param Timeout in ms duing which mower data is expected
 * @return boolean indicating if timeout has been reached (true) or not (false)
 */
bool MowerStatusCheck(const unsigned long Timeout);

#endif