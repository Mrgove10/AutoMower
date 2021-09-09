#ifndef buzzer_h
#define buzzer_h

/**
 * Buzzer Setup function
 *
 */
void BuzzerSetup(void);

/**
 * Buzzer check function
 *
 */
void BuzzerCheck(void);

/**
 * Buzzer tune play function
 * 
 * @param tune pointer to array of structures containing tune to play
 * @param length of array
 * @param repeats number of times tune is played
 * 
 */
void playTune(noteStruct * tune, const int length, const int repeats = 1);

#endif