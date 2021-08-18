#ifndef voltage_h
#define Voltage_h

/**
 * Checks to see if voltage is connected and its level
 * 
 * @return true if voltage check is ok
 */
bool BatteryVoltageCheck(void);

/**
 * Function to read voltage 
 * @param Now optional bool to force immediate voltage read
 * @return Range depending on voltage thresholds
 */
int BatteryVoltageRead(const bool Now = false);

#endif