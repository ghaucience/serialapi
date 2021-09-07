#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zwave_class_cmd.h"

stZWClass_t zcc_ccs[] = {
	{0x73, 1, "powerlevel", 6, {
			{0x02, "get",							'g', 0, ""},
			{0x03, "report",					'r', 0, ""},
			{0x01, "set",							's', 0, ""},
			{0x05, "test_node_get",		'g', 0, ""},
			{0x06, "test_node_report",'r', 0, ""},
			{0x04, "test_node_set",		's', 0, ""},
		},
	},

	{0x25, 1, "switch_binary", 3, {
			{0x02, "get",			'g', 0, ""},
			{0x03, "report",	'r', 0, ""},
			{0x01, "set",			's', 0, ""},
		},
	},

	{0x5e, 1, "zwaveplus_info", 2, {
			{0x01, "get",		'g', 0, ""},
			{0x02, "report",'r', 0, ""}, 
		},
	},
	{0x5e, 2, "zwaveplus_info", 2, {
			{0x01, "get",		'g', 0, ""},
			{0x02, "report",'r', 0, ""}, 
		},
	},

	{0x85, 1, "association", 6, {
			{0x02, "get",						'g', 0, ""},
			{0x05, "groups_get",		'g', 0, ""}, 
			{0x06, "groups_report", 'r', 0, ""},
			{0x04, "remove",				's', 0, ""},
			{0x03, "report",				'r', 0, ""},
			{0x01, "set",						's', 0, ""},
		},
	},
	{0x85, 2, "association", 8, {
			{0x02, "get",										'g', 0, ""},
			{0x05, "groups_get",						'g', 0, ""}, 
			{0x06, "groups_report",					'r', 0, ""},
			{0x04, "remove",								's', 0, ""},
			{0x03, "report",								'r', 0, ""},
			{0x01, "set",										's', 0, ""},
			{0x0B, "specific_group_get",		'g', 0, ""},
			{0x0C, "specfic_group_report",	'r', 0, ""},
		},
	},

	{0x59, 1, "association_grp_info", 6,  {
			{0x01, "group_name_get", 'g', 0, ""},
			{0x02, "group_name_rpt", 'r', 0, ""},
			{0x03, "group_info_get", 'g', 0, ""},
			{0x04, "group_info_rpt", 'r', 0, ""},
			{0x05, "group_cmd_list_get", 'g', 0, ""},
			{0x06, "group_cmd_list_rpt", 'r', 0, ""},
		},
	},

	{0x86, 1, "version", 4, {
			{0x13, "cmd_class_get", 'g', 0, ""},
			{0x14, "cmd_class_rpt", 'r', 0, ""},
			{0x11, "get", 'g', 0, ""},
			{0x12, "rpt", 'r', 0, ""},
		},
	},
	{0x86, 2, "version", 4, {
			{0x13, "cmd_class_get", 'g', 0, ""},
			{0x14, "cmd_class_rpt", 'r', 0, ""},
			{0x11, "get", 'g', 0, ""},
			{0x12, "rpt", 'r', 0, ""},
		},
	},

	{0x72, 1, "manufacturer_specific", 2, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
		},
	},
	{0x72, 2, "manufacturer_specific", 4, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
			{0x06, "dev_specific_get", 'g', 0, ""},
			{0x07, "dev_specific_rpt", 'r', 0, ""},
		},
	},

	{0x5a, 1, "device_reset_locally", 1, {
			{0x01, "notifaction", 'r', 0, ""},
		},
	},

	{0x80, 1, "battery", 2, {
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
		},
	},

	{0x84, 1, "wake_up", 5, {
			{0x05, "interval_get", 'g', 0, ""},
			{0x06, "interval_rpt", 'r', 0, ""},
			{0x04, "interval_set", 's', 0, ""},
			{0x08, "no_more_info", 's', 0, ""},
			{0x07, "notifaction",  'r', 0, ""},
		},
	},
	{0x84, 2, "wake_up", 7, {
			{0x09, "interval_cap_get", 'g', 0, ""},
			{0x0a, "interval_cap_rpt", 'r', 0, ""},
			{0x05, "interval_get", 'g', 0, ""},
			{0x06, "interval_rpt", 'r', 0, ""},
			{0x04, "interval_set", 's', 0, ""},
			{0x08, "no_more_info", 's', 0, ""},
			{0x07, "notifaction",  'r', 0, ""},
		},
	},

	{0x71, 1, "alarm", 2, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
		},
	},
	{0x71, 2, "alarm", 5, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
			{0x06, "set", 's', 0, ""},
			{0x07, "type_supported_get", 'g', 0, ""},
			{0x08, "type_supported_rpt", 'r', 0, ""},
		},
	},
	{0x071, 3, "notifaction", 7, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
			{0x06, "set", 's', 0, ""},
			{0x07, "noti_supported_get", 'g', 0, ""},
			{0x08, "noti_supported_rpt", 'r', 0, ""},
			{0x01, "evt_supported_get", 'g', 0, ""},
			{0x02, "evt_supported_rpt", 'r', 0, ""},
		},
	},
	{0x71, 4, "notifaction", 7, {
			{0x04, "get", 'g', 0, ""},
			{0x05, "rpt", 'r', 0, ""},
			{0x06, "set", 's', 0, ""},
			{0x07, "noti_supported_get", 'g', 0, ""},
			{0x08, "noti_supported_rpt", 'r', 0, ""},
			{0x01, "evt_supported_get", 'g', 0, ""},
			{0x02, "evt_supported_rpt", 'r', 0, ""},
		},
	},

	{0x26, 1, "switch_mul", 5, {
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
			{0x04, "start_level_change", 's', 0, ""},
			{0x05, "stop_level_change", 's', 0, ""},
		},
	},
	{0x26, 2, "switch_mul", 5, {
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
			{0x04, "start_level_change", 's', 0, ""},
			{0x05, "stop_level_change", 's', 0, ""},
		},
	},
	{0x26, 3, "switch_mul", 7, {
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
			{0x04, "start_level_change", 's', 0, ""},
			{0x05, "stop_level_change", 's', 0, ""},
			{0x06, "supported_get", 'g', 0, ""},
			{0x07, "supported_rpt", 'r', 0, ""}
		},
	},

	{0x27, 1, "switch_all", 5, {
			{0x02, "get", 'g', 0, ""},
			{0x05, "off", 's', 0, ""},
			{0x04, "on",  's', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
		},
	},

	{0x75, 1, "protection", 3, {
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
		},
	},
	{0x75, 2, "protection", 11, {
			{0x07, "ec_get", 'g', 0, ""},
			{0x08, "ec_rpt", 'r', 0, ""},
			{0x06, "ec_set", 's', 0, ""},
			{0x02, "get", 'g', 0, ""},
			{0x03, "rpt", 'r', 0, ""},
			{0x01, "set", 's', 0, ""},
			{0x04, "supported_get", 'g', 0, ""},
			{0x05, "supported_rpt", 'r', 0, ""},
			{0x0a, "timeout_get", 'g', 0, ""},
			{0x0b, "timeout_rpt", 'r', 0, ""},
			{0x09, "timeout_set", 's', 0, ""},
		},
	},

	{0x70, 1, "configuration", 3, {
			{0x05, "get", 'g', 0, ""},
			{0x06, "rpt", 'r', 0, ""},
			{0x04, "set", 's', 0, ""},
		}
	},
	{0x70, 2, "configuration", 6, {
			{0x08, "bulk_get", 'g', 0, ""},
			{0x09, "bulk_rpt", 'r', 0, ""},
			{0x07, "bulk_set", 's', 0, ""},
			{0x05, "get", 'g', 0, ""},
			{0x06, "rpt", 'r', 0, ""},
			{0x04, "set", 's', 0, ""},
		},
	},

	{0x60, 1, "multi_instance", 3, {
			{0x06, "ins_cmd_encap", 's', 0, ""},
			{0x04, "ins_get", 'g', 0, ""},
			{0x05, "ins_rpt", 'r', 0, ""},
		},
	},
	{0x60, 2, "multi_channel", 10, {
			{0x09, "cap_get", 'g', 0, ""},
			{0x0a, "cap_rpt", 'r', 0, ""},
			{0x0d, "cmd_encap", 's', 0, ""},
			{0x0b, "ep_find", 'g', 0, ""},
			{0x0c, "ep_find_rpt", 'r', 0, ""},
			{0x07, "ep_get", 'g', 0, ""},
			{0x08, "ep_rpt", 'r', 0, ""},
			{0x06, "ins_cmd_encap", 's', 0, ""},
			{0x04, "ins_get", 'g', 0, ""},
			{0x05, "ins_rpt", 'r', 0, ""},
		},
	},
	{0x60, 3, "multi_channel", 10, {
			{0x09, "cap_get", 'g', 0, ""},
			{0x0a, "cap_rpt", 'r', 0, ""},
			{0x0d, "cmd_encap", 'r', 0, ""},
			{0x0b, "ep_find", 'g', 0, ""},
			{0x0c, "ep_find_rpt", 'r', 0, ""},
			{0x07, "ep_get", 'g', 0, ""},
			{0x08, "ep_rpt", 'r', 0, ""},
			{0x06, "ins_cmd_encap", 's', 0, ""},
			{0x04, "ins_get", 'g', 0, ""},
			{0x05, "ins_rpt", 'r', 0, ""},
		},
	},

	{0x30, 1, "Sensor Binary", 2, {
			{0x02, "snr_get", 'r', 0, ""},
			{0x03, "snr_rpt", 'r', 0, ""},
		},
	},
	{0x30, 2, "Sensor Binary", 4, {
			{0x01, "rpt_snr_get", 'g', 0, ""},
			{0x02, "snr_get",			'g', 0, ""},
			{0x03, "snr_rpt",			'r', 0, ""},
			{0x04, "rpt_snr_rpt", 'r', 0, ""},
		},
	},
};

stZWClass_t *zcc_get_class(char classid, int version) {
	int i = 0;
	int cnt = sizeof(zcc_ccs)/sizeof(zcc_ccs[0]);
	
	stZWClass_t *ret = NULL;
	
	for (i = 0; i < cnt; i++) {
		stZWClass_t *class = &zcc_ccs[i];
		if (class->classid != classid) {
			continue;
		}
		
		if (class->version == version) {
			return class;
		}
		
		if (ret == NULL) {
			ret = class;
		} else {
			if (class->version > ret->version) {
				ret = class;
			}
		}
	}

	return ret;
}
stZWCmd_t *zcc_get_cmd(stZWClass_t *class, char cmdid) {
	int i = 0;
	int cnt = class->cmdcnt;
	for (i = 0; i < cnt; i++) {
		stZWCmd_t *cmd = &class->cmds[i];
		if (cmd->cmdid == cmdid) {
			return cmd;
		}
	}
	return NULL;
}

int zcc_get_class_cmd_rpt(stZWClass_t *class, char cmds[MAX_CMD_NUM]) {
	int i = 0;
	int cnt = class->cmdcnt;
	int j = 0;
	for (i = 0; i < cnt; i++) {
		stZWCmd_t *cmd = &class->cmds[i];
		if (cmd->type == 'r') {
			cmds[j++] = cmd->cmdid;
		}
	}

	return j;
}

const char *zcc_get_class_name(char classid, int version) {
	stZWClass_t *class = zcc_get_class(classid, version);
	if (class == NULL) {
		return "unkown";
	}
	return class->name;
}

const char *zcc_get_cmd_name(char classid,int version, char cmdid) {
	stZWClass_t *class = zcc_get_class(classid, version);
	if (class == NULL) {
		return "unkown";
	}
	stZWCmd_t *cmd = zcc_get_cmd(class, cmdid);
	if (cmd == NULL) {
		return "unkown";
	}
	return cmd->name;

}
