#ifndef fan_h
#define fan_h

/**
 * Fan Setup function
 */
void FanSetup();

/**
 * Fan Start function
 * @param Fan to start
 */
void FanStart(const int Fan);

/**
 * Fan test function
 * @param Fan to test
 */
void FanTest(const int Fan);

/**
 * Fan Stop function
 * @param Fan to stop
 */
void FanStop(const int Fan);

/**
 * Fan Check function
 * @param Fan to check
 * @param Now true to force immediate check
 */
void FanCheck(const int Fan, const bool Now = false);

#endif