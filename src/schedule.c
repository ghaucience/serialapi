#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static stSchduleTask_t * task_list = NULL;


#if USE_RUN_LOOP
static stSchduleEnv_t se;
#endif

long schedue_current() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static stSchduleTask_t * schedue_search(stSchduleTask_t * at) {
	stSchduleTask_t *curr = task_list;
	
	while (curr != NULL) {
		if (curr == at) {
			break;
		}
		curr = curr->next;
	}

	return curr;
}

static stSchduleTask_t * _schedue_add(stSchduleTask_t *at) {
	stSchduleTask_t *prev = task_list;
	
	while (prev != NULL && prev->next != NULL) {
		prev = prev->next;
	}

	if (prev == NULL) {
		task_list = at;
	} else {
		prev->next = at;
	}

	at->next = NULL;

	return at;
}

void schedue_add(stSchduleTask_t *at, long ms, void *func, void *arg) {
	
	if (schedue_search(at) == NULL) {
		_schedue_add(at);
	}

	at->func = func;
	at->arg = arg;
	at->start = schedue_current();
	at->delt = ms;

	#if USE_RUN_LOOP	
		timer_set(se.th, &se.step_timer, 1);
	#endif
}

void schedue_rst(stSchduleTask_t *at) {
	
	if (schedue_search(at) == NULL) {
		_schedue_add(at);
		at->start = schedue_current();
	} else {
		;
	}

	//at->func = func;
	//at->arg = arg;
	//at->delt = ms;

	#if USE_RUN_LOOP	
		timer_set(se.th, &se.step_timer, 1);
	#endif
}

stSchduleTask_t *schedue_first_task_to_exec() {
	stSchduleTask_t *curr = task_list;
	stSchduleTask_t *min  = curr;

	
	while (curr != NULL) {
		if (curr->start + curr->delt <= min->start + min->delt) {
			min = curr;
		}
		curr = curr->next;
	}
	
	return min;
}

long schedue_first_task_delay() {
	stSchduleTask_t *min = schedue_first_task_to_exec();
	if (min == NULL) {
		return -1;
	}
	long ret = min->start + min->delt - schedue_current();

	if (ret < 0) {
		ret = 0; //10ms
	}
	
	return ret;
}


void schedue_del(stSchduleTask_t *at) {
	stSchduleTask_t *prev = NULL;
	stSchduleTask_t *curr = task_list;

	while (curr != NULL) {
		if (curr == at) {
			break;
		}
	
		prev = curr;
		curr = curr->next;
	}

	if (curr == at && curr != NULL) {
		if (prev == NULL) {
			task_list = curr->next;
		} else {
			prev->next = curr->next;
		}
	}
	
}


#if USE_RUN_LOOP
void schedule_run(struct timer *timer) {
	int delt = schedue_first_task_delay();
	if (delt < 0) {	
		return;
	}

	if (delt == 0) {
		stSchduleTask_t *task = schedue_first_task_to_exec();
		schedue_del(task);

		((int (*)(void *))task->func)(task->arg);
	}

	delt = schedue_first_task_delay();
	if (delt < 0) {
		return;
	}

	timer_set(se.th, &se.step_timer, delt);
}

void schedule_init(void *_th, void *_fet) {
	se.th = _th;	
	se.fet = _fet;

	timer_init(&se.step_timer, schedule_run);
}


#endif


