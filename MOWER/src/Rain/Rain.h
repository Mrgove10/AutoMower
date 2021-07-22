#ifndef rain_h
#define rain_h

#define RAIN_SENSOR_CHECK_THRESHOLD 1
#define RAIN_SENSOR_RAINING_THRESHOLD 50 // this may have to be placed in a parameter

/**
 * Checks to see if rain sensor is connected (and hopefully functionning)
 * 
 * @return true if rain sensor check is ok
 */
bool RainSensorCheck(void);

/**
 * Function to know if it is raining
 * 
 * * @return true if rain is detected
 */
bool isRaining(void);

#endif