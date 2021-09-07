#ifndef __ZWAVE_CLASS_CMD_H_
#define __ZWAVE_CLASS_CMD_H_

#define MAX_CMD_NUM 40

typedef struct stZWCmd {
	char				cmdid;
	const				char *name;
	char				type;		//read, write, report
	int					len;
	char				*defparam;
}stZWCmd_t;

typedef struct stZWClass {
	char				classid;
	int					version;
	const	char	*name;
	int					cmdcnt;
	stZWCmd_t		cmds[MAX_CMD_NUM];
}stZWClass_t;

stZWClass_t *zcc_get_class(char classid, int version);
stZWCmd_t *zcc_get_cmd(stZWClass_t *class, char cmdid);
int zcc_get_class_cmd_rpt(stZWClass_t *class, char cmds[MAX_CMD_NUM]);

const char *zcc_get_class_name(char classid, int version);
const char *zcc_get_cmd_name(char classid, int version, char cmdid);
#endif


