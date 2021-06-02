#pragma once

#include <stdio.h>
#include <stdint.h>
#include "spmp_io.h"

#define REG_FMC_MEDIATYPE	(0x4000)
#define REG_FMC_SIO_DATA_TX	(0x4040)
#define REG_FMC_SIO_DATA_RX	(0x4041)
#define REG_FMC_SIO_DTRIG	(0x4042)
#define REG_FMC_SIO_CLKSEL	(0x4046)
#define REG_FMC_SIO_CONFIG	(0x4047)
#define REG_FMC_SIO_MODE	(0x4048)
#define REG_FMC_SIO_BSYRDY	(0x404B)
#define REG_UNK1BC	(0x01BC) // bootrom sets bit 3 on spi init

static inline void spmp_spi_tx(void *ctx, uint8_t val)
{
	spmp_write8(ctx, REG_FMC_SIO_DATA_TX, val);
	while(spmp_read8(ctx, REG_FMC_SIO_BSYRDY) & 1)
		usleep(1000);
}

static inline uint8_t spmp_spi_rx(void *ctx)
{
	spmp_read8(ctx, REG_FMC_SIO_DTRIG);
	while(spmp_read8(ctx, REG_FMC_SIO_BSYRDY) & 1)
		usleep(1000);
	return spmp_read8(ctx, REG_FMC_SIO_DATA_RX);
}

static inline void spmp_spiflash_test(void *ctx)
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
