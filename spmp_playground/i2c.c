#include <unistd.h>
#include "i2c_raw.h"

static void spmp_i2c_start_cond(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 1);
	spmp_i2c_drive_scl(ctx, 1);

	usleep(10);
	spmp_i2c_drive_sda(ctx, 0);

	usleep(10);
	spmp_i2c_drive_scl(ctx, 0);
}

static void spmp_i2c_stop_cond(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 0);
	spmp_i2c_drive_scl(ctx, 1);

	usleep(10);
	spmp_i2c_drive_sda(ctx, 1);

	usleep(10);
	spmp_i2c_drive_scl(ctx, 0);
}

static bool spmp_i2c_tx_ack(void *ctx)
{
	bool ack;

	spmp_i2c_clr_oe_sda(ctx);
	usleep(10);

	ack = spmp_i2c_get_sda(ctx);
	usleep(10);

	spmp_i2c_set_oe_sda(ctx);
	usleep(10);

	spmp_i2c_drive_sda(ctx, 0);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 1);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 0);
	usleep(10);

	if (ack) {
		fprintf(stdout, "tx ack fail\n");
		return false;
	} else {
		fprintf(stdout, "tx ack success\n");
		return true;
	}
}

static void spmp_i2c_rx_ack(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 0);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 1);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 0);
	usleep(10);
}

static void spmp_i2c_send_nak(void *ctx)
{
	spmp_i2c_drive_sda(ctx, 1);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 1);
	usleep(10);

	spmp_i2c_drive_scl(ctx, 0);
	usleep(10);

	spmp_i2c_drive_sda(ctx, 0);
	usleep(10);
}

static void spmp_i2c_send_byte(void *ctx, uint8_t data)
{
	for (unsigned i = 0; i < 8; i++) {
		uint8_t bit = data & 0x80;
		data <<= 1;

		spmp_i2c_drive_sda(ctx, bit);
		spmp_i2c_drive_scl(ctx, 1);
		usleep(10);

		spmp_i2c_drive_scl(ctx, 0);
		spmp_i2c_drive_sda(ctx, 0);
		usleep(10);
	}
}

static void spmp_i2c_recv_byte(void *ctx, uint8_t *data)
{
	*data = 0;
	spmp_i2c_clr_oe_sda(ctx);
	usleep(10);

	for (unsigned i = 0; i < 8; i++) {
		bool bit;

		spmp_i2c_drive_scl(ctx, 1);
		usleep(10);

		bit = spmp_i2c_get_sda(ctx);
		usleep(10);

		if (bit)
			*data |= (1 << (7 - i));

		spmp_i2c_drive_scl(ctx, 0);
		usleep(10);
	}
}

static void bk1080_i2c_read(void *ctx, uint8_t reg, uint8_t *buf, unsigned len)
{
	spmp_i2c_start_cond(ctx);

	spmp_i2c_send_byte(ctx, 0x80);
	spmp_i2c_tx_ack(ctx);

	spmp_i2c_send_byte(ctx, (reg << 1) | 1);
	spmp_i2c_tx_ack(ctx);

	for (unsigned i = 0; i < (len-1); i++) {
		fprintf(stdout, "recv byte %d\n", i);
		spmp_i2c_recv_byte(ctx, &buf[i]);
		spmp_i2c_rx_ack(ctx);
	}

	spmp_i2c_recv_byte(ctx, &buf[len-1]);
	spmp_i2c_send_nak(ctx);
	spmp_i2c_stop_cond(ctx);
}

void bk1080_test(void *ctx)
{
	uint8_t data[72];

	bk1080_i2c_read(ctx, 0, data, 72);

	for (unsigned i = 0; i < 72; i++) {
		fprintf(stdout, "reg[%d] = %02X\n", i, data[i]);
	}
}
