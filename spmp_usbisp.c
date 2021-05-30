/* SPDX-License-Identifier: GPL-2.0-only */

/* low level USB interface stuff */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

typedef struct {
	libusb_context *usb;
	libusb_device_handle *dev;
} spmp_usb_ctx;

#define SPMP_TIMEOUT	(2000)
#define SPMP_BULK_UPDATE	(MAX_BULK_LEN * 64)

#define SPMP_VENDOR	(0x04FC)
#define SPMP_DEVICE	(0x5560)

#define MAX_BULK_LEN	(4096) /* hardware and OS dependant but whatever */

#define SPMP_CMD_FWBOOT	(0x4F1)
#define SPMP_CMD_FWUPLOAD	(0x4F3)
#define SPMP_CMD_FWDOWNLOAD	(0x4F2)

static int spmp_usb_updown(void *ctx, int cmd, int xfer_endpoint,
							uint32_t addr, void *data, int len)
{
	int err, xferd;
	uint32_t pkt[2];
	libusb_device_handle *devh = ((spmp_usb_ctx*)ctx)->dev;

	pkt[0] = addr;
	pkt[1] = len;

	err = libusb_control_transfer(devh, 0x41, 0xFD, 0x00, cmd,
		(void*)pkt, 8, SPMP_TIMEOUT);
	if (err < 0) return err;

	fprintf(stdout, "transferring %d bytes on address %08X...\n", len, addr);

	for (unsigned i = 0; i < len; ) {
		uint32_t blksz = len - i;
		if (blksz > MAX_BULK_LEN)
			blksz = MAX_BULK_LEN;

		err = libusb_bulk_transfer(devh, xfer_endpoint,
			data, blksz, &xferd, SPMP_TIMEOUT);
		if (err < 0) {
			fprintf(stderr, "bulk xfer failed (%s)\n", libusb_error_name(err));
			return err;
		}

		data += blksz;
		i += blksz;

		if (!(i % SPMP_BULK_UPDATE))
			fprintf(stdout, "transferred %d bytes (%.1f%%)\n",
				i, ((float)i / (float)len) * 100.0f);
	}

	fprintf(stdout, "done transferring!\n");
	return err;
}

int spmp_usb_upload(void *ctx, uint32_t addr, void *data, int len)
{
	return spmp_usb_updown(ctx, SPMP_CMD_FWUPLOAD, 3, addr, data, len);
}

int spmp_usb_download(void *ctx, uint32_t addr, void *data, int len)
{
	return spmp_usb_updown(ctx, SPMP_CMD_FWDOWNLOAD, 0x82, addr, data, len);
}

int spmp_usb_boot(void *ctx, void *loader, int pages)
{
	int err, xferd;
	uint32_t pkt[4];
	libusb_device_handle *devh = ((spmp_usb_ctx*)ctx)->dev;

	pkt[0] = 0; /* pkt[0] | 0x24000000 = download address, must be > 0 */
	pkt[1] = pkt[2] = 0; /* if either is set, don't boot (?) */
	pkt[3] = pages; /* number of 256byte pages to send */

	err = libusb_control_transfer(devh, 0x41, 0xFD, 0x00, SPMP_CMD_FWBOOT,
		(void*)pkt, 16, SPMP_TIMEOUT);
	if (err < 0) {
		fprintf(stderr, "failed to signal firmware boot (%s)\n",
			libusb_error_name(err));
		return err;
	}

	err = libusb_bulk_transfer(devh, 3, loader, 0x100, &xferd, SPMP_TIMEOUT);
	if (err < 0)
		fprintf(stderr, "failed to transfer boot code (%s)\n",
			libusb_error_name(err));
	return err;
}

static int spmp_usb_claim(libusb_device_handle *devh)
{
	int err;

	err = libusb_set_configuration(devh, 1);
	if (err < 0) {
		fprintf(stderr, "failed to set configuration (%s)\n",
			libusb_error_name(err));
		return err;
	}

	err = libusb_claim_interface(devh, 1);
	if (err < 0) {
		fprintf(stderr, "failed to claim interface (%s)\n",
			libusb_error_name(err));
		return err;
	}

	err = libusb_set_interface_alt_setting(devh, 1, 0);
	if (err < 0) {
		fprintf(stderr, "failed to set interface alt config (%s)\n",
			libusb_error_name(err));
	}

	return err;
}

void *spmp_usb_init(void)
{
	int err;
	spmp_usb_ctx *ctx;
	libusb_context *usb;
	libusb_device_handle *dev;

	err = libusb_init(&usb);
	if (err) {
		fprintf(stderr, "failed to initialize libusb (%s)\n",
			libusb_error_name(err));
		return NULL;
	}

	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		goto ctx_close;

	dev = libusb_open_device_with_vid_pid(usb, SPMP_VENDOR, SPMP_DEVICE);
	if (!dev) {
		fprintf(stderr, "failed to find device %04X:%04X\n",
			SPMP_VENDOR, SPMP_DEVICE);
		goto ctx_close;
	}

	err = spmp_usb_claim(dev);
	if (err) {
		fprintf(stderr, "failed to claim device (%s)\n",
			libusb_error_name(err));
		goto ctx_close;
	}

	ctx->usb = usb;
	ctx->dev = dev;
	return ctx;

ctx_close:
	free(ctx);
	libusb_exit(usb);
	return NULL;
}

void spmp_usb_exit(void *ctx)
{
	spmp_usb_ctx *usb_ctx = ctx;
	libusb_close(usb_ctx->dev);
	libusb_exit(usb_ctx->usb);
	free(usb_ctx);
}
