#ifndef tilt_h
#define tilt_h

/**
 * Horizontal Tilt sensor ISR function
 * 
 */
ICACHE_RAM_ATTR void horizontalTiltISR(void);

/**
 * Vertical Tilt sensor ISR function
 * 
 */
ICACHE_RAM_ATTR void verticalTiltISR(void);

/**
 * Tilt sensor Setup function
 * 
 */
void TiltSetup(void);

/**
 * Checks to see if tilt sensor is activated on mower startup
 * @param Tilt int    bumper number
 * @return boolean true if sensor check is ok
 */
bool TiltSensorCheck(int tilt);

/**
 * Read, in a protected way, the status of the tilt sensor
 * @param Tilt int tilt number
 * @return boolean true if tilt activated, false if not
 */
bool TiltRead(int tilt);

#endif