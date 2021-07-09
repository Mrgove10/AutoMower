#ifndef mqtt_h
#define mqtt_h

void MQTTCallback(char *topic, byte *message, unsigned int length);
void MQTTInit(const bool Display = true);
void MQTTSendLogMessage(const char *MQTTTopic, const char *Message, const char *Tag, const int Level);
void MQTTSubscribe();
void MQTTUnSubscribe();
void MQTTReconnect();
void MQTTDisconnect();
void MQTTSendTelemetry();

#endif