#ifndef mqtt_h
#define mqtt_h

void MQTTCallback(char *topic, byte *message, unsigned int length);
void MQTTInit(const bool Display = true);
void MQTTSendLogMessage(const char *MQTTTopic, const char *Message, const char *Tag, const int Level);
void MQTTSubscribe();
void MQTTUnSubscribe();
void MQTTReconnect();
void MQTTDisconnect();
void MQTTSendTelemetry(const bool now = false);

/**
 * Send Base station a command to start sleeping on MQTT channel
 */
void BaseSleepingStartSend(void);

/**
 * Send Base station a command to start sending perimeter signal on MQTT channel
 */
void BaseSendingStartSend(void);

#endif