#ifndef _AMBER_SCHEDUE_H_
#define _AMBER_SCHEDUE_H_


typedef struct stSchduleTask {
	void			*func;
	void			*arg;
	long  		start;
	long  		delt;
	
	struct stSchduleTask *next;
}stSchduleTask_t;


void schedule_init(void *_th, void *_fet);

void schedue_add(stSchduleTask_t *at, long ms, void *func, void *arg);

stSchduleTask_t *schedue_first_task_to_exec();

long schedue_first_task_delay();

void schedue_del(stSchduleTask_t *at);

void schedue_rst(stSchduleTask_t *at);

long schedue_current();


#define USE_RUN_LOOP 1

#if USE_RUN_LOOP 
#include "timer.h"
#include "lockqueue.h"
#include "file_event.h"
typedef struct stSchduleEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;
}stSchduleEnv_t;
#endif

#endif
