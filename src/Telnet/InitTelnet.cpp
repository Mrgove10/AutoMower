#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Telnet/InitTelnet.h"
#include "Utils/Utils.h"

void InitTelnet()
{
  char text[50];
  sprintf(text, "TelnetSpy console started for ");
  MySERIAL.setWelcomeMsg(text);
  sprintf(text, "Sorry, TelnetSpy console already connected to ");
  MySERIAL.setRejectMsg(text);
  MySERIAL.setCallbackOnConnect(telnetConnected);
  MySERIAL.setCallbackOnDisconnect(telnetDisconnected);
  MySERIAL.setBufferSize(2048);
  MySERIAL.setMinBlockSize(16);

  MySERIAL.begin(TELNET_BAUD);

  delay(100); // Wait for serial port

  MySERIAL.setDebugOutput(false);
}
void telnetConnected()
{
  char ipoutBuf[18];

  IPAddress ip = WiFi.localIP();
  sprintf(ipoutBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  MySERIAL.println(myTime.dateTime("H:i:s.v:") + "Telnet connection established on " + String(ipoutBuf));
  //  MySERIAL.println("Telnet connection established on " + String(ipoutBuf));
}

void telnetDisconnected()
{
  MySERIAL.println(" Telnet connection closed.");
}
