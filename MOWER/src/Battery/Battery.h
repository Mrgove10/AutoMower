#ifndef battery_h
#define battery_h

/**
 * Battery charge relay setup function
 * 
 */
void BatteryChargeRelaySetup(void);

/**
 * Battery charge relay Open function
 * 
 */
void BatteryChargeRelayOpen(void);

/**
 * Battery charge relay Close function
 * 
 */
void BatteryChargeRelayClose(void);

/**
 * Battery charge check function. If battery is full, charge relay is opened. If battery level drops under a threshold, relay is closed
 * 
 * @param Now optional bool to force immediate battery check
 */
void BatteryChargeCheck(const bool Now = false);

#endif