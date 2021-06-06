/* SPDX-License-Identifier: GPL-2.0-only */
#pragma once

#include "spmp_usb.h"

#include <assert.h>

/* WARNING: only 8bit ops are atomic */
#define spmp_read8(c, r)	({uint8_t v; spmp_reg_rd(c, r, &v, 1); v;})
#define spmp_read16(c, r)	({uint16_t v; spmp_reg_rd(c, r, &v, 2); v;})
#define spmp_read32(c, r)	({uint32_t v; spmp_reg_rd(c, r, &v, 4); v;})
#define spmp_read64(c, r)	({uint64_t v; spmp_reg_rd(c, r, &v, 8); v;})

#define spmp_write8(c, r, v)	({uint8_t _v=(v); spmp_reg_wr(c, r, &_v, 1);})
#define spmp_write16(c, r, v)	({uint16_t _v=(v); spmp_reg_wr(c, r, &_v, 2);})
#define spmp_write32(c, r, v)	({uint32_t _v=(v); spmp_reg_wr(c, r, &_v, 4);})
#define spmp_write64(c, r, v)	({uint64_t _v=(v); spmp_reg_wr(c, r, &_v, 8);})

/* non native size, purely for performance reasons */
#define spmp_read24(c, r)	({uint32_t v=0; spmp_reg_rd(c, r, &v, 3); v;})
#define spmp_write24(c, r, v)	({uint32_t _v=(v); spmp_reg_wr(c, r, &_v, 3);})

/* set/clear ops, very non-atomic */
#define spmp_set8(c, r, v)	spmp_write8(c, r, spmp_read8(c, r) | (v))
#define spmp_set16(c, r, v)	spmp_write16(c, r, spmp_read16(c, r) | (v))
#define spmp_set32(c, r, v)	spmp_write32(c, r, spmp_read32(c, r) | (v))

#define spmp_clr8(c, r, v)	spmp_write8(c, r, spmp_read8(c, r) & ~(v))
#define spmp_clr16(c, r, v)	spmp_write16(c, r, spmp_read16(c, r) & ~(v))
#define spmp_clr32(c, r, v)	spmp_write32(c, r, spmp_read32(c, r) & ~(v))

static void spmp_reg_rd(void *ctx, uint16_t reg, void *val, unsigned len)
{
	int err = 0;
	uint8_t *data = val;
	for (unsigned i = 0; i < len; i++)
		err |= spmp_usb_peek(ctx, reg + i, &data[i]);
	assert(err == 0);
}

static void spmp_reg_wr(void *ctx, uint16_t reg, void *val, unsigned len)
{
	int err = 0;
	uint8_t *data = val;
	for (unsigned i = 0; i < len; i++)
		err |= spmp_usb_poke(ctx, reg + i, data[i]);
	assert(err == 0);
}
