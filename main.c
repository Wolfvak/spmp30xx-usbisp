/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Compile with:
 * cc -O2 main.c spmp_usbisp.c $(pkg-config --cflags --libs libusb-1.0) -o usbisp
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spmp_usbisp.h"

static void *load_file(const char *path, int *len)
{
	int err;
	uint8_t *data;
	size_t fsz, bufsz;
	FILE *fp = fopen(path, "rb+");

	if (fp == NULL) {
		fprintf(stdout, "failed to open %s\n", path);
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	fsz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	/* align buffer size up to 8 bytes */
	bufsz = (fsz + 7) & ~7;
	data = malloc(bufsz);
	assert(data != NULL);

	err = fread(data, fsz, 1, fp);
	assert(err == 1);

	for (unsigned i = fsz; i < bufsz; i++)
		data[i] = 0; /* fill padding with zeroes */

	fclose(fp);
	*len = bufsz;

	return data;
}

static int usbisp_upload(void *ctx, const char *address, const char *path)
{
	int len, err;
	uint32_t addr = strtoul(address, NULL, 0);
	void *data = load_file(path, &len);

	if (!data)
		return -1;

	err = spmp_usb_upload(ctx, addr, data, len);
	if (!err)
		fprintf(stdout, "uploaded %s to %08X (%d bytes)\n", path, addr, len);

	free(data);
	return err;
}

static int usbisp_boot(void *ctx, const char *path)
{
	int len, err;
	void *data = load_file(path, &len);

	if (!data)
		return -1;

	if (len > 256)
		fprintf(stderr, "input bootloader is larger than 256 bytes! ignoring extra data...\n");

	err = spmp_usb_boot(ctx, data);
	if (!err)
		fprintf(stdout, "booted %s\n", path);

	free(data);
	return err;
}

int main(int argc, char *argv[])
{
	int err;
	void *ctx;

	if (argc < 3)
		goto show_usage;

	ctx = spmp_usb_init();
	if (!ctx) {
		fprintf(stderr, "failed to initialize context, exiting...\n");
		return EXIT_FAILURE;
	}

	switch(*argv[1]) {
		case 'u':
		case 'U':
		{
			// upload
			if (argc < 4)
				goto show_usage;

			err = usbisp_upload(ctx, argv[2], argv[3]);
			if (err < 0)
				fprintf(stderr, "failed to upload data\n");
			break;
		}

		case 'b':
		case 'B':
		{
			// boot
			err = usbisp_boot(ctx, argv[2]);
			if (err < 0)
				fprintf(stderr, "failed to trigger bootloader\n");
			break;
		}

		default:
			err = -1;
			break;
	}

	spmp_usb_exit(ctx);

	if (!err)
		return 0;

show_usage:
	fprintf(stdout, "Usage: spmp30xx_usbisp {u, b} {address, file; file}\n");
	return EXIT_FAILURE;
}
