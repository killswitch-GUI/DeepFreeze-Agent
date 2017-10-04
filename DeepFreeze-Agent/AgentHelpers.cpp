#define _CRT_SECURE_NO_DEPRECATE

#include "AgentHelpers.h"
#include "base64.h"
#include <windows.h>
#include <VersionHelpers.h>
#include "stdafx.h"
#include <ntsecapi.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <list>
#include <string>

//Get UID includes
#define SECURITY_WIN32
#include <Security.h>
#pragma comment(lib, "Secur32.lib")

#define _WIN32_WINNT 0x0501
// TODO: build GetLocalIp()
std::wstring AgentHelpers::GetDisplayName()
{
	/*
	Retrieves the name of user associated with the current thread.
	POSSIBLE VALUES FOR NameFormat:
	NameUnknown           = 0,
	NameFullyQualifiedDN  = 1,
	NameSamCompatible     = 2,
	NameDisplay           = 3,
	NameUniqueId          = 6,
	NameCanonical         = 7,
	NameUserPrincipal     = 8,
	NameCanonicalEx       = 9,
	NameServicePrincipal  = 10,
	NameDnsDomain         = 12
	EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT
	*/
	std::wstring returnStr;
	const DWORD buffSize = 1024; //Size of buffer 
	DWORD lpnSize = buffSize; // Initialize buffer of size buffSize
	TCHAR lpNameBuffer[buffSize + 1]; //  Add one to account for null termination char
	EXTENDED_NAME_FORMAT NameFormat = NameDisplay; // Provides UID | See comments for list of choices

	GetUserNameEx(
		//The format of the name.  This is the value from the EXTENDED_NAME_FORMAT enum type
		NameFormat,
		//The ptr to buffer that recieves the name in specified format
		lpNameBuffer,
		//On inut this variable specifies the size of lpNameBuffer in TCHARs. on Success,
		//variable recieves number of TCHARs copied to buffer
		&lpnSize);
	returnStr.append(lpNameBuffer);
	std::wcout << "UID: " << returnStr << std::endl; // Debug STMT.. Comment out 
	return returnStr;
}

std::wstring AgentHelpers::GetSamName()
{
	/*
	Retrieves the name of user associated with the current thread.
	POSSIBLE VALUES FOR NameFormat:
	NameUnknown           = 0,
	NameFullyQualifiedDN  = 1,
	NameSamCompatible     = 2,
	NameDisplay           = 3,
	NameUniqueId          = 6,
	NameCanonical         = 7,
	NameUserPrincipal     = 8,
	NameCanonicalEx       = 9,
	NameServicePrincipal  = 10,
	NameDnsDomain         = 12
	EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT
	*/
	std::wstring returnStr;
	const DWORD buffSize = 1024; //Size of buffer 
	DWORD lpnSize = buffSize; // Initialize buffer of size buffSize
	TCHAR lpNameBuffer[buffSize + 1]; //  Add one to account for null termination char
	EXTENDED_NAME_FORMAT NameFormat = NameSamCompatible; // Provides UID | See comments for list of choices

	GetUserNameEx(
		//The format of the name.  This is the value from the EXTENDED_NAME_FORMAT enum type
		NameFormat,
		//The ptr to buffer that recieves the name in specified format
		lpNameBuffer,
		//On inut this variable specifies the size of lpNameBuffer in TCHARs. on Success,
		//variable recieves number of TCHARs copied to buffer
		&lpnSize);
	returnStr.append(lpNameBuffer);
	std::wcout << "UID: " << returnStr << std::endl; // Debug STMT.. Comment out 
	return returnStr;
}

std::wstring AgentHelpers::GetUID()
{
	/*
	Retrieves the name of user associated with the current thread.
	POSSIBLE VALUES FOR NameFormat:
	NameUnknown           = 0,
	NameFullyQualifiedDN  = 1,
	NameSamCompatible     = 2,
	NameDisplay           = 3,
	NameUniqueId          = 6,
	NameCanonical         = 7,
	NameUserPrincipal     = 8,
	NameCanonicalEx       = 9,
	NameServicePrincipal  = 10,
	NameDnsDomain         = 12
	EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT
	*/
	std::wstring returnStr;
	const DWORD buffSize = 1024; //Size of buffer 
	DWORD lpnSize = buffSize; // Initialize buffer of size buffSize
	TCHAR lpNameBuffer[buffSize + 1]; //  Add one to account for null termination char
	EXTENDED_NAME_FORMAT NameFormat = NameUniqueId; // Provides UID | See comments for list of choices

	GetUserNameEx(
		//The format of the name.  This is the value from the EXTENDED_NAME_FORMAT enum type
		NameFormat,
		//The ptr to buffer that recieves the name in specified format
		lpNameBuffer,
		//On inut this variable specifies the size of lpNameBuffer in TCHARs. on Success,
		//variable recieves number of TCHARs copied to buffer
		&lpnSize);
	returnStr.append(lpNameBuffer);
	std::wcout << "UID: " << returnStr << std::endl; // Debug STMT.. Comment out 
	return returnStr;
}

std::string AgentHelpers::ConvertBSTRToMBS(BSTR bstr)
{
	int wslen = ::SysStringLen(bstr);
	return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

std::string AgentHelpers::ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
	int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

	std::string dblstr(len, '\0');
	len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
		pstr, wslen /* not necessary NULL-terminated */,
		&dblstr[0], len,
		NULL, NULL /* no default char */);

	return dblstr;
}

BSTR AgentHelpers::ConvertMBSToBSTR(const std::string& str)
{
	int wslen = ::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
		str.data(), str.length(),
		NULL, 0);

	BSTR wsdata = ::SysAllocStringLen(NULL, wslen);
	::MultiByteToWideChar(CP_ACP, 0 /* no flags */,
		str.data(), str.length(),
		wsdata, wslen);
	return wsdata;
}


// encode std::string with base64
std::string AgentHelpers::base64StringEnc(const std::string &input)
{
	static std::string encoded;

	Base64::Encode(input, &encoded);
	std::cout << encoded << std::endl;
	return encoded;
}


// decode std::string with base64
std::string AgentHelpers::base64StringDec(const std::string &input)
{
	static std::string decoded;

	Base64::Encode(input, &decoded);
	std::cout << decoded << std::endl;
	return decoded;
}


//Converts a wstring to char
void AgentHelpers::ConvToChar(_In_ std::wstring input, _Out_ char * outputString)
{
	size_t outputSize = input.length() + 1; // +1 for null terminator
	outputString = new char[outputSize];
	size_t charsConverted = 0;
	const wchar_t * inputW = input.c_str();
	wcstombs_s(&charsConverted, outputString, outputSize, inputW, input.length());
}

std::string AgentHelpers::timeNow()
{
	const int MAX_LEN = 200;
	char buffer[MAX_LEN];
	if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
		"HH':'mm':'ss", buffer, MAX_LEN) == 0)
		return "Error in NowTime()";

	char result[100] = { 0 };
	static DWORD first = GetTickCount();
	// TODO: fix this issue with _CRT_SECURE_NO_WARNINGS
	std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
	return result;
}

std::string AgentHelpers::timeDateLocal()
{
	std::string s;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	auto str = oss.str();
	return str;
}

std::string AgentHelpers::dateLocal()
{
	std::string s;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%d");
	auto str = oss.str();
	return str;
}

bool AgentHelpers::isWow64bit()
{
	SYSTEM_INFO lpSystemInfo;
	GetNativeSystemInfo(&lpSystemInfo);
	std::cout << lpSystemInfo.wProcessorArchitecture << std::endl;
	if (lpSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
	{
		//memoryLog(0, "isWow64bit() AMD64 detected");
		return TRUE;
	}
	if (lpSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		//memoryLog(2, "isWow64bit() IA-64 detected");
		return TRUE;
	}
	if (lpSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		//memoryLog(0, "isWow64bit() x86 detected");
		return FALSE;
	}
	if (lpSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_UNKNOWN)
	{
		// WARNING: defualt to x86
		//memoryLog(2, "isWow64bit() UNKOWN arch detected");
		return FALSE;
	}

}


bool AgentHelpers::is64bit()
{
	// get system architecture
	bool is64;
	if (sizeof(void*) == 4)
		is64 = false; // 32bit
	else
		is64 = true; // 64bit
	return is64;
}

int AgentHelpers::CheckOSVersion(void)
{
	/*
	* Windows XP = 1 (NT 5.0)
	* Windows Vista = 2 (NT 6.0)
	* Windows 7 = 3 (NT 6.1)
	* Windows 8 or 8.1 = 4
	* Windows 10 = 5
	* Windows Server (any) = 76
	*/
	if (IsWindowsServer())
	{
		return  6;
	}
	//if (IsWindows10OrGreater())
	//{
	//return 5;
	//}
	if (IsWindows8OrGreater())
	{
		return 4;
	}
	if (IsWindows7OrGreater)
	{
		return 3;
	}
	if (IsWindowsVistaOrGreater())
	{
		return 2;
	}
	if (IsWindowsXPOrGreater())
	{
		return 1;
	}
	else
		return 0;
}

std::string AgentHelpers::s_to_caesar(std::string s1, int k)

//****************************************************************************80
//
//  Purpose:
//
//    S_TO_CAESAR applies a Caesar shift cipher to a string.
//
//  Discussion:
//
//    The Caesar shift cipher incremented each letter by 1, with Z going to A.
//
//    This function can apply a Caesar shift cipher to a string of characters,
//    using an arbitrary shift K, which can be positive, negative or zero.
//
//    Letters A through Z will be shifted by K, mod 26.
//    Letters a through z will be shifted by K, mod 26.
//    Non-alphabetic characters are unchanged.
//
//    s2 = s_to_caesar ( s1, 1 ) will apply the traditional Caesar shift cipher.
//    s3 = s_to_caesar ( s2, -1 ) will decipher the result.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    31 January 2016
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S1, a string of characters.
//
//    Input, int K, the increment.
//
//    Output, string S2, the string of enciphered characters.
//
{
	int i;
	int s1_length;
	std::string s2;

	s1_length = s1.length();

	s2 = s1;

	for (i = 0; i < s1_length; i++)
	{
		if ('A' <= s1[i] && s1[i] <= 'A' + 25)
		{
			s2[i] = i4_modp(s1[i] + k - 'A', 26) + 'A';
		}
		else if ('a' <= s1[i] && s1[i] <= 'a' + 25)
		{
			s2[i] = i4_modp(s1[i] + k - 'a', 26) + 'a';
		}
	}

	return s2;
}

int AgentHelpers::i4_modp(int i, int j)

//****************************************************************************80
//
//  Purpose:
//
//    I4_MODP returns the nonnegative remainder of I4 division.
//
//  Discussion:
//
//    If
//      NREM = I4_MODP ( I, J )
//      NMULT = ( I - NREM ) / J
//    then
//      I = J * NMULT + NREM
//    where NREM is always nonnegative.
//
//    The MOD function computes a result with the same sign as the
//    quantity being divided.  Thus, suppose you had an angle A,
//    and you wanted to ensure that it was between 0 and 360.
//    Then mod(A,360) would do, if A was positive, but if A
//    was negative, your result would be between -360 and 0.
//
//    On the other hand, I4_MODP(A,360) is between 0 and 360, always.
//
//        I         J     MOD  I4_MODP   I4_MODP Factorization
//
//      107        50       7       7    107 =  2 *  50 + 7
//      107       -50       7       7    107 = -2 * -50 + 7
//     -107        50      -7      43   -107 = -3 *  50 + 43
//     -107       -50      -7      43   -107 =  3 * -50 + 43
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    26 May 1999
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I, the number to be divided.
//
//    Input, int J, the number that divides I.
//
//    Output, int I4_MODP, the nonnegative remainder when I is
//    divided by J.
//
{
	int value;

	if (j == 0)
	{
		j == 10;
	}

	value = i % j;

	if (value < 0)
	{
		value = value + abs(j);
	}

	return value;
}


