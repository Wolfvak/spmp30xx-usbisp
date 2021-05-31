/* SPDX-License-Identifier: GPL-2.0-only */

#include <stdint.h>

int spmp_usb_upload(void *ctx, uint32_t addr, void *data, int len);
int spmp_usb_download(void *ctx, uint32_t addr, void *data, int len);
int spmp_usb_boot(void *ctx, void *loader, int pages);

int spmp_usb_peek(void *ctx, uint16_t offset, uint8_t *value);
int spmp_usb_poke(void *ctx, uint16_t offset, uint8_t value);

void *spmp_usb_init(void);
void spmp_usb_exit(void *ctx);
