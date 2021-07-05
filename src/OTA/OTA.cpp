//#include "reconnect.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "OTA/OTA.h"
#include "Utils/Utils.h"
#include "MotionMotor/MotionMotor.h"
#include "MQTT/MQTT.h"

/* OTA init procedure */

void OTASetup(void)
{

  ArduinoOTA.setPort(OTA_PORT);

  ArduinoOTA.setHostname(ESPHOSTNAME);

  // No authentication by default
  //  ArduinoOTA.setPassword("1234");
  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_SPIFFS
                         type = "filesystem";
                       }
                       lcd.clear();
                       lcd.setCursor(0,0);
                       lcd.print(F("OTA Update"));
                       lcd.setCursor(0,2);
                       lcd.print(F("In Progress ..."));

                       // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                       //    MySERIAL.println("Start updating " + type);
                     });

  ArduinoOTA.onEnd([]()
                   { DebugPrintln("\nEnd", DBG_ALWAYS, true); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          lcd.setCursor(16,2);
                          lcd.print((progress*100) / total);
                          lcd.print(F("%"));
                        });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       MySERIAL.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         DebugPrintln("Auth Failed", DBG_ERROR, true);
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         DebugPrintln("Begin Failed", DBG_ERROR, true);
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         DebugPrintln("Connect Failed", DBG_ERROR, true);
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         DebugPrintln("Receive Failed", DBG_ERROR, true);
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         DebugPrintln("End Failed", DBG_ERROR, true);
                       }
                     });

  ArduinoOTA.begin();
}

void OTAHandle(void)
{

  if (otaFlag)
  {
    unsigned long otaStart = millis();
    OTAelapsed = 0;
    IPAddress ip = WiFi.localIP();

    char outBuf[18];
    sprintf(outBuf,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("OTA Update"));
    lcd.setCursor(2,1);
    lcd.print(F("Pending..."));
    lcd.setCursor(2,3);
    lcd.print(outBuf);
    
    MotionMotorStop(MOTION_MOTOR_RIGHT);
    MotionMotorStop(MOTION_MOTOR_LEFT);
    
    setInterval(0); // no NTP update to avoid any interruption during upload

    DebugPrintln("Waiting for OTA upload ", DBG_INFO, true);
    SerialAndTelnet.handle();
    MQTTUnSubscribe();  // no MQTT update to avoid any interruption during upload
    while (OTAelapsed < OTA_TIMEOUT)
    {
      ArduinoOTA.handle();
      OTAelapsed = millis() - otaStart;
      delay(250);
    }
    DebugPrintln("Upload timeout", DBG_ERROR, true);
    lcd.setCursor(2,3);
    lcd.print(F("   Timeout !    "));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    
    MQTTReconnect();

    MQTTSubscribe();
    LogPrintln("OTA upload request timeout", TAG_OTA, DBG_WARNING);

    otaFlag = false;

    setInterval(NTP_REFRESH); // NTP updates back on
  }
}
