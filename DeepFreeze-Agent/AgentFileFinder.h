#pragma once
#ifndef AGENTFILEFINDER_H
#define AGENTFILEFINDER_H
#include "windows.h"
#include "AgentHelpers.h"
#include "stdafx.h"
#include <iostream>
#include <list>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;


class AgentFileFinder
{
public:
	void FileFind(LPCWSTR targetFile);
	void Dir(_In_ LPCWSTR targetDir, _Out_ wstring OutputString);
};

//search the C$ drive for a specfic file or folder and prints the path
void AgentFileFinder::FileFind(LPCWSTR targetFile)
{
	//Build our list
	WIN32_FIND_DATA file;
	std::vector<wstring> dirs;
	std::vector<wstring>::iterator it;
	HANDLE search_handle = FindFirstFile(L"C:\\*", &file);
	if (search_handle == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return;
	}
	else
	{
		do
		{
			if (file.cFileName)
			{
				//std::wcout << file.cFileName << std::endl;
				std::wstring string(file.cFileName);
				it = dirs.begin();
				// it no longer valid get a new one! its a pointer to first value location
				dirs.insert(it, string);
				
			}
		} while (FindNextFile(search_handle, &file));
		if (search_handle)
		{
			for (wstring i : dirs)
			{
				std::wcout << i << std::endl;
			}
				Sleep(2000);
			FindClose(search_handle);
		}
	}

}

void AgentFileFinder::Dir(_In_ LPCWSTR targetDir, _Out_ wstring OutputString)
{
	WIN32_FIND_DATA file;
	std::vector<wstring> dirs;
	std::vector<wstring>::iterator it;
	HANDLE search_handle = FindFirstFile(targetDir, &file);
	if (search_handle == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return;
	}
	else
	{
		do
		{
			if (file.cFileName)
			{
				std::wcout << file.cFileName << std::endl;
				//FileTraverse(file.cFileName);
				std::wstring string(file.cFileName);
				it = dirs.begin();
				// it no longer valid get a new one! its a pointer to first value location
				dirs.insert(it, string);
			}
		} while (FindNextFile(search_handle, &file));
		if (search_handle)
		{
			for (wstring i : dirs)
				std::wcout << i << std::endl;
			Sleep(2000);
			FindClose(search_handle);
		}
	}

}
#endif

