#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "spmp_io.h"

#define REG_SIF_ENABLE	(0x9160)
#define REG_SIF_OUTPUT_ENABLE	(0x9162)
#define REG_SIF_OUTPUT_DATA	(0x9164)
#define REG_SIF_INPUT_DATA	(0x9166)

#define SIF_LINE_SCL	(1 << 2)
#define SIF_LINE_SDA	(1 << 3)


static inline void spmp_i2c_init_hw(void *ctx)
{
	// bit2 of register 0x110 needs to be set
	// toggles the serial I2C clock?
	spmp_set8(ctx, 0x110, 1 << 2);

	// every operation does this but in
	// practice it works to only do it once on startup
	spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SDA | SIF_LINE_SCL);
}

static inline void spmp_i2c_set_oe_sda(void *ctx)
{
	//spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SDA);
	spmp_set8(ctx, REG_SIF_OUTPUT_ENABLE, SIF_LINE_SDA);
}

static inline void spmp_i2c_clr_oe_sda(void *ctx)
{
	//spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SDA);
	spmp_clr8(ctx, REG_SIF_OUTPUT_ENABLE, SIF_LINE_SDA);
}

static inline bool spmp_i2c_get_sda(void *ctx)
{
	//spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SDA);
	spmp_clr8(ctx, REG_SIF_OUTPUT_ENABLE, SIF_LINE_SDA);
	return (spmp_read8(ctx, REG_SIF_INPUT_DATA) & SIF_LINE_SDA) != 0;
}

static inline void spmp_i2c_drive_sda(void *ctx, bool sda)
{
	uint8_t dat;

	//spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SDA);
	spmp_set8(ctx, REG_SIF_OUTPUT_ENABLE, SIF_LINE_SDA);
	dat = spmp_read8(ctx, REG_SIF_OUTPUT_DATA);
	if (sda) {
		dat |= SIF_LINE_SDA;
	} else {
		dat &= ~SIF_LINE_SDA;
	}
	spmp_write8(ctx, REG_SIF_OUTPUT_DATA, dat);
}

static inline void spmp_i2c_drive_scl(void *ctx, bool scl)
{
	uint8_t dat;

	//spmp_set8(ctx, REG_SIF_ENABLE, SIF_LINE_SCL);
	spmp_set8(ctx, REG_SIF_OUTPUT_ENABLE, SIF_LINE_SCL);
	dat = spmp_read8(ctx, REG_SIF_OUTPUT_DATA);
	if (scl) {
		dat |= SIF_LINE_SCL;
	} else {
		dat &= ~SIF_LINE_SCL;
	}
	spmp_write8(ctx, REG_SIF_OUTPUT_DATA, dat);
}
