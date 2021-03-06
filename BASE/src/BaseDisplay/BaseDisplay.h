#ifndef basedisplay_h
#define basedisplay_h

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
 * Display the sleeping state screen
 * @param refresh boolean to force full screen update
 * */
void sleepingDisplay(bool refresh = false);

/**
 * Display the sending state screen
 * @param refresh boolean to force full screen update
 * */
void sendingDisplay(bool refresh = false);

/**
 * Display the ERROR state screen
 * @param refresh boolean to force full screen update
 * */
void errorDisplay(bool refresh = false);

#endif
