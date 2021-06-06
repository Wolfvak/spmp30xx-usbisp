#pragma once

#include <stdint.h>
#include "spmp_io.h"

#define REG_UNK0114	(0x0114) /* unknown, set bit 0 */
#define REG_UNK0005	(0x0005) /* unknown, clear bit 0 */

#define REG_RTC_IFACE_ENABLE	(0xD000)
#define REG_RTC_IFACE_DATA_ADDR	(0xD001)
#define REG_RTC_IFACE_DATA_TX	(0xD002)
#define REG_RTC_IFACE_START	(0xD003)	/* write 1 to TX, 2 to RX */
#define REG_RTC_IFACE_READY	(0xD004)	/* 1 if ready, otherwise busy */
#define REG_RTC_IFACE_DATA_RX	(0xD005)
#define REG_RTC_IFACE_UNK0B	(0xD00B)	/* unknown, set to 0x80 */

enum {
	RTC_STATUS	= 0x00, /* [0] = enable, [1] = reset, [2] = count up */
	RTC_RRP	= 0x01, /* register reduce power, whatever that is */

	/* the RTC is reliable iff the value written here is read back correctly */
	RTC_RELIABLE	= 0x02,

	/* load count register */
	RTC_LOAD0	= 0x10,
	RTC_LOAD1	= 0x11,
	RTC_LOAD2	= 0x12,
	RTC_LOAD3	= 0x13,
	RTC_LOAD4	= 0x14,
	RTC_LOAD5	= 0x15,

	/* alarm data register */
	RTC_ALARM0	= 0x20,
	RTC_ALARM1	= 0x21,
	RTC_ALARM2	= 0x22,
	RTC_ALARM3	= 0x23,
	RTC_ALARM4	= 0x24,
	RTC_ALARM5	= 0x25,

	/* timer register */
	RTC_TIMER0	= 0xA0,
	RTC_TIMER1	= 0xA1,
	RTC_TIMER2	= 0xA2,
	RTC_TIMER3	= 0xA3,
	RTC_TIMER4	= 0xA4,
	RTC_TIMER5	= 0xA5,

	/* timer load register, writing 1 copies the load count to timer */
	RTC_TIMERLOAD	= 0xB0,

	/* write 0 to acknowledge, [0] = SECOND, [1] = ALARM */
	RTC_IRQ_PENDING	= 0xC0,

	/* [0] = SECOND, [1] = ALARM */
	RTC_IRQ_ENABLE	= 0xD0,
};

static inline void spmp_rtc_wait_ready(void *ctx)
{
	while(spmp_read8(ctx, REG_RTC_IFACE_READY) != 1)
		usleep(1000);
}

static inline void spmp_rtc_tx(void *ctx, uint8_t reg, uint8_t val)
{
	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 1);

	spmp_rtc_wait_ready(ctx);

	spmp_write8(ctx, REG_RTC_IFACE_DATA_ADDR, reg);
	spmp_write8(ctx, REG_RTC_IFACE_DATA_TX, val);
	spmp_write8(ctx, REG_RTC_IFACE_START, 1); /* trigger TX */

	spmp_rtc_wait_ready(ctx);

	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 0);
}

static inline uint8_t spmp_rtc_rx(void *ctx, uint8_t reg)
{
	uint8_t val;

	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 1);

	spmp_rtc_wait_ready(ctx);

	spmp_write8(ctx, REG_RTC_IFACE_DATA_ADDR, reg);
	spmp_write8(ctx, REG_RTC_IFACE_START, 2); /* trigger RX */

	spmp_rtc_wait_ready(ctx);

	val = spmp_read8(ctx, REG_RTC_IFACE_DATA_RX);
	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 0);
	return val;
}

static inline uint64_t spmp_rtc_get_time(void *ctx)
{
	uint64_t ctr = 0, x;
	for (unsigned i = 0; i < 6; i++) {
		x = spmp_rtc_rx(ctx, RTC_TIMER0 + i);
		ctr |= x << (i * 8);
	}
	return ctr;
}

/*
 * potentially dangerous?
 * dunno how the stock firmware reacts to "invalid" values
 */
static inline void spmp_rtc_set_time(void *ctx, uint64_t ctr)
{
	for (unsigned i = 0; i < 6; i++) /* set load counter register */
		spmp_rtc_tx(ctx, RTC_LOAD0 + i, ctr >> (i * 8));
	spmp_rtc_tx(ctx, RTC_TIMERLOAD, 1); /* copy to counter */
}

static inline void spmp_rtc_init(void *ctx)
{
	/* enable the device */
	spmp_write8(ctx, REG_UNK0114, spmp_read8(ctx, REG_UNK0114) | 1);
	spmp_write8(ctx, REG_UNK0005, spmp_read8(ctx, REG_UNK0005) & ~1);

	/* seems to enable the interface itself, otherwise the regs are 0 */
	spmp_write8(ctx, REG_RTC_IFACE_UNK0B, 0x80);

	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 0);
	spmp_write8(ctx, REG_RTC_IFACE_ENABLE, 1); /* reset interface? */

	spmp_rtc_tx(ctx, RTC_RRP, 1); /* REGISTER REDUCE POWER ENABLE (?) */

	spmp_rtc_tx(ctx, RTC_STATUS, 5); /* COUNT UP, ENABLE */

	spmp_rtc_tx(ctx, RTC_IRQ_ENABLE, 0); /* DISABLE ALL INTERRUPTS */
	spmp_rtc_tx(ctx, RTC_IRQ_PENDING, 0); /* ACK ALL INTERRUPTS */

	/* write the 'reliable' register */
	spmp_rtc_tx(ctx, RTC_RELIABLE, 90);

	/* check the result */
	fprintf(stdout, "RTC reliability: sent 90, got %d back\n",
		spmp_rtc_rx(ctx, RTC_RELIABLE));
	fprintf(stdout, "RTC counter: 0x%012lX\n", spmp_rtc_get_time(ctx));
}
