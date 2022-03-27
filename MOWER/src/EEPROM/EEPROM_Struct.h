/* ------------------------------------------------------------------------------- */
// Eeprom storage Structure
//

#ifndef eeprom_struct_h
#define eeprom_struct_h

#include <Arduino.h>
#define EEPROM_SIZE 512
#define EEPROM_CRC_SIZE 1

typedef struct
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} DateTimeStruct;

typedef struct
{
    DateTimeStruct LastEepromSaveTime;
    int16_t PerimeterSignalOffset;
    double PerimeterTrackSetpoint;
    double ParamPerimeterTrackPIDKp;
    double ParamPerimeterTrackPIDKi;
    double ParamPerimeterTrackPIDKd;
    int16_t PerimeterSignalLostThreshold;
    int16_t PerimeterSignalLowTrackThreshold;

    long totalObstacleDectections;
    unsigned long totalMowingTime;
    unsigned long partialMowingTime;
    unsigned long operationTime;
    unsigned long chargingTime;

    float GyroErrorX;
    float GyroErrorY;
    float GyroErrorZ;
    float AccelErrorX;
    float AccelErrorY;

    bool OTARequested;
} DataStruct;

#define EEPROM_SPARE_SIZE EEPROM_SIZE - sizeof(DataStruct) - EEPROM_CRC_SIZE

typedef struct
{
    DataStruct Data;
    byte Sparebuffer[EEPROM_SPARE_SIZE]; // 1 byte Checksum will be added after end of buffer
} EEPROMStruct;

typedef union
{
    EEPROMStruct Load;
    byte LoadBuffer[sizeof(EEPROMStruct)];
} EEPROMLoadStruct;

#endif