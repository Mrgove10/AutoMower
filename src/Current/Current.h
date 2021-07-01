#ifndef current_h
#define current_h

#define CHARGE_CURRENT_CHECK_THRESHOLD 1000             // in raw AnalogRead points
#define CHARGE_CURRENT_OFFSET 140                         // in raw AnalogRead 
#define CHARGE_CURRENT_MV_PER_AMP 100.0F                   // From ACS712-20A datasheet
#define CHARGE_CURRENT_ZERO_VOLTAGE 2500                // in mv
#define CHARGE_CURRENT_DEADBAND 0.250F                         // in A 

/**
 * I2C INA219 Current Sensor Setup function
 *
 */
void MotorCurrentSensorSetup();

/**
 * Function to test and read I2C INA219 Current Sensor
 *  
 * @param sensor int sensor to read
 * @return true if current could be read
 */
bool MotorCurrentRead(int sensor);

/**
 * Checks to see if Motor I2C INA219 Current Sensor is connected (and hopefully functionning)
 * @param sensor int sensor to check
 * @return true if current sensor is ok
 */
bool MotorCurrentSensorCheck(int sensor);

/**
 * Checks to see if Battery ACS712 current sensor is connected (and hopefully functionning)
 * 
 * @return true if current sensor is ok
 */
bool BatteryCurrentSensorCheck(void);

/**
 * Function to test and read Battery Charge current
 * 
 * * @return true if charge current could be read
 */
bool BatteryChargeCurrentRead(void);

#endif