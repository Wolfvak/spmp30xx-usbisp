#include <unistd.h>
#include "i2c_raw.h"

/*
 * all the operations included a delay in the original code
 * since each register peek/poke is done via usb and takes
 * a good amount of time, I just removed them
 * the built in latency will work as an I2C bus delay
 */

static void spmp_i2c_start_cond(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 1);
	spmp_i2c_drive_scl(ctx, 1);

	spmp_i2c_drive_sda(ctx, 0);
	spmp_i2c_drive_scl(ctx, 0);
}

static void spmp_i2c_stop_cond(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 0);
	spmp_i2c_drive_scl(ctx, 1);

	spmp_i2c_drive_sda(ctx, 1);
	spmp_i2c_drive_scl(ctx, 0);
}

static bool spmp_i2c_tx_ack(void *ctx)
{
	bool ack;

	spmp_i2c_clr_oe_sda(ctx);
	ack = spmp_i2c_get_sda(ctx);

	spmp_i2c_set_oe_sda(ctx);
	spmp_i2c_drive_sda(ctx, 0);

	spmp_i2c_drive_scl(ctx, 1);
	spmp_i2c_drive_scl(ctx, 0);

	if (ack) {
		fprintf(stdout, "[I2C] TX ACK fail\n");
		return false;
	} else {
		fprintf(stdout, "[I2C] TX ACK success\n");
		return true;
	}
}

static void spmp_i2c_rx_ack(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 0);
	spmp_i2c_drive_scl(ctx, 1);
	spmp_i2c_drive_scl(ctx, 0);
}

static void spmp_i2c_send_nak(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 1);
	spmp_i2c_drive_scl(ctx, 1);
	spmp_i2c_drive_scl(ctx, 0);
	spmp_i2c_drive_sda(ctx, 0);
}

static void spmp_i2c_send_byte(void *ctx, uint8_t data)
{
	for (unsigned i = 0; i < 8; i++) {
		uint8_t bit = data & 0x80;
		data <<= 1;

		spmp_i2c_drive_sda(ctx, bit);
		spmp_i2c_drive_scl(ctx, 1);

		spmp_i2c_drive_scl(ctx, 0);
		spmp_i2c_drive_sda(ctx, 0);
	}
}

static void spmp_i2c_recv_byte(void *ctx, uint8_t *data)
{
	*data = 0;
	spmp_i2c_clr_oe_sda(ctx);

	for (unsigned i = 0; i < 8; i++) {
		bool bit;

		spmp_i2c_drive_scl(ctx, 1);
		bit = spmp_i2c_get_sda(ctx);

		if (bit)
			*data |= (1 << (7 - i));

		spmp_i2c_drive_scl(ctx, 0);
	}
}

static void bk1080_i2c_read(void *ctx, uint8_t reg, uint8_t *buf, unsigned len)
{
	spmp_i2c_start_cond(ctx);

	spmp_i2c_send_byte(ctx, 0x80);
	spmp_i2c_tx_ack(ctx);

	spmp_i2c_send_byte(ctx, (reg << 1) | 1);
	spmp_i2c_tx_ack(ctx);

	fprintf(stdout, "receiving byte ");

	for (unsigned i = 0; i < (len-1); i++) {
		spmp_i2c_recv_byte(ctx, &buf[i]);
		spmp_i2c_rx_ack(ctx);
		fprintf(stdout, "%d.. ", i);
		fflush(stdout);
	}

	spmp_i2c_recv_byte(ctx, &buf[len-1]);
	fprintf(stdout, "%d\n", len-1);

	spmp_i2c_send_nak(ctx);
	spmp_i2c_stop_cond(ctx);
}

void bk1080_test(void *ctx, unsigned len)
{
	uint8_t data[len];

	spmp_i2c_init_hw(ctx);

	bk1080_i2c_read(ctx, 0, data, len);

	fprintf(stdout, "received BK1080 I2C data: ");

	for (unsigned i = 0; i < len; i++)
		fprintf(stdout, "%02X ", data[i]);
	fprintf(stdout, "\n");
}
