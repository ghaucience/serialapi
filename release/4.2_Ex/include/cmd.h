#ifndef __CMD_H_
#define __CMD_H_

#include "timer.h"
#include "lockqueue.h"
#include "file_event.h"

typedef struct stCmd {
	const char *name;
	void (*func)(char *argv[], int argc);
	const char *desc;
}stCmd_t;

typedef struct stCmdEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;
	stLockQueue_t eq;

	int fd;
}stCmdEnv_t;


int cmd_init(void *_th, void *_fet);
int cmd_step();
int cmd_push(stEvent_t *e);
void cmd_run(struct timer *timer);
void cmd_in(void *arg, int fd);

stCmd_t *cmd_search(const char *cmd);

#endif
