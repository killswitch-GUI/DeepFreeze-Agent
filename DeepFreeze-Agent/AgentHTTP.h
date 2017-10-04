#pragma once
#include "json.hpp"
#ifndef AGENTHTTP_H
#define AGENTHTTP_H
#include "stdafx.h"
#include <Winhttp.h>
#pragma comment(lib, "winhttp.lib")




class AgentHTTP
{
public:
	std::string  HttpRequest(LPCWSTR targetSite, LPCWSTR stageUri, std::string  requestData, DWORD dwOptionalLength);
	bool HttpPost(LPCWSTR targetSite, LPCWSTR stageUri, std::string  requestData, DWORD dwOptionalLength);

};

bool AgentHTTP::HttpPost(LPCWSTR targetSite, LPCWSTR stageUri, std::string  requestData, DWORD dwOptionalLength)
{
	/*
	Returns a std::string array
	*/
	LPCWSTR UserAgent1 = L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko";
	LPCWSTR UserAgent2 = L"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)";
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	std::string httpResult;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(UserAgent1,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, targetSite,
			INTERNET_DEFAULT_HTTP_PORT, 0); // Port 80

											// Create an HTTP request handle.
											/*
											HINTERNET WINAPI WinHttpOpenRequest(
											_In_ HINTERNET hConnect,
											_In_ LPCWSTR   pwszVerb,
											_In_ LPCWSTR   pwszObjectName,
											_In_ LPCWSTR   pwszVersion,
											_In_ LPCWSTR   pwszReferrer,
											_In_ LPCWSTR   *ppwszAcceptTypes,
											_In_ DWORD     dwFlags
											);
											*/
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"POST", stageUri,
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_BYPASS_PROXY_CACHE); // WINHTTP_FLAG_SECURE no need for TLS in http vers

											  // Send a request.
	LPVOID lpBuffer = (LPVOID)requestData.c_str();
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			lpBuffer,
			dwOptionalLength,
			dwOptionalLength,
			0);


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else
					httpResult.append(pszOutBuffer);
				//printf("%s", pszOutBuffer);

				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}


	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return TRUE;

}

//Perfrom a GET request on a target site for raw HTML
std::string AgentHTTP::HttpRequest(LPCWSTR targetSite, LPCWSTR stageUri, std::string  requestData, DWORD dwOptionalLength)
{
	/*
	Returns a std::string array
	*/
	LPCWSTR UserAgent1 = L"Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko";
	LPCWSTR UserAgent2 = L"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)";
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	std::string httpResult;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(UserAgent1,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, targetSite,
			INTERNET_DEFAULT_HTTP_PORT, 0); // Port 80

	// Create an HTTP request handle.
	/*
	HINTERNET WINAPI WinHttpOpenRequest(
	_In_ HINTERNET hConnect,
	_In_ LPCWSTR   pwszVerb,
	_In_ LPCWSTR   pwszObjectName,
	_In_ LPCWSTR   pwszVersion,
	_In_ LPCWSTR   pwszReferrer,
	_In_ LPCWSTR   *ppwszAcceptTypes,
	_In_ DWORD     dwFlags
	);
	*/
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", stageUri,
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_BYPASS_PROXY_CACHE); // WINHTTP_FLAG_SECURE no need for TLS in http vers

	// Send a request.
	LPVOID lpBuffer = (LPVOID)requestData.c_str();
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 
			0,
			lpBuffer,
			dwOptionalLength,
			dwOptionalLength,
			0);


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else
					httpResult.append(pszOutBuffer);
					//printf("%s", pszOutBuffer);

				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}


	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return httpResult;

}
#endif