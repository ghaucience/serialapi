#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//#include "zwave_class_cmd.h"
#include "zwave_ccdb.h"
#include "jansson.h"
#include "log.h"

static const char *zwc_file="/etc/config/dusun/zwdev/ZWave_custom_cmd_classes.xml";

static xmlDocPtr   xdoc = NULL;
static xmlNodePtr  xroot = NULL;

int zwave_ccdb_init() {
  xmlKeepBlanksDefault(0);

	if (access(zwc_file, F_OK) != 0) {
		log_warn("not exsit file:%s", zwc_file);
		return -1;
	}

  xdoc = xmlReadFile (zwc_file, NULL, XML_PARSE_RECOVER);
	if (xdoc == NULL) {
		log_warn("can't load zwave cmd class file!");
		return -2;
	}

  xroot = xmlDocGetRootElement (xdoc);
	if (xroot == NULL) {
		log_warn("can't get root !");

		xmlFreeDoc(xdoc);
		xmlCleanupParser();
		xmlMemoryDump();
		xdoc = NULL;
		xroot = NULL;
		return -3;
	}

	log_info("load zwave cmd class file ok!");
	char cmds[MAX_CMD_NUM];
	zwave_ccdb_get_class_cmd_rpt(0x64, 2, cmds);

	return 0;
}

int zwave_ccdb_uninit() {
	xmlFreeDoc(xdoc);
  xmlCleanupParser();
  xmlMemoryDump();
	return 0;
}


int zwave_ccdb_exsit() {
	return (xroot != NULL);
}

const char *zwave_ccdb_get_class_name(char classid, int version) {
	if (xroot == NULL) {
		return  "unknown";
	}

  xmlNodePtr pcur = xroot->xmlChildrenNode;
	while (pcur != NULL) {
		if (strcmp((char *)pcur->name, "cmd_class") != 0) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *skey= xmlGetProp(pcur, BAD_CAST"key");
		if (skey == NULL || strlen((char *)skey) != 4) {
			pcur = pcur->next;
			continue;
		}		
		int ikey = 0;
		if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
			pcur = pcur->next;
			continue;
		}
		char ckey = ikey&0xff;
		
		if (ckey != classid) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *sver= xmlGetProp(pcur, BAD_CAST"version");
		int iver = 0;
		if (sscanf((char *)sver, "%d", &iver) != 1) {
			pcur = pcur->next;
			continue;
		}
		if (iver != version) {
			pcur = pcur->next;
			continue;
		}
	
		xmlChar *name = xmlGetProp(pcur, BAD_CAST"name");
		if (name == NULL) {
			return "unknown";
		} else {
			return (const char *)name;
		}
	}

	return "unknown";
}
const char *zwave_ccdb_get_class_cmd_name(char classid, int version, char cmd) {
	if (xroot == NULL) {
		return  "unknown";
	}

  xmlNodePtr pcur = xroot->xmlChildrenNode;
	while (pcur != NULL) {
		if (strcmp((char *)pcur->name, "cmd_class") != 0) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *skey= xmlGetProp(pcur, BAD_CAST"key");
		if (skey == NULL || strlen((char *)skey) != 4) {
			pcur = pcur->next;
			continue;
		}		
		int ikey = 0;
		if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
			pcur = pcur->next;
			continue;
		}
		char ckey = ikey&0xff;
		
		if (ckey != classid) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *sver= xmlGetProp(pcur, BAD_CAST"version");
		int iver = 0;
		if (sscanf((char *)sver, "%d", &iver) != 1) {
			pcur = pcur->next;
			continue;
		}
		if (iver != version) {
			pcur = pcur->next;
			continue;
		}
		//log_info("key:%02X, version:%d", ckey, iver);
	
		xmlNodePtr pcmd_cur = pcur->xmlChildrenNode;
		while (pcmd_cur != NULL) {
			if (strcmp((char *)pcmd_cur->name, "cmd") != 0) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}

			xmlChar *skey= xmlGetProp(pcmd_cur, BAD_CAST"key");
			if (skey == NULL || strlen((char *)skey) != 4) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}		
			int ikey = 0;
			if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}
			char ckey = ikey&0xff;
			if (ckey != cmd) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}

			xmlChar *name = xmlGetProp(pcmd_cur, BAD_CAST"name");
			if (name == NULL) {
				return "unknown";
			} else {
				return (const char *)name;
			}
		}

		pcur = pcur->next;
	}

	return "unknown";
}
	

int zwave_ccdb_get_class_cmd_rpt(char classid, int version, char cmds[MAX_CMD_NUM]) {
	if (xroot == NULL) {
		return 0;
	}

	int cnt = 0;

  xmlNodePtr pcur = xroot->xmlChildrenNode;
	while (pcur != NULL) {
		if (strcmp((char *)pcur->name, "cmd_class") != 0) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *skey= xmlGetProp(pcur, BAD_CAST"key");
		if (skey == NULL || strlen((char *)skey) != 4) {
			pcur = pcur->next;
			continue;
		}		
		int ikey = 0;
		if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
			pcur = pcur->next;
			continue;
		}
		char ckey = ikey&0xff;
		
		if (ckey != classid) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *sver= xmlGetProp(pcur, BAD_CAST"version");
		int iver = 0;
		if (sscanf((char *)sver, "%d", &iver) != 1) {
			pcur = pcur->next;
			continue;
		}
		if (iver != version) {
			pcur = pcur->next;
			continue;
		}
		log_info("key:%02X, version:%d", ckey&0xff, iver&0xff);
	
		xmlNodePtr pcmd_cur = pcur->xmlChildrenNode;
		while (pcmd_cur != NULL) {
			if (strcmp((char *)pcmd_cur->name, "cmd") != 0) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}

			xmlChar *skey= xmlGetProp(pcmd_cur, BAD_CAST"key");
			if (skey == NULL || strlen((char *)skey) != 4) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}		
			int ikey = 0;
			if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
				pcmd_cur = pcmd_cur->next;
				continue;
			}
			char ckey = ikey&0xff;

			//log_info("cmd key:%02X", ckey);

			cmds[cnt++] = ckey;

			pcmd_cur = pcmd_cur->next;
		}

		pcur = pcur->next;
	}

	log_debug_hex("cmds", cmds, cnt);
	
	return cnt;
}

const char *zwave_ccdb_get_basic_name(char basic, char *bname) {
	if (xroot == NULL) {
		return  "unknown";
	}

  xmlNodePtr pcur = xroot->xmlChildrenNode;
	while (pcur != NULL) {
		if (strcmp((char *)pcur->name, "bas_dev") != 0) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *skey= xmlGetProp(pcur, BAD_CAST"key");
		if (skey == NULL || strlen((char *)skey) != 4) {
			pcur = pcur->next;
			continue;
		}		
		int ikey = 0;
		if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
			pcur = pcur->next;
			continue;
		}
		char ckey = ikey&0xff;
		
		if ((ckey&0xff) != (basic&0xff)) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *sname= xmlGetProp(pcur, BAD_CAST"help");
		if (sname == NULL) {
			sname= xmlGetProp(pcur, BAD_CAST"name");
		}
		if (sname == NULL) {
			strcpy(bname, "unknown");
		} else {
			strcpy(bname, (char *)sname);
		}
		return bname;
	}

	strcpy(bname, "unknown");
	return bname;
}

const char *zwave_ccdb_get_generic_specific_name(char generic, char specific, char *gname, char *sname) {
	if (xroot == NULL) {
		return  "unknown";
	}

	strcpy(gname, "unknown");
	strcpy(sname, "unknown");

  xmlNodePtr pcur = xroot->xmlChildrenNode;
	while (pcur != NULL) {
		if (strcmp((char *)pcur->name, "gen_dev") != 0) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *skey= xmlGetProp(pcur, BAD_CAST"key");
		if (skey == NULL || strlen((char *)skey) != 4) {
			pcur = pcur->next;
			continue;
		}		
		int ikey = 0;
		if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
			pcur = pcur->next;
			continue;
		}
		char ckey = ikey&0xff;
		
		if ((ckey&0xff) != (generic&0xff)) {
			pcur = pcur->next;
			continue;
		}

		xmlChar *_sname= xmlGetProp(pcur, BAD_CAST"help");
		if (_sname == NULL) {
			_sname= xmlGetProp(pcur, BAD_CAST"name");
		}
		if (_sname == NULL) {
			strcpy(gname, "unknown");
		} else {
			strcpy(gname, (char *)_sname);
		}
	
		xmlNodePtr pspe_cur = pcur->xmlChildrenNode;
		while (pspe_cur != NULL) {
			if (strcmp((char *)pspe_cur->name, "spec_dev") != 0) {
				pspe_cur = pspe_cur->next;
				continue;
			}

			xmlChar *skey= xmlGetProp(pspe_cur, BAD_CAST"key");
			if (skey == NULL || strlen((char *)skey) != 4) {
				pspe_cur = pspe_cur->next;
				continue;
			}		
			int ikey = 0;
			if (sscanf((char *)skey, "0x%02X", &ikey) != 1) {
				pspe_cur = pspe_cur->next;
				continue;
			}
			char ckey = ikey&0xff;
			if ((ckey&0xff) != (specific&0xff)) {
				pspe_cur = pspe_cur->next;
				continue;
			}

			xmlChar *_sname= xmlGetProp(pspe_cur, BAD_CAST"help");
			if (_sname == NULL) {
				_sname= xmlGetProp(pspe_cur, BAD_CAST"name");
			}
			if (_sname == NULL) {
				strcpy((char *)sname, (char *)gname);
			} else {
				strcpy((char *)sname, (char *)_sname);
			}
			break;
		}
		break;
	}

	return sname;
}

