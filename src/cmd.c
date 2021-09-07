#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "parse.h"
#include "file_event.h"
#include "jansson.h"
#include "json_parser.h"
#include "hex.h"

#include "cmd.h"

#include "zwave.h"
#include "zwave_device.h"


void do_cmd_exit(char *argv[], int argc);
void do_cmd_help(char *argv[], int argc);
void do_cmd_version(char *argv[], int argc);

void do_cmd_addpass(char *argv[], int argc);
void do_cmd_delpass(char *argv[], int argc);
void do_cmd_modpass(char *argv[], int argc);
void do_cmd_setseed(char *argv[], int argc);
void do_cmd_clrpass(char *argv[], int argc);


void do_cmd_include(char *argv[], int argc);
void do_cmd_exclude(char *argv[], int argc);
void do_cmd_zwversion(char *argv[], int argc);
void do_cmd_list(char *argv[], int argc);
void do_cmd_lists(char *argv[], int argc);
void do_cmd_view(char *argv[], int argc);
void do_cmd_remove(char *argv[], int argc);

void do_cmd_reboot(char *argv[], int argc);
void do_cmd_region(char *argv[], int argc);
static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"help", do_cmd_help, "help info"},
	{"version", do_cmd_version, "display version"},

	{"addpass", do_cmd_addpass, "add password"},
	{"delpass", do_cmd_delpass, "del password"},
	{"modpass", do_cmd_modpass, "mod password"},
	{"setseed",	do_cmd_setseed, "set seed"},
	{"clrpass",	do_cmd_clrpass, "clr password"},

	{"include",	do_cmd_include, "include dev"},
	{"exclude",	do_cmd_exclude, "exclude dev"},
	{"remove",  do_cmd_remove,  "remove dev"},
	{"zwversion",	do_cmd_zwversion, "version master"},
	{"list",		do_cmd_list, "list dev"},
	{"lists",   do_cmd_lists, "list dev simple"},
	{"view",		do_cmd_view,	"view dev"},
	{"reboot",	do_cmd_reboot, "reboot"},
	{"region",		do_cmd_region, "region"},
};


static stCmdEnv_t ce;

int cmd_init(void *_th, void *_fet) {
	ce.th = _th;
	ce.fet = _fet;
	
	timer_init(&ce.step_timer, cmd_run);
	lockqueue_init(&ce.eq);
	
	ce.fd = 0;
	file_event_reg(ce.fet, ce.fd, cmd_in, NULL, NULL);

	return 0;
}
int cmd_step() {
	timer_cancel(ce.th, &ce.step_timer);
	timer_set(ce.th, &ce.step_timer, 10);
	return 0;
}
int cmd_push(stEvent_t *e) {
	lockqueue_push(&ce.eq, e);
	cmd_step();
	return 0;
}
void cmd_run(struct timer *timer) {
	stEvent_t *e;

	if (!lockqueue_pop(&ce.eq, (void**)&e)) {
		return;
	}

	if (e != NULL) {
		FREE(e);
	}

	cmd_step();
}
void cmd_in(void *arg, int fd) {
	char buf[1024];	
	int  size = 0;

	int ret = read(ce.fd, buf, sizeof(buf));
	if (ret < 0) {
		log_debug("what happend?");
		return;
	}

	if (ret == 0) {
		log_debug("error!");
		return;
	}

	size = ret;
	buf[size] = 0;
	if (size >= 1 && buf[size-1] == '\n') {
		buf[size-1] = 0;
		size--;
	}
	char *p = buf;
	while (*p == ' ') {
		p++;
		size--;
	}
	if (size > 0) {
		memcpy(buf, p, size);
	} else {
		buf[0] = 0;
		size = 0;
	}

	if (strcmp(buf, "") != 0) {
		log_debug("console input:[%s]", buf);
		char* argv[20];
		int argc;
		argc = parse_argv(argv, sizeof(argv), buf);

		stCmd_t *cmd = cmd_search(argv[0]);
		if (cmd == NULL) {
			log_debug("invalid cmd!");
		} else {
			cmd->func(argv, argc);
		}
	}
	log_debug("$");
}

stCmd_t *cmd_search(const char *cmd) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		if (strcmp(cmds[i].name, cmd) == 0) {
			return &cmds[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////
void do_cmd_exit(char *argv[], int argc) {
	exit(0);
}
void do_cmd_help(char *argv[], int argc) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		log_debug("%-12s\t-\t%s", cmds[i].name, cmds[i].desc);
	}
}
void do_cmd_version(char *argv[], int argc) {
	log_debug("Version:%d.%d.%d, DateTime:%s %s", MAJOR, MINOR, PATCH, TIME, DATE);
}

void do_cmd_addpass(char *argv[], int argc) {
}

void do_cmd_delpass(char *argv[], int argc) {
}
void do_cmd_modpass(char *argv[], int argc) {
}
void do_cmd_setseed(char *argv[], int argc){
}
void do_cmd_clrpass(char *argv[], int argc){
#if 0
	if (argc != 2) {
		log_warn("valid error : <mac> ");
		return;
	}
	char *mac   = argv[1];

	json_t *arg = json_object();
	json_object_set_new(arg, "msgType",			json_string("DEVICE_CONTROL"));
	json_object_set_new(arg, "devId",				json_string(mac));
	json_object_set_new(arg, "prodTypeId",	json_string("S3"));
	json_object_set_new(arg, "time",				json_string("2018-05-14 10:10:10"));
	json_object_set_new(arg, "sno",					json_string("123"));
	json_object_set_new(arg, "command",			json_string("clear_password"));
	json_object_set_new(arg, "attribute",		json_string("lock_password"));

	json_t *a = json_array();

	/*
	json_t *i = json_object();
	json_object_set_new(i, "k", json_string("seed"));
	char buf[32];
	sprintf(buf, "%d", seed);
	json_object_set_new(i, "v", json_string(buf));
	json_array_append_new(a, i);
	*/

	json_object_set_new(arg, "data", a);

	char *sarg = json_dumps(arg, 0);
	char topic[128];
	sprintf(topic, "local/iot/ziroom/S3/%s/control", mac);
	ziru_on_message(topic, sarg);
	free(sarg);
	json_decref(arg);
#endif
}

void do_cmd_include(char *argv[], int argc) {
	log_debug(" ");
	zwave_include();
}
void do_cmd_exclude(char *argv[], int argc) {
	log_debug(" ");
	zwave_exclude();
}

void do_cmd_remove(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("remove <mac: 0x30AE7B640DBA0707> -- view the nodeid 30AE7B640DBA0707");
		return;
	}
	
	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), argv[1], 0);

	int ret = zwave_remove_failed_node(mac);
	ret = ret;
}
void do_cmd_zwversion(char *argv[], int argc) {
	log_debug(" ");
	char version[64];
	if (zwave_version(version) == 0) {
		log_info("version : %s", version);
	}
}

void do_cmd_list(char *argv[], int argc) {
	device_view_all();
}
void do_cmd_lists(char *argv[], int argc) {
	device_view_all_simple();
}
void do_cmd_view(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("view <nodeid: 0x31> -- view the nodeid 0x31");
		return;
	}
	int x;
	sscanf(argv[1], "0x%02X", &x);
	device_view(x&0xff);
}


void do_cmd_reboot(char *argv[], int argc) {
	ZW_SetDefault(NULL);
	//zwave_reboot();
}
void do_cmd_region(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("region <region: 0x00>");
		log_warn("0x00-EU, 0x01-US, 0x02-ANZ, 0x03-HK, 0x04-Malaysia");
		log_warn("0x05-India, 0x06-Israel, 0x07-Russia, 0x08-China, 0x20-Japan, 0x21-Korea");
		return;
	}
	int x;
	sscanf(argv[1], "0x%02X", &x);
	int retx = zwave_region(x&0xff);
	if (retx != 1) {
		log_warn("region set failed!");
	} else {
		log_info("region set success, need to reboot");
	}
}

