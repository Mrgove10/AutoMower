#include <Arduino.h>
#include "myGlobals_definition.h"
#include "MowerZones/MowerZones.h"
#include "MowerMoves/MowerMoves.h"
#include "MowerStates/MowerStates.h"
#include "Utils/Utils.h"

/**
 * @brief Mower zone step execution
 * @param Action zone step action to perform
 * @param Param1 first action parameter
 * @param Param2 first action parameter
 */
void ZoneStepAction(const int Action, const int Param1, const int Param2)
{
    static int FindWirePhase = 0;
    static bool FindWireReset = true;
    static unsigned long FindWireStepStart = 0;
    bool wirefound = false;

    static bool PIDReset = true;
    static unsigned long FollowWireStepStart = 0;

    DebugPrintln("ZoneStepAction: " + String(Action) + " " + String(Param1) + " " + String(Param2), DBG_DEBUG, true);

    switch (Action)
    {
      case ACT_REVERSE:
        MowerReverse(Param2, Param1 * 1000, true);
        g_ZoneStepDuration = 0;
        break;

      case ACT_FORWARD:

        // Obstacle Collision detection
        if (OBSTACLE_DETECTED_NONE != CheckObstacleAndAct(true,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          SONAR_MIN_DISTANCE_FOR_STOP,
                                                          false, //  no Perimeter detection (and action)
                                                          MOTION_MOTOR_OVERCURRENT_THRESHOLD,
                                                          true))
        {
          // Check if number of consecutive obstacle detection is above threshold and put mower in Error mode
          if (g_successiveObstacleDetections > MOWER_MOWING_MAX_CONSECUTIVE_OBSTACLES)
          {
            g_CurrentState = MowerState::error;
            g_CurrentErrorCode = ERROR_MOWING_CONSECUTIVE_OBSTACLES;
            return;
          }
          else
          {
            MowerForward(Param2, true);
          }
        }
        g_ZoneStepDuration = Param1 * 1000;
        break;
    
      case ACT_SPOTTURN:

        if (Param2 == ACT_CCW)
        {
          MowerTurn(-Param1, true);
        }
        else
        {
          MowerTurn(Param1, true);
        }

        g_ZoneStepDuration = 0;
        break;

      case ACT_FINDWIRE:
        g_ZoneStepDuration = Param1 * 1000;
        
        if (FindWireReset) 
        {
          FindWireStepStart = millis();
        }

        if (Param2 == ACT_CW)
        {
          wirefound = MowerFindWire(FindWireReset, &FindWirePhase, BACK_TO_BASE_HEADING, true);
        }
        else
        {
          wirefound = MowerFindWire(FindWireReset, &FindWirePhase, BACK_TO_BASE_HEADING, false);
        }

        if (FindWireReset) 
        {
          FindWireReset = false;
        }

        if (wirefound)
        {
          // Stop wire finding by forcing duration to 0, which will case to move to next step
          g_ZoneStepDuration = 0;
          FindWireReset = true;   // set reset indicator for next time
        }
        else
        {
          //  Check if wire find duration limit has been reached
          if ( millis() - FindWireStepStart > g_ZoneStepDuration)
          {
            DebugPrintln("End of wire find step duration expired", DBG_WARNING, true);
            // Stop wire finding by forcing duration to 0, which will case to move to next step
            g_ZoneStepDuration = 0;
            FindWireReset = true;   // set reset indicator for next time
          }
        }
        break;

      case ACT_FOLLOWWIRE:
        g_ZoneStepDuration = Param1 * 1000;

        if (PIDReset) 
        {
          FollowWireStepStart = millis();
        }

        if (Param2 == ACT_CW)
        {
          MowerFollowWire(&PIDReset, BACK_TO_BASE_HEADING, true);
        }
        else
        {
          MowerFollowWire(&PIDReset, BACK_TO_BASE_HEADING, false);
        }

        //  Check if wire following duration limit has been reached
        if (millis() - FollowWireStepStart > g_ZoneStepDuration)
        {
          DebugPrintln("Wire following step completed", DBG_INFO, true);
          // Stop wire finding by forcing duration to 0, which will case to move to next step
          g_ZoneStepDuration = 0;
          PIDReset = true;   // set reset indicator for next time
        }

        break;

      case ACT_END:
        MowerStop();
        DebugPrintln("End of zone steps with no mowing", DBG_INFO, true);
        g_CurrentState = MowerState::idle;
        break;

      case ACT_STARTMOWING:
        MowerStop();
        DebugPrintln("End of zone steps and mowing starts", DBG_INFO, true);
        g_mowingMode = Param1;
        g_ZoneMowDuration = Param2 * 1000;
        g_CurrentState = MowerState::mowing;
        g_ZoneStepDuration = 0;
        break;

      default:
        DebugPrintln("Step Action Undefined !", DBG_ERROR, true);
        g_CurrentState = MowerState::error;
        g_CurrentErrorCode = ERROR_UNDEFINED_STEP_ACTION;
        break;
    }
}

/**
 * @brief Mower zone definition initialisation
 */
void mowZoneStepsInit(void)
{
    DebugPrintln("Step Actions initialisation...", DBG_INFO, true);

    // ZONE 0 step definition -- In front of docking station
    g_mowZoneSteps[MOW_ZONE_0][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_0][0].param1 = 10;   g_mowZoneSteps[MOW_ZONE_0][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_0][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_0][1].param1 = 45;   g_mowZoneSteps[MOW_ZONE_0][1].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_0][2].action = ACT_END;         g_mowZoneSteps[MOW_ZONE_0][2].param1 = 0;    g_mowZoneSteps[MOW_ZONE_0][2].param2 = 0;

    // ZONE 1 step definition -- In front of SPA
    g_mowZoneSteps[MOW_ZONE_1][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_1][0].param1 = 20;   g_mowZoneSteps[MOW_ZONE_1][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_1][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_1][1].param1 = 120;  g_mowZoneSteps[MOW_ZONE_1][1].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_1][2].action = ACT_FORWARD;     g_mowZoneSteps[MOW_ZONE_1][2].param1 = 20;   g_mowZoneSteps[MOW_ZONE_1][2].param2 = 100;
    g_mowZoneSteps[MOW_ZONE_1][3].action = ACT_STARTMOWING; g_mowZoneSteps[MOW_ZONE_1][3].param1 = MOWER_MOWING_MODE_RANDOM;    g_mowZoneSteps[MOW_ZONE_1][3].param2 = 120;

    // ZONE 2 step definition -- At back of SPA
    g_mowZoneSteps[MOW_ZONE_2][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_2][0].param1 = 10;   g_mowZoneSteps[MOW_ZONE_2][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_2][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_2][1].param1 = 45;   g_mowZoneSteps[MOW_ZONE_2][1].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_2][2].action = ACT_FINDWIRE;    g_mowZoneSteps[MOW_ZONE_2][2].param1 = 120;  g_mowZoneSteps[MOW_ZONE_2][2].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_2][3].action = ACT_FOLLOWWIRE;  g_mowZoneSteps[MOW_ZONE_2][3].param1 = 60;   g_mowZoneSteps[MOW_ZONE_2][3].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_2][4].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_2][4].param1 = 45;   g_mowZoneSteps[MOW_ZONE_2][4].param2 = ACT_CW;
    g_mowZoneSteps[MOW_ZONE_2][5].action = ACT_STARTMOWING; g_mowZoneSteps[MOW_ZONE_2][5].param1 = MOWER_MOWING_MODE_RANDOM;    g_mowZoneSteps[MOW_ZONE_2][5].param2 = 120;

    // ZONE 3 step definition -- In front of Terrace
    g_mowZoneSteps[MOW_ZONE_3][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_3][0].param1 = 15;   g_mowZoneSteps[MOW_ZONE_3][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_3][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_3][1].param1 = 100;  g_mowZoneSteps[MOW_ZONE_3][1].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_3][2].action = ACT_FINDWIRE;    g_mowZoneSteps[MOW_ZONE_3][2].param1 = 120;  g_mowZoneSteps[MOW_ZONE_3][2].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_3][3].action = ACT_FOLLOWWIRE;  g_mowZoneSteps[MOW_ZONE_3][3].param1 = 80;   g_mowZoneSteps[MOW_ZONE_3][3].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_3][4].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_3][4].param1 = 45;   g_mowZoneSteps[MOW_ZONE_3][4].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_3][5].action = ACT_STARTMOWING; g_mowZoneSteps[MOW_ZONE_3][5].param1 = MOWER_MOWING_MODE_RANDOM;    g_mowZoneSteps[MOW_ZONE_3][5].param2 = 120;

    // ZONE 4 step definition -- Back of House
    g_mowZoneSteps[MOW_ZONE_4][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_4][0].param1 = 8;    g_mowZoneSteps[MOW_ZONE_4][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_4][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_4][1].param1 = 30;   g_mowZoneSteps[MOW_ZONE_4][1].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_4][2].action = ACT_FORWARD;     g_mowZoneSteps[MOW_ZONE_4][2].param1 = 3;    g_mowZoneSteps[MOW_ZONE_4][2].param2 = 100;
    g_mowZoneSteps[MOW_ZONE_4][3].action = ACT_FINDWIRE;    g_mowZoneSteps[MOW_ZONE_4][3].param1 = 120;  g_mowZoneSteps[MOW_ZONE_4][3].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_4][4].action = ACT_FOLLOWWIRE;  g_mowZoneSteps[MOW_ZONE_4][4].param1 = 140;  g_mowZoneSteps[MOW_ZONE_4][4].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_4][5].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_4][5].param1 = 45;   g_mowZoneSteps[MOW_ZONE_4][5].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_4][6].action = ACT_STARTMOWING; g_mowZoneSteps[MOW_ZONE_4][6].param1 = MOWER_MOWING_MODE_RANDOM;    g_mowZoneSteps[MOW_ZONE_4][6].param2 = 120;

    // ZONE 5 step definition -- Perimeter Mow
    g_mowZoneSteps[MOW_ZONE_5][0].action = ACT_REVERSE;     g_mowZoneSteps[MOW_ZONE_5][0].param1 = 7;    g_mowZoneSteps[MOW_ZONE_5][0].param2 = LEAVING_BASE_REVERSE_SPEED;
    g_mowZoneSteps[MOW_ZONE_5][1].action = ACT_SPOTTURN;    g_mowZoneSteps[MOW_ZONE_5][1].param1 = 25;   g_mowZoneSteps[MOW_ZONE_5][1].param2 = ACT_CCW;
    g_mowZoneSteps[MOW_ZONE_5][2].action = ACT_STARTMOWING; g_mowZoneSteps[MOW_ZONE_5][2].param1 = MOWER_MOWING_MODE_PERIMETER_CLOCKWISE;  g_mowZoneSteps[MOW_ZONE_5][2].param2 = 30;
}
