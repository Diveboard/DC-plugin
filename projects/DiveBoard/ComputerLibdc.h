#pragma once
#include "computer.h"
#include "../../libdivecomputer/src/deviceL.h"
#include "../../libdivecomputer/src/parser.h"
#include "../../libdivecomputer/src/buffer.h"

//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\buffer.h"
//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\deviceL.h"
//#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\parser.h"


#ifdef WIN32


#elif __MACH__
#define __cdecl 

#endif

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
public:
	ComputerLibdc(std::string type, std::string filename);
	virtual ~ComputerLibdc(void);
	ComputerModel _get_model();
	int _get_all_dives(std::string &xml);
	virtual ComputerStatus get_status();
};

