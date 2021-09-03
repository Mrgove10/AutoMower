#ifndef motion_h
#define motion_h

/**
 * Motion Motor Setup function
 */
void MotionMotorSetup();

/**
 * Motion Motor Start function
 * @param Motor to start
 * @param Direction to set speed
 * @param Speed to set
 */
void MotionMotorStart(const int Motor, const int Direction, const int Speed);

/**
 * Motion Motor speed setting function. Speed can be set either in absolute or relative values depending on call parameters. Relative speed can either be positive or negative
 * @param Motor to set speed (in %)
 * @param Speed to set or to change
 * @param Relative boolean indicating if Speed parameter is given in relative or absolute value
 */
void MotionMotorSetSpeed(const int Motor, const int Speed, const bool Relative = false);

/**
 * Motion Motor test function
 * @param Motor to test
 */
void MotionMotorTest(const int Motor);

/**
 * Motion Motor Stop function
 * @param Motor to stop
 */
void MotionMotorStop(const int Motor);

/**
 * Motion Motor Check function
 * @param Motor to check
 * @param Now true to force immediate check
 */
void MotionMotorCheck(const int Motor, const bool Now = false);

#endif