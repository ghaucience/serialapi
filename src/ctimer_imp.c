#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ctimer_imp.h"
#include "schedule.h"

long ctimer_now_imp() {
	return schedue_current();
}

void ctimer_add_imp(void *_c, long interval, void *f, void *ptr) {
	stSchduleTask_t  *st = (stSchduleTask_t *)_c;
	schedue_add(st, st->delt, st->func, st->arg);
}

void ctimer_del_imp(void *_c) {
	stSchduleTask_t  *st = (stSchduleTask_t *)_c;
	schedue_del(st);
}

void ctimer_rst_imp(void *_c) {
	stSchduleTask_t  *st = (stSchduleTask_t *)_c;
	schedue_rst(st);
}



