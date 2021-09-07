#ifndef __ZWAVE_CCDB_H_
#define __ZWAVE_CCDB_H_

#include "zwave_class_cmd.h"

int zwave_ccdb_init();
int zwave_ccdb_uninit();
int zwave_ccdb_exsit();
int zwave_ccdb_get_class_cmd_rpt(char classid, int version, char cmds[MAX_CMD_NUM]);
const char *zwave_ccdb_get_class_cmd_name(char classid, int version, char cmd);
const char *zwave_ccdb_get_class_name(char classid, int version);


const char *zwave_ccdb_get_generic_specific_name(char generic, char specific, char *gname, char *sname);
const char *zwave_ccdb_get_basic_name(char basic, char *bname);

#endif
