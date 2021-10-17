#ifndef eeprom_h
#define eeprom_h

#include "EEPROM_Struct.h"

#define EEPROM_BASE_ADDRESS 0
#define EEPROM_MAX_SIZE EEPROM_SIZE + 10      // not sure if needed to add a few bytes
#define EEPROM_WRITE_FREQUENCY 15 * 60 * 1000 // Eeprom update frequency, in ms

/**
 * Sets up EEPROM environement : reads EEPROM into EEPROM RAM image if checksum is valid or initialises EEPROM if checksum invalid
 * 
 */
void EEPROMSetup(void);

/**
 * Flushes content of EEPROM RAM image into EEPROM and commits
 * 
 */
void EEPROMWrite(void);

/**
 * Updates EEPROM RAM image before saving to EEPROM (triggers EPPROMWrite)
 * @param immediatly   Boolean     triggers imediate save
 * 
 */
void EEPROMSave(boolean immediatly);

/**
 * Initialises and RESETs the EEPROM to default initial values (ALL DATA LOST)
 * 
 */
void EEPROMInitialise(void);

#endif