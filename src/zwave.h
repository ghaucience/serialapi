#ifndef __ZWAVE_H_
#define __ZWAVE_H_

#include "timer.h"
#include "lockqueue.h"
#include "file_event.h"

typedef struct stZWaveEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;
	stLockQueue_t eq;

	char dev[64];
	int  buad;

	int fd;
}stZWaveEnv_t;

int  zwave_init(void *_th, void *_fet, char *dev, int buad);
int		zwave_step();
int		zwave_push(stEvent_t *e);
void	zwave_run(struct timer *timer);
void	zwave_in(void *arg, int fd);


int zwave_start(int argc, char *argv[]);
int zwave_include();
int zwave_exclude();
int zwave_version();
#endif


