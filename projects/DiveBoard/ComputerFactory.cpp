#include "stdafx.h"
#include "ComputerFactory.h"

#ifdef _WIN32
#include <tchar.h>
#include <setupapi.h>
#endif

#include "Logger.h"

#include "ComputerSuunto.h"
#include "ComputerMares.h"
#include "ComputerLibdc.h"

#include <stdio.h>
#include <iostream>
#include <errno.h>

#ifdef __MACH__
#include <dirent.h>
#endif

ComputerFactory::ComputerFactory(void)
{
}


ComputerFactory::~ComputerFactory(void)
{
}


bool IsNumeric(WCHAR *pszString, BOOL bIgnoreColon)
{
  size_t nLen = wcslen(pszString);
  if (nLen == 0)
    return FALSE;

  //What will be the return value from this function (assume the best)
  BOOL bNumeric = TRUE;

  for (size_t i=0; i<nLen && bNumeric; i++)
  {
    bNumeric = (isdigit(static_cast<int>(pszString[i])) != 0);
    if (bIgnoreColon && (pszString[i] == ':'))
      bNumeric = TRUE;
  }

  return bNumeric;
}


#ifdef _WIN32

typedef HKEY (__stdcall SETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
typedef BOOL (__stdcall SETUPDICLASSGUIDSFROMNAME)(LPCTSTR, LPGUID, DWORD, PDWORD);
typedef BOOL (__stdcall SETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
typedef BOOL (__stdcall SETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef HDEVINFO (__stdcall SETUPDIGETCLASSDEVS)(LPGUID, LPCTSTR, HWND, DWORD);
typedef BOOL (__stdcall SETUPDIGETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);

//This function gets a list of all COM ports
//////
//Copyright (c) 1998 - 2010 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)
//
//All rights reserved.
//
//Copyright / Usage Details:
//
//You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
//when your product is released in binary form. You are allowed to modify the source code in any way you want 
//except you cannot modify the copyright details at the top of each module. If you want to distribute source 
//code with your application, then you are only allowed to distribute versions released by the author. This is 
//to maintain a single distribution point for the source code. 
//
void UsingSetupAPI1(std::vector<std::string>& ports, std::vector<std::string>& friendlyNames)
{
  //Make sure we clear out any elements which may already be in the array(s)
	ports.empty();
  friendlyNames.empty();

  //Get the various function pointers we require from setupapi.dll
  HINSTANCE hSetupAPI = LoadLibrary(_T("SETUPAPI.DLL"));
  if (hSetupAPI == NULL)
  {
    // Retrieve the system error message for the last-error code
  	Logger::append("Failed to load SetupAPI");
  	throw DBException(str(boost::format("Error loading SetupAPI - Error code : %d") % GetLastError()));

  }

  SETUPDIOPENDEVREGKEY* lpfnLPSETUPDIOPENDEVREGKEY = reinterpret_cast<SETUPDIOPENDEVREGKEY*>(GetProcAddress(hSetupAPI, "SetupDiOpenDevRegKey"));
  SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = reinterpret_cast<SETUPDIGETCLASSDEVS*>(GetProcAddress(hSetupAPI, "SetupDiGetClassDevsA"));
  SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = reinterpret_cast<SETUPDIGETDEVICEREGISTRYPROPERTY*>(GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyA"));
  SETUPDIDESTROYDEVICEINFOLIST* lpfnSETUPDIDESTROYDEVICEINFOLIST = reinterpret_cast<SETUPDIDESTROYDEVICEINFOLIST*>(GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList"));
  SETUPDIENUMDEVICEINFO* lpfnSETUPDIENUMDEVICEINFO = reinterpret_cast<SETUPDIENUMDEVICEINFO*>(GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo"));

  if ((lpfnLPSETUPDIOPENDEVREGKEY == NULL) || (lpfnSETUPDIDESTROYDEVICEINFOLIST == NULL) ||
      (lpfnSETUPDIENUMDEVICEINFO == NULL) || (lpfnSETUPDIGETCLASSDEVS == NULL) || (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY == NULL))
  {
	try {
		//Unload the setup dll
		FreeLibrary(hSetupAPI);
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	} catch (...) {
		Logger::append("Warning : Error unloading SetupAPI.dll");
	}

	throw DBException("Error getting pointers from SetupAPI");
  }
  
  //Now create a "device information set" which is required to enumerate all the ports
  GUID guid = GUID_DEVINTERFACE_COMPORT;
  HDEVINFO hDevInfoSet = lpfnSETUPDIGETCLASSDEVS(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfoSet == INVALID_HANDLE_VALUE)
  {
	  DWORD dwLastError;
	  try {
		dwLastError = GetLastError();
		//Unload the setup dll
		FreeLibrary(hSetupAPI);
		SetLastError(dwLastError);
	  } catch (...) {
		Logger::append("Warning : Error unloading SetupAPI.dll");
	  }

	  throw DBException(str(boost::format("Error creating information set from SetupAPI - %d") % dwLastError ));
  }

  //Finally do the enumeration
  BOOL bMoreItems = TRUE;
  int nIndex = 0;
  SP_DEVINFO_DATA devInfo;
  while (bMoreItems)
  {
    //Enumerate the current device
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreItems = lpfnSETUPDIENUMDEVICEINFO(hDevInfoSet, nIndex, &devInfo);
    if (bMoreItems)
    {
      //Did we find a serial port for this device
      BOOL bAdded = FALSE;

      //Get the registry key which stores the ports settings
      HKEY hDeviceKey = lpfnLPSETUPDIOPENDEVREGKEY(hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
      if (hDeviceKey)
      {
        //Read in the name of the port
        WCHAR szPortName[256];
        szPortName[0] = _T('\0');
        DWORD dwSize = sizeof(szPortName);
        DWORD dwType = 0;
  	    if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, reinterpret_cast<LPBYTE>(szPortName), &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
        {
          //If it looks like "COMX" then
          //add it to the array which will be returned
          size_t nLen = _tcslen(szPortName);
          if (nLen > 3)
          {
            if ((_tcsnicmp(szPortName, _T("COM"), 3) == 0) && IsNumeric(&(szPortName[3]), FALSE))
            {
              //Work out the port number
              int nPort = _ttoi(&(szPortName[3]));
			  ports.push_back(str(boost::format("\\\\.\\COM%1%") % nPort));
              bAdded = TRUE;
            }
          }
        }

        //Close the key now that we are finished with it
        RegCloseKey(hDeviceKey);
      }

      //If the port was a serial port, then also try to get its friendly name
      if (bAdded)
      {
        BYTE szFriendlyName[256];
        szFriendlyName[0] = _T('\0');
        DWORD dwSize = sizeof(szFriendlyName);
        DWORD dwType = 0;
        if (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, szFriendlyName, dwSize, &dwSize) && (dwType == REG_SZ))
        {
			//todo fix unicode support....
          friendlyNames.push_back(std::string((char *)szFriendlyName));
        }
        else
        {
			//todo fix unicode support....
          friendlyNames.push_back("");
        }
      }
    }

    ++nIndex;
  }

  //Free up the "device information set" now that we are finished with it
  lpfnSETUPDIDESTROYDEVICEINFOLIST(hDevInfoSet);

  //Unload the setup dll
  FreeLibrary(hSetupAPI);

  for (unsigned int i=0; i< ports.size();i++)
	  Logger::append(str(boost::format("COM port found : %s - %s") % ports[i] % friendlyNames[i]));
}

void ComputerFactory::listPorts(std::string &a)
{
	std::vector<std::string> ports;
	std::vector<std::string> friendlyNames;
	unsigned int i;

	a.empty();
	UsingSetupAPI1(ports, friendlyNames);

	for (i=0; i< ports.size();i++)
		a += str(boost::format("%d : %s\n") % ports[i] % friendlyNames[i]);
}


#endif

#ifdef __MACH__

void ListTTY(std::vector<std::string>& files, std::vector<std::string>& friendlyNames)
{
    int r;
	DIR *dp;
    struct dirent *dirp;
    
	friendlyNames.empty();
	files.empty();
	
	if((dp  = opendir("/dev")) == NULL)
		throw DBException(str(boost::format("Error (%1%) while opening /dev") % errno));
	
    while ((dirp = readdir(dp)) != NULL) {
		//todo : a NULL can also mean an error....
		Logger::append(str(boost::format("Filename : %1% %2%") % dirp->d_name % ((int)strncmp("tty.usbserial-", dirp->d_name, 14))));
        if (!strncmp("tty.", dirp->d_name, 4)) {
			files.push_back(str(boost::format("/dev/%1%") % dirp->d_name));
			friendlyNames.push_back(std::string(dirp->d_name));
		}
    }
    
	r = closedir(dp);
	if (r) Logger::append("Warning - Error while closing dir in ListTTY");
}

#endif


//This function decides which driver to test on which COM port
bool ComputerFactory::mapDevice(std::string identifier, std::string &found)
{

#ifdef _WIN32
	if (!identifier.compare("Suunto USB Serial Port")) {
		found = "Suunto";
		return true;
	} 
#elif __MACH__
	if (!identifier.compare("tty.usbserial-PtTFP8W4")) {
		return(new ComputerSuunto(fileName));
	}
	else if (!identifier.compare("tty.SLAB_USBtoUART")) {
		return(new ComputerMares(fileName));
	}
								 
#else
#error Platform not supported
#endif
	return false;
}




//This functions goes through all COM ports and checks if one is a known computer
std::map <std::string, std::string> ComputerFactory::detectConnectedDevice()
{
	std::map <std::string, std::string> ret;
	std::vector<std::string> fileNames;
	std::vector<std::string> friendlyNames;
	std::string driverName;
	unsigned int i;
	Computer *foundComputer;

	//1 list ports
#ifdef _WIN32
	Logger::append("Using SetupAPI");
	UsingSetupAPI1(fileNames, friendlyNames);
#elif __MACH__
	ListTTY(fileNames, friendlyNames);
#else
#error "Platform not supported"
#endif
	
	//2 filter interesting ones
	for (i=0; i< fileNames.size();i++) {
		Logger::append("Checking port %s - %s", fileNames[i].c_str(), friendlyNames[i].c_str());
		bool found = mapDevice(friendlyNames[i], driverName);
		if (found) {
			foundComputer = createComputer(driverName, fileNames[i]);
			if (foundComputer) {

				//We don't really care about the exact model... so if a driver is found let's just use it !
				ret[std::string("type")] = driverName;
				ret[std::string("filename")] = fileNames[i];
				return(ret);

				/*
				//3 test port with Computer Driver
				ComputerModel model = foundComputer->get_model();
				Logger::append("Device found on port %s has modeltype %d", fileNames[i].c_str(), model);

				//4 return identified device
				if (model != COMPUTER_MODEL_UNKNOWN){			
					ret[std::string("type")] = driverName;
					ret[std::string("filename")] = fileNames[i];
					return(ret);
				}
				*/
			}
		}
	}
	Logger::append("No interesting port found !");
	return(ret);
}


Computer *ComputerFactory::createComputer(const std::string &type, const std::string &filename)
{
	if (!type.compare("Suunto")){
		return new ComputerSuunto(filename);
	}
	else if (!type.compare("Emu Suunto")){
		return new ComputerSuunto("");
	}
	else if (!type.compare("Mares M2")){
		return new ComputerMares(filename);
	}
	else if (!type.compare("Emu Mares M2")){
		return new ComputerMares("");
	}
	else if (!type.compare(0,4,"LDC ")){
		return new ComputerLibdc(type, filename);
	}
	else throw DBException(std::string("ComputerFactory::createComputer : Computer requested unknown : ")+type);
}

