#ifndef GPS_h
#define GPS_h

/**
 * NEO-M8N GPS Setup function
 *
 */
void GPSSetup();

/**
 * Function to read GPS
 *  
 * @param Now true to force an immediate read
 */
void GPSRead(const bool Now = false);

/**
 * Checks to see if GPS is connected (and hopefully functionning)
 * @return true if is ok
 */
bool GPSCheck(void);

/**
 * Displays on debug port GPS details
 */
void GPSDetails(void);

#endif