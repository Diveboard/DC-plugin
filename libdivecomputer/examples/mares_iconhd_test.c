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

#include <stdio.h>	// fopen, fwrite, fclose
#include <stdlib.h>

#include <libdivecomputer/mares_iconhd.h>

#include "utils.h"
#include "common.h"

dc_status_t
test_dump_memory (const char* name, const char* filename, unsigned int model)
{
	dc_context_t *context = NULL;
	dc_device_t *device = NULL;

	dc_context_new (&context);
	dc_context_set_loglevel (context, DC_LOGLEVEL_ALL);
	dc_context_set_logfunc (context, logfunc, NULL);

	message ("mares_iconhd_device_open\n");
	dc_status_t rc = mares_iconhd_device_open (&device, context, name, model);
	if (rc != DC_STATUS_SUCCESS) {
		WARNING ("Error opening serial port.");
		dc_context_free (context);
		return rc;
	}

	dc_buffer_t *buffer = dc_buffer_new (0);

	message ("dc_device_dump\n");
	rc = dc_device_dump (device, buffer);
	if (rc != DC_STATUS_SUCCESS) {
		WARNING ("Cannot read memory.");
		dc_buffer_free (buffer);
		dc_device_close (device);
		dc_context_free (context);
		return rc;
	}

	message ("Dumping data\n");
	FILE* fp = fopen (filename, "wb");
	if (fp != NULL) {
		fwrite (dc_buffer_get_data (buffer), sizeof (unsigned char), dc_buffer_get_size (buffer), fp);
		fclose (fp);
	}

	dc_buffer_free (buffer);

	message ("dc_device_close\n");
	rc = dc_device_close (device);
	if (rc != DC_STATUS_SUCCESS) {
		WARNING ("Cannot close device.");
		dc_context_free (context);
		return rc;
	}

	dc_context_free (context);

	return DC_STATUS_SUCCESS;
}


int main(int argc, char *argv[])
{
	message_set_logfile ("ICONHD.LOG");

#ifdef _WIN32
	const char* name = "COM1";
#else
	const char* name = "/dev/ttyS0";
#endif
	unsigned int model = 0;

	if (argc > 1) {
		name = argv[1];
	}

	if (argc > 2) {
		model = strtoul (argv[2], NULL, 0);
	}

	message ("DEVICE=%s\n", name);
	message ("MODEL=0x%02x\n", model);

	dc_status_t a = test_dump_memory (name, "ICONHD.DMP", model);

	message ("\nSUMMARY\n");
	message ("-------\n");
	message ("test_dump_memory:          %s\n", errmsg (a));

	message_set_logfile (NULL);

	return 0;
}
