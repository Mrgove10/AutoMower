#ifndef rain_h
#define rain_h

/**
 * Checks to see if rain sensor is connected (and hopefully functioning)
 * 
 * @return true if rain sensor check is ok
 */
bool RainSensorCheck(void);

/**
 * Function to know if it is raining
 * 
 * @param Now optional bool to force immediate rain sensor read
 * @return true if rain is detected
 */
bool isRaining(const bool Now = false);

#endif