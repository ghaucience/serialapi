#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ctimer_imp.h"
#include "ctimer.h"

static struct ctimer *head = NULL;

long ctimer_now_imp() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void ctimer_add_imp(void *_c, long interval, void *f, void *ptr) {
	struct ctimer *h = head;
	struct ctimer *p = NULL;
	struct ctimer *c = (struct ctimer *)_c;
	while (h != NULL) {
		if (h == c) {
			h->ptr = c->ptr;
			h->f = c->f;
			h->start = c->start;
			h->interval = c->interval;
			h->running = c->running;
			return;
		}
		p = h;
		h = h->next;
	}
	
	if (p != NULL) {
		p->next = c;
		c->next = NULL;
	} else {
		head = c;
		c->next = NULL;
	}
}

void ctimer_del_imp(void *_c) {
	struct ctimer *h = head;
	struct ctimer *p = NULL;
	struct ctimer *c = (struct ctimer *)_c;
	int flag = 0;
	while (h != NULL) {
		if (h == c) {
			flag = 1;
			break;
		}
		p = h;
		h = h->next;
	}
	
	if (flag == 0) {
		return;
	}

	if (p != NULL) {
		p->next = c->next;
		c->next = NULL;
	} else {
		head = c->next;
	}
}

void ctimer_rst_imp(void *_c) {
	struct ctimer *c = (struct ctimer *)_c;
	if (c == NULL) {
		return;
	}
	
	c->running = 1;
	ctimer_add_imp(c, c->interval, c->f, c->ptr);
}


void ctimer_poll(void) {
	struct ctimer *h = head;
	struct ctimer *p = NULL;
	p = p;
	struct ctimer *a = NULL;
	while (h != NULL) {
		if (ctimer_expired(h)) {
			a = h;
			break;
		}
		p = h;
		h = h->next;
	}
	
	if (a == NULL) {
		return;
	}

	if (a->f != NULL) {
		//LIBCTIMER_PRINTF("[ctimer_callback]\n");
		a->f(a->ptr);
	}

	ctimer_del_imp(a);
}


