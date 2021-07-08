#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "GPS/GPS.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * NEO-M8N GPS Setup function
 *
 */
void GPSSetup()
{
  GPS_UART.begin(GPS_BAUD, SERIAL_8N1, PIN_ESP_GPS_RX, PIN_ESP_GPS_TX);

  GPS_UART.flush();

  DebugPrintln("GPS setup Done", DBG_VERBOSE, true);
}

/**
 * Function to read GPS
 *  
 * @param Now true to force an immediate read
 */
void GPSRead(const bool Now)
{
  static unsigned long LastGPSRead = 0;

  if ((millis() - LastGPSRead > GPS_READ_INTERVAL) || Now)
  {
    if (GPS_UART.available() > 0)
    {
      //      Serial.println("GPS Serial data: " + String (GPS_UART.available()) + " ");
      DebugPrintln("GPS Serial data: " + String(GPS_UART.available()) + " ", DBG_VERBOSE, true);
    }

    String buf;
    while (GPS_UART.available() > 0)
    {
      char c = GPS_UART.read();
      buf = buf + c;
      GPS.encode(c);
    }
    /*
    if (buf.length() > 0 ) 
    {
      Serial.println("GPS Buffer:[" + buf + "]");
//      DebugPrintln (String(buf), DBG_VERBOSE);
      GPSDetails();
    }
*/
    if (GPS.location.isValid())
    {
      GPSSatellitesFix = GPS.satellites.value();
      GPSHdop = GPS.hdop.hdop();
      GPSSpeed = GPS.speed.kmph();
      GPSAltitude = GPS.altitude.meters();
      GPSLatitude = GPS.location.lat();
      GPSLongitude = GPS.location.lng();
      GPSHeading = GPS.course.value();

      DebugPrintln("Sat: " + String(GPSSatellitesFix) + " HDop: " + String(GPSHdop, 2) + " Head: " + String(GPSHeading), DBG_VERBOSE, true);
    }
    LastGPSRead = millis();
  }
}

/**
 * Checks to see if GPS is connected (and hopefully functionning)
 * @return true if is ok
 */
bool GPSCheck(void)
{
  GPSRead(true);

  DisplayClear();
  DisplayPrint(0, 0, F("GPS Test"));

  if (GPS.charsProcessed() > GPS_CHARS_TO_DETECT)
  {
    DebugPrintln("GPS ok", DBG_INFO, true);
    DisplayPrint(2, 2, "GPS Ok");
    DisplayPrint(2, 3 , "Statelites: " + String(GPS.satellites.value()));
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("GPS not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2, "GPS ERROR");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Displays GPS details
 */
void GPSDetails(void)
{
  DebugPrintln("Hdop: " + String(GPS.hdop.hdop()) + ", " + "Speed: " + String(GPS.speed.kmph()) + ", " + "alt: " + String(GPS.altitude.meters()) + ", " + String(GPS.satellites.value()) + " sats");
  DebugPrint("charsProcessed: " + String(GPS.charsProcessed()) + " | ");
  DebugPrint("sentencesWithFix: " + String(GPS.sentencesWithFix()) + " | ");
  DebugPrint("failedChecksum: " + String(GPS.failedChecksum()) + " | ");
  DebugPrintln("passedChecksum: " + String(GPS.passedChecksum()) + " | ");
}
