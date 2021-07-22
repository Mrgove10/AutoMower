#ifndef voltage_h
#define Voltage_h

#define VOLTAGE_DETECTION_THRESHOLD 9
#define BATTERY_VOLTAGE_LOW_THRESHOLD 11500    // in mV
#define BATTERY_VOLTAGE_MEDIUM_THRESHOLD 11900 // in mV
#define VOLTAGE_NORMAL_THRESHOLD 12000         // in mV

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