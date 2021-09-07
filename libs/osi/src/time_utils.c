/*
 * Copyright 2016-2017 Ayla Networks, Inc.  All rights reserved.
 *
 * Use of the accompanying software is permitted only in accordance
 * with and subject to the terms of the Software License Agreement
 * with Ayla Networks, Inc., a copy of which can be obtained from
 * Ayla Networks, Inc.
 */

#include <time.h>

#include "common.h"
#include "time_utils.h"


/*
 * Get time since boot in milliseconds.  Returns a 64-bit value, eliminating
 * rollover concerns.  This is assumed low-overhead (no system calls)
 * using shared kernel page.
 */
_u64 time_mtime_ms(void)
{
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now)) {
		ASSERT(1);
	}
	return ((_u64)now.tv_sec) * 1000 + (_u64)(now.tv_nsec / 1000000);
}
