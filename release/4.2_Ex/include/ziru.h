#ifndef __ZIRU_H_
#define __ZIRU_H_


#include "jansson.h"


int ziru_init(const char *server, int port);






int ziru_connect(const char *devId, const char *prodTypeId) ;
int ziru_disconnect(const char *devId, const char *prodTypeId) ;

int ziru_info_query(const char *pid, const char *did, json_t *jmsg) ;
int ziru_info_report(const char *devId, const char *prodTypeId) ;
int ziru_meta(const char *devId, const char *prodTypeId) ;

int ziru_control(const char *pid, const char *did, json_t *jmsg) ;
int ziru_control_resp(const char *devId, const char *prodTypeId, const char *sno, const char *attribute, const char *command, const char *key, const char *value) ;

int ziru_notify(const char *devId, const char *prodTypeId, const char *attribute, const char *command, const char *key, const char *value) ;

int ziru_notify_array(const char *devId, const char *prodTypeId, const char *attribute, const char *command, const char **keys, const char **values, int cnt);

int ziru_unknown(const char *pid, const char *did, json_t *jmsg) ;


int ziru_control_cmd_result(const char *uuid, int code);

#endif
