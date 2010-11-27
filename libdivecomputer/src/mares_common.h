/*
 * libdivecomputer
 *
 * Copyright (C) 2009 Jef Driesen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef MARES_COMMON_H
#define MARES_COMMON_H

#include "device-private.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct mares_common_layout_t {
	unsigned int memsize;
	unsigned int rb_profile_begin;
	unsigned int rb_profile_end;
	unsigned int rb_freedives_begin;
	unsigned int rb_freedives_end;
} mares_common_layout_t;

typedef struct mares_common_device_t {
	device_t base;
	unsigned char fingerprint[5];
	const mares_common_layout_t *layout;
} mares_common_device_t;

void
mares_common_device_init (mares_common_device_t *device, const device_backend_t *backend);

device_status_t
mares_common_device_set_fingerprint (device_t *device, const unsigned char data[], unsigned int size);

device_status_t
mares_common_extract_dives (mares_common_device_t *device, const mares_common_layout_t *layout, const unsigned char data[], dive_callback_t callback, void *userdata);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MARES_COMMON_H */
