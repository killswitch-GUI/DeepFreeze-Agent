#pragma once
#include "json.hpp"
#include "AgentHelpers.h"

// TODO: fix up failure and add messagelog
json WmiProcQuey()
{
	json processObject;
	// final json object to return
	json processObjects;
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_Process"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	AgentHelpers h;
	processObject["CollectionTime"] = h.timeNow();
	processObject["CollectionDate"] = h.dateLocal();
	//CIMTYPE cimType;
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;
		// TODO: fix "temp" error with multiple inits, find some way to reset temp?

		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			std::wstring temp = vtProp.bstrVal;
			processObject["Name"] = temp;
		}
		VariantClear(&vtProp);

		// Get the value of the Caption property
		hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			//wcout << " Caption: " << vtProp.bstrVal << endl;
			std::wstring temp4 = vtProp.bstrVal;
			processObject["Caption"] = temp4;
		}
		VariantClear(&vtProp);


		// Get the value of the Caption property
		/*hr = pclsObj->Get(L"ExecutablePath", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			wcout << " ExecutablePath: " << vtProp.bstrVal << endl;
			std::wstring temp8 = vtProp.bstrVal;
			processObject["ExecutablePath"] = temp8;
		}
		VariantClear(&vtProp);*/

		// Get the value of the Caption property
		/*hr = pclsObj->Get(L"Handle", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
		std::wstring temp5 = vtProp.bstrVal;
		processObject["Handle"] = temp5;
		}
		VariantClear(&vtProp);*/

		// Get the value of the PID property
		hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			//std::cout << " Process ID: " << vtProp.bstrVal << std::endl;
			std::uint32_t temp2 = vtProp.uintVal;
			processObject["ProcessId"] = temp2;
		}
		VariantClear(&vtProp);

		// Get the value of the Caption property
		hr = pclsObj->Get(L"ParentProcessId", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			//std::cout << " Process ID: " << vtProp.bstrVal << std::endl;
			std::uint32_t temp3 = vtProp.uintVal;
			processObject["ParentProcessId"] = temp3;
		}
		VariantClear(&vtProp);

		// Get the value of the Caption property
		hr = pclsObj->Get(L"HandleCount", 0, &vtProp, 0, 0);
		if (hr == ERROR_SUCCESS) {
			std::uint32_t temp6 = vtProp.uintVal;
			processObject["HandleCount"] = temp6;
		}
		VariantClear(&vtProp);
		pclsObj->Release();

		std::string time = h.timeDateLocal();
		Sleep(100);
		processObjects[time] = processObject;
		processObject.clear();
	}
	
	// Cleanup
	// ========
	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return processObjects;   // Program successfully completed.

}
