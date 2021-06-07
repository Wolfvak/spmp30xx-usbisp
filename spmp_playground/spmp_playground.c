// SPDX-License-Identifier: GPL-2.0-only
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "spmp_usb.h"
#include "spmp_io.h"

#include "i2c.h"
#include "rtc.h"
#include "clocks.h"
#include "timers.h"
#include "fmc_spi.h"

int main(void)
{
	void *ctx = spmp_usb_init();
	if (!ctx) {
		fprintf(stderr, "failed to initialize usb context\n");
		return -1;
	}

	bk1080_test(ctx, 4);

	/* get clocks */
	spmp_clk_test(ctx);

	/* get timer frequencies */
	for (unsigned i = 0; i < 4; i++)
		spmp_timer_test(ctx, i);

	/* simple SPI chip detection test */
	spmp_spiflash_test(ctx);

	/* RTC testing */
	spmp_rtc_init(ctx);

	spmp_usb_exit(ctx);
	return 0;
}
