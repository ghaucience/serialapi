#include "web.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"
#include "jansson.h"
#include "json_parser.h"
#include "zwave.h"
#include "zwave_device.h"

static stWebEnv_t we = {
	.ip		= "192.168.0.6",
	.port = 10888,
	.base	= "./www",
	.svr	= NULL,
	.user = "admin",
	.pass = "admin",
	//.currpage = 3,
	.th = NULL,
	.fet = NULL,

};

static int include_flag = 0;
static int exclude_flag = 0;

static void center_func(httpd *server, httpReq *req);
static void include_func(httpd *server, httpReq *req);
static void exclude_func(httpd *server, httpReq *req);
static void list_func(httpd *server, httpReq *req);

static stWebNode_t wns[] = {
	{PT_WILD, "/image",		INVALID_ITEM,	INVALID_ITEM,	NULL, INVALID_ITEM,		"image",			INVALID_ITEM},

	{PT_WILD, "/css",			INVALID_ITEM,		INVALID_ITEM,	NULL, INVALID_ITEM,		"css",				INVALID_ITEM},
	{PT_WILD, "/js",			INVALID_ITEM,		INVALID_ITEM,	NULL, INVALID_ITEM,		"js",					INVALID_ITEM},
	{PT_WILD, "/site",		INVALID_ITEM,		INVALID_ITEM,	NULL, INVALID_ITEM,		"site",				INVALID_ITEM},
	{PT_WILD, "/font",		INVALID_ITEM,		INVALID_ITEM,	NULL, INVALID_ITEM,		"font",				INVALID_ITEM},


	{PT_CFUNC, "/",				"",							HTTP_TRUE,		NULL, center_func,		INVALID_ITEM, INVALID_ITEM},
	{PT_CFUNC, "/",				"index.html",		HTTP_TRUE,		NULL, center_func,		INVALID_ITEM, INVALID_ITEM},
	{PT_CFUNC, "/",				"index.htm",		HTTP_TRUE,		NULL, center_func,		INVALID_ITEM, INVALID_ITEM},

	{PT_CFUNC, "/",				"include",			0,						NULL, include_func,		INVALID_ITEM, INVALID_ITEM},
	{PT_CFUNC, "/",				"exclude",			0,						NULL, exclude_func,		INVALID_ITEM, INVALID_ITEM},
	{PT_CFUNC, "/",				"list",					0,						NULL, list_func,			INVALID_ITEM, INVALID_ITEM},

	{PT_FILE,	"/",				"error",				0,						NULL, INVALID_ITEM,		"page/error.html",	INVALID_ITEM},
	{PT_FILE, "/",				"favicon.ico",	0,						NULL, INVALID_ITEM,		"image/favicon.ico",INVALID_ITEM},
};

static void web_add_nodes(httpd *svr) {
	int cnt = sizeof(wns)/sizeof(wns[0]);
	int i = 0;

	//httpdAddFileContent(svr, "/", "index", HTTP_TRUE, NULL, "/opt/au/tmp/smartdd/algo/www/index.html");
	//httpdAddCContent(svr, "/", "", HTTP_TRUE, NULL, index_function);
	//httpdAddCContent(svr, "/", "index", HTTP_TRUE, NULL, index_function);
	//httpdAddCContent(svr, "/", "index.html", HTTP_TRUE, NULL, index_function);
	//httpdAddCContent(svr, "/", "index.htm", HTTP_TRUE, NULL, index_function);
	//httpdAddWildcardContent(svr, "/images",NULL, "images");

	for (i = 0; i <  cnt; i++) {
		stWebNode_t *wn = &wns[i];
		switch (wn->type) {
			case PT_CFUNC:
				httpdAddCContent(svr, wn->dir, wn->name, wn->isindex, 
						(WEB_PRELOAD)wn->preload, (WEB_FUNCTION)wn->function);
				break;
			case PT_FILE:
				httpdAddFileContent(svr, wn->dir, wn->name, wn->isindex, wn->preload,
						wn->path);
				break;
			case PT_WILD:
				httpdAddWildcardContent(svr, wn->dir, wn->preload, wn->path);
				break;
			case PT_CWILD:
				httpdAddCWildcardContent(svr, wn->dir, wn->preload, wn->function);
				break;
			case PT_STATIC:
				httpdAddStaticContent(svr, wn->dir, wn->name, wn->isindex, wn->preload, 
					wn->html);
				break;
			default:
				break;
		}
	}
}

void web_init(void *_th, void *_fet, const char *ip, int port, const char *base) {
	we.th = _th;
	we.fet = _fet;

	timer_init(&we.step_timer, web_run);

	strcpy(we.ip, ip);
	we.port = port;
	strcpy(we.base, base);

	we.svr = httpdCreate(we.ip, we.port);
	if (we.svr == NULL) {
		printf("Couldn't create http server : %s %d\n", we.ip, we.port);
		return; 
	}

	httpdSetAccessLog(we.svr,	stdout);
	httpdSetErrorLog(we.svr,	stdout);
	httpdSetFileBase(we.svr,	we.base);

	web_add_nodes(we.svr);

	log_info("http server start at : %s:%d", we.ip, we.port);

	file_event_reg(we.fet, web_getfd(), web_in, NULL, NULL);
}
void web_step() {
	timer_cancel(we.th, &we.step_timer);
	timer_set(we.th, &we.step_timer, 10);
	return;
}
void web_push(void *data, int len) {
}
void web_run(struct timer *timer) {
	if (include_flag == 1) {
		zwave_include();
		include_flag = 0;
	}

	if (exclude_flag == 1) {	
		char mac[8];
		memset(mac, 0, sizeof(mac));
		zwave_exclude(mac);
		exclude_flag = 0;
	}
}
void web_in(void *arg, int fd) {
	while (1) {
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int result;
		httpReq *req =  httpdReadRequest(we.svr, &timeout, &result);
		if (req == NULL && result == 0) {
			if (include_flag == 1 || exclude_flag == 1) {
				web_step();
			}
			return;
			continue;
		} 
		if (result < 0) {
			/* error */
			continue;
		}
		httpdProcessRequest(we.svr, req);
		httpdEndRequest(we.svr, req);
	}

}
int  web_getfd() {
	if (we.svr != NULL) {
		return we.svr->serverSock;
	}
	return -1;
}
////////////////////////////////////////////////////////////////
extern stZWaveCache_t zc;
static void render_devices_list(httpd *server, httpReq *req) {
	httpdPrintf(server, req, (char *)
		"						<div>\n"
		"						<table  border=\"1\" bordercolor=\"#a0c6e5\" style=\"border-collapse:collapse;\">\n"
		"							<tr>\n"
		"								<th>MAC Address</th>\n"
		"								<th>Device Type</th>\n"
		"								<th>Device Battery</th>\n"
		"								<th>Firmware Version</th>\n"
		"								<th>Pir Status</th>\n"
		"							</tr>\n");

	int i = 0;
	int cnt = sizeof(zc.devs)/sizeof(zc.devs[0]);
	for (i = 0; i < cnt; i++) {
		stZWaveDevice_t *zd = &zc.devs[i];
		if (zd->used == 0) {
			continue;
		}

		httpdPrintf(server, req, (char *)
				"<tr>\n"
				"<td>%s</td>\n"
				"<td>%s</td>\n"
				"<td>%d</td>\n"
				"<td>%s</td>\n"
				"<td>%d</td>\n"
				"</tr>\n", 
				device_make_macstr(zd),
				device_make_typestr(zd),
				device_get_battery(zd),
				device_make_versionstr(zd),
				device_get_pir_status(zd)
				);
	}

	httpdPrintf(server, req, (char *)
		"						</div>\n"
		"						</table>\n");
}

static void render_buttons(httpd *server, httpReq *req) {
	httpdPrintf(server, req, (char *)
		"<button id=\"include\">Include</button>\n"
		"<button id=\"exclude\">Exclude</button>\n"
	);
}

static void render_header(httpd *server, httpReq *req) {
	httpdPrintf(server, req, (char *)
		"<html>\n"\
		"	<head>\n"\
		"		<meta charset=\"UTF-8\">\n"
		"		<title>Pir Test</title>\n"
		"		<script language=\"JavaScript\" src=\"js/jquery-3.2.1.min.js\">\n"\
		"		</script>\n"\
		"		<script language=\"JavaScript\" src=\"js/dr.js\">\n"\
		"		</script>\n"\
		"	</head>\n"\
		"	<body><div id=\"body\">\n"\
	);
}
static void render_footer(httpd *server, httpReq *req) {
	httpdPrintf(server, req, (char *)
		"	</div></body>\n"\
		"</html>\n"
	);
}


static void center_func(httpd *server, httpReq *req) {
	render_header(server, req);
	render_buttons(server, req);
	render_devices_list(server, req);
	render_footer(server, req);
}
static void include_func(httpd *server, httpReq *req) {
	//zwave_iface_include();
	include_flag = 1;
	httpdPrintf(server, req, (char *)"{\"ret\":\"0\"}");
}
static void exclude_func(httpd *server, httpReq *req) {
	//char mac[8];
	//memset(mac, 0, sizeof(mac));
	//zwave_iface_exclude(mac);
	exclude_flag = 1;
	httpdPrintf(server, req, (char *)"{\"ret\":\"0\"}");
}
static void list_func(httpd *server, httpReq *req) {
	//render_devices_list();
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
		//json_object_set_new(jd, "model",		json_string(device_make_modelstr(zd)));
		json_object_set_new(jd, "model",		json_string(device_make_modelstr_new(zd)));
		json_object_set_new(jd, "online",		json_integer(device_get_online(zd)));
		json_object_set_new(jd, "battery",	json_integer(device_get_battery(zd)));
		json_object_set_new(jd, "ModelStr", json_string(device_make_modelstr_new(zd)));
	}
	
	if (jary != NULL) {
		char *jmsg = json_dumps(jary, 0);
		if (jmsg != NULL) {
			httpdPrintf(server, req, (char *)"%s", jmsg);
			free(jmsg);
		}
		json_decref(jary);
	}
}





