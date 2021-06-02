#pragma once

#include <stdio.h>
#include <stdint.h>
#include "spmp_io.h"

#define REG_TIMER_FLAGS(n)	(0x1040 + (n))
#define REG_TIMER_ENABLE	(0x1044)
#define REG_TIMER_COUNTER(n)	(0x1030 + ((n) * 4))
#define REG_TIMER_PERIOD(n)	(0x1318 + ((n) * 2))

#define TIMER_FLAG_UPCOUNT	0x01
#define TIMER_FLAG_RELOAD	0x10

void spmp_timer_test(void *ctx, unsigned id)
{
	uint32_t counter;

	if (id > 3)
		fprintf(stderr, "unknown timer %d\n", id);

	spmp_write8(ctx, REG_TIMER_ENABLE, 0);

	spmp_write8(ctx, REG_TIMER_FLAGS(id), 0);
	spmp_write16(ctx, REG_TIMER_PERIOD(id), 0);
	spmp_write32(ctx, REG_TIMER_COUNTER(id), 0x1000);

	spmp_write16(ctx, REG_TIMER_PERIOD(id), 0);

	/*
	 * dont reload the counter register
	 * if it saturates it should get stuck
	 */
	spmp_write8(ctx, REG_TIMER_FLAGS(id), TIMER_FLAG_UPCOUNT);

	spmp_write8(ctx, REG_TIMER_ENABLE, 1 << id);
	/* timing critical region begins */

	usleep(200 * 1000); /* totally very accurate */

	counter = spmp_read24(ctx, REG_TIMER_COUNTER(id));
	/* timing critical region ends */

	spmp_write8(ctx, REG_TIMER_ENABLE, 0);

	/*
	 * this doesnt give an accurate number
	 * but at least its an upper limit thats
	 * very close to the actual timer frequency
	 */
	fprintf(stdout, "timer %d frequency is ~= %dHz\n", id, 5 * counter);
}
