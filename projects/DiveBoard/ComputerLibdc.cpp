#include "ComputerLibdc.h"
#include "stdafx.h"
#include <tchar.h>
#include "Logger.h"

#ifdef WIN32
#define DLL_PATH _T("D:\\DATA\\My Documents\\Personnel\\DB_plugins\\libdivecomputer\\Debug\\libdivecomputer.dll")
#endif

HINSTANCE openDLLLibrary()
{
	//Load the LibDiveComputer library
	HINSTANCE libdc = LoadLibrary(DLL_PATH);
	if (!libdc)
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

		LOGERROR(str(boost::format("Error loading DLL : %d - %s") % dw % (char*)lpMsgBuf));
		throw DBException("Error while loading dll");
	}

	return libdc;
}

void *getDLLFunction(HINSTANCE libdc, const char *function)
{
	void *ptr;
	ptr = GetProcAddress(libdc, function);
	if (!ptr)  {
		LOGERROR("Error fetching DLL pointer to %s", function);
		throw DBException("Error fetching DLL pointer");
	}
	return(ptr);
}




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

typedef struct event_data_t {
	ComputerStatus *status;
	device_data_t *devdata;
} event_data_t;

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
	LOGINFO("Creating the parser.");
	parser_t *parser = NULL;
	parser_status_t rc = PARSER_STATUS_SUCCESS;

	//getting general pointers to functions of libDiveComputer
	//todo factorize this in ComputerLibdc constructor
	LOGDEBUG("Opening LibDiveComputer");
	HINSTANCE libdc = openDLLLibrary();
	LOGDEBUG("LibDiveComputer DLL loaded");
	LCDDEVFOREACH* device_foreach = reinterpret_cast<LCDDEVFOREACH*>(getDLLFunction(libdc, "device_foreach"));
	LCDDEVCLOSE* device_close = reinterpret_cast<LCDDEVCLOSE*>(getDLLFunction(libdc, "device_close"));
	LCDDCBUFFERFREE* dc_buffer_free = reinterpret_cast<LCDDCBUFFERFREE*>(getDLLFunction(libdc, "dc_buffer_free"));
	LDCDEVSETEVENTS *device_set_events = reinterpret_cast<LDCDEVSETEVENTS*>(getDLLFunction(libdc, "device_set_events"));
	LCDDCBUFFERAPPEND *dc_buffer_append = reinterpret_cast<LCDDCBUFFERAPPEND*>(getDLLFunction(libdc, "dc_buffer_append"));
	LCDDCBUFFERNEW *dc_buffer_new = reinterpret_cast<LCDDCBUFFERNEW*>(getDLLFunction(libdc, "dc_buffer_new"));
	LDCPARSERSAMPLESFOREACH *parser_samples_foreach = reinterpret_cast<LDCPARSERSAMPLESFOREACH*>(getDLLFunction(libdc, "parser_samples_foreach"));
	LDCPARSERGETDATETIME *parser_get_datetime = reinterpret_cast<LDCPARSERGETDATETIME*>(getDLLFunction(libdc, "parser_get_datetime"));
	LDCPARSERDESTROY *parser_destroy = reinterpret_cast<LDCPARSERDESTROY*>(getDLLFunction(libdc, "parser_destroy"));
	LDCPARSERSETDATA *parser_set_data = reinterpret_cast<LDCPARSERSETDATA*>(getDLLFunction(libdc, "parser_set_data"));
	LOGDEBUG("General pointers to LibDiveComputer fetched");

	parser_status_t (*create_parser1)(parser_t **) = NULL;
	parser_status_t (*create_parser2)(parser_t **, unsigned int) = NULL;
	parser_status_t (*create_parser3)(parser_t **, unsigned int, dc_ticks_t) = NULL;
	parser_status_t (*create_parser4)(parser_t **, unsigned int, unsigned int, dc_ticks_t) = NULL;

	switch (devdata->backend) {
	case DEVICE_TYPE_SUUNTO_SOLUTION:
		create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "suunto_solution_parser_create");
		rc = create_parser1 (&parser);
		break;
	case DEVICE_TYPE_SUUNTO_EON:
		create_parser2 = (parser_status_t (*)(parser_t **, unsigned int))getDLLFunction(libdc, "suunto_eon_parser_create");
		rc = create_parser2(&parser, 0);
		break;
	case DEVICE_TYPE_SUUNTO_VYPER:
		if (devdata->devinfo.model == 0x01) {
			create_parser2 = (parser_status_t (*)(parser_t **, unsigned int))getDLLFunction(libdc, "suunto_eon_parser_create");
			rc = create_parser2(&parser, 1);
		}
		else {
			create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "suunto_vyper_parser_create");
			rc = create_parser1(&parser);
		}
		break;
	case DEVICE_TYPE_SUUNTO_VYPER2:
	case DEVICE_TYPE_SUUNTO_D9:
		create_parser2 = (parser_status_t (*)(parser_t **, unsigned int))getDLLFunction(libdc, "suunto_d9_parser_create");
		rc = create_parser2(&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_UWATEC_ALADIN:
	case DEVICE_TYPE_UWATEC_MEMOMOUSE:
		create_parser3 = (parser_status_t (*)(parser_t **, unsigned int, dc_ticks_t))getDLLFunction(libdc, "uwatec_memomouse_parser_create");
		rc = create_parser3(&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_UWATEC_SMART:
		create_parser4 = (parser_status_t (*)(parser_t **, unsigned int, unsigned int, dc_ticks_t))getDLLFunction(libdc, "uwatec_smart_parser_create");
		rc = create_parser4(&parser, devdata->devinfo.model, devdata->clock.devtime, devdata->clock.systime);
		//rc = uwatec_smart_parser_create (&parser, devdata->devinfo.model, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUS:
		create_parser3 = (parser_status_t (*)(parser_t **, unsigned int, dc_ticks_t))getDLLFunction(libdc, "reefnet_sensus_parser_create");
		rc = create_parser3(&parser, devdata->clock.devtime, devdata->clock.systime);
		//rc = reefnet_sensus_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUSPRO:
		create_parser3 = (parser_status_t (*)(parser_t **, unsigned int, dc_ticks_t))getDLLFunction(libdc, "reefnet_sensuspro_parser_create");
		rc = create_parser3(&parser, devdata->clock.devtime, devdata->clock.systime);
		//rc = reefnet_sensuspro_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_REEFNET_SENSUSULTRA:
		create_parser3 = (parser_status_t (*)(parser_t **, unsigned int, dc_ticks_t))getDLLFunction(libdc, "reefnet_sensusultra_parser_create");
		rc = create_parser3(&parser, devdata->clock.devtime, devdata->clock.systime);
		//rc = reefnet_sensusultra_parser_create (&parser, devdata->clock.devtime, devdata->clock.systime);
		break;
	case DEVICE_TYPE_OCEANIC_VTPRO:
		create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "oceanic_vtpro_parser_create");
		rc = create_parser1(&parser);
		//rc = oceanic_vtpro_parser_create (&parser);
		break;
	case DEVICE_TYPE_OCEANIC_VEO250:
		create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "oceanic_veo250_parser_create");
		rc = create_parser1(&parser);
		//rc = oceanic_veo250_parser_create (&parser);
		break;
	case DEVICE_TYPE_OCEANIC_ATOM2:
		create_parser2 = (parser_status_t (*)(parser_t **, unsigned int))getDLLFunction(libdc, "oceanic_atom2_parser_create");
		rc = create_parser2(&parser, devdata->devinfo.model);
		//rc = oceanic_atom2_parser_create (&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_MARES_NEMO:
	case DEVICE_TYPE_MARES_PUCK:
		create_parser2 = (parser_status_t (*)(parser_t **, unsigned int))getDLLFunction(libdc, "mares_nemo_parser_create");
		rc = create_parser2(&parser, devdata->devinfo.model);
		//rc = mares_nemo_parser_create (&parser, devdata->devinfo.model);
		break;
	case DEVICE_TYPE_HW_OSTC:
		create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "hw_ostc_parser_create");
		rc = create_parser1(&parser);
		//rc = hw_ostc_parser_create (&parser);
		break;
	case DEVICE_TYPE_CRESSI_EDY:
		create_parser1 = (parser_status_t (*)(parser_t **))getDLLFunction(libdc, "cressi_edy_parser_create");
		rc = create_parser1(&parser);
		//rc = cressi_edy_parser_create (&parser);
		break;
	default:
		rc = PARSER_STATUS_ERROR;
		break;
	}

	if (rc != PARSER_STATUS_SUCCESS) {
		LOGINFO("Error creating the parser.");
		return rc;
	}

	// Register the data.
	LOGINFO("Registering the data.\n");
	//LDCPARSERSETDATA* parser_set_data = reinterpret_cast<LDCPARSERSETDATA*>(GetProcAddress(libdc, "parser_set_data"));
	//LDCPARSERDESTROY* parser_destroy = reinterpret_cast<LDCPARSERDESTROY*>(GetProcAddress(libdc, "parser_destroy"));
	rc = parser_set_data (parser, data, size);
	if (rc != PARSER_STATUS_SUCCESS) {
		LOGINFO("Error registering the data.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error registering the data - Error code : %1%") % rc));
	}

	// Parse the datetime.
	LOGINFO("Parsing the datetime.\n");
	dc_datetime_t dt = {0};
	//LDCPARSERGETDATETIME* parser_get_datetime = reinterpret_cast<LDCPARSERGETDATETIME*>(GetProcAddress(libdc, "parser_get_datetime"));
	rc = parser_get_datetime (parser, &dt);
	if (rc != PARSER_STATUS_SUCCESS && rc != PARSER_STATUS_UNSUPPORTED) {
		LOGINFO("Error parsing the datetime.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error parsing the datetime - Error code : %1%") % rc));
	}

	*out += str(boost::format("<DATE>%04i-%02i-%02i</DATE><TIME>%02i:%02i:%02i</TIME>\n")
		% dt.year % dt.month % dt.day
		% dt.hour % dt.minute % dt.second);

	// Initialize the sample data.
	unsigned int nsamples = 0;

	// Parse the sample data.
	LOGINFO("Parsing the sample data.\n");
	*out += "<SAMPLES>";
	//LDCPARSERSAMPLESFOREACH* parser_samples_foreach = reinterpret_cast<LDCPARSERSAMPLESFOREACH*>(GetProcAddress(libdc, "parser_samples_foreach"));
	rc = parser_samples_foreach (parser, sample_cb, out);
	if (rc != PARSER_STATUS_SUCCESS) {
		LOGINFO("Error parsing the sample data.");
		parser_destroy (parser);
		throw DBException (str(boost::format("Error parsing the sample data - Error code : %1%") % rc));
	}

	*out += "</SAMPLES>\n";

	// Destroy the parser.
	LOGINFO("Destroying the parser.\n");
	rc = parser_destroy (parser);
	if (rc != PARSER_STATUS_SUCCESS) {
		LOGINFO("WARNING - Error destroying the parser - Error %d", rc);
	}

	return PARSER_STATUS_SUCCESS;
}



static void event_cb (device_t *device, device_event_t event, const void *data, void *userdata)
{
	const device_progress_t *progress = (device_progress_t *) data;
	const device_devinfo_t *devinfo = (device_devinfo_t *) data;
	const device_clock_t *clock = (device_clock_t *) data;

	event_data_t *evdata = (event_data_t *)userdata;

	device_data_t *devdata = evdata->devdata;

	switch (event) {
	case DEVICE_EVENT_WAITING:
		LOGINFO("Event: waiting for user action\n");
		break;
	case DEVICE_EVENT_PROGRESS:
		if (progress->maximum >0)
		{
			LOGINFO("Event: progress %3.2f%% (%u/%u)\n",
			100.0 * (double) progress->current / (double) progress->maximum,
			progress->current, progress->maximum);

			evdata->status->percent = 100.0 * (double) progress->current / (double) progress->maximum;
		}
		else
		{
			LOGINFO("Event: progress negative !");
		}
		break;
	case DEVICE_EVENT_DEVINFO:
		devdata->devinfo = *devinfo;
		LOGINFO("Event: model=%u (0x%08x), firmware=%u (0x%08x), serial=%u (0x%08x)\n",
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
		LOGINFO("Event: systime=" DC_TICKS_FORMAT ", devtime=%u\n",
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

	//getting general pointers to functions of libDiveComputer
	//todo factorize this in ComputerLibdc constructor
	LOGDEBUG("Opening LibDiveComputer DLL");
	HINSTANCE libdc = openDLLLibrary();
	LOGDEBUG("LibDiveComputer DLL loaded");
	LCDDEVFOREACH* device_foreach = reinterpret_cast<LCDDEVFOREACH*>(getDLLFunction(libdc, "device_foreach"));
	LCDDEVCLOSE* device_close = reinterpret_cast<LCDDEVCLOSE*>(getDLLFunction(libdc, "device_close"));
	LCDDCBUFFERFREE* dc_buffer_free = reinterpret_cast<LCDDCBUFFERFREE*>(getDLLFunction(libdc, "dc_buffer_free"));
	LDCDEVSETEVENTS *device_set_events = reinterpret_cast<LDCDEVSETEVENTS*>(getDLLFunction(libdc, "device_set_events"));
	LCDDCBUFFERAPPEND *dc_buffer_append = reinterpret_cast<LCDDCBUFFERAPPEND*>(getDLLFunction(libdc, "dc_buffer_append"));
	LCDDCBUFFERNEW *dc_buffer_new = reinterpret_cast<LCDDCBUFFERNEW*>(getDLLFunction(libdc, "dc_buffer_new"));
	LDCPARSERSAMPLESFOREACH *parser_samples_foreach = reinterpret_cast<LDCPARSERSAMPLESFOREACH*>(getDLLFunction(libdc, "parser_samples_foreach"));
	LDCPARSERGETDATETIME *parser_get_datetime = reinterpret_cast<LDCPARSERGETDATETIME*>(getDLLFunction(libdc, "parser_get_datetime"));
	LDCPARSERDESTROY *parser_destroy = reinterpret_cast<LDCPARSERDESTROY*>(getDLLFunction(libdc, "parser_destroy"));
	LDCPARSERSETDATA *parser_set_data = reinterpret_cast<LDCPARSERSETDATA*>(getDLLFunction(libdc, "parser_set_data"));
	LOGDEBUG("General pointers to LibDiveComputer fetched");
	

	LOGINFO("Dive: number=%u, size=%u, fingerprint=", divedata->number, size);
	for (unsigned int i = 0; i < fsize; ++i)
		LOGINFO("%02X", fingerprint[i]);
	LOGINFO("\n");

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


void dowork (device_type_t &backend, const std::string &devname, std ::string &diveXML, dc_buffer_t *fingerprint, ComputerStatus &status)
{
	device_status_t rc = DEVICE_STATUS_SUCCESS;

	// Initialize the device data.
	device_data_t devdata;
	devdata.backend = backend;

	//getting general pointers to functions of libDiveComputer
	//todo factorize this in ComputerLibdc constructor
	LOGDEBUG("Opening LibDiveComputer");
	HINSTANCE libdc = openDLLLibrary();
	LOGDEBUG("LibDiveComputer DLL loaded");
	LCDDEVFOREACH* device_foreach = reinterpret_cast<LCDDEVFOREACH*>(getDLLFunction(libdc, "device_foreach"));
	LCDDEVCLOSE* device_close = reinterpret_cast<LCDDEVCLOSE*>(getDLLFunction(libdc, "device_close"));
	LCDDCBUFFERFREE* dc_buffer_free = reinterpret_cast<LCDDCBUFFERFREE*>(getDLLFunction(libdc, "dc_buffer_free"));
	LDCDEVSETEVENTS *device_set_events = reinterpret_cast<LDCDEVSETEVENTS*>(getDLLFunction(libdc, "device_set_events"));
	LCDDCBUFFERAPPEND *dc_buffer_append = reinterpret_cast<LCDDCBUFFERAPPEND*>(getDLLFunction(libdc, "dc_buffer_append"));
	LCDDCBUFFERNEW *dc_buffer_new = reinterpret_cast<LCDDCBUFFERNEW*>(getDLLFunction(libdc, "dc_buffer_new"));
	LDCPARSERSAMPLESFOREACH *parser_samples_foreach = reinterpret_cast<LDCPARSERSAMPLESFOREACH*>(getDLLFunction(libdc, "parser_samples_foreach"));
	LDCPARSERGETDATETIME *parser_get_datetime = reinterpret_cast<LDCPARSERGETDATETIME*>(getDLLFunction(libdc, "parser_get_datetime"));
	LDCPARSERDESTROY *parser_destroy = reinterpret_cast<LDCPARSERDESTROY*>(getDLLFunction(libdc, "parser_destroy"));
	LDCPARSERSETDATA *parser_set_data = reinterpret_cast<LDCPARSERSETDATA*>(getDLLFunction(libdc, "parser_set_data"));
	LOGDEBUG("General pointers to LibDiveComputer fetched");
	
	// Open the device.
	LOGINFO("Opening the device (%s, %s).\n", lookup_name (backend), devname.c_str());
	device_t *device = NULL;


	LDCOPEN1* device_open1 = NULL;
	LDCOPEN2* device_open2 = NULL;


	switch (backend) {
	case DEVICE_TYPE_SUUNTO_SOLUTION:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "suunto_solution_device_open");
		//rc = suunto_solution_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_EON:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "suunto_eon_device_open");
		//rc = suunto_eon_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_VYPER:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "suunto_vyper_device_open");
		break;
	case DEVICE_TYPE_SUUNTO_VYPER2:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "suunto_vyper2_device_open");
		//rc = suunto_vyper2_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_SUUNTO_D9:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "suunto_d9_device_open");
		//rc = suunto_d9_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_ALADIN:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "uwatec_aladin_device_open");
		//rc = uwatec_aladin_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_MEMOMOUSE:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "uwatec_memomouse_device_open");
		//rc = uwatec_memomouse_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_UWATEC_SMART:
		device_open1 = (LDCOPEN1*)getDLLFunction(libdc, "uwatec_smart_device_open");
		//rc = uwatec_smart_device_open (&device);
		break;
	case DEVICE_TYPE_REEFNET_SENSUS:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "reefnet_sensus_device_open");
		//rc = reefnet_sensus_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_REEFNET_SENSUSPRO:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "reefnet_sensuspro_device_open");
		//rc = reefnet_sensuspro_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_REEFNET_SENSUSULTRA:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "reefnet_sensusultra_device_open");
		//rc = reefnet_sensusultra_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_VTPRO:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "oceanic_vtpro_device_open");
		//rc = oceanic_vtpro_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_VEO250:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "oceanic_veo250_device_open");
		//rc = oceanic_veo250_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_OCEANIC_ATOM2:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "oceanic_atom2_device_open");
		//rc = oceanic_atom2_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_MARES_NEMO:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "mares_nemo_device_open");
		//rc = mares_nemo_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_MARES_PUCK:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "mares_puck_device_open");
		//rc = mares_puck_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_HW_OSTC:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "hw_ostc_device_open");
		//rc = hw_ostc_device_open (&device, devname.c_str());
		break;
	case DEVICE_TYPE_CRESSI_EDY:
		device_open2 = (LDCOPEN2*)getDLLFunction(libdc, "cressi_edy_device_open");
		//rc = cressi_edy_device_open (&device, devname.c_str());
		break;
	default:
		LOGERROR("Type of device unrecognised (%d)", backend);
		throw DBException("Error in getching DLL pointers to function");
	}

	if (device_open2) rc = device_open2(&device, devname.c_str());
	else if (device_open1) rc = device_open1(&device);
	else {
		LOGERROR("Error in getching DLL pointers to function");
		throw DBException("Error in getching DLL pointers to function");
	}
		
	if (rc != DEVICE_STATUS_SUCCESS)
		throw DBException (std::string("Error opening device - Error code : ") + errmsg(rc));

	// todo Register the event handler.
	LOGINFO("Registering the event handler.\n");
	int events = DEVICE_EVENT_WAITING | DEVICE_EVENT_PROGRESS | DEVICE_EVENT_DEVINFO | DEVICE_EVENT_CLOCK;
	event_data_t evdata;
	evdata.devdata = &devdata;
	evdata.status = &status;
	rc = device_set_events (device, events, event_cb, &evdata);
	if (rc != DEVICE_STATUS_SUCCESS) {
		LOGINFO("Error registering the event handler.");
		device_close (device);
		throw DBException ("Error setting the event handler ");
	}

	// todo Register the cancellation handler.
	/*LOGINFO("Registering the cancellation handler.\n");
	rc = device_set_cancel (device, cancel_cb, NULL);
	if (rc != DEVICE_STATUS_SUCCESS) {
		LOGINFO("Error registering the cancellation handler.");
		device_close (device);
		return rc;
	}
	*/

	// todo Register the fingerprint data.
	/*if (fingerprint) {
		LOGINFO("Registering the fingerprint data.\n");
		rc = device_set_fingerprint (device, dc_buffer_get_data (fingerprint), dc_buffer_get_size (fingerprint));
		if (rc != DEVICE_STATUS_SUCCESS) {
			LOGINFO("Error registering the fingerprint data.");
			device_close (device);
			return rc;
		}
	}
	*/

	//Dump the data
	// Allocate a memory buffer.
	/* works but is too slow !!
	dc_buffer_t *buffer = dc_buffer_new (0);

	LOGINFO("Dumping the memory from device");
	rc = device_dump (device, buffer);
	if (rc != DEVICE_STATUS_SUCCESS) {
		LOGINFO("WARNING - Error downloading the memory dump.");
		dc_buffer_free (buffer);
	}
	else {
		LOGINFO("Adding data to Logger::binary");
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
		LOGINFO("Downloading the dives.\n");

		rc = device_foreach (device, dive_cb, &divedata);
		if (rc != DEVICE_STATUS_SUCCESS) {
			LOGINFO("Error downloading the dives.");
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
		LOGINFO("Closing the device.\n");
		rc = device_close (device);
		if (rc != DEVICE_STATUS_SUCCESS)
			LOGINFO("WARNING - Error closing the device. %s", errmsg(rc));
}




















int ComputerLibdc::_get_all_dives(std::string &diveXML)
{
	//Get the various function pointers we require from setupapi.dll
	//HINSTANCE libdc = LoadLibrary(_T("libdivecomputer-0.dll"));

	//todo handle error
	//if (hSetupAPI == NULL)
	//  return;

	//LDCOPEN* ldcOpen = reinterpret_cast<LDCOPEN*>(GetProcAddress(libdc, "suunto_vyper2_device_open"));

	LOGINFO(str(boost::format("Using backend on %1%") % devname));
	try
	{
		status.state = COMPUTER_RUNNING;
		dowork (backend, devname, diveXML, NULL, status);
	}
	catch (...) {
		status.state = COMPUTER_FINISHED;
		throw;
	}

	status.state = COMPUTER_FINISHED;
	return(0);
} 





ComputerLibdc::ComputerLibdc(std::string type, std::string file)
{
	unsigned int nbackends = sizeof (g_backends) / sizeof (g_backends[0]);

	status.state = COMPUTER_NOT_STARTED;

	for (unsigned int i = 0; i < nbackends; i++) {
		if (type.compare(g_backends[i].name) == 0)
			backend = g_backends[i].type;
	}

	LOGINFO(str(boost::format("Using type %1% on %2%") % type % file));
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
ComputerStatus ComputerLibdc::get_status()
{
	return(status);
}

