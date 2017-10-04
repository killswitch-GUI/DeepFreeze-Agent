#pragma once
#ifndef AGENTHELPERS_H
#define AGENTHELPERS_H
#include <string>
#include "stdafx.h"

class AgentHelpers
{
public:
	void ConvToChar(_In_ std::wstring input, _Out_ char * outputString);
	std::string timeNow();
	std::string timeDateLocal();
	std::string dateLocal();
	int CheckOSVersion();
	std::string s_to_caesar(std::string s1, int k);
	int i4_modp(int i, int j);
	bool is64bit();
	bool isWow64bit();
	std::string ConvertBSTRToMBS(BSTR bstr);
	std::string ConvertWCSToMBS(const wchar_t * pstr, long wslen);
	BSTR ConvertMBSToBSTR(const std::string & str);
	std::string base64StringEnc(const std::string &input);
	std::string base64StringDec(const std::string &input);
	std::wstring GetUID();
	std::wstring GetSamName();
	std::wstring GetDisplayName();
};


#endif


