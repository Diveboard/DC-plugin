// COMputer.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "DeviceSuuntoEmu.h"
#include "DeviceNemoEmu.h"

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned int choice = 1;
	ComDevice *com;

	if (argc <= 1)
	{
		printf("Which computer would you like to emulate ?\n");
		printf("  1. Suunto Vyper\n");
		printf("  2. Mares Nemo Excel\n");
	}
	else {
		choice = _wtoi(argv[1]);
	}

	switch (choice)
	{
	case 1:
		com = new DeviceSuuntoEmu ("\\\\.\\CNCA0");
		break;
	case 2:
		com = new DeviceNemoEmu ("\\\\.\\CNCA0");
		break;
	default:
		printf("Choice unrecognised");
		return(-1);
	}

	com->open();
	com->run();

	free(com);
	return 0;
}



