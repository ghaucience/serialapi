
#include <stdio.h>

#include "ctimer.h"

#ifdef LIBDEBUG
#define LIBCTIMER_PRINTF printf
#else
#define LIBCTIMER_PRINTF(...) 
#endif

void ctimer_set(struct ctimer *c, int t,void (*f)(void *), void *ptr) {
	LIBCTIMER_PRINTF("%s\n", __func__);
#if 0
	if (c == NULL) {
		return;
	}
	c->running = 1;
	
	c->start = ctimer_now_imp();
	c->f = f;
	c->ptr = ptr;
	c->interval = t;

	LIBCTIMER_PRINTF("start:%d, interval:%d\n", c->start, c->interval);

	ctimer_add_imp(c, t, f, ptr);
#else
	ctimer_add_imp(c, t, f, ptr);
#endif
}

void ctimer_reset(struct ctimer *c) {
	LIBCTIMER_PRINTF("%s\n", __func__);
#if 0

	if (c == NULL) {
		return;
	}
	
	c->running = 1;
	//c->start = ctimer_now_imp();
	//c->f = f;
	//c->ptr = ptr;
	//c->interval = t;

	ctimer_add_imp(c, c->interval, c->f, c->ptr);
#else
	ctimer_rst_imp(c);
#endif
}
void ctimer_restart(struct ctimer *c) {
	LIBCTIMER_PRINTF("%s\n", __func__);
#if 0
	if (c == NULL) {
		return;
	}
	
	c->start = ctimer_now_imp();
	//c->f = f;
	//c->ptr = ptr;
	//c->interval = t;

	ctimer_add_imp(c, c->interval, c->f, c->ptr);
#else
	ctimer_add_imp(c, c->interval, c->f, c->ptr);
#endif
}

void ctimer_stop(struct ctimer *c) {
	LIBCTIMER_PRINTF("%s\n", __func__);
#if 0
	c->running = 0;

	ctimer_del(c);
#else
	ctimer_del_imp(c);
#endif
}
int	 ctimer_expired(struct ctimer *c) {
	LIBCTIMER_PRINTF("%s\n", __func__);
	if (c->running == 0) {
		return 0;
	}

	long now = ctimer_now_imp();
	if (c->start + c->interval > now) {
		return 1;
	}
	return 0;
}



