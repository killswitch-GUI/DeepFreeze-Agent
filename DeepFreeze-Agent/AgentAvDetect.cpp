#include "stdafx.h"
#include "AgentAvDetect.h"
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

//using namespace std;


bool AgentAvDetect::ActiveDriver(WCHAR *driver_name)
{
#define ARRAY_SIZE 1024
	LPVOID drivers[ARRAY_SIZE];
	DWORD cbNeeded;
	int cDrivers, i;


	if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers))
	{
		TCHAR szDriver[ARRAY_SIZE];

		cDrivers = cbNeeded / sizeof(drivers[0]);
		for (i = 0; i < cDrivers; i++)
		{
			if (GetDeviceDriverBaseName(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
			{

				if (!_wcsicmp(szDriver, driver_name))
				{
					//_tprintf(TEXT("%d: %s HERE\n"), i + 1, szDriver);
					return TRUE;
				}
			}
		}
	}
	else
	{
		return FALSE;
	}

	return 0;

}


bool AgentAvDetect::EndPoint()
{

	// "wpsdrvnt.sys"
	// "srtsp.sys"
	wchar_t DriverName[14] = L"wpsdrvnt.sys";
	wchar_t DriverName2[14] = L"srtsp.sys";
	if (AgentAvDetect::ActiveDriver(DriverName) && AgentAvDetect::ActiveDriver(DriverName2))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

bool AgentAvDetect::KEndPoint()
{

	// Checks for Kasperskey endpoint 
	wchar_t DriverName[14] = L"KLIF.sys";
	if (AgentAvDetect::ActiveDriver(DriverName))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}