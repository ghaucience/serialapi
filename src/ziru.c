#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ziru.h"
#include "mqtt.h"

#include "log.h"

#include "jansson.h"
#include "json_parser.h"
#include "uproto.h"
#include "system.h"

/////////////////////////////////////////////////////////////////////////////////////
// ziru control command cache
typedef struct stZrCtrlCommand {
	char devId[32];
	char prodTypeId[32];
	char sno[64];
	char timestr[32];	
	char attribute[48];
	char command[48];
	char used;
} stZrCtrlCommand_t;

static stZrCtrlCommand_t cmds[10*200];
static int cmd_head = 0;
static int cmd_tail = 0;
static int cmd_cnt = 0;

int ziru_control_command_cache_init() {
	memset(cmds, 0, sizeof(cmds));
	return 0;
}

int ziru_control_command_cache_push(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command){
	int i = 0;
	int cnt = sizeof(cmds)/sizeof(cmds[0]);
	stZrCtrlCommand_t *zc  = NULL;
	for (i = 0; i < cnt; i++) {
		stZrCtrlCommand_t *zi = &cmds[i];
		if (zi->used == 0) {
			zc = zi;
			break;
		}
	}
	if (zc == NULL) {
		return -1;
	}

	strcpy(zc->devId, devId);
	strcpy(zc->prodTypeId, prodTypeId);
	strcpy(zc->sno, sno);
	strcpy(zc->timestr, timestr);
	strcpy(zc->attribute, attribute);
	strcpy(zc->command, command);
	zc->used = 1;
	return 0;
}

int ziru_control_command_cache_pop(const char *uuid, stZrCtrlCommand_t *cmd) {
	int i = 0;
	int cnt = sizeof(cmds)/sizeof(cmds[0]);
	for (i = 0; i < cnt; i++) {
		stZrCtrlCommand_t *zc = &cmds[i];
		if (zc->used == 0) {
			continue;
		}
		if (strcmp(zc->sno, uuid) != 0) {
			continue;
		}
		memcpy(cmd, zc, sizeof (*zc));
		zc->used = 0;
		return  0;
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////
// ziru control table

typedef int				(*ATTR_COMMAND_ACTION_FUNC)(
	const char *devId,
	const char *prodTypeId,
	const char *sno, 
	const char *timestr,
	const char *attribute,
	const char *command,
	json_t		 *jdata);

typedef struct stAttrCommandAction {
	const char								*attribute;
	const char								*command;
	ATTR_COMMAND_ACTION_FUNC	 func;
} stAttrCommandAction_t;


int touchswitch_switchstate_set_switchstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int metersocket_switchstate_set_switchstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int zigbee_netstate_set_netstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int lock_network_add_lock_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int lock_password_delete_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);
int lock_password_add_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);
int lock_password_reset_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);
int lock_password_clear_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int lock_seed_set_seed_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);

int winctr_switchstate_set_switchstate(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata);


static stAttrCommandAction_t acas[] = {
	{"touchswitch_switchstate", "set_switchstate", touchswitch_switchstate_set_switchstate_func},

	{"metersocket_switchstate",	"set_switchstate", metersocket_switchstate_set_switchstate_func},

	{"zigbee_netstate",					"set_netstate",		 zigbee_netstate_set_netstate_func},

	{"lock_network",						"add_lock",				 lock_network_add_lock_func},

	{"lock_password",						"delete_password", lock_password_delete_password_func},
	{"lock_password",						"add_password",		 lock_password_add_password_func},
	{"lock_password",						"reset_password",  lock_password_reset_password_func},
	{"lock_password",						"clear_password",  lock_password_clear_password_func},
	
	{"lock_seed",								"set_seed",				 lock_seed_set_seed_func},

	{"winctr_switchstate",			"set_swtichstate",	winctr_switchstate_set_switchstate},
};


stAttrCommandAction_t*	AttrCmdActionSearch(const char *attribute, const char *command) {
	int cnt = sizeof(acas)/sizeof(acas[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stAttrCommandAction_t *aca = &acas[i];
		if (strcmp(aca->attribute, attribute) != 0) {
			continue;
		}
		if (strcmp(aca->command, command) != 0) {
			continue;
		}

		return aca;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
static void ziru_on_connect() {
	log_info("on connect .");

	//log_info("subcribe to [local/iot/ziroom/+/+/control]");
	//mqtt_subscribe("local/iot/ziroom/+/+/control", 1);

	log_info("subcribe to [local/iot/ziroom/broadcast]");
	mqtt_subscribe("local/iot/ziroom/broadcast", 1);

	//log_info("subcribe to [t]");
	//mqtt_subscribe("t", 1);
}

static void ziru_on_disconnect() {
	log_info("on diconnect .");
}

static void ziru_parse_protypeid(const char *topic, char *buf) {
	buf[0] = 0;

	char *p1 = strstr(topic, "/ziroom/");
	if (p1 == NULL) {
		return;
	}

	p1 += strlen("/ziroom/") + 1;

	char *p2 = strstr(p1, "/");
	if (p2 == NULL) {
		return;
	}
	
	memcpy(buf, p1, p2 - p1);
}
static void ziru_parse_did(const char *topic, char *buf) {
	buf[0] = 0;

	char *p1 = strstr(topic, "/ziroom/");
	if (p1 == NULL) {
		return;
	}

	p1 += strlen("/ziroom/") + 1;

	p1 = strstr(p1, "/");
	p1 += 1;

	char *p2 = strstr(p1, "/");
	if (p2 == NULL) {
		return;
	}
	
	memcpy(buf, p1, p2 - p1);
}

void ziru_on_message(const char *topic, const char *msg) {
	log_info("on message, topic:%s, context:[%s]", topic, msg);

	json_error_t error;
	json_t *jmsg = json_loads(msg, 0, &error);
	if (jmsg == NULL) {
		log_warn("error: not correct json format:%s", msg);
		return;
	}

	char protypeid[128]; protypeid[0] = 0;
	char did[128];			 did[0] = 0;
	if (strcmp(topic, "local/iot/ziroom/broadcast") == 0) {
		;
	} else {
		ziru_parse_protypeid(topic, protypeid);
		ziru_parse_did(topic, did);
	}

	const char *msgType = json_get_string(jmsg, "msgType");

	if (strcmp(msgType, "DEVICE_INFO_QUERY") == 0) {
		/*
		if (strcmp(protypeid, "") == 0) {
			log_warn("null protypeid:[%s]", protypeid);
			json_decref(jmsg);
			return;
		} 
		if (strcmp(did, "") == 0) {
			log_warn("null did:[%s]", did);
			json_decref(jmsg);
			return;
		}
		*/
		ziru_info_query(protypeid, did, jmsg);
	} else if (strcmp(msgType, "DEVICE_CONTROL") == 0) {
		ziru_control(protypeid, did, jmsg);
	} else if (strcmp(msgType, "DEVICE_UNKNOWN") == 0) {
		log_warn("error : gateway report unknown message Type:");
		json_decref(jmsg);
		return;
	} else {
		ziru_unknown(msgType, did, jmsg);
		log_warn("error: not support communicate command:%s", msgType);
		json_decref(jmsg);
		return;
	}
	json_decref(jmsg);
}


int ziru_init(const char *server, int port) {
	mqtt_init(server, port, 
									ziru_on_connect, 
									ziru_on_disconnect, 
									ziru_on_message);
	mqtt_run();
	return 0;
}

int ziru_connect(const char *devId, const char *prodTypeId) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	
	//json_object_set_new(jmsg, "msgType",		json_string("DEVICE_CONNECT"));
	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_INFO_REPORT"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -2;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);

	char topicx[256];
	sprintf(topicx, "local/iot/ziroom/%s/%s/control", prodTypeId, devId);
	log_info("subcribe to %s", topicx);
	mqtt_subscribe(topicx, 1);
	
	return 0;
}
int ziru_disconnect(const char *devId, const char *prodTypeId) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	
	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_DISCONNECT"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -2;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	char topicx[256];
	sprintf(topicx, "local/iot/ziroom/%s/%s/control", prodTypeId, devId);
	log_info("subcribe to %s", topicx);
	mqtt_unsubscribe(topicx);
	

	return 0;
}
int ziru_info_report(const char *devId, const char *prodTypeId) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	 time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	
	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_INFO_REPORT"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -2;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	return 0;
}
int ziru_control_resp(const char *devId, const char *prodTypeId, const char *sno, const char *attribute, const char *command, const char *key, const char *value) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	 time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	

	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_CONTROL_RESP"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));
	json_object_set_new(jmsg, "time",				json_string(timestr));
	json_object_set_new(jmsg, "sno",				json_string(sno));
	json_object_set_new(jmsg, "attribute",	json_string(attribute));
	
	char commandresp[64];
	sprintf(commandresp, "%s_resp", command);
	json_object_set_new(jmsg, "command",		json_string(commandresp));

	json_t *ja = json_array();
	if (ja == NULL) {
		log_warn("no enough memory! 2");
		json_decref(jmsg);
		return -2;
	}
	json_t *ji = json_object();
	if (ji == NULL ) {
		log_warn("no enuough memory! 3");
		json_decref(jmsg);
		json_decref(ja);
		return -3;
	}
	json_object_set_new(ji, "k", json_string(key));
	json_object_set_new(ji, "v", json_string(value));
	json_array_append_new(ja, ji);
	json_object_set_new(jmsg, "data", ja);

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -4;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	return 0;
}
int ziru_notify(const char *devId, const char *prodTypeId, const char *attribute, const char *command, const char *key, const char *value) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	 time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	
	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_NOTIFY"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));
	json_object_set_new(jmsg, "time",				json_string(timestr));
	json_object_set_new(jmsg, "sno",				json_string("0"));
	json_object_set_new(jmsg, "attribute",	json_string(attribute));
	json_object_set_new(jmsg, "command",		json_string(command));
	json_object_set_new(jmsg, "catelog",		json_string(""));

	json_t *ja = json_array();
	if (ja == NULL) {
		log_warn("no enough memory! 2");
		json_decref(jmsg);
		return -2;
	}
	json_t *ji = json_object();
	if (ji == NULL ) {
		log_warn("no enuough memory! 3");
		json_decref(jmsg);
		json_decref(ja);
		return -3;
	}
	json_object_set_new(ji, "k", json_string(key));
	json_object_set_new(ji, "v", json_string(value));
	json_array_append_new(ja, ji);
	json_object_set_new(jmsg, "data", ja);

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -4;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	return 0;
}


int ziru_notify_array(const char *devId, const char *prodTypeId, const char *attribute, const char *command, const char **keys, const char **values, int cnt) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	struct tm t_tm;
	 time_t t_tt = time(NULL);
	char timestr[64];
	gmtime_r(&t_tt,&t_tm);
	sprintf(timestr, "%d-%d-%d %d:%d:%d",  t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday, t_tm.tm_hour, t_tm.tm_min, t_tm.tm_sec);
	
	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_NOTIFY"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));
	json_object_set_new(jmsg, "time",				json_string(timestr));
	json_object_set_new(jmsg, "sno",				json_string("0"));
	json_object_set_new(jmsg, "attribute",	json_string(attribute));
	json_object_set_new(jmsg, "command",		json_string(command));
	json_object_set_new(jmsg, "catelog",		json_string(""));

	json_t *ja = json_array();
	if (ja == NULL) {
		log_warn("no enough memory! 2");
		json_decref(jmsg);
		return -2;
	}

	int i = 0;
	for (i = 0; i < cnt; i++) {
		json_t *ji = json_object();
		if (ji == NULL ) {
			log_warn("no enuough memory! 3");
			json_decref(jmsg);
			json_decref(ja);
			return -3;
		}
		json_object_set_new(ji, "k", json_string(keys[i]));
		json_object_set_new(ji, "v", json_string(values[i]));
		json_array_append_new(ja, ji);
	}
	json_object_set_new(jmsg, "data", ja);

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -4;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	return 0;
}


int ziru_meta(const char *devId, const char *prodTypeId) {
	json_t *jmsg = json_object();
	if (jmsg == NULL) {
		log_warn("no enough memory! 1");
		return -1;
	}

	char timestr[64];

	json_object_set_new(jmsg, "msgType",		json_string("DEVICE_META"));
	json_object_set_new(jmsg, "devId",			json_string(devId));
	json_object_set_new(jmsg, "prodTypeId",	json_string(prodTypeId));

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		log_warn("json dumps error!");
		json_decref(jmsg);
		return -2;
	}
	json_decref(jmsg);
	
	char topic[256];
	sprintf(topic, "local/iot/ziroom/%s/%s/notify", prodTypeId, devId);
	log_info("publish to [%s] : %s", topic, smsg);
	mqtt_publish(topic, smsg, strlen(smsg)+1);

	free(smsg);
	
	return 0;
}



int ziru_info_query(const char *pid, const char *did, json_t *jmsg) {
	json_t *jarg = json_object();
	char mac[32];
	system_get_mac(mac, sizeof(mac));
	json_t *jret = uproto_call(mac, "mod.device_list", "getAttribute",  jarg, 0, "ziru_info_query");
	if (jret != NULL) {
		json_decref(jret);
	}
	return 0;
}

int ziru_control(const char *pid, const char *did, json_t *jmsg) {
	const char *devId				= json_get_string(jmsg, "devId");
	const char *prodTypeId	= json_get_string(jmsg, "prodTypeId");
	const char *sno					= json_get_string(jmsg, "sno");
	const char *timestr			= json_get_string(jmsg, "time");
	const char *attribute		= json_get_string(jmsg, "attribute");
	const char *command			= json_get_string(jmsg, "command");
	json_t *jdata						= json_object_get(jmsg, "data");

	log_info("devId:%s, prodTypeId:%s, sno:%s, time:%s, attribute:%s, command:%s, jdata:%p",
						devId, prodTypeId, sno, timestr, attribute, command, jdata);
	if (devId == NULL || prodTypeId == NULL || sno == NULL || timestr == NULL || attribute == NULL || command == NULL || jdata == NULL) {
		log_warn("NULL argments!");
		return -1;
	}
	if (!json_is_array(jdata)) {
		log_warn("data is not a json array!");
		return -2;
	}

#if 1
	stAttrCommandAction_t *aca = AttrCmdActionSearch(attribute, command);
	if (aca == NULL) {
		log_warn("not support attribute:%s or command:%s", attribute, command);
		return -3;
	}

	if (aca->func == NULL) {
		log_warn("NULL Action Function for attr : %s, command:%s", attribute, command);
		return -4;
	}

	int ret = aca->func(devId, prodTypeId, sno, timestr, attribute, command, jdata);
	if (ret != 0) {
		log_warn("Action for attr:%s, command:%s failed: %d", attribute, command, ret);
		return -5;
	}
	
	log_info("attribute :%s, command: %s, execute ok: %d", attribute, command, ret);

	return 0;
#else
	if (strcmp(attribute, "touchswitch_switchstate") == 0) {
		if (strcmp(command, "set_switchstate") == 0) {
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				
				if (strcmp(k, "switchstate") == 0) {
					int ep		= (v[0] - '0')&0xf;
					int value = (v[1] - '0')&0xf;

					log_info("switch onff %s, ep:%d, value:%d", devId, ep, value);

					json_t *jarg = json_object();
					json_object_set_new(jarg, "ep", json_integer(ep));
					json_object_set_new(jarg, "value", json_integer(value));

					ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
					uproto_call(devId, "device.switch.onoff", "setAttribute", jarg, 0, sno);
				} else {
					log_warn("not support k:%s", k);
					continue;
				}
			}
		} else {
			log_warn("not support command:%s, 1", command);
		}
	} else if (strcmp(attribute, "metersocket_switchstate") == 0) {
		if (strcmp(command, "set_switchstate") == 0) {
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				
				if (strcmp(k, "switchstate") == 0) {
					int ep		= (v[0] - '0')&0xf;
					int value = (v[1] - '0')&0xf;

					log_info("switch onff %s, ep:%d, value:%d", devId, ep, value);

					json_t *jarg = json_object();
					json_object_set_new(jarg, "ep", json_integer(ep));
					json_object_set_new(jarg, "value", json_integer(value));

					ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
					uproto_call(devId, "device.switch.onoff", "setAttribute", jarg, 0, sno);
				} else {
					log_warn("not support k:%s", k);
					continue;
				}
			}
		} else {
			log_warn("not support command:%s, 2", command);
		}
	} else if (strcmp(attribute, "zigbee_netstate") == 0) {
		if (strcmp(command, "set_netstate") == 0) {
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}

				if (strcmp(k, "timeout") == 0) {
					int value = atoi(v);

					log_info("permit join for:%d", value);

					json_t *jarg = json_object();
					json_object_set_new(jarg, "mac", json_string("ffffffffffffffff"));
					json_object_set_new(jarg, "type", json_string("0000"));

					//ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
					uproto_call(devId, "mod.add_device", "setAttribute", jarg, 0, sno);
					ziru_control_resp("0000000000000000", "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
				} else {
					log_warn("not support k:%s", k);
					continue;
				}
			}
		} else {
			log_warn("not support command:%s, 3", command);
		}
	} else if (strcmp(attribute, "lock_network") == 0) {
		if (strcmp(command, "add_lock") == 0) {
			int		index = -1;
			char	mac[32] = {0};
		
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "index") == 0) {
					index = atoi(v);
				} else if (strcmp(k, "mac") == 0) {
					strcpy(mac, v);
				}
			}
			if (index < 0) {
				log_warn("error index");
			} else if (strlen(mac) < 16) {
				log_warn("erro mac len");
			} else {
				log_info("permit join for:%d", index);

				json_t *jarg = json_object();
				json_object_set_new(jarg, "mac", json_string("ffffffffffffffff"));
				json_object_set_new(jarg, "type", json_string("0000"));

				//ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				uproto_call(devId, "mod.add_device", "setAttribute", jarg, 0, sno);
				ziru_control_resp("0000000000000000", "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
			}
		} else {
			log_warn("not support command:%s, 4", command);
		}
	} else if (strcmp(attribute, "lock_password") == 0) {
		if (strcmp(command, "delete_password") == 0) {
			int idx;
			json_t *jval;

			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "index") != 0) {
					log_warn("nor expect k:%s", k);
					continue;
				}
				int index = atoi(v);
				if (index < 0 || index > 99) {
					log_warn("error index value:%d", index);
					continue;
				}

				log_info("lock del password, mac:%s, index: %d", devId, index);

				json_t *jarg = json_object();
				json_object_set_new(jarg, "passId", json_integer(index));
				json_object_set_new(jarg, "clearAll", json_integer(0));

				ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				uproto_call(devId, "device.lock.del_password", "setAttribute", jarg, 0, sno);
			}
		}  else if (strcmp(command, "add_password") == 0) {
			int index = -1;
			int starttime = -1;
			int deadline = -1;
			int content = -1;
			int status = -1;

			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "index") == 0) {
					index = atoi(v);
				} else if (strcmp(k, "starttime") == 0) {
					starttime = atoi(v);
				} else if (strcmp(k, "status") == 0) {
					status = atoi(v);
				} else if (strcmp(k, "deadline") == 0) {
					deadline = atoi(v);
				} else if (strcmp(k, "content") == 0) {
					content = atoi(v);
				} else {
					continue;
				}
			}

			log_info("lock add password, mac:%s, index:%d, starttime:%d, status:%d, deadline:%d, content:%d",
							devId, index, starttime, status, deadline, content);
			if (index == -1 || starttime == -1 || status == -1 || deadline == -1 || content == -1 || status == -1 || index >= 99)  {
				log_warn("invalid argments!");
			} else {
				if (status == 1) { // 禁止 , suspend = 1
					status = 1;
				} else {					// 使用， suspend = 0
					status = 0;
				}

				json_t *jarg = json_object();
				json_object_set_new(jarg, "passType", json_integer(0));
				json_object_set_new(jarg, "passId", json_integer(index));
				json_object_set_new(jarg, "passVal1", json_integer(content));
				json_object_set_new(jarg, "passVal2", json_integer(0));
				json_object_set_new(jarg, "startTime", json_integer(starttime));
				json_object_set_new(jarg, "endTime", json_integer(deadline));
				json_object_set_new(jarg, "suspend", json_integer(status));

				ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
			} 
		} else if (strcmp(command, "reset_password") == 0) {
			int index = -1;
			int starttime = -1;
			int deadline = -1;
			int content = -1;
			int status = -1;

			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "index") == 0) {
					index = atoi(v);
				} else if (strcmp(k, "starttime") == 0) {
					starttime = atoi(v);
				} else if (strcmp(k, "status") == 0) {
					status = atoi(v);
				} else if (strcmp(k, "deadline") == 0) {
					deadline = atoi(v);
				} else if (strcmp(k, "content") == 0) {
					content = atoi(v);
				} else {
					continue;
				}
			}

			log_info("lock add password, mac:%s, index:%d, starttime:%d, status:%d, deadline:%d, content:%d",
							devId, index, starttime, status, deadline, content);
			if ((index == -1 || index > 99) || ((starttime == -1 && deadline == -1) &&  status == -1  && content == -1))  {
				log_warn("invalid argments!");
			} else {
				if (status == 1) { // 禁止 , suspend = 1
					status = 1;
				} else if (status == -1) {					// 使用， suspend = 0
					status = -1;
				} else {
					status = 0;
				}
				



				json_t *jarg = json_object();
				json_object_set_new(jarg, "passType", json_integer(0));
				json_object_set_new(jarg, "passId", json_integer(index));
				json_object_set_new(jarg, "passVal1", json_integer(content));
				json_object_set_new(jarg, "passVal2", json_integer(0));
				json_object_set_new(jarg, "startTime", json_integer(starttime));
				json_object_set_new(jarg, "endTime", json_integer(deadline));
				json_object_set_new(jarg, "suspend", json_integer(status ));

				ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				//uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
				uproto_call(devId, "device.lock.z3.modpin", "setAttribute", jarg, 0, sno);
			}
		} else if (strcmp(command, "clear_password") == 0) {
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
			}

			log_info("lock clr password, mac:%s",devId);

			json_t *jarg = json_object();

			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			//uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
			uproto_call(devId, "device.lock.z3.clrpin", "setAttribute", jarg, 0, sno);
		} else {
			log_warn("not support command:%s, 7", command);
		}
	} else if (strcmp(attribute, "lock_seed") == 0) {
		if (strcmp(command, "set_seed") == 0) {
			int seed = -1;

			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "seed") == 0) {
					seed = atoi(v);
				}
			}

			log_info("lock add dynamic, mac:%s, seed:%d", devId, seed);

			if (seed == -1) {
				log_warn("invalid seed");
			} else {
				json_t *jarg = json_object();
				int index = 99;
				int starttime = time(NULL);
				int deadline = starttime + 3600 * 24 * 365 * 10;
				json_object_set_new(jarg, "passType", json_integer(1));
				json_object_set_new(jarg, "passId", json_integer(99));
				json_object_set_new(jarg, "passVal1", json_integer(seed));
				json_object_set_new(jarg, "passVal2", json_integer(0));
				json_object_set_new(jarg, "startTime", json_integer(starttime));
				json_object_set_new(jarg, "endTime", json_integer(deadline));
				json_object_set_new(jarg, "suspend", json_integer(0));

				ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
			}
		} else {
			log_warn("not support command:%s, 8", command);
		}
	} else {
		log_warn("not support attribute:%s", attribute);
	}
#endif

	return 0;
}

int ziru_unknown(const char *pid, const char *did, json_t *jmsg) {
	log_warn("can't parsed message!");
	return 0;
}

int ziru_control_cmd_result(const char *uuid, int code) {
	log_info("control result , uuid:%s, code : %d", uuid, code);

	if (code == 99) {
		log_info("do't care 99 code");
		return -1;
	}
	stZrCtrlCommand_t zc;
	int ret = ziru_control_command_cache_pop(uuid, &zc);
	if (ret != 0) {
		log_warn("not cached command: %s", uuid);
		return -1;
	}
	
	char codestr[16];
	sprintf(codestr, "%d", code);
	ziru_control_resp(zc.devId, zc.prodTypeId, zc.sno, zc.attribute, zc.command, "code", codestr);
	return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
int touchswitch_switchstate_set_switchstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}

		if (strcmp(k, "switchstate") == 0) {
			int ep		= (v[0] - '0')&0xf;
			int value = (v[1] - '0')&0xf;

			log_info("switch onff %s, ep:%d, value:%d", devId, ep, value);

			json_t *jarg = json_object();
			json_object_set_new(jarg, "ep", json_integer(ep));
			json_object_set_new(jarg, "value", json_integer(value));

			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			uproto_call(devId, "device.switch.onoff", "setAttribute", jarg, 0, sno);
		} else {
			log_warn("not support k:%s", k);
			continue;
		}
	}

	return 0;
}

int metersocket_switchstate_set_switchstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}

		if (strcmp(k, "switchstate") == 0) {
			int ep		= (v[0] - '0')&0xf;
			int value = (v[1] - '0')&0xf;

			log_info("switch onff %s, ep:%d, value:%d", devId, ep, value);

			json_t *jarg = json_object();
			json_object_set_new(jarg, "ep", json_integer(ep));
			json_object_set_new(jarg, "value", json_integer(value));

			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			uproto_call(devId, "device.switch.onoff", "setAttribute", jarg, 0, sno);
		} else {
			log_warn("not support k:%s", k);
			continue;
		}
	}

	return 0;
}

int zigbee_netstate_set_netstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}

		if (strcmp(k, "timeout") == 0) {
			int value = atoi(v);

			log_info("permit join for:%d", value);

			json_t *jarg = json_object();
			json_object_set_new(jarg, "mac", json_string("ffffffffffffffff"));
			json_object_set_new(jarg, "type", json_string("0000"));

			//ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			uproto_call(devId, "mod.add_device", "setAttribute", jarg, 0, sno);

			char coordname[32];
			coordname[0] = 0;
			system_get_coord_mac(coordname, sizeof(coordname));

			//ziru_control_resp("0000000000000000", "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
			ziru_control_resp(coordname, "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
		} else {
			log_warn("not support k:%s", k);
			continue;
		}
	}

	return 0;
}

int lock_network_add_lock_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
			int		index = -1;
			char	mac[32] = {0};
		
			int idx;
			json_t *jval;
			json_array_foreach(jdata, idx, jval) {
				const char *k = json_get_string(jval, "k");
				const char *v = json_get_string(jval, "v");
				if (k == NULL || v == NULL) {
					log_warn("error k:%s or v:%s", k, v);
					continue;
				}
				if (strcmp(k, "index") == 0) {
					index = atoi(v);
				} else if (strcmp(k, "mac") == 0) {
					strcpy(mac, v);
				}
			}
			if (index < 0) {
				log_warn("error index");
			} else if (strlen(mac) < 16) {
				log_warn("erro mac len");
			} else {
				log_info("permit join for:%d", index);

				json_t *jarg = json_object();
				json_object_set_new(jarg, "mac", json_string("ffffffffffffffff"));
				json_object_set_new(jarg, "type", json_string("0000"));

				//ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
				uproto_call(devId, "mod.add_device", "setAttribute", jarg, 0, sno);
				char coordname[32];
				coordname[0] = 0;
				system_get_coord_mac(coordname, sizeof(coordname));


				//ziru_control_resp("0000000000000000", "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
				ziru_control_resp(coordname, "Shuncom_zigbeeGate", sno, attribute, command, "code", "0");
			}
	
	return 0;
}

int lock_password_delete_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;

	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}
		if (strcmp(k, "index") != 0) {
			log_warn("nor expect k:%s", k);
			continue;
		}
		int index = atoi(v);
		if (index < 0 || index > 99) {
			log_warn("error index value:%d", index);
			continue;
		}

		log_info("lock del password, mac:%s, index: %d", devId, index);

		json_t *jarg = json_object();
		json_object_set_new(jarg, "passId", json_integer(index));
		json_object_set_new(jarg, "clearAll", json_integer(0));

		ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
		uproto_call(devId, "device.lock.del_password", "setAttribute", jarg, 0, sno);
	}

	return 0;
}

int lock_password_add_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int index = -1;
	int starttime = -1;
	int deadline = -1;
	int content = -1;
	int status = -1;

	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}
		if (strcmp(k, "index") == 0) {
			index = atoi(v);
		} else if (strcmp(k, "starttime") == 0) {
			starttime = atoi(v);
		} else if (strcmp(k, "status") == 0) {
			status = atoi(v);
		} else if (strcmp(k, "deadline") == 0) {
			deadline = atoi(v);
		} else if (strcmp(k, "content") == 0) {
			content = atoi(v);
		} else {
			continue;
		}
	}

	log_info("lock add password, mac:%s, index:%d, starttime:%d, status:%d, deadline:%d, content:%d",
			devId, index, starttime, status, deadline, content);
	if (index == -1 || starttime == -1 || status == -1 || deadline == -1 || content == -1 || status == -1 || index >= 99)  {
		log_warn("invalid argments!");
	} else {
		if (status == 1) { // 禁止 , suspend = 1
			status = 1;
		} else {					// 使用， suspend = 0
			status = 0;
		}

		json_t *jarg = json_object();
		json_object_set_new(jarg, "passType", json_integer(0));
		json_object_set_new(jarg, "passId", json_integer(index));
		json_object_set_new(jarg, "passVal1", json_integer(content));
		json_object_set_new(jarg, "passVal2", json_integer(0));
		json_object_set_new(jarg, "startTime", json_integer(starttime));
		json_object_set_new(jarg, "endTime", json_integer(deadline));
		json_object_set_new(jarg, "suspend", json_integer(status));

		ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
		uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
	} 

	return 0;
}
int lock_password_reset_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int index = -1;
	int starttime = -1;
	int deadline = -1;
	int content = -1;
	int status = -1;

	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}
		if (strcmp(k, "index") == 0) {
			index = atoi(v);
		} else if (strcmp(k, "starttime") == 0) {
			starttime = atoi(v);
		} else if (strcmp(k, "status") == 0) {
			status = atoi(v);
		} else if (strcmp(k, "deadline") == 0) {
			deadline = atoi(v);
		} else if (strcmp(k, "content") == 0) {
			content = atoi(v);
		} else {
			continue;
		}
	}

	log_info("lock add password, mac:%s, index:%d, starttime:%d, status:%d, deadline:%d, content:%d",
			devId, index, starttime, status, deadline, content);
	if ((index == -1 || index > 99) || ((starttime == -1 && deadline == -1) &&  status == -1  && content == -1))  {
		log_warn("invalid argments!");
	} else {
		if (status == 1) { // 禁止 , suspend = 1
			status = 1;
		} else if (status == -1) {					// 使用， suspend = 0
			status = -1;
		} else {
			status = 0;
		}

		json_t *jarg = json_object();
		json_object_set_new(jarg, "passType", json_integer(0));
		json_object_set_new(jarg, "passId", json_integer(index));
		json_object_set_new(jarg, "passVal1", json_integer(content));
		json_object_set_new(jarg, "passVal2", json_integer(0));
		json_object_set_new(jarg, "startTime", json_integer(starttime));
		json_object_set_new(jarg, "endTime", json_integer(deadline));
		json_object_set_new(jarg, "suspend", json_integer(status ));

		ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
		//uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
		uproto_call(devId, "device.lock.z3.modpin", "setAttribute", jarg, 0, sno);
	}

	return 0;
}
int lock_password_clear_password_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}
	}

	log_info("lock clr password, mac:%s",devId);

	json_t *jarg = json_object();

	ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
	//uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
	uproto_call(devId, "device.lock.z3.clrpin", "setAttribute", jarg, 0, sno);

	return 0;
}

int lock_seed_set_seed_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int seed = -1;

	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}
		if (strcmp(k, "seed") == 0) {
			seed = atoi(v);
		}
	}

	log_info("lock add dynamic, mac:%s, seed:%d", devId, seed);

	if (seed == -1) {
		log_warn("invalid seed");
	} else {
		{
		json_t *jarg = json_object();
		int index = 99;
		int starttime = time(NULL);
		int deadline = starttime + 3600 * 24 * 365 * 10;
		json_object_set_new(jarg, "passType", json_integer(1));
		json_object_set_new(jarg, "passId", json_integer(99));
		json_object_set_new(jarg, "passVal1", json_integer(seed));
		json_object_set_new(jarg, "passVal2", json_integer(0));
		json_object_set_new(jarg, "startTime", json_integer(starttime));
		json_object_set_new(jarg, "endTime", json_integer(deadline));
		json_object_set_new(jarg, "suspend", json_integer(0));

			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			log_info("Del Dynaic Seed!!");
			uproto_call(devId, "device.lock.del_password", "setAttribute", jarg, 0, "111");
		}
		{
			json_t *jarg = json_object();
			int index = 99;
			int starttime = time(NULL);
			int deadline = starttime + 3600 * 24 * 365 * 10;
			json_object_set_new(jarg, "passType", json_integer(1));
			json_object_set_new(jarg, "passId", json_integer(99));
			json_object_set_new(jarg, "passVal1", json_integer(seed));
			json_object_set_new(jarg, "passVal2", json_integer(0));
			json_object_set_new(jarg, "startTime", json_integer(starttime));
			json_object_set_new(jarg, "endTime", json_integer(deadline));
			json_object_set_new(jarg, "suspend", json_integer(0));


			log_info("Add Dynaic Seed!!");
			uproto_call(devId, "device.lock.add_password", "setAttribute", jarg, 0, sno);
		}
	}

	return 0;
}


int touchswitch_switchstate_set_switchstate_func(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
	int idx;
	json_t *jval;
	json_array_foreach(jdata, idx, jval) {
		const char *k = json_get_string(jval, "k");
		const char *v = json_get_string(jval, "v");
		if (k == NULL || v == NULL) {
			log_warn("error k:%s or v:%s", k, v);
			continue;
		}

		if (strcmp(k, "switchstate") == 0) {
			int ep		= (v[0] - '0')&0xf;
			int value = (v[1] - '0')&0xf;

			log_info("switch onff %s, ep:%d, value:%d", devId, ep, value);

			json_t *jarg = json_object();
			json_object_set_new(jarg, "ep", json_integer(ep));
			json_object_set_new(jarg, "value", json_integer(value));

			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			uproto_call(devId, "device.switch.onoff", "setAttribute", jarg, 0, sno);
		} else {
			log_warn("not support k:%s", k);
			continue;
		}
	}

	return 0;
}

int winctr_switchstate_set_switchstate(const char *devId, const char *prodTypeId, const char *sno, const char *timestr, const char *attribute, const char *command, json_t		 *jdata) {
		/**Parse Input argment */

		/*
	  if (OK){
			int value = 0; //0, 1, 2, parse from jdata
			json_t *jarg = json_object();
			json_object_set_new(jarg, "ep", json_integer(ep));
			json_object_set_new(jarg, "value", json_integer(value));


			ziru_control_command_cache_push(devId, prodTypeId, sno, timestr, attribute, command);
			uproto_call(devId, "device.windowcovering", "setAttribute", jarg, 0, sno);
		} else {
			warning something 
		}
		*/
}




