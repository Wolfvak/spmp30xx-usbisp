// SPDX-License-Identifier: GPL-2.0-only
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "spmp_usb.h"

static void *load_file(const char *path, int *len)
{
	int err;
	uint8_t *data;
	size_t fsz, bufsz;
	FILE *fp = fopen(path, "rb+");

	if (fp == NULL) {
		fprintf(stderr, "failed to open %s\n", path);
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	fsz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	/* align buffer size up to 256 bytes */
	bufsz = (fsz + 255) & ~255;

	data = malloc(bufsz);
	assert(data != NULL);

	err = fread(data, fsz, 1, fp);
	assert(err == 1);

	if (bufsz != fsz) /* fill padding with zero */
		memset(&data[fsz], 0, bufsz - fsz);

	fclose(fp);
	*len = bufsz;
	return data;
}

static int save_file(const char *path, void *data, int len)
{
	FILE *fp = fopen(path, "wb+");
	if (fp == NULL) {
		fprintf(stderr, "failed to open file \"%s\" for writing\n", path);
		return -1;
	}
	fwrite(data, 1, len, fp);
	fclose(fp);
	return 0;
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

static int usbisp_download(void *ctx, const char *address,
	const char *length, const char *path)
{
	void *data;
	int len, err;
	uint32_t addr;

	len = strtoul(length, NULL, 0);
	addr = strtoul(address, NULL, 0);
	if (len < 8) {
		fprintf(stderr, "firmware download must be larger than 8 bytes!\n");
		return -1;
	}

	data = malloc(len);
	err = spmp_usb_download(ctx, addr, data, len);
	if (!err) {
		err = save_file(path, data, len);
	} else {
		fprintf(stderr, "failed to download firmware\n");
	}

	free(data);
	return err;
}

static int usbisp_boot(void *ctx, const char *path)
{
	int len, err;
	void *data = load_file(path, &len);

	if (!data)
		return -1;

	err = spmp_usb_boot(ctx, data, len / 256);
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

		case 'd':
		case 'D':
		{
			// download
			if (argc < 5)
				goto show_usage;

			err = usbisp_download(ctx, argv[2], argv[3], argv[4]);
			if (err < 0)
				fprintf(stderr, "failed to download data\n");
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
			fprintf(stderr, "unrecognized action '%c'\n", *argv[1]);
			err = -1;
			break;
	}

	spmp_usb_exit(ctx);

	if (!err)
		return 0;

show_usage:
	fprintf(stdout,
		"Usage: spmp30xx_usbisp [OPTIONS]\n"
		" [OPTIONS]:\n"
		" - to upload data: 'u address file'\n"
		" - to download data: 'd address length file'\n"
		" - to boot a firmware: 'b file'\n\n"
		"Remember this needs to run as super user!\n\n");
	return EXIT_FAILURE;
}
