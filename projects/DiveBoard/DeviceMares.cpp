/*
 *  DeviceMares.cpp
 *  FireBreath
 *
 *  Created by Pascal Manchon on 27/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "DeviceMares.h"

#include "Logger.h"

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
/* for strptime prototype - see man strptime */
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

#ifdef __MACH__
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <stdio.h>
#endif


DeviceMares::DeviceMares(std::string name)
{
	filename = name;
	hCom = NULL;
	open();
}

DeviceMares::~DeviceMares()
{
	close();
}



int DeviceMares::open() 
{
#ifdef _WIN32
	//struct termios settings;
	bool fSuccess;
	DCB dcb;
	
	LOGINFO("Opening %s", filename);
	
	//todo fix unicode
	hCom = CreateFile((LPCWSTR)filename.c_str(), 
					  GENERIC_READ | GENERIC_WRITE, 
					  0, 
					  NULL, 
					  OPEN_EXISTING, 
					  0, 
					  NULL 
					  ); 
	
	if (hCom == INVALID_HANDLE_VALUE) 
	{
		//  Handle the error.
		LOGINFO ("CreateFile failed with error %d.", GetLastError());
		hCom = NULL;
		return (SUUNTO_ERR_CREATEFILE);
	}
	
	//  Initialize the DCB structure.
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	//  Build on the current configuration by first retrieving all current
	//  settings.
	fSuccess = GetCommState(hCom, &dcb);
	
   	dcb.BaudRate = CBR_2400;
	dcb.ByteSize = 8;
	dcb.Parity = ODDPARITY;
	dcb.StopBits = ONESTOPBIT;
	
	dcb.fBinary=1; 
	dcb.fParity=0; 
	dcb.fOutxCtsFlow=false; 
	dcb.fOutxDsrFlow=false; 
	dcb.fDtrControl=DTR_CONTROL_DISABLE; 
	dcb.fDsrSensitivity=0; 
	dcb.fTXContinueOnXoff=0; 
	dcb.fRtsControl=RTS_CONTROL_DISABLE; 
	
	fSuccess = SetCommState(hCom, &dcb);
	
	if (!fSuccess) 
	{
		//  Handle the error.
		LOGINFO("SetCommState failed with error %d.", GetLastError());
		hCom = NULL;
		return (SUUNTO_ERR_SETCOMMSTATE);
	}
	
	
	try {
		set_dtr(DTR_STATUS_ON);
		set_rts(RTS_STATUS_OFF);
		Sleep(100);
		return (0);
	} catch (...) {
		hCom = NULL;
		return (SUUNTO_ERR_SETSIG);
	}	
	
#elif __MACH__
	
	struct termios options;
	
	hCom = ::open(filename.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(hCom, F_SETFL, 0);
	
	/* get the current options */
	tcgetattr(hCom, &options);
	
	cfsetspeed(&options, B2400);
	
	/* set raw input, 1 second timeout */
	options.c_cflag = CS8|CREAD|PARENB|PARODD|CLOCAL;
	options.c_iflag = IGNBRK; /* Ignore parity checking */
	options.c_lflag=0;
	options.c_oflag=0;
	
	/* Setup blocking, return on 1 character */
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 0;
	
	/* Clear the line */
	tcflush(hCom,TCIFLUSH);
	
	if(tcsetattr(hCom,TCSANOW,&options)!=-1) {
		//todo : should be in a try/catch ?
		set_dtr(DTR_STATUS_ON);
		set_rts(RTS_STATUS_OFF);
		usleep(100000);
	}
	else {
		LOGINFO("Error while setting properties of Com port");
		return(-1);
	}
	
	unsigned char b;
	while(read_serial(&b,1,2)>0){}
	
#else	
#error Platform not supported
	
#endif
	return(0);
}



/*static bool suunto_send_testcmd(char *cmd) {
 
 int len;
 DWORD  out;
 bool rval=FALSE;
 
 len=strlen(cmd);
 
 rval = WriteFile(hCom, cmd, len, &out, NULL);
 FlushFileBuffers(hCom);
 
 return rval;
 }*/


int DeviceMares::read_serial(unsigned char * buff, unsigned int num, int timeoutmod) 
{
#ifdef _WIN32  
	DWORD  out;
	bool rval; 
	
	if (!hCom)
	{
		int ret = open();
		if (ret < 0) return(ret);
	}
	
	/*----------------------------------*/ 
	/*    DÈfinition des timeouts       */ 
	/*----------------------------------*/
	COMMTIMEOUTS tTimeout;
	tTimeout.ReadIntervalTimeout = MAXWORD; 
	tTimeout.ReadTotalTimeoutMultiplier = 0; 
	tTimeout.ReadTotalTimeoutConstant = 10000; // pas de time out = 0 
	tTimeout.WriteTotalTimeoutMultiplier = 0; 
	tTimeout.WriteTotalTimeoutConstant = 0; 
	
	// configurer le timeout 
	SetCommTimeouts(hCom,&tTimeout); 
	
	rval = ReadFile(hCom, buff, num, &out, NULL);
	if (rval && out > 0) return rval;
	
#elif __MACH__
	
	fd_set fds;
	struct timeval tv;
	int rval;
	
	FD_ZERO(&fds);
	FD_SET(hCom,&fds);
	tv.tv_sec = timeoutmod;
	tv.tv_usec = TIMEOUT;
	if(select(hCom+1,&fds,NULL,NULL,&tv)==1) {
		rval = read(hCom,buff,num);
		return rval;
	}
	
#else	
#error Platform not supported
#endif
	
	return SUUNTO_ERR_READ;
}

int DeviceMares::write_serial(unsigned char *buffer,int len) 
{
	int rc;
	
	set_rts(RTS_STATUS_ON);
	
#ifdef _WIN32
	DWORD  out;
	rc = WriteFile(hCom, buffer, len, &out, NULL);
	FlushFileBuffers(hCom);
	Sleep(200);
	
#elif __MACH__
	rc = write(hCom,buffer,len);
	tcdrain(hCom);
	usleep(200000);
	
#else	
#error Platform not supported
#endif
	
	set_rts(RTS_STATUS_OFF);
	return rc;
}
