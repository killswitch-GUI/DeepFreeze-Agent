#include "AgentPrivs.h"
#include "stdafx.h"
#include <Windows.h>
#include <ntsecapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <iostream> 


// https://github.com/iceboy-sjtu/ntdrvldr/blob/master/main.c
bool AgentPrivs::EnablePrivilege(
	LPCWSTR lpPrivilegeName
	)
{
	TOKEN_PRIVILEGES Privilege;
	HANDLE hToken;
	DWORD dwErrorCode;

	Privilege.PrivilegeCount = 1;
	Privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValueW(NULL, lpPrivilegeName,
		&Privilege.Privileges[0].Luid))
		return GetLastError();

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES, &hToken))
		return GetLastError();

	if (!AdjustTokenPrivileges(hToken, FALSE, &Privilege, sizeof(Privilege),
		NULL, NULL)) {
		dwErrorCode = GetLastError();
		CloseHandle(hToken);
		return dwErrorCode;
	}

	CloseHandle(hToken);
	return TRUE;
}