#pragma once

#include <stdio.h>
#include <stdint.h>
#include "spmp_io.h"

#define REG_MAINCLK	(0x0B2)
#define REG_HCLK	(0x122)
#define REG_MCLK	(0x123)
#define REG_CPUCLK	(0x130)
#define REG_AHBCLK	(0x131)
#define REG_BUSCLK	(0x132)
#define REG_DISPCLK	(0x13A)

static void spmp_clk_test(void *ctx)
{
	uint8_t rmain, rhclk, rmclk, rcpu, rahb, rbus, rdisp;
	unsigned main, hclk, mclk, cpu, ahb, bus, disp;

	rmain = spmp_read8(ctx, REG_MAINCLK);
	rhclk = spmp_read8(ctx, REG_HCLK);
	rmclk = spmp_read8(ctx, REG_MCLK);
	rcpu = spmp_read8(ctx, REG_CPUCLK);
	rahb = spmp_read8(ctx, REG_AHBCLK);
	rbus = spmp_read8(ctx, REG_BUSCLK);
	rdisp = spmp_read8(ctx, REG_DISPCLK);

	main = 144 + ((rmain >> 1) & 3) * 24;
	hclk = (2 * main) / (rhclk + 1);
	mclk = (2 * main) / (rmclk + 1);

	cpu = hclk / (rcpu + 1);
	ahb = hclk / (rahb + 1);
	bus = mclk / (rbus + 1);

	if (rdisp & 0x10) {
		disp = 27 / ((rdisp&1) + 1);
	} else {
		disp = main / (spmp_read8(ctx, 0x135)+1) / (spmp_read8(ctx, 0x13A) + 1);
	}

	fprintf(stdout,
		"MAIN: %d - %dMHz\n"
		"HCLK: %d - %dMHz\n"
		"MCLK: %d - %dMHz\n"
		"CPUCLK: %d - %dMHz\n"
		"AHBCLK: %d - %dMHz\n"
		"BUSCLK: %d - %dMHz\n"
		"DISPCLK: %d - %dMHz\n",
		rmain, main,
		rhclk, hclk,
		rmclk, mclk,
		rcpu, cpu,
		rahb, ahb,
		rbus, bus,
		rdisp, disp
	);
}
