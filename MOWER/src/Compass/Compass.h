#ifndef Compass_h
#define Compass_h

/**
 * I2C HMC5883L Compasss Sensor Setup function
 *
 */
void CompassSensorSetup();

/**
 * Function to test and read I2C HMC5883L Compasss Sensor
 *  
 * @param Now true to force an immediate read
 */
void CompassRead(const bool Now = false);


/**
 * @brief Function to read I2C HMC5883L Compass Sensor values
 *  
 * @param magX pointer to float used to store X magnitude value (output)
 * @param magY pointer to float used to store Y magnitude value (output)
 * @param magZ pointer to float used to store Z magnitude value (output)
 * @param Calibrated true to return calibrated value. If false, raw values are returned
 */
void getCompassValues(float *magX, float *magY, float *magZ, const bool Calibrated = false);

/**
 * @brief Function to calibrate compass for hard ans soft iron effects
 *  
 * @param samples number of samples to base calibration on
 */
void CompassCalibrate(int samples);

/**
 * Checks to see if I2C HMC5883L Compasss Sensor is connected (and hopefully functionning)
 * @return true if current sensor is ok
 */
bool CompassSensorCheck(void);

/**
 * Displays I2C HMC5883L Compasss Sensor details
 */
void DisplayCompassDetails(void);

#endif