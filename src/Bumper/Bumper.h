#ifndef bumper_h
#define bumper_h

#define BUMPER_LEFT 1
#define BUMPER_RIGHT 2

#define BUMPER_DEBOUNCE_TIMEOUT 100         // in ms

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
 * @return boolean true if rain sensor check is ok
 */
bool BumperSensorCheck(int bumper);

/**
 * Function to know if it is raining
 * 
 * * @return true if rain is detected
 */
//bool isRaining(void);

#endif