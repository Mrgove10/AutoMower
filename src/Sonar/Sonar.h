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
 * @return float sensor distance
 */
float SonarRead(int sensor);

#endif