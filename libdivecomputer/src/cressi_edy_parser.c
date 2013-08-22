/*
 * libdivecomputer
 *
 * Copyright (C) 2010 Jef Driesen
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

#include <stdlib.h>

#include <libdivecomputer/cressi_edy.h>

#include "context-private.h"
#include "parser-private.h"
#include "array.h"

#define ISINSTANCE(parser) dc_parser_isinstance((parser), &cressi_edy_parser_vtable)

typedef struct cressi_edy_parser_t cressi_edy_parser_t;

struct cressi_edy_parser_t {
	dc_parser_t base;
	unsigned int model;
};

static dc_status_t cressi_edy_parser_set_data (dc_parser_t *abstract, const unsigned char *data, unsigned int size);
static dc_status_t cressi_edy_parser_get_datetime (dc_parser_t *abstract, dc_datetime_t *datetime);
static dc_status_t cressi_edy_parser_get_field (dc_parser_t *abstract, dc_field_type_t type, unsigned int flags, void *value);
static dc_status_t cressi_edy_parser_samples_foreach (dc_parser_t *abstract, dc_sample_callback_t callback, void *userdata);
static dc_status_t cressi_edy_parser_destroy (dc_parser_t *abstract);

static const dc_parser_vtable_t cressi_edy_parser_vtable = {
	DC_FAMILY_CRESSI_EDY,
	cressi_edy_parser_set_data, /* set_data */
	cressi_edy_parser_get_datetime, /* datetime */
	cressi_edy_parser_get_field, /* fields */
	cressi_edy_parser_samples_foreach, /* samples_foreach */
	cressi_edy_parser_destroy /* destroy */
};


dc_status_t
cressi_edy_parser_create (dc_parser_t **out, dc_context_t *context, unsigned int model)
{
	if (out == NULL)
		return DC_STATUS_INVALIDARGS;

	// Allocate memory.
	cressi_edy_parser_t *parser = (cressi_edy_parser_t *) malloc (sizeof (cressi_edy_parser_t));
	if (parser == NULL) {
		ERROR (context, "Failed to allocate memory.");
		return DC_STATUS_NOMEMORY;
	}

	// Initialize the base class.
	parser_init (&parser->base, context, &cressi_edy_parser_vtable);

	// Set the default values.
	parser->model = model;

	*out = (dc_parser_t*) parser;

	return DC_STATUS_SUCCESS;
}


static dc_status_t
cressi_edy_parser_destroy (dc_parser_t *abstract)
{
	// Free memory.
	free (abstract);

	return DC_STATUS_SUCCESS;
}


static dc_status_t
cressi_edy_parser_set_data (dc_parser_t *abstract, const unsigned char *data, unsigned int size)
{
	return DC_STATUS_SUCCESS;
}


static dc_status_t
cressi_edy_parser_get_datetime (dc_parser_t *abstract, dc_datetime_t *datetime)
{
	if (abstract->size < 32)
		return DC_STATUS_DATAFORMAT;

	const unsigned char *p = abstract->data;

	if (datetime) {
		datetime->year = bcd2dec (p[4]) + 2000;
		datetime->month = (p[5] & 0xF0) >> 4;
		datetime->day = (p[5] & 0x0F) * 10 + ((p[6] & 0xF0) >> 4);
		datetime->hour = bcd2dec (p[14]);
		datetime->minute = bcd2dec (p[15]);
		datetime->second = 0;
	}

	return DC_STATUS_SUCCESS;
}


static dc_status_t
cressi_edy_parser_get_field (dc_parser_t *abstract, dc_field_type_t type, unsigned int flags, void *value)
{
	cressi_edy_parser_t *parser = (cressi_edy_parser_t *) abstract;

	if (abstract->size < 32)
		return DC_STATUS_DATAFORMAT;

	const unsigned char *p = abstract->data;

	dc_gasmix_t *gasmix = (dc_gasmix_t *) value;

	if (value) {
		switch (type) {
		case DC_FIELD_DIVETIME:
			if (parser->model == 0x08)
				*((unsigned int *) value) = bcd2dec (p[0x0C] & 0x0F) * 60 + bcd2dec (p[0x0D]);
			else
				*((unsigned int *) value) = (bcd2dec (p[0x0C] & 0x0F) * 100 + bcd2dec (p[0x0D])) * 60;
			break;
		case DC_FIELD_MAXDEPTH:
			*((double *) value) = (bcd2dec (p[0x02] & 0x0F) * 100 + bcd2dec (p[0x03])) / 10.0;
			break;
		case DC_FIELD_GASMIX_COUNT:
			*((unsigned int *) value) = 3;
			break;
		case DC_FIELD_GASMIX:
			gasmix->helium = 0.0;
			gasmix->oxygen = bcd2dec (p[0x17 - flags]) / 100.0;
			gasmix->nitrogen = 1.0 - gasmix->oxygen - gasmix->helium;
			break;
		default:
			return DC_STATUS_UNSUPPORTED;
		}
	}

	return DC_STATUS_SUCCESS;
}


static dc_status_t
cressi_edy_parser_samples_foreach (dc_parser_t *abstract, dc_sample_callback_t callback, void *userdata)
{
	cressi_edy_parser_t *parser = (cressi_edy_parser_t *) abstract;

	const unsigned char *data = abstract->data;
	unsigned int size = abstract->size;

	unsigned int time = 0;
	unsigned int interval = 0;
	if (parser->model == 0x08)
		interval = 1;
	else
		interval = 30;

	unsigned int offset = 32;
	while (offset + 2 <= size) {
		dc_sample_value_t sample = {0};

		if (data[offset] == 0xFF)
			break;

		unsigned int extra = 0;
		if (data[offset] & 0x80)
			extra = 4;

		// Time (seconds).
		time += interval;
		sample.time = time;
		if (callback) callback (DC_SAMPLE_TIME, sample, userdata);

		// Depth (1/10 m).
		unsigned int depth = bcd2dec (data[offset + 0] & 0x0F) * 100 + bcd2dec (data[offset + 1]);
		sample.depth = depth / 10.0;
		if (callback) callback (DC_SAMPLE_DEPTH, sample, userdata);

		offset += 2 + extra;
	}

	return DC_STATUS_SUCCESS;
}
