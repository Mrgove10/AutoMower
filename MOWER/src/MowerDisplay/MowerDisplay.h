#ifndef mowerdisplay_h
#define mowerdisplay_h


/**
 * Display menu bar on last line of display
 * @param title screen title to display on 1st line
 * @param now boolean to force update
 * */
void headerDisplay(String title = "", bool now = false);

/**
 * Display menu bar on last line of display
 * @param state Mower state as an int or -1 for return menu
 */
void menuDisplay(int state);

/**
 * Display the idle state screen
 * @param refresh boolean to force full screen update
 * */
void idleDisplay(bool refresh = false);

/**
 * Display the mowing state screen
 * @param refresh boolean to force full screen update
 * */
void mowingDisplay(bool refresh = false);

/**
 * Display the going to base state screen
 * @param refresh boolean to force full screen update
 * */
void toBaseDisplay(bool refresh = false);

/**
 * Display the docked state screen
 * @param refresh boolean to force full screen update
 * */
void dockedDisplay(bool refresh = false);

/**
 * Display the ERROR state screen
 * @param refresh boolean to force full screen update
 * */
void errorDisplay(bool refresh = false);

/**
 * Display the test state screen
 * @param refresh boolean to force full screen update
 * */
void testDisplay(bool refresh = false);

#endif
