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
 * Send Perimeter signal status on MQTT channel
 * @param now boolean indicating the sending is to be performed immediatly
 */
void PerimeterSignalStatusSend(const bool now = false);

/**
 * Send rain status on MQTT channel
 * @param now boolean indicating the sending is to be performed immediatly
 */
void BaseRainStatusSend(const bool now = false);

#endif