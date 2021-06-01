/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Compile with:
 * cc -O2 usbreg.c spmp_usbisp.c $(pkg-config --cflags --libs libusb-1.0) -o usbreg
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spmp_usbisp.h"

#define spmp_read8(c, r)	({uint8_t v=0; spmp_read(c, r, &v, 1); v;})
#define spmp_read16(c, r)	({uint16_t v=0; spmp_read(c, r, &v, 2); v;})
#define spmp_read24(c, r)	({uint32_t v=0; spmp_read(c, r, &v, 3); v;})
#define spmp_read32(c, r)	({uint32_t v=0; spmp_read(c, r, &v, 4); v;})
#define spmp_read64(c, r)	({uint64_t v=0; spmp_read(c, r, &v, 8); v;})

#define spmp_write8(c, r, v)	({uint8_t _v=(v); spmp_write(c, r, &_v, 1);})
#define spmp_write16(c, r, v)	({uint16_t _v=(v); spmp_write(c, r, &_v, 2);})
#define spmp_write24(c, r, v)	({uint32_t _v=(v); spmp_write(c, r, &_v, 3);})
#define spmp_write32(c, r, v)	({uint32_t _v=(v); spmp_write(c, r, &_v, 4);})
#define spmp_write64(c, r, v)	({uint64_t _v=(v); spmp_write(c, r, &_v, 8);})

static void spmp_read(void *ctx, uint16_t reg, void *val, int len)
{
	uint8_t *data = val;
	for (unsigned i = 0; i < len; i++) {
		int err = spmp_usb_peek(ctx, reg + i, &data[i]);
		assert(err == 0);
	}
}

static void spmp_write(void *ctx, uint16_t reg, void *val, int len)
{
	uint8_t *data = val;
	for (unsigned i = 0; i < len; i++) {
		int err = spmp_usb_poke(ctx, reg + i, data[i]);
		assert(err == 0);
	}
}

#define REG_FMC_MEDIATYPE	(0x4000)
#define REG_FMC_SIO_DATA_TX	(0x4040)
#define REG_FMC_SIO_DATA_RX	(0x4041)
#define REG_FMC_SIO_DTRIG	(0x4042)
#define REG_FMC_SIO_CLKSEL	(0x4046)
#define REG_FMC_SIO_CONFIG	(0x4047)
#define REG_FMC_SIO_MODE	(0x4048)
#define REG_FMC_SIO_BSYRDY	(0x404B)
#define REG_UNK1BC	(0x01BC) // set bit 3

static void spmp_spi_tx(void *ctx, uint8_t val)
{
	spmp_write8(ctx, REG_FMC_SIO_DATA_TX, val);
	while(spmp_read8(ctx, REG_FMC_SIO_BSYRDY) & 1)
		usleep(1000);
}

static uint8_t spmp_spi_rx(void *ctx)
{
	spmp_read8(ctx, REG_FMC_SIO_DTRIG);
	while(spmp_read8(ctx, REG_FMC_SIO_BSYRDY) & 1)
		usleep(1000);
	return spmp_read8(ctx, REG_FMC_SIO_DATA_RX);
}

static void spmp_spi_test(void *ctx)
{
	uint8_t id[3];

	fprintf(stdout, "testing SPI chip...\n");

	// spiInitialize(0, 0)
	spmp_write8(ctx, REG_FMC_MEDIATYPE, 4);
	spmp_write8(ctx, REG_FMC_SIO_MODE, 0);
	spmp_write8(ctx, REG_FMC_SIO_CLKSEL, 0);
	spmp_write8(ctx, REG_FMC_SIO_CONFIG, 0x8);
	spmp_write8(ctx, REG_FMC_SIO_CONFIG, 0xC);
	spmp_write8(ctx, REG_UNK1BC, spmp_read8(ctx, REG_UNK1BC) | 8);

	// spiFlashRDID
	spmp_write8(ctx, REG_FMC_SIO_CONFIG, 0xC);
	spmp_spi_tx(ctx, 0x9F);
	for (unsigned i = 0; i < 3; i++)
		spmp_spi_tx(ctx, 0);
	for (unsigned i = 0; i < 3; i++)
		id[i] = spmp_spi_rx(ctx);
	spmp_write8(ctx, REG_FMC_SIO_MODE, 0xE);

	fprintf(stdout, "SPI ID is: ");
	for (unsigned i = 0; i < 3; i++)
		fprintf(stdout, "%02X ", id[i]);
	fprintf(stdout, "\n");
}

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

#define REG_MAINCLK	(0x0B0)
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

int main(int argc, char *argv[])
{
	void *ctx = spmp_usb_init();
	if (!ctx) {
		fprintf(stderr, "failed to initialize usb context\n");
		return -1;
	}

	/* get clocks */
	spmp_clk_test(ctx);

	/* get timer frequencies */
	for (unsigned i = 0; i < 4; i++)
		spmp_timer_test(ctx, i);

	/* simple SPI chip detection test */
	spmp_spi_test(ctx);

	spmp_usb_exit(ctx);
	return 0;
}
