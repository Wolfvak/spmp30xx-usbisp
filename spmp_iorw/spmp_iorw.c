// SPDX-License-Identifier: GPL-2.0-only
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "spmp_usb.h"
#include "spmp_io.h"

#define SPMP_REG_MAX_SIZE	8 /* hold up to a single u64 */

static int validate_ranges(uint64_t reg, uint64_t sz, uint64_t val)
{
	if (reg >= 0x10000) {
		fprintf(stderr, "register 0x%lX is outside the operation range\n",
			reg);
		return -1;
	}

	if (!sz || (sz % 8)) {
		fprintf(stderr, "size needs to be a non-zero multiple of 8\n");
		return -1;
	}

	if (sz > (SPMP_REG_MAX_SIZE * 8)) {
		fprintf(stderr, "size needs to be at most %d bits\n",
			SPMP_REG_MAX_SIZE * 8);
		return -1;
	}

	if (val >= (1ULL << sz)) {
		fprintf(stderr, "value is out of size range\n");
		return -1;
	}

	return 0;
}

static int ioread(void *ctx, char *reg_name, char *sz_name)
{
	int err;
	uint64_t reg, sz;
	uint8_t val[SPMP_REG_MAX_SIZE];

	reg = strtoull(reg_name, NULL, 0);
	sz = strtoull(sz_name, NULL, 0);

	err = validate_ranges(reg, sz, 0);
	if (err) return err;

	sz /= 8;
	spmp_reg_rd(ctx, reg, val, sz);

	fprintf(stdout, "0x");
	for (int i = sz - 1; i >= 0; i--)
		fprintf(stdout, "%02X", val[i]);
	fprintf(stdout, "\n");
	return 0;
}

static int iowrite(void *ctx, char *reg_name, char *sz_name, char *val_name)
{
	int err;
	uint64_t reg, val, sz;

	reg = strtoull(reg_name, NULL, 0);
	val = strtoull(val_name, NULL, 0);
	sz = strtoull(sz_name, NULL, 0);

	err = validate_ranges(reg, sz, val);
	if (err) return err;

	sz /= 8;
	spmp_reg_wr(ctx, reg, &val, sz);
	return 0;
}

int main(int argc, char *argv[])
{
	void *ctx;
	int err = EXIT_SUCCESS;

	if (argc != 3 && argc != 4)
		goto show_usage;

	ctx = spmp_usb_init();
	if (!ctx) {
		fprintf(stderr, "failed to initialize usb context\n");
		return EXIT_FAILURE;
	}

	if (argc == 3) {
		/* read back from address and print to stdout */
		err = ioread(ctx, argv[1], argv[2]);
	} else {
		/* write value to address */
		err = iowrite(ctx, argv[1], argv[2], argv[3]);
	}

	if (err)
		fprintf(stderr, "failed to do operation (%d)\n", err);

	spmp_usb_exit(ctx);
	return err;

show_usage:
	fprintf(stderr, "spmp_iorw address size [value]\n");
	return EXIT_FAILURE;
}
