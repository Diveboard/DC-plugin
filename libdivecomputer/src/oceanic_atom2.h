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

#ifndef OCEANIC_ATOM2_H
#define OCEANIC_ATOM2_H

#include "deviceL.h"
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

device_status_t
oceanic_atom2_device_open (device_t **device, const char* name);

device_status_t
oceanic_atom2_device_keepalive (device_t *device);

parser_status_t
oceanic_atom2_parser_create (parser_t **parser, unsigned int model);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* OCEANIC_ATOM2_H */
