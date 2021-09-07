#ifndef __ZWAVE_DEVICE_H_
#define __ZWAVE_DEVICE_H_

#include "schedule.h"

#define MAX_CMD_DATA_LEN		32
#define MAX_DEV_NUM					50

/* when a command has been received, wo query a command table and decied if it can be store in the command data part, 
 * if yes, we only store the data to the data pointer
 * if not, we parse it to a parse function to parse it to a plat data such as endpoint ...
 */
typedef struct stZWaveCommand {
	char						cmdid;
	int							len;
	char						data[32];
	int							last;
	int							nbit;
	int							nbit_next;
} stZWaveCommand_t;

typedef struct stZWaveClass {
	char						classid;
	int							version;
	int							cmdcnt;
	stZWaveCommand_t	*cmds;
	int							nbit;
	int							nbit_next;
	int							nbit_cmds_head;
}stZWaveClass_t;

typedef struct stZWaveEndPoint {
	char	ep;

	char	basic;
	char	generic;
	char	specific;

	int		classcnt;
	stZWaveClass_t *classes;

		int		nbit;
	int   nbit_next;
	int		nbit_classes_head;
}stZWaveEndPoint_t;

enum {
	DS_ADDED								= 0x00,

	DS_READ_VERSION					= 0X01,
	DS_READ_VERSION_ING			= 0x02,
	DS_READ_VERSION_SUCCSSS	= 0x03,
	DS_READ_VERSION_FAILED	= 0x04,

	DS_READ_ASSOC_GRP					= 0x05,
	DS_READ_ASSOC_GRP_ING			= 0x06,
	DS_READ_ASSOC_GRP_SUCCESS = 0x07,
	DS_READ_ASSOC_GRP_FAILED	= 0x08,

	DS_ASSOCIATE							= 0x09,
	DS_ASSOCIATING						= 0x10,
	DS_ASSOCIATE_SUCCESS			= 0x11,
	DS_ASSOCIATE_FAILED				= 0x12,

	DS_READ_RPT_VALUE					= 0x13,
	DS_READ_RPT_VALUEING			= 0x14,
	DS_READ_RPT_VALUE_SUCCESS	= 0x15,
	DS_READ_RPT_VALUE_FAILED	= 0x16,

	DS_WORKED									= 0x17,

	DS_REMOVED								= 0x18,
};
typedef struct stReadRptCmd {
	unsigned char class;
	unsigned char cmd;
	unsigned char rptcmd;
	unsigned char rflag;
} stReadRptCmd_t;
typedef struct stZWaveDevice {
	char							bNodeID;
	char							security;
	char							capability;

	int								used;
	int								state;
	stSchduleTask_t		task;
	int								trycnt;
	int								tvar;
	int								rrcs_size;
	stReadRptCmd_t		rrcs[8];
	int								reserved[2];

	stZWaveEndPoint_t	root;
	int								subepcnt;
	stZWaveEndPoint_t *subeps;
	int								nbit;
	int								nbit_subeps_head;

	//int							online;
	long							last;
	char							mac[8];
}stZWaveDevice_t;

typedef struct stZWaveCache {
	int								devcnt;
	stZWaveDevice_t		devs[MAX_DEV_NUM];
}stZWaveCache_t;


int device_add(char bNodeID, char basic, char generic, char specific, char security, char capability, int classcnt, char *classes); 
int device_del(char bNodeID);
stZWaveDevice_t *device_get_by_nodeid(char bNodeID);
stZWaveDevice_t *device_get_by_extaddr(char extaddr[8]);

int device_add_subeps(stZWaveDevice_t *zd, int epcnt, char *eps);
int device_fill_ep(stZWaveEndPoint_t *ze, char ep, char basic, char generic, char specific, int classcnt, char *classes);
stZWaveEndPoint_t *device_get_subep(stZWaveDevice_t *zd, char ep);

int device_add_cmds(stZWaveClass_t *class, int cmdcnt, char *cmds);
stZWaveCommand_t *device_get_cmd(stZWaveClass_t *class, char cmd);
int device_update_cmds_data(stZWaveCommand_t *zc, char *data, int len);


char *device_get_extaddr(stZWaveDevice_t *zd);


stZWaveClass_t *device_get_class(stZWaveDevice_t *zd, char ep, char classid);


const char *device_make_macstr(stZWaveDevice_t *zd);
int device_get_battery(stZWaveDevice_t *zd);
int device_get_online(stZWaveDevice_t *zd);
const char *device_make_modelstr(stZWaveDevice_t *zd);
const char *device_make_modelstr_new(stZWaveDevice_t *zd);
int device_is_lowpower(stZWaveDevice_t *zd);
const char *device_make_typestr(stZWaveDevice_t *zd);
const char *device_make_versionstr(stZWaveDevice_t *zd);
char device_get_nodeid_by_mac(const char *mac);

const char *device_sensor_binary_zone_typestr(int i);
const char *device_sensor_binary_zonestr(int i);

int device_get_pir_status(stZWaveDevice_t *zd);

void device_view_all();
void devcie_view_all_simple();
void devcie_view(unsigned char nodeid);

int device_get_class_no_version(stZWaveDevice_t *zd);
int device_exsit_assoc_class(stZWaveDevice_t *zd);

int device_set_read_rpt_cmd(stZWaveDevice_t *zd);

int device_get_read_rpt_cmd(stZWaveDevice_t *zd, unsigned char *class, unsigned char *cmd);

int device_set_read_rpt_cmd_by_class_cmd(stZWaveDevice_t *zd, unsigned char class, unsigned cmd);
#endif
