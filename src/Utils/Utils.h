#ifndef utils_h
#define utils_h

#include <Arduino.h>
String char_reset_reason(byte reason);

// Debug Levels
#define DBG_ALWAYS 0
#define DBG_ERROR 1
#define DBG_WARNING 2
#define DBG_INFO 3
#define DBG_DEBUG 4
#define DBG_VERBOSE 5

#define DBG_ALWAYS_TEXT " "
#define DBG_ERROR_TEXT "E"
#define DBG_WARNING_TEXT "W"
#define DBG_INFO_TEXT "I"
#define DBG_DEBUG_TEXT "D"
#define DBG_VERBOSE_TEXT "V"

/**
 * Prints a debug message on Telnet and USB serial connection with no new line
 * 
 * @param message   String     Message to print
 * @param level     int        Debug message level.
 * @param time      boolean    True to print current time on message.
 */
void DebugPrint(const String message, const int level = DBG_ALWAYS, const boolean time = false);

/**
 * Prints a debug message on Telnet and USB serial connection with new line
 * 
 * @param message   String     Message to print
 * @param level     int        Debug message level. 
 * @param time      boolean    True to print current time on message.
 */
void DebugPrintln(const String message, const int level = DBG_ALWAYS, const boolean time = false);

/**
 * Logs a message to server message log and prints a debug message on Telnet and USB serial connection (new line)
 *  * 
 * @param message   String     Message to print
 * @param tags      String     List of tags (comma separated)
 * @param level     int        Debug message level. 
 */
void LogPrintln(const String message, const String tags, const int level = DBG_ALWAYS);

#endif