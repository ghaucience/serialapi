#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "hex.h"

#include "zwave_device_storage.h"
#include "zwave_class_cmd.h"
#include "zwave_ccdb.h"
#include "system.h"



stZWaveCache_t zc;

static stZWaveDevice_t* device_malloc() {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 1) {
			continue;
		}
	
		memset(&devs[i], 0, sizeof(devs[i]));
		devs[i].nbit = -1;
		devs[i].nbit_subeps_head = -1;
		devs[i].used = 1;
		return &devs[i];
	}
	return NULL;
}
static void device_free(stZWaveDevice_t* zd) {
	zd->used = 0;
}

static void device_clear_cmd(stZWaveCommand_t *cmd) {
	cmd->cmdid = -1;
}

static void device_clear_class(stZWaveClass_t *class) {
	class->classid = -1;
	if (class->cmdcnt > 0) {
		int i = 0;
		for (i = 0; i < class->cmdcnt; i++) {
			device_clear_cmd(&class->cmds[i]);
		}
		free(class->cmds);
		class->cmds =NULL;
		class->cmdcnt = 0;
	}
	return ;
}

static void device_clear_endpoint(stZWaveEndPoint_t *ep) {
	ep->ep = -1;
	if (ep->classcnt > 0) {
		int i = 0;
		for (i = 0; i < ep->classcnt; i++) {
			device_clear_class(&ep->classes[i]);
		}
		free(ep->classes);
		ep->classes = NULL;
		ep->classcnt = 0;
	}
}

char *device_get_extaddr(stZWaveDevice_t *zd) {
	char mac[8];
	static char gwmac[32] ={0};
	if (gwmac[0] == 0) {
		char macstr[32];
		//system_mac_get(macstr);
		system_get_mac(macstr, sizeof(macstr));
		hex_parse((unsigned char *)gwmac, sizeof(gwmac), macstr, NULL);
		log_debug_hex("gwmac", gwmac, 6);
	}

	if (zd->mac[0] == 0) {
		memset(mac, zd->bNodeID, 8);
		memcpy(mac, gwmac, 6);

#if 0
		stZWaveClass_t *class = device_get_class(zd, 0, 0x72);
		if (class != NULL && class->version >= 2) {
			stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
			if (cmd != NULL) {
				memcpy(mac, cmd->data + 2, 8);
			} else {
				log_warn("no manufacturer id(no cmd 0x07), use zwave node id as mac!!!");
			}
		} else {
			log_warn("no manufacturer id(no class 0x72), use zwave node id as mac!!!");
		}
#endif

		memcpy(zd->mac, mac, 8);
	}


	return zd->mac;
}

int device_add(char bNodeID, char basic, char generic, 
		char specific, char security, char capability,
		int classcnt, char *classes) {
	stZWaveDevice_t *zd = device_get_by_nodeid(bNodeID);
	if (zd != NULL) {
		return 0;
	}
	
	zd = device_malloc();
	if (zd == NULL) {
		return -1;
	}

	zd->bNodeID = bNodeID;
	zd->security = security;
	zd->capability = capability;


	zd->nbit = -1;
	zd->nbit_subeps_head = -1;

	device_fill_ep(&zd->root, 0, basic, generic, specific, classcnt, classes);
	
	return 0;
}

int device_del(char bNodeID) {
	stZWaveDevice_t *zd = device_get_by_nodeid(bNodeID);
	if (zd == NULL) {
		return 0;
	}
	
	device_clear_endpoint(&zd->root);
	if (zd->subepcnt > 0) {
		int i = 0;
		for (i = 0; i < zd->subepcnt; i++) {
			device_clear_endpoint(&zd->subeps[i]);
		}
		free(zd->subeps);
		zd->subeps = NULL;
		zd->subepcnt = 0;
	}

	device_free(zd);

	return 0;
}

stZWaveDevice_t *device_get_by_nodeid(char bNodeID) {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}
	
		if (devs[i].bNodeID == bNodeID) {
			return &devs[i];
		}
	}
	
	return NULL;
}

stZWaveDevice_t *device_get_by_extaddr(char extaddr[8]) {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}
	
		if (memcmp(device_get_extaddr(&devs[i]), extaddr, 8) == 0) {
			return &devs[i];
		}
	}
	
	return NULL;
}

int device_add_subeps(stZWaveDevice_t *zd, int epcnt, char *eps) {
	if (epcnt <= 0 || eps == NULL || zd == NULL) {
		return -1;
	}
	
	zd->subepcnt = epcnt;
	int i = 0;
	zd->subeps = (stZWaveEndPoint_t *)malloc(sizeof(stZWaveEndPoint_t)*zd->subepcnt);
	for (i = 0; i < epcnt; i++) {
		memset(&zd->subeps[i], 0, sizeof(stZWaveEndPoint_t));
		zd->subeps[i].ep = eps[i]&0xff;
		zd->subeps[i].nbit = -1;
		zd->subeps[i].nbit_next = -1;
		zd->subeps[i].nbit_classes_head = -1;
	}
	
	return 0;
}

int device_fill_ep(stZWaveEndPoint_t *ze, char ep, char basic, char generic,
		char specific, int classcnt, char *classes) {
	ze->ep = 0;
	ze->basic = basic;
	ze->generic = generic;
	ze->specific = specific;
	ze->classcnt = 0;
	ze->classes = NULL;
	ze->nbit = -1;
	ze->nbit_next = -1;
	ze->nbit_classes_head = -1;
	if (classcnt > 0) {
		ze->classcnt = classcnt;
		ze->classes = (stZWaveClass_t*)malloc(sizeof(stZWaveClass_t)*ze->classcnt);
		int i = 0;
		for (i = 0; i < ze->classcnt; i++) {
			ze->classes[i].classid = classes[i]&0xff;
			ze->classes[i].version = -1;
			ze->classes[i].cmdcnt = 0;
			ze->classes[i].cmds = NULL;
			ze->classes[i].nbit = -1;
			ze->classes[i].nbit_next = -1;
			ze->classes[i].nbit_cmds_head = -1;

			/*
			stZWClass_t *c = zcc_get_class(ze->classes[i].classid, ze->classes[i].version);
			char cmds[MAX_CMD_NUM];
			int cmdcnt = zcc_get_class_cmd_rpt(c, cmds);
			if (cmdcnt > 0) {
				device_add_cmds(&ze->classes[i], cmdcnt, cmds);
			}
			*/
		}
	} 

	return 0;
}

stZWaveEndPoint_t *device_get_subep(stZWaveDevice_t *zd, char ep) {
	int i;	
	for (i = 0; i < zd->subepcnt; i++) {
		if (zd->subeps[i].ep == ep) {
			return &zd->subeps[i];
		}
	}
	return 0;
}

int device_add_cmds(stZWaveClass_t *class, int cmdcnt, char *cmds) {
	if (class == NULL || cmdcnt <= 0 || cmds == NULL) {
		return -1;
	}
	
	class->cmdcnt = cmdcnt;
	class->cmds = (stZWaveCommand_t*)malloc(sizeof(stZWaveCommand_t)*class->cmdcnt);
	class->nbit = -1;
	class->nbit_next = -1;
	class->nbit_cmds_head = -1;
	int i = 0;
	for (i = 0; i < class->cmdcnt; i++) {
		memset(&class->cmds[i], 0, sizeof(stZWaveCommand_t));
		class->cmds[i].cmdid = cmds[i]&0xff;
		class->cmds[i].nbit = -1;
		class->cmds[i].nbit_next = -1;
	}

	return 0;
}

stZWaveCommand_t *device_get_cmd(stZWaveClass_t *class, char cmd) {
	int i = 0; 
	for (i = 0; i < class->cmdcnt; i++) {	
		if (class->cmds[i].cmdid == cmd) {
			return &class->cmds[i];
		}
	}

	return NULL;
}

int device_update_cmds_data(stZWaveCommand_t *zc, char *data, int len) {
	len = len > sizeof(zc->data) ? sizeof(zc->data) : len;
	zc->len = len;
	memcpy(zc->data, data, len);
	return 0;
}



stZWaveClass_t *device_get_class(stZWaveDevice_t *zd, char epid, char classid) {
	stZWaveEndPoint_t *ep = NULL;
	
	if (epid == 0) {
		ep = &zd->root;
	} else {
		ep = device_get_subep(zd, epid);
	}

	if (ep == NULL) {
		return NULL;
	}


	int i = 0;
	for (i = 0; i < ep->classcnt; i++) {
		stZWaveClass_t *class = &ep->classes[i];
		if (class->classid == classid) {
			return class;
		}
	}

	return NULL;
}

const char *device_make_macstr(stZWaveDevice_t *zd) {
#if 0
	char mac[8];

	memset(mac, zd->bNodeID, 8);

	stZWaveClass_t *class = device_get_class(zd, 0, 0x72);
	if (class != NULL && class->version >= 2) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x07);
		if (cmd != NULL) {
			memcpy(mac, cmd->data + 2, 8);
		} else {
			log_warn("no manufacturer id(no cmd 0x07), use zwave node id as mac!!!");
		}
	}  else {
		log_warn("class:%p, version:%d\n", class, class != NULL ? class->version : -1);
		log_warn("no manufacturer id(no class 0x72), use zwave node id as mac!!!");
	}
#else
	char *mac = device_get_extaddr(zd);
#endif

	static char macstr[32];
	hex_string(macstr, sizeof(macstr), (u8*)mac, 8, 1, 0);

	return macstr;
}
int device_get_battery(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x80);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x03);
		if (cmd != NULL) {
			char *buf = cmd->data;
			int battery = buf[0]&0xff;
			if ((battery&0xff) == (0xff&0xff)) {
				battery = 100;
			}
			return battery;
		}
	}
	return 100;
}

int device_get_pir_status(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x71);
	if (class == NULL) {
		return 0;
	}
	stZWaveCommand_t *cmd = device_get_cmd(class, 0x05);
	if (cmd == NULL) {
		return 0;
	}

	char *buf = cmd->data;
	int len = cmd->len;
	if (buf == NULL || len == 0) {
		return 0;
	}

	char notification_type	= buf[4]&0xff;
	char notification_event	= buf[5]&0xff;
	if (notification_type == 0x07 && notification_event == 0x08) {
		char paramlen						= buf[6]&0xff;
		if (paramlen == 1) {
			char param								= buf[7]&0xff;
			return !!(param&0x80);
		} else {
			return 1;
		}
	} 

	return 0;
}
int device_get_online(stZWaveDevice_t *zd) {
	
#if 1
	int last = zd->last;

	if (schedue_current() - last > 45 * 60 * 10000) {
		return 0;
	}

	return 1;
#else
	int online = zd->online;
	return !!online;
#endif
}
int device_is_lowpower(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x80);
	if (class != NULL) {
		return 1;
	}
	return 0;
}
const char *device_sensor_binary_modelstr(int i) {
	static char *mod_zone_type[] = {
		"reserved", 
		"smoke",
		"co",
		"co2",
		"heat",
		"leak",
		"door",
		"homesecurity",
		"powermanage",
		"system",
		"emergency",
		"clock",	 //motion
		"applicance",
		"health",
		"siren",
		"leak",
		"leak",
		"irrigation",
		"gas",
		"pest",
		"lux",
		"waterquatiy",
		"homemonitor",
		"first",
	};
	i = i % (sizeof(mod_zone_type)/sizeof(mod_zone_type[0]));
	return mod_zone_type[i];
}

const char *device_sensor_binary_zonestr(int i) {
	static char *snr_zone_type[] = {
			"reserved", 
			"general",
			"smoke",
			"co",
			"co2",
			"heat",
			"leak",
			"freeze",
			"tamper",
			"aux",
			"door",
			"tilt",
			"pir",	 //motion
			"glass",
			"first",
		};
		i = i % (sizeof(snr_zone_type)/sizeof(snr_zone_type[0]));
		return snr_zone_type[i];
}
const char *device_sensor_binary_typestr(int i) {
	static char *snr_type[] = {
			"unknow", 
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"unknow",
			"1209",	 //motion
			"unknow",
			"unknow",
		};
		i = i % (sizeof(snr_type)/sizeof(snr_type[0]));
		return snr_type[i];
}

const char *device_make_typestr(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x25);
	if (class != NULL) {
		return "1213";
	}
	
	class = device_get_class(zd, 0, 0x71);
	if (class != NULL) {
		return "1209";
	}

	class = device_get_class(zd, 0, 0x30);
	if (class != NULL) {
			stZWaveCommand_t *cmd = device_get_cmd(class, 0x04);
			if (cmd != NULL) {
				return device_sensor_binary_typestr(cmd->data[0]&0xff);

			}
	}

	return "unkonw";
}

const char *device_make_versionstr(stZWaveDevice_t *zd) {
	stZWaveClass_t *class = device_get_class(zd, 0, 0x86);
	if (class != NULL) {
		stZWaveCommand_t *cmd = device_get_cmd(class, 0x12);
		if (cmd != NULL) {
			char *buf = cmd->data;
			static char version[32];
			sprintf(version, "%02X-%02X.%02X-%02X.%02X", buf[0]&0xff, buf[1]&0xff,buf[2]&0xff, buf[3]&0xff, buf[4]&0xff);
			return version;
		}
	}
	return "unknow";
}
const char *device_make_modelstr(stZWaveDevice_t *zd) {
	return "dusun";
}

const char *device_make_modelstr_new(stZWaveDevice_t *zd) {
	char bname[128];
	char gname[128];
	char sname[128];
	stZWaveEndPoint_t *ep = &zd->root;
	zwave_ccdb_get_basic_name(ep->basic&0xff, bname);
	zwave_ccdb_get_generic_specific_name(ep->generic&0xff, ep->specific&0xff, gname, sname);

	static char _typestr[128];

	if (strcmp(sname, "unknow") != 0 && strcmp(sname, "Not Used") != 0) {
		strcpy(_typestr, sname);
		/*
		char yname[128];
		stZWaveClass_t *class = device_get_class(zd, 0, 0x71);
		if (class != NULL) {
			stZWaveCommand_t *cmd = device_get_cmd(class, 0x08);
			if (cmd != NULL && cmd->data[0] != 0) {
				int i = cmd->data[1]&0xff;
				strcpy(yname, device_sensor_binary_modelstr(i));
				if (strcmp(yname, "reserved") != 0) {
					strcpy(_typestr, yname);
				}
			}
		}
		*/

		return _typestr;
	}

	if (strcmp(gname, "unknow") != 0 && strcmp(gname, "Not Used") != 0) {
		strcpy(_typestr, gname);
		return _typestr;
	}

	if (strcmp(bname, "unknow") != 0 && strcmp(bname, "Not Used") != 0) {
		strcpy(_typestr, gname);
		return _typestr;
	}

	return "unknow";
}


char device_get_nodeid_by_mac(const char *mac) {
	stZWaveDevice_t *zd = device_get_by_extaddr((char *)mac);
	if (zd == NULL) {
		return -1;
	}
	
	return zd->bNodeID;
}

static void device_view_command(stZWaveClass_t *class, stZWaveCommand_t *command) {
	char buf[sizeof(command->data)*3 + 4];
	int i = 0; 
	int len = 0;
	buf[0] = 0;
	for (i = 0; i < command->len; i++) {
		len += sprintf(buf + len, "%02X ", command->data[i]&0xff);
	}

	const char *name = NULL;
	if (!zwave_ccdb_exsit()) {
		name = zcc_get_cmd_name(class->classid, class->version, command->cmdid);
	} else {
		name = zwave_ccdb_get_class_cmd_name(class->classid, class->version, command->cmdid);
	}
	//printf("      cmdid:%02X(%s), len: %d, data:[ %s], nbit:%d, nbit_next:%d\n", command->cmdid&0xff, name,  command->len, buf, command->nbit, command->nbit_next);
	printf("      cmdid:%02X(%s), nbit:%d, nbit_next:%d, len: %d, data:[ %s]\n", command->cmdid&0xff, name,  command->nbit, command->nbit_next,  command->len, buf);
}

static void device_view_class(stZWaveClass_t *class) {
	const char *name = NULL;
	if (!zwave_ccdb_exsit()) {
		name = zcc_get_class_name(class->classid, class->version);
	} else {
		name = zwave_ccdb_get_class_name(class->classid, class->version);
	}

	printf("    classid:%02X(%s), version:%d, nbit:%d, nbit_next:%d, dhead:%d\n", class->classid&0xff, name, class->version, class->nbit, class->nbit_next, class->nbit_cmds_head);
	int i = 0;
	for (i = 0; i < class->cmdcnt; i++) {
		device_view_command(class, &class->cmds[i]);
	}
}

static void device_view_endpoint(stZWaveEndPoint_t *ep) {
	char bname[128];
	char gname[128];
	char sname[128];
	zwave_ccdb_get_basic_name(ep->basic&0xff, bname);
	zwave_ccdb_get_generic_specific_name(ep->generic&0xff, ep->specific&0xff, gname, sname);
	
	printf("  ep:%02X, basic:%02X(%s), generic: %02X(%s), specific:%02X(%s) , nbit:%d, nbit_next:%d, chead:%d\n",
					ep->ep&0xff, ep->basic&0xff, bname, ep->generic&0xff, gname, ep->specific&0xff, sname,  ep->nbit, ep->nbit_next, ep->nbit_classes_head);
	int i = 0;
	for (i = 0; i < ep->classcnt; i++) {
		device_view_class(&ep->classes[i]);
	}
}

static void device_view_device(stZWaveDevice_t *dev) {
	long last = dev->last;
	long interval = schedue_current() - last;
	printf("mac:%s, nodeid:%02X, security:%02X, capacility:%02X, last:%ldmin,%ldsec, online:%d, state:%02X, nbit:%d\n",
						device_make_macstr(dev), dev->bNodeID&0xff, dev->security&0xff, dev->capability&0xff, 
						(interval / 1000) /60, 
						(interval / 1000) %60,
						device_get_online(dev),
						dev->state&0xff,
						dev->nbit);
	device_view_endpoint(&dev->root);
	int i = 0;
	for (i = 0; i < dev->subepcnt; i++) {
		device_view_endpoint(&dev->subeps[i]);
	}
}

void device_view_all() {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}

		device_view_device(&devs[i]);
	}
}
void device_view_all_simple() {
	stZWaveDevice_t *devs = zc.devs;
	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);

	for (i = 0; i < cnt; i++) {
		if (devs[i].used == 0) {
			continue;
		}

		stZWaveDevice_t *dev = &devs[i];
		long last = dev->last;
		long interval = schedue_current() - last;
		printf("mac:%s, nodeid:%02X, security:%02X, capacility:%02X, last:%ldmin,%ldsec, online:%d, state:%02X\n",
				device_make_macstr(dev), dev->bNodeID&0xff, dev->security&0xff, dev->capability&0xff, 
				(interval / 1000) /60, 
				(interval / 1000) %60,
				device_get_online(dev),
				dev->state&0xff);

		stZWaveEndPoint_t *ep = &dev->root;
		char bname[128];
		char gname[128];
		char sname[128];
		zwave_ccdb_get_basic_name(ep->basic&0xff, bname);
		zwave_ccdb_get_generic_specific_name(ep->generic&0xff, ep->specific&0xff, gname, sname);

		printf("  ep:%02X, basic:%02X(%s), generic: %02X(%s), specific:%02X(%s) \n",
				ep->ep&0xff, ep->basic&0xff, bname, ep->generic&0xff, gname, ep->specific&0xff, sname);
		
		int j = 0;
		printf("  classes: ");
		for (j = 0; j < ep->classcnt; j++) {
			printf("%02X(V%d) ", ep->classes[j].classid&0xff, ep->classes[j].version);
		}
		printf("\n");
	}
}
void device_view(unsigned char nodeid) {
	stZWaveDevice_t *zd = device_get_by_nodeid(nodeid);
	if (zd == NULL) {
		return;
	}
	device_view_device(zd);
}



int device_get_class_no_version(stZWaveDevice_t *zd) {
	int i = 0;
	int cnt = zd->root.classcnt;
	for (i = 0; i < cnt; i++) {
		stZWaveClass_t *zc = &zd->root.classes[i];
		if (zc->version < 0) {
			return zc->classid&0xff;
		}
	}
	return -1;
}

int device_get_read_rpt_cmd(stZWaveDevice_t *zd, unsigned char *class, unsigned char *cmd) {
	int i = 0; 
	int cnt = zd->rrcs_size;
	for (i = 0; i < cnt; i++) {
		stReadRptCmd_t *rrc = &zd->rrcs[i];
		if (rrc->rflag != 0) {
			continue;
		}
		
		if (device_get_class(zd, 0, rrc->class) == NULL) {
			continue;
		}
		
		*class = rrc->class;
		*cmd = rrc->cmd;
		
		return 0;
	}
	return -1;
}

int device_set_read_rpt_cmd(stZWaveDevice_t *zd) {
	int i = 0; 
	int cnt = zd->rrcs_size;
	for (i = 0; i < cnt; i++) {
		stReadRptCmd_t *rrc = &zd->rrcs[i];
		if (rrc->rflag != 0) {
			continue;
		}
		
		rrc->rflag = 1;

		return 0;
	}
	return 0;
}

int device_set_read_rpt_cmd_by_class_cmd(stZWaveDevice_t *zd, unsigned char class, unsigned cmd) {
	int i = 0; 
	int cnt = zd->rrcs_size;
	for (i = 0; i < cnt; i++) {
		stReadRptCmd_t *rrc = &zd->rrcs[i];
		if (rrc->rflag != 0) {
			continue;
		}
		if ((rrc->class&0xff) != (class&0xff)) {
			continue;
		}
		if ((rrc->rptcmd&0xff) != (cmd&0xff)) {
			continue;
		}

		rrc->rflag = 1;

		return 0;
	}
	return 0;
}

int device_exsit_assoc_class(stZWaveDevice_t *zd) {
	int i = 0;
	int cnt = zd->root.classcnt;
	for (i = 0; i < cnt; i++) {
		stZWaveClass_t *zc = &zd->root.classes[i];
		if ((zc->classid&0xff) == (0x85&0xff)) {
			return 1;
		}
	}
	return 0;
}
