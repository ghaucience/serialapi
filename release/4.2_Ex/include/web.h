#ifndef __WEB_H_
#define __WEB_H_

#include "timer.h"
#include "lockqueue.h"
#include "file_event.h"

#include "httpd.h"

typedef void (*WEB_FUNCTION)(httpd *server, httpReq *req);
typedef int  (*WEB_PRELOAD)(httpd *server,	httpReq *req);


typedef enum emPageType {
	PT_CFUNC	= 0x00,
	PT_FILE		= 0x01,
	PT_WILD		= 0x02,
	PT_CWILD	= 0x03,
	PT_STATIC = 0x04,
}emPageType_t;

#define INVALID_ITEM	0

typedef struct stWebNode {
	emPageType_t		type;
	char						*dir;
	char						*name;
	int							isindex;
	WEB_PRELOAD			preload;
	WEB_FUNCTION		function;
	char						*path;
	char						*html;
}stWebNode_t;


typedef struct stWebEnv {
	char						ip[32];
	int							port;
	char						base[256];

	httpd						*svr;

	char						user[64];
	char						pass[64];

	//int							currpage;
	void						*th;
	void						*fet;
	struct timer step_timer;
}stWebEnv_t;

void web_init(void *_th, void *_fet, const char *ip, int port, const char *base);
void web_step();
void web_push(void *data, int len);
void web_run(struct timer *timer);
void web_in(void *arg, int fd);
int  web_getfd();


#endif
