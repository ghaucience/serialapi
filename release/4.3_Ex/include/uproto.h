#ifndef _AMBER_UPROTO_H_
#define _AMBER_UPROTO_H_


#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"

#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubus.h>
#include <jansson.h>

//#define UPROTO_EVENT_ID_LISTEN "DS.GREENPOWER"
#define UPROTO_EVENT_ID_LISTEN "DS.ZWAVE"
#define UPROTO_EVENT_ID_REPORT "DS.GATEWAY"
#define UPROTO_ME								"ZWAVE"

enum emUprotoError{
	CODE_SUCCESS									= 0,			// 成功
	CODE_NXP_SECURITY_HANDINGSHAKE= 96,			// NXP加锁和设备时，设备已经通讯上，正在进行加密握手操作
	CODE_UPGRADE_STARTED					= 97,			// 升级开始
	CODE_UPGRADE_SUCCESS					= 98,			// 升级成功
	CODE_WAIT_TO_EXECUTE					= 99,			// 命令已经收到，等待执行
	CODE_UNKNOW_ERROR							= 199,		// 未知错误
	CODE_WRONG_FORMAT							= 101,		// 报文格式错误
	CODE_UNKNOW_DEVICE						= 102,		// 未知的设备，指定的设备不存在
	CODE_UNKNOW_ATTRIBUTE					= 103,		// 未知的属性值
	CODE_TIMEOUT									= 104,		// 操作超时
	CODE_BUSY											= 105,		// 设备忙，已经有待执行的命令，且这个命令不能同时进行
	CODE_UNKNDOW_CMD							=	106,		// 未知的命令
	CODE_NOT_SUPPORTED						= 107,		// 不支持的操作
	CODE_UPGRADE_MD5SUM_FAILED		= 108,		// 升级时MD5SUM校验失败
	CODE_UPGRADE_DOWNLOAD_FAILED	= 109,		// 升级时固件下载失败
	CODE_UPGRADE_FAILED						= 110,		// 升级失败
	CODE_PASSWORD_ALREADY_EXISTS	= 111,		// 密码已经存在
	CODE_PASSWORD_FULL						= 112,		// 密码表已经满了
	CODE_PASSWORD_NOT_EXISTS			= 113,		// 要删除的密码不成功
	CODE_MINUS_1									= -1,			// 未执行且不再执行的命令，由服务端更新
}emUprotoError_t;

enum {
	UE_SEND_MSG		= 0x00,
};

enum {
	PROTO_DUSUN = 0x01,
};

typedef struct stUprotoEnv {
	struct file_event_table *fet;
	struct timer_head *th;
	struct timer step_timer;
	
	stLockQueue_t msgq;

	struct ubus_context *ubus_ctx;
	struct ubus_event_handler listener;

	int	protoused;		/* 0-> dusun, 1->alink */
}stUprotoEnv_t;

typedef int (*UPROTO_HANDLER)(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
typedef struct stUprotoCmd {
	const char *name;
	UPROTO_HANDLER handler;
}stUprotoCmd_t;

typedef int (*UPROTO_CMD_GET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef int (*UPROTO_CMD_SET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef int (*UPROTO_CMD_RPT)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef struct stUprotoAttrCmd {
	const char *name;
	UPROTO_CMD_GET get;
	UPROTO_CMD_SET set;
	UPROTO_CMD_RPT rpt;
}stUprotoAttrCmd_t;


int uproto_init(void *_th, void *_fet);
int uproto_step();
int uproto_push_msg(int eid, void *param, int len);
void uproto_run(struct timer *timer);
void uproto_in(void *arg, int fd);


int uproto_rpt_register(const char *extaddr);
int uproto_rpt_unregister(const char *extaddr);
int uproto_rpt_status(const char *extaddr);
int uproto_rpt_attr(const char *extaddr, unsigned char ep, unsigned char clsid, const char *buf, int len);
int uproto_rpt_cmd(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len);


typedef int (*PROTO_CMD_HANDLER)(const char *cmd);
typedef int (*PROTO_RPT_REGISTER)(const char *extaddr);
typedef int (*PROTO_RPT_UNREGISTER)(const char *extaddr);
typedef int (*PROTO_RPT_STATUS)(const char *extaddr);
typedef int (*PROTO_RPT_ATR)(const char *extaddr, unsigned char ep, unsigned char clsid, const char *buf, int len);
typedef int (*PROTO_RPT_CMD)(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len);
typedef struct stProto {
	PROTO_CMD_HANDLER			handler;
	PROTO_RPT_REGISTER		rptregister;
	PROTO_RPT_UNREGISTER	rptunregister;
	PROTO_RPT_STATUS			rptstatus;
	PROTO_RPT_ATR					rptatr;
	PROTO_RPT_CMD					rptcmd;
}stProto_t;

/* dusun */
int uproto_handler_cmd_dusun(const char *cmd);
int uproto_rpt_register_dusun(const char *extaddr);
int uproto_rpt_unregister_dusun(const char *extaddr);
int uproto_rpt_status_dusun(const char *extaddr);
int uproto_rpt_attr_dusun(const char *extaddr, unsigned char ep, unsigned char clsid, const char *buf, int len);
int uproto_rpt_cmd_dusun(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len);
#endif
