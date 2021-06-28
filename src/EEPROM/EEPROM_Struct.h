/* ------------------------------------------------------------------------------- */
// Eeprom storage Structure 
//

#ifndef eeprom_struct_h
#define eeprom_struct_h

#include <Arduino.h>
#define EEPROM_SIZE 512
#define EEPROM_CRC_SIZE 1

typedef struct DateTimeStruct {
  int year;
  int month;
  int day; 
  int hour;
  int minute;
  int second;
};

typedef struct DataStruct {
    byte val1;
    byte val2;
    byte val3;
    int val4;
    DateTimeStruct LastEepromSaveTime;
};

#define EEPROM_SPARE_SIZE EEPROM_SIZE-sizeof(DataStruct)-EEPROM_CRC_SIZE

typedef struct EEPROMStruct {
    DataStruct Data;
    byte Sparebuffer[EEPROM_SPARE_SIZE];   // 1 byte Checksum will be added after end of buffer
}; 

typedef union EEPROMLoadStruct {
    EEPROMStruct Load;
    byte LoadBuffer[sizeof(EEPROMStruct)];
};

#endif