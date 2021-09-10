#ifndef cut_h
#define cut_h

/**
 * Cut Motor Setup function
 */
void CutMotorSetup();

/**
 * Cut Motor Start function
 * @param Direction to set speed
 * @param Speed to set
 */
void CutMotorStart(const int Direction, const int Speed);

/**
 * Cut Motor speed setting function
 * @param Speed to set
 */
void CutMotorSetSpeed(const int Speed);

/**
 * Cut Motor test function
 */
void CutMotorTest(void);

/**
 * Cut Motor Stop function
 * @param Immediate optional bool to force a fast motor stop
 */
void CutMotorStop(const bool Immedate = false);

/**
 * Cut Motor Check function
 * @param cutOffCurrent (in mA) cutoff current threshold to trigger motor protection stop
 * @param Now true to force immediate check
 * @return boolean indicating if check is ok (true) or not (false)
 * 
 */
bool CutMotorCheck(int cutOffCurrent, const bool Now = false);

#endif