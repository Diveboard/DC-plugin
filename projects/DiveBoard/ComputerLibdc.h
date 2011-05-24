#pragma once
#include "Computer.h"
#include "../../libdivecomputer/src/deviceL.h"
#include "../../libdivecomputer/src/parser.h"
#include "../../libdivecomputer/src/buffer.h"

//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\buffer.h"
//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\deviceL.h"
//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\parser.h"


#ifdef WIN32
#define LIBTYPE HINSTANCE
#elif defined(__MACH__) || defined(__linux__)
#define __cdecl
#define LIBTYPE void*
#endif


//local types
typedef struct device_data_t {
	device_type_t backend;
	device_devinfo_t devinfo;
	device_clock_t clock;
} device_data_t;



//For DLL loading
typedef device_status_t (__cdecl LDCOPEN2)(device_t**, const char *);
typedef device_status_t (__cdecl LDCOPEN1)(device_t**);
typedef parser_status_t (__cdecl LDCPARSER)(parser_t**);
typedef parser_status_t (__cdecl LDCPARSERDESTROY)(parser_t *);
typedef parser_status_t (__cdecl LDCPARSERSETDATA)(parser_t *, const unsigned char *, unsigned int );
typedef parser_status_t (__cdecl LDCPARSERGETDATETIME)(parser_t *, dc_datetime_t *);
typedef parser_status_t (__cdecl LDCPARSERSAMPLESFOREACH)(parser_t *, sample_callback_t , void *);
typedef device_status_t (__cdecl LCDDEVFOREACH)(device_t *device, dive_callback_t callback, void *userdata);
typedef device_status_t (__cdecl LCDDEVCLOSE)(device_t *device);
typedef void (__cdecl LCDDCBUFFERFREE)(dc_buffer_t *buffer);
typedef dc_buffer_t * (__cdecl LCDDCBUFFERNEW)(size_t capacity);
typedef int (__cdecl LCDDCBUFFERAPPEND)(dc_buffer_t *buffer, const unsigned char data[], size_t size);
typedef device_status_t (__cdecl LDCDEVSETEVENTS)(device_t *, unsigned int, device_event_callback_t, void *);



class ComputerLibdc : public Computer
{
protected:
	std::string devname;
    device_type_t backend;
	ComputerStatus status;
	LIBTYPE libdc;
	void dowork (device_type_t &backend, const std::string &devname, std ::string &diveXML, dc_buffer_t *fingerprint, ComputerStatus &status);
	parser_status_t doparse (std::string *out, device_data_t *devdata, const unsigned char data[], unsigned int size);
public:
	int dive_cb (const unsigned char *data, unsigned int size, const unsigned char *fingerprint, unsigned int fsize, void *userdata);
	ComputerLibdc(std::string type, std::string filename);
	virtual ~ComputerLibdc(void);
	ComputerModel _get_model();
	int _get_all_dives(std::string &xml);
	virtual ComputerStatus get_status();
};



typedef struct dive_data_t {
	device_data_t *devdata;
	//FILE* fp;
	std::string *out;
	unsigned int number;
	dc_buffer_t *fingerprint;
	ComputerStatus *status;
	ComputerLibdc *computer;
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

