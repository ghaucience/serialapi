#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <mosquitto.h>
#include <errno.h>
#include <signal.h>
#include "mqtt.h"
#include "system.h"

#include "log.h"

typedef struct stMqttEnv {
	bool inited;
	char server[200];
	int  port;
	bool connected;
	struct mosquitto * mqtt;
  unsigned int  pubedTime;
	char gateway_mac[64];

	MQTT_ON_CONNECT			on_connectted;
	MQTT_ON_DISCONNECT	on_disconnectted;
	MQTT_ON_MESSAGE			on_message;
	pthread_t						threadId;
} stMqttEnv_t;

static stMqttEnv_t cb = {0};

#define LOG printf
//#define LOG log_debug
//#define LOG(...) 

static void mqtt_on_connect(struct mosquitto *mosq, void *obj, int rc) {
	LOG("mqtt_on_connect rc=%d\n", rc);

	if (rc == 0 ) {
		LOG("mqtt_on_connected\n");
		cb.connected = true;
	} else if (rc == 4  || rc == 2) {
		LOG("username/password error, exit\n");
		cb.on_connectted();
		exit(1);
	} else {
		cb.connected = false;
	}

	if (cb.on_connectted != NULL) {
		cb.on_connectted();
	}


#if 0
	led_off("errled");
#endif
}

static void mqtt_on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
	LOG("mqtt_on_disconnect %d, errno:%s\n", rc, strerror(errno));

	cb.connected = false;

	if (cb.on_disconnectted != NULL) {
		cb.on_disconnectted();
	}



#if 0
	led_on("errled");
#endif
}

static time_t get_clock_time(void) {   
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec;
} 
static void mqtt_on_publish(struct mosquitto *mosq, void *obj, int mid) {
	LOG("mqtt msg pubed mid=%d\n", mid);
  cb.pubedTime = get_clock_time();
}

static void mqtt_on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message * message) {
	int len = message->payloadlen;
	char * data = message->payload;
	//data[len-1] = '\0';

	char *tbuf = malloc(len + 1);
	if (tbuf == NULL) {
		LOG("No Enough Memory!\n");
		return;
	}
	memcpy(tbuf, data, len);
	tbuf[len] = 0;

	LOG("mqtt on message len:%d\n", len);
	
	LOG("[MQMSG]: %s\n", tbuf);

	if (cb.on_message != NULL) {
		cb.on_message(message->topic, tbuf);
	}

	free(tbuf);
}

static uint32_t mqtt_pubed_time(void) {
    return cb.pubedTime;
}
static uint8_t mqtt_loopThAlive(void) {
	int kill_rc;
	uint8_t ret = 1;

	if (!cb.threadId) {
		LOG("No thread id\n");
		return ret;
	}

	kill_rc = pthread_kill(cb.threadId,0);
	if(kill_rc == ESRCH){
		LOG("the specified thread did not exists or already quit\n");
		ret = 0; 
	}else if(kill_rc == EINVAL){
		LOG("signal is invalid\n");
	}else {
		//LOG("the specified thread is alive\n");
	}
	return ret;

}

void mqtt_check() {
	uint32_t now, pubedTime;
	pubedTime = mqtt_pubed_time();
	now = get_clock_time();
	if ((now - pubedTime) > 43200 && pubedTime > 1532571500) {
		LOG("Last Published Time out time, exit!exit!!exit!!!\n");
		exit(0);
	}

	if (!mqtt_loopThAlive()) {
		LOG("No mqtt loop thread, exit!exit!!exit!!!\n");
		exit(0);
	}
	LOG("mqtt check ok!\n");
}

static void connect_mqtt() {
	char username[100];

	int keepalive = system_get_mqtt_keepalive();
	keepalive = 60;
	LOG("mqtt keep alive is %d\n", keepalive);

	char mqtt_name[128];
	char mqtt_pass[128];
	sprintf(username, "%s-00000001", cb.gateway_mac);
	if (system_get_mqtt_username_password(mqtt_name, mqtt_pass) != 0) {
		strcpy(mqtt_name, username);
		strcpy(mqtt_pass, username);
		system_set_mqtt_username_password(mqtt_name, mqtt_pass);
	} else {
		;
	}
	LOG("name:%s, password:%s\n", mqtt_name, mqtt_pass);
 

	LOG("mqtt sed default argments(name, pass, reconnect_delay, conn_cb, disconn_cb, on_msg, on_pub...\n");
	mosquitto_username_pw_set(cb.mqtt, mqtt_name, mqtt_pass);
	mosquitto_reconnect_delay_set(cb.mqtt, 5,300, false);
	mosquitto_connect_callback_set(cb.mqtt, mqtt_on_connect);
	mosquitto_disconnect_callback_set(cb.mqtt, mqtt_on_disconnect);
	mosquitto_publish_callback_set(cb.mqtt, mqtt_on_publish);
	mosquitto_message_callback_set(cb.mqtt, mqtt_on_message);

#if 0
	char *topic = "local/iot/ziroom/onoffline";
	char *payload = "{\"msgType\":\"DEVICE_DISCONNECT\",\"devId\":\"80987834\", \"prodTypeId\":\"XIYIJI\"}";
	mosquitto_will_set(cb.mqtt,topic, strlen(payload), payload, 0, 0);
#endif

	LOG("connecting to server->%s, port->%d...\n", cb.server, cb.port);
	mosquitto_connect_async(cb.mqtt, cb.server, cb.port, keepalive);

	LOG("mqtt start loop thread...\n");
	int retFunc = mosquitto_loop_start(cb.mqtt);
	if (!retFunc) {
		cb.threadId = mosquitto_loop_threadId(cb.mqtt);
		LOG("Mosquitto Loop thread:%ld", cb.threadId);

#if 0
		pthread_t tid;
		pthread_attr_t attr;
		memset(&attr, 0, sizeof(attr));
		pthread_attr_setdetachstate(&attr,1); 
		pthread_attr_setstacksize(&attr, 2*1024);
		pthread_create(&tid, &attr, mqtt_check_thread, NULL);
		pthread_attr_destroy(&attr);
#else
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool mqtt_init(const char * server, int port, MQTT_ON_CONNECT on_connect, MQTT_ON_DISCONNECT on_disconnect, MQTT_ON_MESSAGE on_message) {
	memset(&cb, 0, sizeof(cb));

	cb.on_connectted = on_connect;
	cb.on_disconnectted = on_disconnect;
	cb.on_message = on_message;

	system_get_mac(cb.gateway_mac, sizeof(cb.gateway_mac));
	LOG("gw mac : %s\n", cb.gateway_mac);

	//sprintf(topic_x, "local/iot/ziroom/+/+/control");
	//sprintf(topic_y, "local/iot/ziroom/broadcast");
	//LOG("topics: [%s] and [%s]\n", topic_x, topic_y);

	strcpy(cb.server, server);
	cb.port = port;
	cb.inited = true;
	mosquitto_lib_init();
	cb.mqtt =  mosquitto_new(cb.gateway_mac, true, NULL);

	//led_on("errled");
	return true;
}

void mqtt_run() {
	connect_mqtt();
}

int mqtt_publish(const char *topic, const char *data, int len) {
	int mid;
	int ret = mosquitto_publish(cb.mqtt, &mid, topic, len, data, 1, 0);
	LOG("mqtt_pub mid=%d, ret:%d\n", mid, ret);
	return ret;
}


int	 mqtt_subscribe(const char *topic, int qos) {
	int mid;
	int ret = mosquitto_subscribe(cb.mqtt, &mid, topic, 1);
	LOG("mqtt_pub mid=%d, ret:%d\n", mid, ret);
	return ret;
}

int mqtt_unsubscribe(const char *topic) {
	int mid;
	int ret = mosquitto_unsubscribe(cb.mqtt, &mid, topic);
	LOG("mqtt_pub mid=%d, ret:%d\n", mid, ret);
	return ret;
}

void mqtt_dump() {
	char buf[4096];
	char * p = buf;

	p += sprintf(p, "###CLOUD DUMP:\n");
	p += sprintf(p, "inited: %d\n", cb.inited);
	p += sprintf(p, "server: %s\n", cb.server);
	p += sprintf(p, "port: %d\n", cb.port);
	p += sprintf(p, "connected: %d\n", cb.connected);

	LOG("%s\n", buf);
}
