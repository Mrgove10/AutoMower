#ifndef sonar_h
#define sonar_h

#include "pin_definitions.h"

/**
 * Sonar sensor setup function
 * 
 */
void SonarSensorSetup(void);

/**
 * Checks to see if Sonar sensor is connected and functionning
 * @param sensor int Sonar to check
 * @return true if sensor check is ok
 */
bool SonarSensorCheck(int sensor);

/**
 * Function to read distance
 * @param sensor int functional sensor to read distance from
 * @param Now optional bool true if immediate read
 * @return float sensor distance
 */
int SonarRead(const int sensor, const bool Now = false);

/**
 * Sonar Read task main loop
 * @param dummyParameter is unused but required
 */
void SonarReadLoopTask(void *dummyParameter);

/**
 * Sonar Read task creation function
 */
void SonarReadLoopTaskCreate(void);

/**
 * Sonar Read task suspension function
 */
void SonarReadLoopTaskSuspend(void);

/**
 * Sonar Read task resume from suspension function
 */
void SonarReadLoopTaskResume(void);

/**
 * Sonar Read task delete
 */
void SonarReadLoopTaskDelete(void);

/**
 * Sonar Read task monitoring function to check if task is running
 *
 * @param task boolean indicating if check is to be performed on task counter
 * @param distance boolean indicating if check is to be performed on distance values
 * @return boolean indicating if task appears not to be running (false) or is running (true)
 */
bool SonarReadLoopTaskMonitor(bool task = true, bool distance = true);

#endif