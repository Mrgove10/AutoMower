#ifndef current_h
#define current_h

/**
 * I2C INA219 Motor Current Sensor Setup function
 *
 */
void MotorCurrentSensorSetup();

/**
 * Function to test and read I2C INA219 Current Sensor
 *  
 * @param sensor int sensor to read
 * @param Now true to force an immediate read
 * @return true if current could be read
 */
bool MotorCurrentRead(const int sensor, const bool Now = false);

/**
 * Checks to see if Motor I2C INA219 Current Sensor is connected (and hopefully functionning)
 * @param sensor int sensor to check
 * @return true if current sensor is ok
 */
bool MotorCurrentSensorCheck(int sensor);

/**
 * I2C INA219 Battery Charge Current Sensor Setup function
 *
 */
void BatteryCurrentSensorSetup();

/**
 * Checks to see if Battery I2C INA219 current sensor is connected (and hopefully functionning)
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

#endif