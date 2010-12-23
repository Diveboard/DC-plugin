#pragma once
#include "computer.h"
#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\buffer.h"
#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\deviceL.h"
#include "d:\DATA\My Documents\Personnel\DB_plugins\libdivecomputer\src\parser.h"



//For DLL loading
typedef device_status_t (__stdcall LDCOPEN)(device_t**, const char *);
typedef parser_status_t (__stdcall LDCPARSER)(parser_t**);
typedef  parser_status_t (__stdcall LDCPARSERDESTROY)(parser_t *);
typedef  parser_status_t (__stdcall LDCPARSERSETDATA)(parser_t *, const unsigned char *, unsigned int );
typedef  parser_status_t (__stdcall LDCPARSERGETDATETIME)(parser_t *, dc_datetime_t *);
typedef  parser_status_t (__stdcall LDCPARSERSAMPLESFOREACH)(parser_t *, sample_callback_t , void *);
typedef  device_status_t (__stdcall LCDDEVFOREACH)(device_t *device, dive_callback_t callback, void *userdata);
typedef  device_status_t (__stdcall LCDDEVCLOSE)(device_t *device);
typedef  void (__stdcall LCDDCBUFFERFREE)(dc_buffer_t *buffer);
typedef  dc_buffer_t * (__stdcall LCDDCBUFFERNEW)(size_t capacity);
typedef  int (__stdcall LCDDCBUFFERAPPEND)(dc_buffer_t *buffer, const unsigned char data[], size_t size);



class ComputerLibdc : public Computer
{
protected:
	std::string devname;
    device_type_t backend;
	ComputerStatus status;
public:
	ComputerLibdc(std::string type, std::string filename);
	~ComputerLibdc(void);
	ComputerModel _get_model();
	int _get_all_dives(std::string &xml);
	virtual ComputerStatus get_status();
};

