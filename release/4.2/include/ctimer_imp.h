#ifndef __CTIMER_IMP_H_
#define __CTIMER_IMP_H_
long ctimer_now_imp();

void ctimer_add_imp(void *c, long interval, void *f, void *ptr);
void ctimer_del_imp(void *c);
void ctimer_rst_imp(void *c);

void ctimer_poll(void);
#endif
