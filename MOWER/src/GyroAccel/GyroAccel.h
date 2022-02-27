#ifndef gyroaccel_h
#define gyroaccel_h


// This code is adapted from 
// https://electronoobs.com/images/Robotica/tut_9/MPU6050_acc_read_example.zip, and 
// Joop Brokking Website: http://www.brokking.net/imu.html

/**
 * GY-521 MPU6050 Setup function
 *
 */
void GyroAccelSetup();

/**
 * GY-521 MPU6050 gyroscope and Acceleration raw values read function
* @return true if read successfull, false if sensor not found
 */
bool GyroAccelDataRead(void);

/**
 * GY-521 MPU6050 Pitch and Roll calculation function
 * @param Now true to force an immediate read
 * @param reset true to force a pitch an roll reset
 * 
 */
void PitchRollCalc(const bool Now = false, const bool reset = false);

/**
 * GY-521 MPU6050 Acceleration error calibration function
 * @param number of samples to establish calibration
 *
 */
// void AccelErrorCalibration(const int samples);

/**
 * GY-521 MPU6050 Gyroscope error calibration function
 *
 * @param number of samples to establish calibration
 */
 void GyroErrorCalibration(const int samples);

/**
 * GY-521 MPU6050 Acceleration Angle value read function
 *
 */
// void AccelAngleRead(void);

/**
 * GY-521 MPU6050 Gyro Angle value read function
 *
 */
// void GyroAngleRead(void);

/**
 * GY-521 MPU6050 Combined Gyro/Accel Angle read function
 * 
 * @param Now true to force an immediate read
 */
// void GyroAccelAngleRead(const bool Now = false);

/**
 * Checks to see if Gyro/Accel is connected (and hopefully functionning)
 * @return true if is ok
 */
bool GyroAccelCheck(void);

#endif
