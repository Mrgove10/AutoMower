#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Sonar/Sonar.h"
#include "Utils/Utils.h"

/**
 * Sonar sensor setup function
 * 
 */
void SonarSensorSetup(void)
{
  DebugPrintln("Sonar Sensor Setup start", DBG_VERBOSE, true);

/*  TemperatureSensors.begin();
  delay(1500);
  
  TemperatureOneWire.reset_search();

  DebugPrintln("Device 1-Red Address: " + TempSensorAddress(temp_1_RedSensor), DBG_VERBOSE, true); 
  DebugPrintln("Device 2-Blue Address: " + TempSensorAddress(temp_2_BlueSensor), DBG_VERBOSE, true);

  if (TemperatureSensors.isConnected(temp_1_RedSensor)){
    DebugPrintln("Device 1-Red found", DBG_INFO, true); 
  }
  else {
    DebugPrintln("Device 1-Red MISSING !", DBG_ERROR, true); 
  }

  if (TemperatureSensors.isConnected(temp_2_BlueSensor)){
    DebugPrintln("Device 2-Blue found", DBG_INFO, true); 
  }
  else {
    DebugPrintln("Device 2-Blue MISSING !", DBG_ERROR, true); 
  }

  int nbTempSensors = 0;
  nbTempSensors = TemperatureSensors.getDeviceCount();

  DebugPrint("Locating sensors...", DBG_INFO, true);
  DebugPrintln(String(nbTempSensors, DEC) + " sensors found");

  TemperatureSensors.setResolution(TEMPERATURE_PRECISION);

  DebugPrintln("Device 1-Red Resolution: " + String(TemperatureSensors.getResolution(temp_1_RedSensor), DEC), DBG_VERBOSE, true );
  DebugPrintln("Device 2-Blue Resolution: " + String(TemperatureSensors.getResolution(temp_2_BlueSensor), DEC), DBG_VERBOSE, true );

  DebugPrint("Parasite power is: ", DBG_INFO, true);
  if (TemperatureSensors.isParasitePowerMode()) {
    DebugPrintln("ON");
  }
  else 
  {
    DebugPrintln("OFF");
  } 

  TemperatureSensors.requestTemperatures();
  delay(1000);
  */
}

/**
 * Checks to see if Sonar sensor is connected and functionning
 * @param sensor int Sonar to check
 * @return true if sensor check is ok
 */
bool SonarSensorCheck(int sensor)
{
  bool sensorCheck = false;
  String sensorStr[SONAR_COUNT] = {"Front", "Left","Right" } ;
  unsigned int Distance = UNKNOWN_INT;

  DebugPrintln("SonarSensorCheck start #" + String(sensor), DBG_VERBOSE, true);
  if (sensor == 1) 
  {
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print(F("Sonar Tests"));

  Distance = sonar[sensor-1].ping_cm(SONAR_MAX_DISTANCE);
  sensorCheck = Distance != 0;

  lcd.setCursor(2, 0+sensor);

  DebugPrintln(sensorStr[sensor-1] + " Distance " + String(Distance), DBG_INFO, true);

  if (sensorCheck) 
  {
    DebugPrintln(sensorStr[sensor-1] + " Sonar sensor Ok : " + String(Distance), DBG_INFO, true);
    lcd.print(sensorStr[sensor-1] + " OK ");
    lcd.print(Distance);
    lcd.print(F(" cm"));
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(sensorStr[sensor-1] + " No sensor echo", TAG_CHECK, DBG_WARNING);
    lcd.print(sensorStr[sensor-1] + " NO ECHO");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to read distance
 * @param sensor int functional sensor to read distance from
 * @return float sensor distance
 */
float SonarRead(int sensor)
{
  /*
  float tempC = UNKNOWN_FLOAT;

  if (sensor == TEMPERATURE_1_RED)
  {
    TemperatureSensors.requestTemperaturesByAddress(temp_1_RedSensor);
    tempC = TemperatureSensors.getTempC(temp_1_RedSensor);
  }
  if (sensor == TEMPERATURE_2_BLUE)
  {
    TemperatureSensors.requestTemperaturesByAddress(temp_2_BlueSensor);
    tempC = TemperatureSensors.getTempC(temp_2_BlueSensor);
  }

//  DebugPrintln("TemperatureRead value " + String(tempC,2), DBG_VERBOSE, true);

  if (tempC > -127.0f && tempC < 85.0f) {
    return tempC;
  }
  else
  {
    if (sensor == TEMPERATURE_1_RED) {Temp1ErrorCount = Temp1ErrorCount + 1;}
    if (sensor == TEMPERATURE_2_BLUE) {Temp2ErrorCount = Temp2ErrorCount + 1;}
    return UNKNOWN_FLOAT;
  }
*/
  return UNKNOWN_FLOAT;

}
