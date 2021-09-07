#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "log.h"
#include "jansson.h"
#include "json_parser.h"

#include "uproto.h"

static stProto_t protos[] = {
	{	// 0 -> dusun
		uproto_handler_cmd_dusun,
		uproto_rpt_register_dusun,
		uproto_rpt_unregister_dusun,
		uproto_rpt_status_dusun,
		uproto_rpt_attr_dusun,
		uproto_rpt_cmd_dusun,
	},
	{	// 1 -> alink
		uproto_handler_cmd_dusun,
		uproto_rpt_register_dusun,
		uproto_rpt_unregister_dusun,
		uproto_rpt_status_dusun,
		uproto_rpt_attr_dusun,
		uproto_rpt_cmd_dusun,
	},
};

/* uproto env, default to alink proto*/
static stUprotoEnv_t ue = {
	.protoused = PROTO_DUSUN,
	.fet = NULL,
	.th = NULL, 
	.step_timer = {0},
};

/* Miscellaneous */
static struct blob_buf b;
static void ubus_receive_event(struct ubus_context *ctx,
		struct ubus_event_handler *ev, 
		const char *type,
		struct blob_attr *msg);


int uproto_init(void *_th, void *_fet) {
	log_debug(" ");
	ue.th = _th;
	ue.fet = _fet;

	extern void parse_type_load();
	parse_type_load();

	timer_init(&ue.step_timer, uproto_run);
	
	lockqueue_init(&ue.msgq);

	ue.ubus_ctx = ubus_connect(NULL);
	memset(&ue.listener, 0, sizeof(ue.listener));
	ue.listener.cb = ubus_receive_event;
	ubus_register_event_handler(ue.ubus_ctx, &ue.listener, UPROTO_EVENT_ID_LISTEN);
  file_event_reg(ue.fet, ue.ubus_ctx->sock.fd, uproto_in, NULL, NULL);

	return 0;
}

/* Module Driver */
static int uproto_handler_event(stEvent_t *e) {
	if (e == NULL) {
		return 0;
	}

	if (e->eid == UE_SEND_MSG && e->param != NULL) {
		json_t *jmsg = e->param;

		char *smsg= json_dumps(jmsg, 0);
		if (smsg != NULL) {
			blob_buf_init(&b, 0);
			blobmsg_add_string(&b, "PKT", smsg);
			ubus_send_event(ue.ubus_ctx, UPROTO_EVENT_ID_REPORT, b.head);
			free(smsg);
		}

		json_decref(jmsg);
	}
	return 0;
}

int uproto_step() {
	/* TODO */
	timer_cancel(ue.th, &ue.step_timer);
	timer_set(ue.th, &ue.step_timer, 10);
	return 0;
}

void uproto_run(struct timer *timer) {

	stEvent_t *e = NULL;
	if (lockqueue_pop(&ue.msgq, (void **)&e) && e != NULL) {
		uproto_handler_event(e);
		FREE(e);
		uproto_step();
	}
}

int uproto_push_msg(int eid, void *param, int len) {
	if (eid == UE_SEND_MSG) {
		stEvent_t *e = MALLOC(sizeof(stEvent_t));
		if (e == NULL) {
			return -1;
		}
		e->eid = eid;
		e->param = param;
		lockqueue_push(&ue.msgq, e);
		uproto_step();
	}
	return 0;
}


void uproto_in(void *arg, int fd) {
	ubus_handle_event(ue.ubus_ctx);
}

//Receive/////////////////////////////////////////////////////////////////////////////
static void ubus_receive_event(struct ubus_context *ctx,struct ubus_event_handler *ev, 
		const char *type,struct blob_attr *msg) {
	char *str;

	log_debug("-----------------[ubus msg]: handler ....-----------------");
	str = blobmsg_format_json(msg, true);
	if (str != NULL) {
		log_debug("[ubus msg]: [%s]", str);

		json_error_t error;
		json_t *jmsg = json_loads(str, 0, &error);
		if (jmsg != NULL) {
			const char *spkt = json_get_string(jmsg, "PKT");
			if (spkt != NULL) {
				log_debug("pks : %s", spkt);
				uproto_handler_cmd_dusun(spkt);
			} else {
				log_debug("not find 'PKT' item!");
			}
			json_decref(jmsg);
		} else {
			log_debug("error: on line %d: %s", error.line, error.text);
		}
		free(str);
	} else {
		log_debug("[ubus msg]: []");
	}
	log_debug("-----------------[ubus msg]: handler over-----------------");
}

//rpt interface///////////////////////////////////////////////////////////////////////////////////////
int uproto_rpt_register(const char *extaddr) {
	log_debug("[%s]", __func__);
	
	protos[ue.protoused].rptregister(extaddr);

	return 0;
}
int uproto_rpt_unregister(const char *extaddr) {
	log_debug("[%s]", __func__);

	protos[ue.protoused].rptunregister(extaddr);

	return 0;
}
int uproto_rpt_status(const char *extaddr) {
	log_debug("[%s]", __func__);

	protos[ue.protoused].rptstatus(extaddr);

	return 0;
}
int uproto_rpt_attr(const char *extaddr, unsigned char ep,  unsigned char clsid, const char *buf, int len) {
	log_debug("[%s]", __func__);

	protos[ue.protoused].rptatr(extaddr, ep, clsid,  buf, len);

	return 0;
}
int uproto_rpt_cmd(const char *extaddr, unsigned char ep, unsigned char clsid, unsigned char cmdid, const char *buf, int len) {
	log_debug("[%s]", __func__);

	protos[ue.protoused].rptcmd(extaddr, ep, clsid, cmdid,  buf, len);

	return 0;
}


