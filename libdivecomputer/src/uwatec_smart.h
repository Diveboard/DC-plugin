/* 
 * libdivecomputer
 * 
 * Copyright (C) 2008 Jef Driesen
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

#ifndef UWATEC_SMART_H
#define UWATEC_SMART_H


#include "deviceL.h"
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UWATEC_SMART_VERSION_SIZE 9

device_status_t
uwatec_smart_device_open (device_t **device);

device_status_t
uwatec_smart_device_set_timestamp (device_t *device, unsigned int timestamp);

device_status_t
uwatec_smart_extract_dives (device_t *device, const unsigned char data[], unsigned int size, dive_callback_t callback, void *userdata);

parser_status_t
uwatec_smart_parser_create (parser_t **parser, unsigned int model, unsigned int devtime, dc_ticks_t systime);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* UWATEC_SMART_H */
