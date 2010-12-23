#include "ComputerLibdc.h"
#include "stdafx.h"
#include <tchar.h>
#include "Logger.h"

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
#include <string.h>
#include <signal.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef _WIN32
#define DC_TICKS_FORMAT "%I64d"
#else
#define DC_TICKS_FORMAT "%lld"
#endif

#include <suunto.h>
#include <reefnet.h>
#include <uwatec.h>
#include <oceanic.h>
#include <mares.h>
#include <hw.h>
#include <cressi.h>
#include <utils.h>


static const char *g_cachedir = NULL;
static int g_cachedir_read = 1;

typedef struct device_data_t {
	device_type_t backend;
	device_devinfo_t devinfo;
	device_clock_t clock;
} device_data_t;

typedef struct dive_data_t {
	device_data_t *devdata;
	//FILE* fp;
	std::string *out;
	unsigned int number;
	dc_buffer_t *fingerprint;
} dive_data_t;

typedef struct sample_data_t {
	//FILE* fp;
	std::string *out;
	unsigned int nsamples;
} sample_data_t;

typedef struct backend_table_t {
	const char *name;
	device_type_t type;
} backend_table_t;

static const backend_table_t g_backends[] = {
	{"LDC solution",	DEVICE_TYPE_SUUNTO_SOLUTION},
	{"LDC eon",			DEVICE_TYPE_SUUNTO_EON},
	{"LDC vyper",		DEVICE_TYPE_SUUNTO_VYPER},
	{"LDC vyper2",		DEVICE_TYPE_SUUNTO_VYPER2},
	{"LDC d9",			DEVICE_TYPE_SUUNTO_D9},
	{"LDC aladin",		DEVICE_TYPE_UWATEC_ALADIN},
	{"LDC memomouse",	DEVICE_TYPE_UWATEC_MEMOMOUSE},
	{"LDC smart",		DEVICE_TYPE_UWATEC_SMART},
	{"LDC sensus",		DEVICE_TYPE_REEFNET_SENSUS},
	{"LDC sensuspro",	DEVICE_TYPE_REEFNET_SENSUSPRO},
	{"LDC sensusultra",	DEVICE_TYPE_REEFNET_SENSUSULTRA},
	{"LDC vtpro",		DEVICE_TYPE_OCEANIC_VTPRO},
	{"LDC veo250",		DEVICE_TYPE_OCEANIC_VEO250},
	{"LDC atom2",		DEVICE_TYPE_OCEANIC_ATOM2},
	{"LDC nemo",		DEVICE_TYPE_MARES_NEMO},
	{"LDC puck",		DEVICE_TYPE_MARES_PUCK},
	{"LDC ostc",		DEVICE_TYPE_HW_OSTC},
	{"LDC edy",			DEVICE_TYPE_CRESSI_EDY}
};

static device_type_t
lookup_type (const char *name)
{
	unsigned int nbackends = sizeof (g_backends) / sizeof (g_backends[0]);
	for (unsigned int i = 0; i < nbackends; ++i) {
		if (strcmp (name, g_backends[i].name) == 0)
			return g_backends[i].type;
	}

	return DEVICE_TYPE_NULL;
}

static const char *
lookup_name (device_type_t type)
{
	unsigned int nbackends = sizeof (g_backends) / sizeof (g_backends[0]);
	for (unsigned int i = 0; i < nbackends; ++i) {
		if (g_backends[i].type == type)
			return g_backends[i].name;
	}

	return NULL;
}

static unsigned char
hex2dec (unsigned char value)
{
	if (value >= '0' && value <= '9')
		return value - '0';
	else if (value >= 'A' && value <= 'F')
		return value - 'A' + 10;
	else if (value >= 'a' && value <= 'f')
		return value - 'a' + 10;
	else
		return 0;
}

static dc_buffer_t *
fpconvert (const char *fingerprint)
{
	// Get the length of the fingerprint data.
	size_t nbytes = (fingerprint ? strlen (fingerprint) / 2 : 0);
	if (nbytes == 0)
		return NULL;

	// Allocate a memory buffer.
	dc_buffer_t *buffer = dc_buffer_new (nbytes);

	// Convert the hexadecimal string.
	for (unsigned int i = 0; i < nbytes; ++i) {
		unsigned char msn = hex2dec (fingerprint[i * 2 + 0]);
		unsigned char lsn = hex2dec (fingerprint[i * 2 + 1]);
		unsigned char byte = (msn << 4) + lsn;

		dc_buffer_append (buffer, &byte, 1);
	}

	return buffer;
}

/*
static dc_buffer_t *
fpread (const char *dirname, device_type_t backend, unsigned int serial)
{
	// Build the filename.
	char filename[1024] = {0};
	snprintf (filename, sizeof (filename), "%s/%s-%08X.bin",
		dirname, lookup_name (backend), serial);

	// Open the fingerprint file.
	FILE *fp = fopen (filename, "rb");
	if (fp == NULL)
		return NULL;

	// Allocate a memory buffer.
	dc_buffer_t *buffer = dc_buffer_new (0);

	// Read the entire file into the buffer.
	size_t n = 0;
	unsigned char block[1024] = {0};
	while ((n = fread (block, 1, sizeof (block), fp)) > 0) {
		dc_buffer_append (buffer, block, n);
	}

	// Close the file.
	fclose (fp);

	return buffer;
}
*/

/*
static void
fpwrite (dc_buffer_t *buffer, const char *dirname, device_type_t backend, unsigned int serial)
{
	// Check the buffer size.
	if (dc_buffer_get_size (buffer) == 0)
		return;

	// Build the filename.
	char filename[1024] = {0};
	snprintf (filename, sizeof (filename), "%s/%s-%08X.bin",
		dirname, lookup_name (backend), serial);

	// Open the fingerprint file.
	FILE *fp = fopen (filename, "wb");
	if (fp == NULL)
		return;

	// Write the fingerprint data.
	fwrite (dc_buffer_get_data (buffer), 1, dc_buffer_get_size (buffer), fp);

	// Close the file.
	fclose (fp);
}
*/


volatile sig_atomic_t g_cancel = 0;

void
sighandler (int signum)
{
#ifndef _WIN32
	// Restore the default signal handler.
	signal (signum, SIG_DFL);
#endif

	g_cancel = 1;
}

static int
cancel_cb (void *userdata)
{
	return g_cancel;
}


void
sample_cb (parser_sample_type_t type, parser_sample_value_t value, void *userdata)
{
	static const char *events[] = {
		"none", "DECO", "rbt", "ASCENT", "CEILING", "workload", "transmitter",
		"violation", "bookmark", "surface", "safety stop", "gaschange",
		"safety stop (voluntary)", "safety stop (mandatory)", "deepstop",
		"ceiling (safety stop)", "unknown", "divetime", "maxdepth",
		"OLF", "PO2", "airtime", "rgbm", "heading", "tissue level warning"};

	//sample_data_t *sampledata = (sample_data_t *) userdata;
	unsigned int nsamples = 0;
	std::string *diveXML = (std::string *)userdata;

	switch (type) {
	case SAMPLE_TYPE_TIME:
		nsamples++;
		*diveXML += str(boost::format("<t>%02u</t>") % value.time);
		break;
	case SAMPLE_TYPE_DEPTH:
		*diveXML += str(boost::format("<d>%.2f</d>") % value.depth);
		break;
	case SAMPLE_TYPE_PRESSURE:
		//todo
		*diveXML += str(boost::format("<pressure tank=\"%u\">%.2f</pressure>\n") %  value.pressure.tank % value.pressure.value);
		break;
	case SAMPLE_TYPE_TEMPERATURE:
		*diveXML += str(boost::format("<temperature>%.2f</temperature>") % value.temperature);
		break;
	case SAMPLE_TYPE_EVENT:
		*diveXML += str(boost::format("<ALARM type=\"%u\" time=\"%u\" flags=\"%u\" value=\"%u\">%s</ALARM>")
			% value.event.type % value.event.time % value.event.flags % value.event.value % events[value.event.type]);
		break;
	case SAMPLE_TYPE_RBT:
		*diveXML += str(boost::format("<rbt>%u</rbt>") % value.rbt);
		break;
	case SAMPLE_TYPE_HEARTBEAT:
		//todo
		*diveXML += str(boost::format("<heartbeat>%u</heartbeat>") % value.heartbeat);
		break;
	case SAMPLE_TYPE_BEARING:
		//todo
		*diveXML += str(boost::format("<bearing>%u</bearing>") % value.bearing);
		break;
	case SAMPLE_TYPE_VENDOR:
		*diveXML += str(boost::format("<vendor type=\"%u\" size=\"%u\">") % value.vendor.type % value.vendor.size);
		for (unsigned int i = 0; i < value.vendor.size; ++i)
			*diveXML += str(boost::format("%02X") % (((unsigned char *) value.vendor.data)[i]));
		*diveXML += "</vendor>\n";
		break;
	default:
		break;
	}
}



static parser_status_t doparse (std::string *out, device_data_t *devdata, const unsigned char data[], unsigned int size)
{
	// Create the parser.
	Logger::append("Creating the parser.");
	parser_t *parser = NULL;
	parser_status_t rc = PARSER_STATUS_SUCCESS;

			//HINSTANCE libdc = LoadLibrary(_T("D:\\DATA\\My Documents\\Personnel\\DB_plugins\\build\\Debug\\libdivecomputer.dll"));
			//LDCPARSER* suunto_vyper_parser_create = reinterpret_cast<LDCPARSER*>(GetProcAddress(libdc, "suunto_vyper_parser_create"));


	switch (devdata->backend) {
	case DEVICE_TYPE_SUUNTO_SOLUTION:
		rc = suunto_solution_parser_create (&parser);
		break;
	case DEVICE_TYPE_SUUNTO_EON:
		rc = suunto_eon_parser_create (&parser, 0);
		break;
	case DEVICE_TYPE_SUUNTO_VYPER:
		if (devdata->devinfo.model == 0x01)
			rc = suunto_eon_parser_create (&parser, 1);
		else
			rc = suunto_vyper_parser_create (&parser);
		break;
	case DEVICE_TYPE_SUUNTO_VYPER2:
	case DEVICE_TYPE_SUUNTO_D9:
		rc = suunto_d9_parser_create (&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_UWATEC_ALADIN:
	case DEVICE_TYPE_UWATEC_MEMOMOUSE:
		rc = uwatec_memomouse_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_UWATEC_SMART:
		rc = uwatec_smart_parser_create (&parser, devdata->devinfo.model, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUS:
		rc = reefnet_sensus_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUSPRO:
		rc = reefnet_sensuspro_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUSULTRA:
		rc = reefnet_sensusultra_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_OCEANIC_VTPRO:
		rc = oceanic_vtpro_parser_create (&parser);
		break;
	case DEVICE_TYPE_OCEANIC_VEO250:
		rc = oceanic_veo250_parser_create (&parser);
		break;
	case DEVICE_TYPE_OCEANIC_ATOM2:
		rc = oceanic_atom2_parser_create (&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_MARES_NEMO:
	case DEVICE_TYPE_MARES_PUCK:
		rc = mares_nemo_parser_create (&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_HW_OSTC:
		rc = hw_ostc_parser_create (&parser);
		break;
	case DEVICE_TYPE_CRESSI_EDY:
		rc = cressi_edy_parser_create (&parser);
		break;
	default:
		rc = PARSER_STATUS_ERROR;
		break;
	}
	if (rc != PARSER_STATUS_SUCCESS) {
		Logger::append("Error creating the parser.");
		return rc;
	}

	// Register the data.
	Logger::append("Registering the data.\n");
	//LDCPARSERSETDATA* parser_set_data = reinterpret_cast<LDCPARSERSETDATA*>(GetProcAddress(libdc, "parser_set_data"));
	//LDCPARSERDESTROY* parser_destroy = reinterpret_cast<LDCPARSERDESTROY*>(GetProcAddress(libdc, "parser_destroy"));
	rc = parser_set_data (parser, data, size);
	if (rc != PARSER_STATUS_SUCCESS) {
		Logger::append("Error registering the data.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error registering the data - Error code : %1%") % rc));
	}

	// Parse the datetime.
	Logger::append("Parsing the datetime.\n");
	dc_datetime_t dt = {0};
	//LDCPARSERGETDATETIME* parser_get_datetime = reinterpret_cast<LDCPARSERGETDATETIME*>(GetProcAddress(libdc, "parser_get_datetime"));
	rc = parser_get_datetime (parser, &dt);
	if (rc != PARSER_STATUS_SUCCESS && rc != PARSER_STATUS_UNSUPPORTED) {
		Logger::append("Error parsing the datetime.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error parsing the datetime - Error code : %1%") % rc));
	}

	*out += str(boost::format("<DATE>%04i-%02i-%02i</DATE><TIME>%02i:%02i:%02i</TIME>\n")
		% dt.year % dt.month % dt.day
		% dt.hour % dt.minute % dt.second);

	// Initialize the sample data.
	unsigned int nsamples = 0;

	// Parse the sample data.
	Logger::append("Parsing the sample data.\n");
	*out += "<SAMPLES>";
	//LDCPARSERSAMPLESFOREACH* parser_samples_foreach = reinterpret_cast<LDCPARSERSAMPLESFOREACH*>(GetProcAddress(libdc, "parser_samples_foreach"));
	rc = parser_samples_foreach (parser, sample_cb, out);
	if (rc != PARSER_STATUS_SUCCESS) {
		Logger::append("Error parsing the sample data.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error parsing the sample data - Error code : %1%") % rc));
	}

	if (nsamples)
		*out += "</SAMPLES>\n";

	// Destroy the parser.
	Logger::append("Destroying the parser.\n");
	rc = parser_destroy (parser);
	if (rc != PARSER_STATUS_SUCCESS) {
		Logger::append("WARNING - Error destroying the parser - Error %d", rc);
	}

	return PARSER_STATUS_SUCCESS;
}



static void event_cb (device_t *device, device_event_t event, const void *data, void *userdata)
{
	const device_progress_t *progress = (device_progress_t *) data;
	const device_devinfo_t *devinfo = (device_devinfo_t *) data;
	const device_clock_t *clock = (device_clock_t *) data;

	device_data_t *devdata = (device_data_t *) userdata;

	switch (event) {
	case DEVICE_EVENT_WAITING:
		Logger::append("Event: waiting for user action\n");
		break;
	case DEVICE_EVENT_PROGRESS:
		Logger::append("Event: progress %3.2f%% (%u/%u)\n",
			100.0 * (double) progress->current / (double) progress->maximum,
			progress->current, progress->maximum);
		//todo fire event
		break;
	case DEVICE_EVENT_DEVINFO:
		devdata->devinfo = *devinfo;
		Logger::append("Event: model=%u (0x%08x), firmware=%u (0x%08x), serial=%u (0x%08x)\n",
			devinfo->model, devinfo->model,
			devinfo->firmware, devinfo->firmware,
			devinfo->serial, devinfo->serial);
		//todo handle fingerprints
		/*if (g_cachedir && g_cachedir_read) {
			dc_buffer_t *fingerprint = fpread (g_cachedir, devdata->backend, devinfo->serial);
			device_set_fingerprint (device,
				dc_buffer_get_data (fingerprint),
				dc_buffer_get_size (fingerprint));
			dc_buffer_free (fingerprint);
		}*/
		break;
	case DEVICE_EVENT_CLOCK:
		devdata->clock = *clock;
		Logger::append("Event: systime=" DC_TICKS_FORMAT ", devtime=%u\n",
			clock->systime, clock->devtime);
		break;
	default:
		break;
	}
}

static int dive_cb (const unsigned char *data, unsigned int size, const unsigned char *fingerprint, unsigned int fsize, void *userdata)
{
	dive_data_t *divedata = (dive_data_t *) userdata;

	divedata->number++;

	//HINSTANCE libdc = LoadLibrary(_T("D:\\DATA\\My Documents\\Personnel\\DB_plugins\\build\\Debug\\libdivecomputer.dll"));
	//LCDDCBUFFERNEW* dc_buffer_new = reinterpret_cast<LCDDCBUFFERNEW*>(GetProcAddress(libdc, "dc_buffer_new"));
	//LCDDCBUFFERAPPEND* dc_buffer_append = reinterpret_cast<LCDDCBUFFERAPPEND*>(GetProcAddress(libdc, "dc_buffer_append"));

	Logger::append("Dive: number=%u, size=%u, fingerprint=", divedata->number, size);
	for (unsigned int i = 0; i < fsize; ++i)
		Logger::append("%02X", fingerprint[i]);
	Logger::append("\n");

	if (divedata->number == 1) {
		divedata->fingerprint = dc_buffer_new (fsize);
		dc_buffer_append (divedata->fingerprint, fingerprint, fsize);
	}

	*(divedata->out) += "<DIVE><PROGRAM><fingerprint>";
	for (unsigned int i = 0; i < fsize; ++i)
			*(divedata->out) += str(boost::format( "%02X") % fingerprint[i]);
	*(divedata->out) += "</fingerprint></PROGRAM>";

		doparse (divedata->out, divedata->devdata, data, size);

		*(divedata->out) += "</DIVE>";

	return 1;
}


static const char*
errmsg (device_status_t rc)
{
	switch (rc) {
	case DEVICE_STATUS_SUCCESS:
		return "Success";
	case DEVICE_STATUS_UNSUPPORTED:
		return "Unsupported operation";
	case DEVICE_STATUS_TYPE_MISMATCH:
		return "Device type mismatch";
	case DEVICE_STATUS_ERROR:
		return "Generic error";
	case DEVICE_STATUS_IO:
		return "Input/output error";
	case DEVICE_STATUS_MEMORY:
		return "Memory error";
	case DEVICE_STATUS_PROTOCOL:
		return "Protocol error";
	case DEVICE_STATUS_TIMEOUT:
		return "Timeout";
	case DEVICE_STATUS_CANCELLED:
		return "Cancelled";
	default:
		return "Unknown error";
	}
}


void dowork (device_type_t &backend, const std::string &devname, std ::string &diveXML, dc_buffer_t *fingerprint)
{
	device_status_t rc = DEVICE_STATUS_SUCCESS;

	// Initialize the device data.
	device_data_t devdata;
	devdata.backend = backend;

	// Open the device.
	Logger::append("Opening the device (%s, %s).\n", lookup_name (backend), devname.c_str());
	device_t *device = NULL;

	//HINSTANCE libdc = LoadLibrary(_T("D:\\DATA\\My Documents\\Personnel\\DB_plugins\\build\\Debug\\libdivecomputer.dll"));
	/*if (!libdc)
	{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

		Logger::append(str(boost::format("Error loading DLL : %d - %s") % dw % (char*)lpMsgBuf));
		return DEVICE_STATUS_ERROR;
	}
	LDCOPEN* suunto_vyper_device_open = reinterpret_cast<LDCOPEN*>(GetProcAddress(libdc, "suunto_vyper_device_open"));
	*/

	switch (backend) {
	case DEVICE_TYPE_SUUNTO_SOLUTION:
		rc = suunto_solution_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_EON:
		rc = suunto_eon_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_VYPER:
		rc = suunto_vyper_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_VYPER2:
		rc = suunto_vyper2_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_D9:
		rc = suunto_d9_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_ALADIN:
		rc = uwatec_aladin_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_MEMOMOUSE:
		rc = uwatec_memomouse_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_SMART:
		rc = uwatec_smart_device_open (&device);
		break;
	case DEVICE_TYPE_REEFNET_SENSUS:
		rc = reefnet_sensus_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_REEFNET_SENSUSPRO:
		rc = reefnet_sensuspro_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_REEFNET_SENSUSULTRA:
		rc = reefnet_sensusultra_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_VTPRO:
		rc = oceanic_vtpro_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_VEO250:
		rc = oceanic_veo250_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_ATOM2:
		rc = oceanic_atom2_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_MARES_NEMO:
		rc = mares_nemo_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_MARES_PUCK:
		rc = mares_puck_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_HW_OSTC:
		rc = hw_ostc_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_CRESSI_EDY:
		rc = cressi_edy_device_open (&device, devname.c_str());
		break;
	default:
		rc = DEVICE_STATUS_ERROR;
		break;
	}
	if (rc != DEVICE_STATUS_SUCCESS)
		throw DBException (std::string("Error opening device - Error code : ") + errmsg(rc));

	// todo Register the event handler.
	Logger::append("Registering the event handler.\n");
	int events = DEVICE_EVENT_WAITING | DEVICE_EVENT_PROGRESS | DEVICE_EVENT_DEVINFO | DEVICE_EVENT_CLOCK;
	rc = device_set_events (device, events, event_cb, &devdata);
	if (rc != DEVICE_STATUS_SUCCESS) {
		Logger::append("Error registering the event handler.");
		device_close (device);
		throw DBException ("Error setting the event handler ");
	}

	// todo Register the cancellation handler.
	/*Logger::append("Registering the cancellation handler.\n");
	rc = device_set_cancel (device, cancel_cb, NULL);
	if (rc != DEVICE_STATUS_SUCCESS) {
		Logger::append("Error registering the cancellation handler.");
		device_close (device);
		return rc;
	}
	*/

	// todo Register the fingerprint data.
	/*if (fingerprint) {
		Logger::append("Registering the fingerprint data.\n");
		rc = device_set_fingerprint (device, dc_buffer_get_data (fingerprint), dc_buffer_get_size (fingerprint));
		if (rc != DEVICE_STATUS_SUCCESS) {
			Logger::append("Error registering the fingerprint data.");
			device_close (device);
			return rc;
		}
	}
	*/

	//Dump the data
	// Allocate a memory buffer.
	/* works but is too slow !!
	dc_buffer_t *buffer = dc_buffer_new (0);

	Logger::append("Dumping the memory from device");
	rc = device_dump (device, buffer);
	if (rc != DEVICE_STATUS_SUCCESS) {
		Logger::append("WARNING - Error downloading the memory dump.");
		dc_buffer_free (buffer);
	}
	else {
		Logger::append("Adding data to Logger::binary");
		Logger::binary("LIBDC", dc_buffer_get_data (buffer), dc_buffer_get_size (buffer));
		// Free the memory buffer.
		dc_buffer_free (buffer);
	}
	*/


		// Initialize the dive data.
		dive_data_t divedata = {0};
		divedata.devdata = &devdata;
		divedata.fingerprint = NULL;
		divedata.number = 0;
		divedata.out = &diveXML;

		diveXML.append("<profile udcf='1'><REPGROUP>");

		// Download the dives.
		Logger::append("Downloading the dives.\n");
		//LCDDEVFOREACH* device_foreach = reinterpret_cast<LCDDEVFOREACH*>(GetProcAddress(libdc, "device_foreach"));
		//LCDDEVCLOSE* device_close = reinterpret_cast<LCDDEVCLOSE*>(GetProcAddress(libdc, "device_close"));
		//LCDDCBUFFERFREE* dc_buffer_free = reinterpret_cast<LCDDCBUFFERFREE*>(GetProcAddress(libdc, "dc_buffer_free"));
		rc = device_foreach (device, dive_cb, &divedata);
		if (rc != DEVICE_STATUS_SUCCESS) {
			Logger::append("Error downloading the dives.");
			dc_buffer_free (divedata.fingerprint);
			device_close (device);
			throw DBException (std::string("Error opening device - Error code : ") + errmsg(rc));
		}

		// todo Store the fingerprint data.
		/*if (g_cachedir) {
			fpwrite (divedata.fingerprint, g_cachedir, devdata.backend, devdata.devinfo.serial);
		}*/

		// todo Free the fingerprint buffer.
		//dc_buffer_free (divedata.fingerprint);

		diveXML.append("</REPGROUP></profile>");

		// Close the device.
		Logger::append("Closing the device.\n");
		rc = device_close (device);
		if (rc != DEVICE_STATUS_SUCCESS)
			Logger::append("WARNING - Error closing the device. %s", errmsg(rc));
}




















int ComputerLibdc::_get_all_dives(std::string &diveXML)
{
  //Get the various function pointers we require from setupapi.dll
  //HINSTANCE libdc = LoadLibrary(_T("libdivecomputer-0.dll"));

  //todo handle error
  //if (hSetupAPI == NULL)
  //  return;

  //LDCOPEN* ldcOpen = reinterpret_cast<LDCOPEN*>(GetProcAddress(libdc, "suunto_vyper2_device_open"));

  Logger::append(str(boost::format("Using backend on %1%") % devname));
  dowork (backend, devname, diveXML, NULL);

  return(0);
} 







ComputerLibdc::ComputerLibdc(std::string type, std::string file)
{
	unsigned int nbackends = sizeof (g_backends) / sizeof (g_backends[0]);

	for (unsigned int i = 0; i < nbackends; i++) {
		if (type.compare(g_backends[i].name) == 0)
			backend = g_backends[i].type;
	}

	Logger::append(str(boost::format("Using type %1% on %2%") % type % file));
	devname = file;

	//message_set_logfile("d:\\temp\\libdc.log");
	//return COMPUTER_MODEL_UNKNOWN;
}


ComputerLibdc::~ComputerLibdc(void)
{
}




//todo
ComputerModel ComputerLibdc::_get_model()
{

	return(COMPUTER_MODEL_UNKNOWN);
}

//todo
ComputerState ComputerLibdc::_get_status()
{
	return(COMPUTER_RUNNING);
}

