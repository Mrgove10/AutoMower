#ifndef tilt_h
#define tilt_h

#define TILT_HORIZONTAL 1
#define TILT_VERTICAL 2

#define TILT_DEBOUNCE_TIMEOUT 100         // in ms

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


#endif