#ifndef zones_h
#define zones_h

#define ACT_CW 0            // Clockwise turn or wire following
#define ACT_CCW 1           // counter-clockwise turn or wire following

#define ACT_END 0           // Stop step actions : to be used in last step if no mowing is to start - Parameter 1 = Not used; Parameter 2 = Not used
#define ACT_FORWARD 1       // Forward step action - Parameter 1 = duration in sec; Parameter 2 = speed in %
#define ACT_REVERSE 2       // Reverse step action - Parameter 1 = duration in sec; Parameter 2 = speed in %
#define ACT_SPOTTURN 3      // On the spot turn step action - Parameter 1 = angle in degrees (always positive); Parameter 2 = ACT_CW or ACT_CCW
#define ACT_FINDWIRE 4      // Find wire - Parameter 1 = maximum duration in sec; Parameter 2 = ACT_CW or ACT_CCW
#define ACT_FOLLOWWIRE 5    // Follow wire - Parameter 1 = duration in sec; Parameter 2 = ACT_CW or ACT_CCW
#define ACT_STARTMOWING 6   // Start mowing - Parameter 1 = mowing mode; Parameter 2 = Mowing duration in minutes before return to base

#define MOW_ZONE_0 0        // In front of docking station
#define MOW_ZONE_1 1        // In front of SPA
#define MOW_ZONE_2 2        // At back of SPA
#define MOW_ZONE_3 3        // In front of Terrace
#define MOW_ZONE_4 4        // Back of House
#define MOW_ZONE_5 5        // Perimeter mow counter-clockwise

/**
 * @brief Mower zone definition initialisation
 */
void mowZoneStepsInit(void);

/**
 * @brief Mower zone step execution
 * @param Action zone step action to perform
 * @param Param1 first action parameter
 * @param Param2 first action parameter
 */
void ZoneStepAction(const int Action, const int Param1, const int Param2);

#endif