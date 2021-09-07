#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "common.h"
#include "log.h"
#include "hex.h"
#include "jansson.h"
#include "json_parser.h"
#include "schedule.h"

#include "uproto.h"

#include "zwave_device.h"
#include "zwave.h"
#include "system.h"

static int parse_type = 0;
extern stZWaveCache_t zc;

/* Funcstion for cmd */
static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static stUprotoCmd_t ucmds[] = {
	{"getAttribute", uproto_cmd_handler_attr_get},
	{"setAttribute", uproto_cmd_handler_attr_set},
	{"reportAttribute", uproto_cmd_handler_attr_rpt},
};


/* Functions for attr */
static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_remove_failed_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_indicator(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_device_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int get_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);


static int rpt_zone_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_cmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_parse_type(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_gw_upgrade(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_class_cmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static stUprotoAttrCmd_t uattrcmds[] = {
	{"gw.upgrade",								NULL,									set_gw_upgrade,				NULL},
	/* mod */
	{"mod.device_list",						get_mod_device_list,	NULL,									rpt_mod_device_list},
	{"mod.del_device",						NULL,									set_mod_del_device,		NULL},
	{"mod.remove_failed_device",	NULL,									set_mod_remove_failed_device,		NULL},
	{"mod.new_device_added",			NULL,									NULL,									rpt_new_device_added},
	{"mod.add_device",						NULL,									set_mod_add_device,		NULL},
	{"device.status",							NULL,									NULL,									rpt_device_status},
	{"mod.device_deleted",				NULL,									NULL,									rpt_device_deleted},
	{"mod.zwave_info",						get_zwave_info,				NULL,									rpt_zwave_info},

	/* switch */
	{"device.switch.onoff",				NULL,									set_switch_onoff,			rpt_switch_onoff},
	{"device.onoff",				NULL,									set_switch_onoff,			rpt_switch_onoff},
	{"device.zone_status",				NULL,									NULL,									rpt_zone_status},
	{"device.indicator",			NULL,									set_indicator,						NULL},

	{"zw.rptcmd",									NULL,									NULL,									rpt_cmd},
	{"zw.parse_type",							set_parse_type,				NULL,									NULL},

	{"zw.class.cmd",							NULL,				set_class_cmd,									NULL},
};

//Receive/////////////////////////////////////////////////////////////////////////////
static stUprotoCmd_t *uproto_search_ucmd(const char *command) {
	int i;
	for (i = 0; i < sizeof(ucmds)/sizeof(ucmds[0]); i++) {
		if (strcmp(command, ucmds[i].name) == 0) {
			return &ucmds[i];
		}
	}
	return NULL;
}
static stUprotoAttrCmd_t *uproto_search_uattrcmd(const char *attr) {
	int i;
	for (i = 0; i < sizeof(uattrcmds)/sizeof(uattrcmds[0]); i++) {
		if (strcmp(attr, uattrcmds[i].name) == 0) {
			return &uattrcmds[i];
		}
	}
	return NULL;
}

static int _uproto_handler_cmd(const char *from, 
		const char *to,
		const char *ctype,
		const char *mac, 
		int dtime, 
		const char *uuid,
		const char *command,	
		const char *cmdmac,
		const char *attr,
		json_t *value) {
	stUprotoCmd_t *ucmd = uproto_search_ucmd(command);	
	if (ucmd == NULL) {
		log_warn("not support command:%s", command);
		return -1;
	}


	log_info("handler name : %s", ucmd->name);
	return ucmd->handler(uuid, cmdmac, attr, value);
}

int uproto_handler_cmd_dusun(const char *cmd) {
	json_error_t error;
	json_t *jpkt = json_loads(cmd, 0, &error);
	if (jpkt == NULL) {
		log_warn("error: on line %d: %s", error.line, error.text);
		goto parse_jpkt_error;
	}


	const char *from = json_get_string(jpkt, "from");
	const char *to = json_get_string(jpkt, "to");
	/* CLOUD, GATEWAY, NXP, GREENPOWER BLUETOOTH <ZB3> */
	if (strcmp(from, "CLOUD") != 0 ) {
		log_warn("now not support ubus source : %s", from);
		goto parse_jpkt_error;
	}
	if (strcmp(to, UPROTO_ME) != 0) {
		log_warn("now not support ubus dest : %s", to);
		goto parse_jpkt_error;
	}


	/* registerReq, registerRsp, reportAttribute, reportAttributeResp cmd cmdResult */
	const char *ctype = json_get_string(jpkt, "type");
	if (strcmp(ctype, "cmd") != 0) {
		log_warn("now not support ubus type : %s", ctype);
		goto parse_jpkt_error;
	}

	const char *mac = json_get_string(jpkt, "mac");
	int   dtime = 0; json_get_int(jpkt, "time", &dtime);

	/* verify jdata */
	json_t	*jdata = json_object_get(jpkt, "data");
	if (jdata == NULL) {
		log_warn("not find data item!");
		goto parse_jpkt_error;
	}

	const char *uuid = json_get_string(jdata, "id");
	const char *command = json_get_string(jdata, "command");
	json_t *jarg = json_object_get(jdata, "arguments");
	if (jarg == NULL) {
		log_warn("not find arguments!");
		goto parse_jpkt_error;
	}

	const char *cmdmac = json_get_string(jarg, "mac");
	const char *attr   = json_get_string(jarg, "attribute");
	json_t *    value  = json_object_get(jarg, "value");


	log_info("from:%s,to:%s,type:%s,time:%d,,uuid:%s,command:%s,cmdmac:%s, attr:%s",
			from, to, ctype, dtime, uuid, command, cmdmac, attr);

	_uproto_handler_cmd(from, to, ctype, mac, dtime, uuid, command, cmdmac, attr, value);

parse_jpkt_error:
	if (jpkt != NULL) json_decref(jpkt);
	return -1;
}

//Response////////////////////////////////////////////////////////////////////////
static int uproto_response_ucmd(const char *uuid, int retval) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "CLOUD";
	const char *deviceCode	= "00000";
	const char *type				= "cmdResult";
	int ctime								= time(NULL); 
	char mac[32];             system_get_mac(mac,sizeof(mac));

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(mac));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "id",	 json_string(uuid));
	json_object_set_new(jdata, "code", json_integer(retval));
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);

	return 0;
}
//Report///////////////////////////////////////////////////////////////////////////
static int uproto_report_umsg(const char *submac, const char *attr, json_t *jret) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "CLOUD";
	const char *deviceCode	= "00000";
	const char *type				= "reportAttribute";
	int ctime								= time(NULL);
	char mac[32];             system_get_mac(mac,sizeof(mac));

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(mac));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "attribute", json_string(attr));
	//char submac[32];				
	json_object_set_new(jdata, "mac", json_string(submac));

	if (1) {
		char *s = json_dumps(jret, 0);
		if (s != NULL) {
			log_debug(s);
			free(s);
		}
	}
	int ep = -1; json_get_int(jret, "ep", &ep);
	//if (json_object_del(jret, "ep") == 0) {
	if (ep >= 0) {
		json_object_set_new(jdata, "ep", json_integer(ep));
	}

	char *p = strstr(submac, ":");
	if (p == NULL) {
		char  devmac[32];
		hex_parse(devmac, sizeof(devmac), submac, NULL);
		stZWaveDevice_t *zd = device_get_by_extaddr(devmac);
		if (zd != NULL) {
			const char *mstr = device_make_modelstr_new(zd);
			json_object_set_new(jret, "ModelStr", json_string(mstr));
		}
	}

	json_object_set_new(jdata, "value", jret);
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);

	return 0;
}

static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	log_info("handler name : %s", uattrcmd->name);
	if (uattrcmd->get == NULL) {
		log_warn("get function is null!!!");
		return -2;
	}
	return uattrcmd->get(uuid, cmdmac, attr, value);
}
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	if (uattrcmd->set == NULL) {
		log_warn("set function is null!!!");
		return -2;
	}
	return uattrcmd->set(uuid, cmdmac, attr, value);
}

static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		uproto_report_umsg(cmdmac, attr, value);
		return -1;
	}
	if (uattrcmd->rpt == NULL) {
		log_warn("rpt function is null!!!");
		uproto_report_umsg(cmdmac, attr, value);
		return -2;
	}
	return uattrcmd->rpt(uuid, cmdmac, attr, value);
}

// Functions //////////////////////////////////////////////////////////////////////////////////////////
static int get_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	//_uproto_handler_cmd(from, to, ctype, mac,dtime, uuid, command,cmdmac,attr,value)
	_uproto_handler_cmd("", "", "", "", 0, uuid, "reportAttribute", cmdmac, "mod.device_list", NULL);

	uproto_response_ucmd(uuid, 0);
	
	return 0;
}
static int rpt_mod_device_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);


	json_t *jret = json_object();
	json_t *jary = json_array();

	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);
	for (i = 0; i < cnt; i++) {
		stZWaveDevice_t *zd = &zc.devs[i];
		if (zd->used == 0) {
			continue;
		}
		if (zd->state != DS_WORKED) {
			continue;
		}
		json_t *jd = json_object();
		json_array_append_new(jary, jd);

		//json_object_set_new(jd, "subtype",	json_string(zigbee_make_subtypestr(zd)));
		
		json_object_set_new(jd, "mac",			json_string(device_make_macstr(zd)));
		json_object_set_new(jd, "type",			json_string(device_make_typestr(zd)));
		json_object_set_new(jd, "version",	json_string(device_make_versionstr(zd)));
		json_object_set_new(jd, "model",		json_string(device_make_modelstr(zd)));
		json_object_set_new(jd, "online",		json_integer(device_get_online(zd)));
		json_object_set_new(jd, "battery",	json_integer(device_get_battery(zd)));
		json_object_set_new(jd, "ModelStr", json_string(device_make_modelstr_new(zd)));
	}
	json_object_set_new(jret, "device_list", jary);

	char submac[32];
	system_get_mac(submac, sizeof(submac));
	uproto_report_umsg(submac, attr, jret);

	return 0;
}
static int get_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	_uproto_handler_cmd("", "", "", "", 0, uuid, "reportAttribute", cmdmac, "mod.zwave_info", NULL);

	uproto_response_ucmd(uuid, 0);

	return 0;
}
static int rpt_zwave_info(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	json_t *jret = json_object();
	int homeid, nodeid;
	zwave_info(&homeid, &nodeid);

	char sbuf[32];
	sprintf(sbuf, "%08X", homeid);
	json_object_set_new(jret, "HomeID", json_string(sbuf));
	sprintf(sbuf, "%02X", nodeid&0xff);
	json_object_set_new(jret, "NodeID", json_string(sbuf));
	sprintf(sbuf, "V%d.%d.%d", MAJOR, MINOR, PATCH);
	json_object_set_new(jret, "version", json_string(sbuf));

	char submac[32];
	system_get_mac(submac, sizeof(submac));
	uproto_report_umsg(submac, attr, jret);

	return 0;
}



static int set_mod_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	
	const char *macstr		= json_get_string(value, "mac");
	const char *typestr  = json_get_string(value, "type");

	if (macstr == NULL) {
		macstr = "";
	}
	if(typestr == NULL) {
		typestr = "";
	}

	char mac[32];
	if (strcmp(macstr, "") == 0) {
		memset(mac, 0, sizeof(mac));
	} else {
		hex_parse((u8*)mac, sizeof(mac), macstr, 0);
	}

	int ret = zwave_exclude();

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	char gwmac[32];             system_get_mac(gwmac, sizeof(gwmac));
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", gwmac, "mod.device_list", NULL);

	return 0;
}


static int set_mod_remove_failed_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	
	const char *macstr		= json_get_string(value, "mac");
	if (macstr == NULL) {
		log_warn("error arguments (mac/type null?)!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -2;
	}
	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), macstr, 0);

	int ret = zwave_remove_failed_node(mac);

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	char gwmac[32];             system_get_mac(gwmac, sizeof(gwmac));
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", gwmac, "mod.device_list", NULL);
	return 0;
}

static int rpt_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	uproto_report_umsg(cmdmac, attr, value);

	return 0;
}

static int rpt_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	uproto_report_umsg(cmdmac, attr, value);

	return 0;
}
static int set_mod_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	int ret = zwave_include();
	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	return 0;
}

static int rpt_device_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

static int set_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	char sval[32];
	const char *val   = json_get_string(value, "value");
	if (val == NULL) { 
		int ival = -1; json_get_int(value, "value", &ival);
		if (ival < 0) {
			log_warn("error arguments (value null?)!");
			uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
			return -2;
		} else {
			sprintf(sval, "%d", ival);
		}
	} else {
		strcpy(sval, val);
	}

	int ep = 0;
	char sep[32];
	val = json_get_string(value, "ep");
	if (val == NULL) {
		int iep = -1; json_get_int(value, "ep", &iep);
		if (iep < 0) {
			log_warn("error arguments (ep null?)!");
			uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
			return -3;
		} else {
			sprintf(sep, "%d", iep);
		}
	} else {
		strcpy(sep, val);
	}
	ep = atoi(sep);

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);

	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		log_warn("no such device");
		uproto_response_ucmd(uuid, CODE_UNKNOW_DEVICE);
		return -3;
	}

	char buf[8] = {0};
	if ((sval[0]-'0') == 0) {
		buf[0] = 0x00;
	} else {
		buf[0] = 0xFF;
	}
	int ret = zwave_ep_class_cmd(zd->bNodeID, ep, 0x25, 0x01, buf, 1, NULL);

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);
	return 0;
}

static int set_indicator(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	char sval[32];
	const char *val   = json_get_string(value, "value");
	if (val == NULL) { 
		int ival = -1; json_get_int(value, "value", &ival);
		if (ival < 0) {
			log_warn("error arguments (value null?)!");
			uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
			return -2;
		} else {
			sprintf(sval, "%d", ival);
		}
	} else {
		strcpy(sval, val);
	}

	int ep = 0;
	char sep[32];
	val = json_get_string(value, "ep");
	if (val == NULL) {
		int iep = -1; json_get_int(value, "ep", &iep);
		if (iep < 0) {
			log_warn("error arguments (ep null?)!");
			uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
			return -3;
		} else {
			sprintf(sep, "%d", iep);
		}
	} else {
		strcpy(sep, val);
	}
	ep = atoi(sep);

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);

	stZWaveDevice_t *zd = device_get_by_extaddr(mac);
	if (zd == NULL) {
		log_warn("no such device");
		uproto_response_ucmd(uuid, CODE_UNKNOW_DEVICE);
		return -3;
	}

	char buf[8] = {0};
	//buf[0] = sval[0] - '0';
	if ((sval[0]-'0') == 0) {
		buf[0] = 0x00;
	} else {
		buf[0] = 0xFF;
	}

	log_debug_hex("buf:", buf, 1);
	int ret = zwave_ep_class_cmd(zd->bNodeID, ep, 0x87, 0x01, buf, 1, NULL);

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);
	return 0;

}

static int rpt_switch_onoff(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

static int rpt_zone_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}

static int rpt_cmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);
	uproto_report_umsg(cmdmac, attr, value);
	return 0;
}


void parse_type_load() {
	if (access("/etc/config/dusun/zwdev/parsetype", F_OK) != 0) {
		parse_type = 0;
		return;
	}
	
	parse_type = 1;
}
static int set_parse_type(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	int pt = -1; json_get_int(value, "parseType", &pt);
	if (pt < 0) {
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	if (pt != 0 && pt != 1) {
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	parse_type = pt;
	if (parse_type != 0) {
		system("rm -rf /etc/config/dusun/zwdev/parsetype");
	} else {
		char buf[256];
		sprintf(buf, "echo %d > /etc/config/dusun/zwdev/parsetype", parse_type);
		system(buf);
	}
	
	uproto_response_ucmd(uuid, CODE_SUCCESS);
	return 0;
}

static int set_gw_upgrade(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	log_info("[%s]", __func__);

	if (value == NULL) {
		log_warn("error arguments!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	
	const char *target			= json_get_string(value, "target");
	const char *url					= json_get_string(value, "url");
	const char *keepsetting	= json_get_string(value, "keepsetting");
	if (target == NULL || url == NULL || keepsetting == NULL) {
		log_warn("error arguments (target/keepsetting/url null?)!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -2;
	}
	int	ret = system_upgrade(target, url, keepsetting);

	if (ret != 0) ret = CODE_TIMEOUT;
	uproto_response_ucmd(uuid, ret);

	return 0;
}

//rpt interface///////////////////////////////////////////////////////////////////////////////////////
int uproto_rpt_register_dusun(const char *extaddr) {
	log_info("[%s]", __func__);
		
	
	//_uproto_handler_cmd(from, to, ctype, mac,dtime, uuid, command,cmdmac,attr,value)
	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}

	json_t *jret = json_object();
	json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
	json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
	json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "model", json_string(device_make_modelstr(zd)));
	json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
	json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));

	json_t *jeps = json_array();
	if (jeps != NULL) {
		json_object_set_new(jret, "eps", jeps);

		int ie = 0;
		for (ie = 0; ie < zd->subepcnt+1; ie++) {
			json_t *jep = json_object();
			if (jep == NULL) {
				continue;
			}
			json_array_append_new(jeps, jep);

			stZWaveEndPoint_t *subep = NULL;
			if (ie == 0 ) {
				subep = &zd->root;
			} else {
				subep = &zd->subeps[ie-1];
			}
			json_object_set_new(jep, "ep", json_integer(subep->ep&0xff));

			json_t *jclss = json_array();
			if (jclss == NULL) {
				continue;
			}
			json_object_set_new(jep, "cs", jclss);

			int ic = 0;
			for (ic = 0; ic < subep->classcnt; ic++) {
				json_t *jc = json_object();
				if (jc == NULL) {
					continue;
				}
				json_array_append_new(jclss, jc);

				stZWaveClass_t *cls = &subep->classes[ic];

				char cbuf[48];
				sprintf(cbuf, "%02X", cls->classid&0xff);
				json_object_set_new(jc, "c",		json_string(cbuf));
				json_object_set_new(jc, "v",	json_integer(cls->version&0xff));
			}
		}
	}


	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "mod.new_device_added", jret);

	char mac[32];             system_get_mac(mac, sizeof(mac));
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", mac, "mod.device_list", NULL);
	return 0;
}

static stSchduleTask_t after_deleted_rpt_list_task;
static void after_deleted_rpt_list_func(void *arg) {
	char mac[32];             system_get_mac(mac, sizeof(mac));
	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", mac, "mod.device_list", NULL);
}
void uproto_dusun_start_up_rpt_devlist() {
	schedue_add(&after_deleted_rpt_list_task, 5000, after_deleted_rpt_list_func, NULL);
}
int uproto_rpt_unregister_dusun(const char *extaddr) {
	log_info("[%s]", __func__);

	json_t *jret = json_object();
	char sbuf[32];
	hex_string(sbuf, sizeof(sbuf), (u8*)extaddr, 8, 1, 0);
	json_object_set_new(jret, "mac", json_string(sbuf));

	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", sbuf, "mod.device_deleted", jret);
	schedue_add(&after_deleted_rpt_list_task, 1000, after_deleted_rpt_list_func, NULL);
	return 0;
}

int uproto_rpt_status_dusun(const char *extaddr) {
	log_info("[%s]", __func__);

	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}

	json_t *jret = json_object();
	json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
	json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
	json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
	json_object_set_new(jret, "model", json_string(device_make_modelstr(zd)));
	json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
	json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));

	_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.status", jret);

	return 0;
}


int uproto_rpt_cmd_dusun(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len) {
	log_info("[%s]", __func__);

	stZWaveDevice_t *zd = device_get_by_extaddr((char *)extaddr);
	if (zd == NULL) {
		char buf[32];
		hex_string(buf, sizeof(buf), (u8*)extaddr, 8, 1, 0);
		log_warn("can't find this device at device table:%s!", buf);
		return 0;
	}
	
	log_debug_hex("buf:", buf, len);
	if ((clsid&0xff) == 0x60 && (cmdid&0xff) == 0x0D) {
		ep = buf[0]&0xff;
		clsid = buf[2]&0xff;
		cmdid = buf[3]&0xff;
		buf = buf + 4;
		len = len - 4;
		log_debug("ep:%02X, clsid:%02X, cmdid:%02X", ep&0xff, clsid&0xff, cmdid&0xff);
		log_debug_hex("buf:", buf, len);
	}

	if (parse_type == 1) {
		json_t *jret = json_object(); 
		json_object_set_new(jret, "ep",			json_integer(ep&0xff));
		json_object_set_new(jret, "clsid",	json_integer(clsid&0xff));
		json_object_set_new(jret, "cmdid",	json_integer(cmdid&0xff));
		int i = 0;
		char data[256];
		for (i = 0; i < len; i++) {
			sprintf(data + i *2, "%02X", buf[i]&0xff);
		}
		json_object_set_new(jret, "data",	json_string(data));
		
		_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "zw.rptcmd", jret);
		return 0;
	}

	
	if ((clsid == 0x25 && cmdid == 0x03) || (clsid == 0x20 && cmdid == 0x03)) { /* switch onoff */
		json_t *jret = json_object(); 
		char sbuf[32];
		sprintf(sbuf, "%d", !!buf[0]);
		json_object_set_new(jret, "value", json_string(sbuf));
#if 0
		sprintf(sbuf, "%d", ep);
		json_object_set_new(jret, "ep", json_string(sbuf));
#else
		json_object_set_new(jret, "ep", json_integer(ep&0xff));
#endif

		_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.switch.onoff", jret);
	} else if ((clsid&0xff) == 0x71 && (cmdid&0xff) == 0x05) { /* security event rpt */
		char notification_type		= buf[4]&0xff;
		char notification_event	= buf[5]&0xff;
		if (notification_type == 0x07 && notification_event == 0x08) {
			char paramlen						= buf[6]&0xff;
			if (paramlen == 1) {
				char param								= buf[7]&0xff;

				json_t *jret = json_object(); 
				char sbuf[32];
				sprintf(sbuf, "%d", !!(param&0x80));

				json_object_set_new(jret, "value", json_string(sbuf));
				json_object_set_new(jret, "ep", json_integer(ep&0xff));
				json_object_set_new(jret, "zone", json_string("pir"));
				_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.zone_status", jret);
			} else {
				json_t *jret = json_object(); 
				char sbuf[32];
				sprintf(sbuf, "%d", 1);

				json_object_set_new(jret, "value", json_string(sbuf));
				json_object_set_new(jret, "ep", json_integer(ep&0xff));
				json_object_set_new(jret, "zone", json_string("pir"));
				_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.zone_status", jret);
			}
		} else {
				log_warn("not support class(%02X) cmd(%02X)  notification(%02X), event(%02x)\n", 
							clsid&0xff, cmdid&0xff, notification_type&0xff, notification_event&0xff);
		}
	} else if ((clsid&0xff) == 0x80 && (cmdid&0xff) == 0x03) { /* battery event rpt */
			//char bl = buf[4]&0xff;

		json_t *jret = json_object(); 

		json_object_set_new(jret, "mac", json_string(device_make_macstr(zd)));
		json_object_set_new(jret, "type", json_string(device_make_typestr(zd)));
		json_object_set_new(jret, "version", json_string(device_make_versionstr(zd)));
		json_object_set_new(jret, "model", json_string(device_make_modelstr(zd)));
		json_object_set_new(jret, "online", json_integer(device_get_online(zd)));
		json_object_set_new(jret, "battery", json_integer(device_get_battery(zd)));
		json_object_set_new(jret, "ModelStr", json_string(device_make_modelstr_new(zd)));

		_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.status", jret);
		
	} else if ((clsid&0xff) == 0x30 && (cmdid&0xff) == 0x03) {
		/*
		static char *snr_type[] = {
			"reserved", 
			"general",
			"smoke",
			"co",
			"co2",
			"heat",
			"water",
			"freeze",
			"tamper",
			"aux",
			"doorwin",
			"tilt",
			"pir",	 //motion
			"glass",
			"first",
		};
		*/
		int value = buf[0]&0xff;
		int zone  = buf[1]&0xff;
		/*
		zone = zone % (sizeof(snr_type)/sizeof(snr_type[0]));
		*/

		char sbuf[32];
		sprintf(sbuf, "%d", !!value);

		json_t *jret = json_object(); 
		json_object_set_new(jret, "value", json_string(sbuf));
		json_object_set_new(jret, "ep", json_integer(ep&0xff));
		//json_object_set_new(jret, "zone", json_string(snr_type[zone]));
		json_object_set_new(jret, "zone", json_string(device_sensor_binary_zonestr(zone)));
		_uproto_handler_cmd("", "", "", "", 0, "", "reportAttribute", device_make_macstr(zd), "device.zone_status", jret);
	} else {
		log_warn("not support class(%02X) cmd(%02X)\n", clsid, cmdid);
	}

	return 0;
}


int uproto_rpt_attr_dusun(const char *extaddr, unsigned char ep, unsigned char clsid,  const char *buf, int len) {
	log_info("[%s]", __func__);
	/*
	*/
	return 0;
}

static int set_class_cmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	const char *macstr = json_get_string(value, "mac");
	if (macstr < 0) {
		log_warn("no macstr segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	char mac[256];
	int len = hex_parse((unsigned char *)mac, sizeof(mac), macstr, NULL);
	if (len < 0 || len != 8) {
		log_warn("err macstr segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}
	stZWaveDevice_t *zd = device_get_by_extaddr((char *)mac);
	if (zd == NULL) {
		log_warn("no such dev:%s", macstr);
		uproto_response_ucmd(uuid, CODE_UNKNOW_DEVICE);
		return -1;
	}

	int ep = -1;			json_get_int(value, "ep",			&ep);
	if (ep < 0) {
		log_warn("no ep segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	int class = -1;		json_get_int(value, "clsid",	&class);
	if (class < 0) {
		log_warn("err class segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	int cmd = -1;		json_get_int(value, "cmdid",	&cmd);
	if (cmd < 0) {
		log_warn("err cmd segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	const char *data = json_get_string(value, "data");
	if (data == NULL) {
		log_warn("err data segment!");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	char buf[256];
	len = hex_parse((unsigned char *)buf, sizeof(buf), data, NULL);
	if (len < 0) {
		log_warn("err data segment! 1");
		uproto_response_ucmd(uuid, CODE_WRONG_FORMAT);
		return -1;
	}

	log_info("nodeid:%s, ep:%02X, class:%02X, cmd:%02X",macstr, ep&0xff, class&0xff, cmd&0xff);
	zwave_ep_class_cmd(zd->bNodeID, ep&0xff, class&0xff, cmd&0xff, buf, len, NULL);
	log_debug_hex("buf:", buf, len);
	
	uproto_response_ucmd(uuid, CODE_SUCCESS);
	return 0;
}


