//#include "reconnect.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Framework/OTA.h"
#include "Framework/Utils.h"

/* OTA init procedure */

void OTASetup(void)
{

  ArduinoOTA.setPort(OTAPort);

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

                       // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                       //    MySERIAL.println("Start updating " + type);
                     });

  ArduinoOTA.onEnd([]()
                   { DebugPrintln("\nEnd", DBG_ALWAYS, true); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          //    display.clear();
                          //    display.drawProgressBar(4, 32, 120, 10, progress / (total / 100));

                          // draw the percentage as String
                          //    display.setTextAlignment(TEXT_ALIGN_CENTER);
                          //    display.drawString(64,15,String(progress / (total / 100)) + "%");
                          //    display.display();

                          //    MySERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
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

    //    char outBuf[18];
    //    sprintf(outBuf,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
    //    display.clear();
    //    display.setBrightness(Mybrightness);
    //    display.setTextAlignment(TEXT_ALIGN_CENTER);
    //    display.setFont(ArialMT_Plain_16);
    //    display.drawString(64, 15, String("OTA Pending"));
    //    display.drawString(64, 35, String(outBuf));
    //    display.display();

    setInterval(0); // no NTP update to avoid any interruption during upload

    DebugPrintln("Uploading ", DBG_ALWAYS, true);
    SerialAndTelnet.handle();
    //    MQTTclient.unsubscribe(LinkyDataChan);  // no MQTT update to avoid any interruption during upload
    while (OTAelapsed < OTATimeout)
    {
      ArduinoOTA.handle();
      OTAelapsed = millis() - otaStart;
      delay(250);
    }
    DebugPrintln("Upload timeout", DBG_ERROR, true);
    //    reconnect();
    String msg = myTime.dateTime("H:i:s ") + "OTA upload request timeout";
    //    MQTTSendNote(CarPlugNoteChan, msg.c_str(), "OTA");
    //    MQTTclient.loop();
    //    MQTTclient.setCallback(MQTTcallback);
    //    MQTTclient.subscribe(LinkyDataChan);

    otaFlag = false;

    setInterval(NTPRefresh); // NTP updates back on
  }
}
