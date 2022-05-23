#ifndef battery_h
#define battery_h

/**
 * I2C INA219 Battery Charge Current Sensor Setup function
 *
 */
void BatteryCurrentSensorSetup();

/**
 * Checks to see if Battery I2C INA219 current sensor is connected (and hopefully functioning)
 * 
 * @return true if current sensor is ok
 */
bool BatteryCurrentSensorCheck(void);

/**
 * Function to test and read Battery Charge current
 * 
 * * @param Now true to force an immediate read
 * * @return true if charge current could be read
 */
bool BatteryChargeCurrentRead(const bool Now = false);

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