/* SPDX-License-Identifier: GPL-2.0-only */

#include <stdint.h>

int spmp_usb_upload(void *ctx, uint32_t addr, void *data, int len);
int spmp_usb_boot(void *ctx, void *loader);

void *spmp_usb_init(void);
void spmp_usb_exit(void *ctx);
