#ifndef gyroaccel_h
#define gyroaccel_h


// This code is adapted from https://electronoobs.com/images/Robotica/tut_9/MPU6050_acc_read_example.zip

/**
 * GY-521 MPU6050 Setup function
 *
 */
void GyroAccelSetup();

/**
 * GY-521 MPU6050 Acceleration error calibration function
 * @param number of samples to establish calibration
 *
 */
void AccelErrorCalibration(const int samples);

/**
 * GY-521 MPU6050 Gyroscope error calibration function
 *
 * @param number of samples to establish calibration
 */
void GyroErrorCalibration(const int samples);

#endif
