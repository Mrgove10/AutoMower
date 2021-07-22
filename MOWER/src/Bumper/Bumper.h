#ifndef bumper_h
#define bumper_h

/**
 * Left Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void LeftBumperISR(void);

/**
 * Right Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void RightBumperISR(void);

/**
 * Bumper Setup function
 * 
 */
void BumperSetup(void);

/**
 * Checks to see if bumper sensor is connected (and hopefully functionning)
 * @param bumper int    bumper number
 * @return boolean true if sensor check is ok
 */
bool BumperSensorCheck(int bumper);

/**
 * Read, in a protected way, the status of a bumper sensor
 * @param bumper int bumper number
 * @return boolean true if bumper activated, false if not
 */
bool BumperRead(int bumper);

#endif