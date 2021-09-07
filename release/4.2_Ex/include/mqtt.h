#ifndef CLOUD_H
#define CLOUD_H
#include <stdbool.h>
#include <stdint.h>


typedef void (*MQTT_ON_CONNECT)();
typedef void (*MQTT_ON_DISCONNECT)();
typedef void (*MQTT_ON_MESSAGE)(const char *topic, const char *msg);

bool mqtt_init(const char * server, int port, MQTT_ON_CONNECT on_connect, MQTT_ON_DISCONNECT on_disconnect, MQTT_ON_MESSAGE on_message);
void mqtt_run();
int	 mqtt_publish(const char *topic, const char *data, int len);
int	 mqtt_subscribe(const char *topic, int qos);
int	 mqtt_unsubscribe(const char *topic);
void mqtt_dump();
void mqtt_check();

#endif
