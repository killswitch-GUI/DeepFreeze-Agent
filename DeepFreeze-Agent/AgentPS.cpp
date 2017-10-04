#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <comdef.h>
#include "AgentPS.h"
#include "AgentHelpers.h"
#include "json.h"
#include <sstream>
#include <TlHelp32.h>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#define MAX_NAME 256

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;

// Get current pid of agent
int AgentPS::agentPid()
{
	int pid;
	pid = GetCurrentProcessId();
	return pid;
}
// Get the list of process identifiers.
json AgentPS::AgentPSEnumID()
{	
	json processObjects;
	memoryLog(0, "AgentPSEnumID() started TaskListing");
	std::string finalResult = "";
	//BOOL WINAPI EnumProcesses(
	//	  _Out_ DWORD *pProcessIds,
	//	  _In_  DWORD cb,
	//	  _Out_ DWORD *pBytesReturned);
	// Declare all at one time (Cool)
	DWORD pProcessIds[1024], cbNeeded, cProcesses;
	unsigned int i;
	// aProcesses is the rturned array, aProcesses will be the bytes needed. and cb will be the size of the array
	if (!EnumProcesses(pProcessIds, sizeof(pProcessIds), &cbNeeded))
	{
		memoryLog(3, "AgentPSEnumID() Failed to EnumProcesses IDs");
		return FALSE;
	}
	// Calculate how many process identifiers were returned.
	if (pProcessIds)
	{
		memoryLog(0, "AgentPSEnumID() Process IDs returned starting enumeration");
		// Now get system arch 
		bool isSystem64bit;
		AgentHelpers help;
		isSystem64bit = help.isWow64bit();
		cProcesses = cbNeeded / sizeof(DWORD);
		for (i = 0; i < cProcesses; i++)
		{
			if (pProcessIds[i] != 0)
			{
				//now we need to get the name of the process
				try {
					std::string time = help.timeDateLocal();
					std::string s = SSTR(i);
					json temp = AgentPS::PSNameAndID(pProcessIds[i], isSystem64bit);
					processObjects[s] = temp;
				}
				catch (...) {
					finalResult.append("unkown....");
					std::cout << "Fail" << std::endl;
				}
			}
		}
	}
	return processObjects;

}

json AgentPS::PSNameAndID(DWORD processID, BOOL isArch64bit)
{
	json processObject;
	std::wstring finalResult;
	std::wstring finalBuild;
	std::wstring procArch;
	std::wstring ProcessPath;
	std::wstring ProcessName;
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	LPWSTR myTrusteeName = _T(""), myDomainName = _T("");
	myTrusteeName = (LPWSTR)GlobalAlloc(GMEM_FIXED, 255 * sizeof(wchar_t));
	myDomainName = (LPWSTR)GlobalAlloc(GMEM_FIXED, 255 * sizeof(wchar_t));

	// Get a handle to the process.
	HANDLE hProcess;

	try {
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, processID);
		memoryLog(0, "PSNameAndID() OpenProcess has handle on proc");
	}
	catch (...) {
		if (hProcess)
		{
			memoryLog(3, "PSNameAndID() OpenProcess Failed to get handle on process ID");
			// Make sure we clean up in a crash
			CloseHandle(hProcess);
		}
	}
	// detect if the proc is x64 or x86
	BOOL is64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(hProcess, &is64Process))
		{
			//handle error
			memoryLog(3, "PSNameAndID() IsWow64Process Failed to get handle on process for system arch");
			// default to x86
		}
	}
	if (isArch64bit)
	{
		if (!is64Process)
		{
			procArch = L"64bit";
		}
		else
		{
			procArch = L"32bit";
		}
		// If process is true and 64 is true proc is 64bit
	}
	if (!isArch64bit)
	{
		if (!is64Process)
		{
			procArch = L"32bit";
		}
	}
	// Get the process name.

	if (hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		wchar_t lpExeName[MAX_PATH];
		DWORD charsCarried = MAX_PATH;
		try {
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),&cbNeeded))
			{
				memoryLog(0, "PSNameAndID() EnumProcessModules handle on modules");
				GetModuleBaseName(hProcess, hMod, szProcessName,
					sizeof(szProcessName) / sizeof(TCHAR));
				ProcessName = szProcessName;
				memoryLog(0, "PSNameAndID() GetModuleBaseName obtained the processName");
				if (QueryFullProcessImageName(hProcess, 0, lpExeName, &charsCarried)) {
					//std::wcout << lpExeName << std::endl;
					ProcessPath = lpExeName;
					memoryLog(0, "PSNameAndID() QueryFullProcessImageName obtained the ProcessPath");
				}
				else {
					memoryLog(0, "PSNameAndID() QueryFullProcessImageName failed to obtain the ProcessPath");
					ProcessPath = L"<unkown>";
				}
	
				HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, processID);
				if (hSnapshot)
				{
					memoryLog(0, "PSNameAndID() CreateToolhelp32Snapshot obtained a handle");
					PROCESSENTRY32W peInfo;
					peInfo.dwSize = sizeof(peInfo); // this line is REQUIRED
					BOOL nextProcess = Process32First(hSnapshot, &peInfo);
					bool found = false;
					while (nextProcess)
					{
						if (peInfo.th32ProcessID == processID)
						{
							found = true;
							break;
						}
						nextProcess = Process32Next(hSnapshot, &peInfo);
					}
					if (found)
					{
						memoryLog(0, "PSNameAndID() CreateToolhelp32Snapshot process structure found");
						//std::wcout << peInfo.szExeFile << std::endl;
						ProcessName = peInfo.szExeFile;
					}
					memoryLog(0, "PSNameAndID() CreateToolhelp32Snapshot Closed Handle");
					CloseHandle(hSnapshot);
				}
				else {
					memoryLog(3, "PSNameAndID() CreateToolhelp32Snapshot Failed to obtain handle");
				}

			}
		}
		catch (...) {
			if (hProcess) 
			{
				memoryLog(3, "PSNameAndID() EnumProcessModules Failed to get base name");
				// Make sure we clean up in a crash
				CloseHandle(hProcess);
			}
		}
	}

	// Now enumerate the user token to get the Owner of the precess

	if (NULL != hProcess)
	{
		DWORD dwSize = MAX_NAME;  
		HANDLE h_Token;
		DWORD dwLength = 0;
		PTOKEN_USER ptu = NULL;
		try {
			if (OpenProcessToken(hProcess, TOKEN_READ, &h_Token))
			{
				memoryLog(0, "PSNameAndID() OpenProcessToken handle obtained");
				// Now use the token to enumerate the Process
				PTOKEN_USER ptu = NULL;
				// First Gather the Token Byts required bypassing a 0 to the func
				if (!GetTokenInformation(h_Token, TokenUser, (LPVOID)ptu, 0, &dwLength))
				{
					memoryLog(0, "PSNameAndID() GetTokenInformation size set for token enum");
					if (ptu != NULL)
						HeapFree(GetProcessHeap(), 0, (LPVOID)ptu);
					// Re-Alicate the space for the known Byte size
					ptu = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
						HEAP_ZERO_MEMORY, dwLength);
					memoryLog(0, "PSNameAndID() GetTokenInformation ptu size zeroed");
				}
				//Re-try the function with required and alloc space
				if (GetTokenInformation(h_Token, TokenUser, (LPVOID)ptu, dwLength, &dwLength))
				{
					memoryLog(0, "PSNameAndID() GetTokenInformation enum with proper size");
					//Clean Up
					if (ptu != NULL)
					{
						HeapFree(GetProcessHeap(), 0, (LPVOID)ptu);
						memoryLog(0, "PSNameAndID() GetTokenInformation ptu heap freed");
					}
				}
				//Now use the Token to enumerate the owner of the proc
				SID_NAME_USE SidType;

				if (ptu)
				{
					memoryLog(0, "PSNameAndID() GetTokenInformation ptu found starting SID lookup");
					if (LookupAccountSid(NULL, ptu->User.Sid, myTrusteeName, &dwSize, myDomainName, &dwSize, &SidType))
					{
						memoryLog(0, "PSNameAndID() LookupAccountSid returned OK");
						DWORD dwResult = GetLastError();
						if (dwResult == ERROR_NONE_MAPPED)
						{
							memoryLog(0, "PSNameAndID() LookupAccountSid failed to look up sid");
							myTrusteeName = L"NONE_MAPPED";
							myDomainName = L"NONE_MAPPED";
						}
						//_tprintf(TEXT("%s  (PID: %u) (Domain: %s) (User: %s) \n"), szProcessName, processID, myDomainName, myTrusteeName);
						//td::wcout << szProcessName << " " << processID << " " << myDomainName << " " << myTrusteeName << " " << procArch << std::endl;
						std::wstring wstr = std::to_wstring(processID);
						std::wstring DomainName = myDomainName;
						std::wstring TrusteeName = myTrusteeName;
						std::wstring ProcessArch = procArch;
						std::wstring ProcessPid = wstr;
						processObject["ProcessName"] = ProcessName;
						processObject["ProcessPath"] = ProcessPath;
						processObject["DomainName"] = DomainName;
						processObject["TrusteeName"] = TrusteeName;
						processObject["ProcessArch"] = ProcessArch;
						processObject["ProcessPid"] = ProcessPid;
					}
					else {
						std::cout << GetLastError() << std::endl;
					}
				}

			}
		}
		catch (...) {
			CloseHandle(h_Token);
			CloseHandle(hProcess);
		}
		// release token handle
		CloseHandle(h_Token);

	}

	// Print the process name and identifier.

	// Release the handle to the process.
	CloseHandle(hProcess);
	return processObject;
}
