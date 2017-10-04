#include "stdafx.h"
#include "EventSink.h"
#include "Objbase.h"
#include "AgentWMIMonitor.h"
#include "AgentWMIProcessEventSink.h"
#include "AgentWMILogEventSink.h"
#include "AgentWMIServiceEventSink.h"
#include <queue>

#define _WIN32_DCOM

bool AgentWMIMonitor::monitorOrchestrator()
{
	/*
	Main entry for the WMI IR
	monitors that will be executed on launch.
	*/
	return true;
}

void AgentWMIMonitor::monitorWMIData(std::vector<json> processrules)
{
	/*
	Monitor all WMI threads for tracking of alerts, takes a vector
	of json rule sets:

	Process alert rule json structure:
	{
		"EventType" : "InstanceCreationEvent"
		"MonitorType" : "ProcessEvent"
	}
	*/
	memoryLog(0, "monitorWMIData() Starting up WMI event consumer");
	bool breakVal = TRUE;
	json qItem;
	while (breakVal)
	{
		if (!hostEventContainer.empty())
		{
			// queue has data ready to process
			qItem = hostEventContainer.front(); // Get oldest item
			hostEventContainer.pop();
			for (json i : processrules)
			{
				// check if the rule matches to reduce itterations of if statements
				if (qItem["MonitorType"] == i["MonitorType"])
				{
					// check if the rule type matches and process event type
					if (qItem["MonitorType"] == "ProcessEvent") {
						std::cout << qItem.dump(4) << std::endl;
						std::string processName = qItem["Name"];
						std::string executablePath = qItem["ExecutablePath"];
						if (executablePath.find(i["Name"]) != std::string::npos) {
							// if a match occurs trigger panic 
							memoryLog(2, "monitorWMIData() Trigger executed");
							// call the panic handler to perform logic
							wmiMonitorPanic(i, qItem);
						}
						/*if (executablePath.find(i["ExecutablePath"]) != std::string::npos)
						{
							memoryLog(2, "monitorWMIData() ExecutablePath Trigger executed");
						}*/
					}
					if (qItem["MonitorType"] == "LogEvent") {
						// TODO: implement Log Event Data
						std::cout << qItem.dump(4) << std::endl;
					}
					if (qItem["MonitorType"] == "ServiceEvent") {
						// TODO: implement Service Event Data
						std::cout << qItem.dump(4) << std::endl;
					}
				}
			}
		}
		else
		{
			// sleep the thread till we have data
			memoryLog(0, "monitorWMIData() WMI event consumer is sleeping");
			Sleep(3000);
		}
	}

}

void AgentWMIMonitor::monitorDriverInstall(std::queue<std::string> pillEventLogContainer)
{
	/*
	Setup Monitor for event log sub.
	*/
	// Init COM. or re INIT
	std::string tempErr;
	if (!InitializeCOM())
	{
		memoryLog(3, "monitorProcessCreation() The COM library failed");
	}
	memoryLog(3, "monitorProcessCreation() The COM library loaded");
	// Set general COM security levels --------------------------
	if (!initializesSecurityLayer())
	{
		memoryLog(0, "monitorProcessCreation() initializesSecurityLayer failed");
	}
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator *pLoc = NULL;
	HRESULT hres;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (hres = S_OK)
	{
		// all is good
		memoryLog(0, "monitorProcessCreation() An instance of the specified object class was successfully created");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() Failed to create IWbemLocator object.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		return;
	}
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices *pSvc = NULL;

	// Connect to the local root\cimv2 namespace
	// and obtain pointer pSvc to make IWbemServices calls.

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,  // curent context
		NULL,  // current context
		0,
		NULL,
		0,
		0,
		&pSvc
		);
	/*if (hres = WBEM_E_ACCESS_DENIED)
	{
	memoryLog(3, "monitorEventLog() The current or specified user name and password were not valid or authorized to make the connection");
	CoUninitialize();
	pLoc->Release();
	CoUninitialize();
	return;
	}*/
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() Failed to connect to WMI namespace. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	if (hres = WBEM_S_NO_ERROR)
	{
		memoryLog(0, "monitorProcessCreation() Connected to ROOT\\CIMV2 WMI namespace");
	}

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
	if (hres = S_OK)
	{
		memoryLog(0, "monitorProcessCreation() CoSetProxyBlanket security set");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() CoSetProxyBlanket security failed. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	// Receive event notifications -----------------------------
	// Use an unsecured apartment for security
	IUnsecuredApartment* pUnsecApp = NULL;

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	AgentProcessEventSink* pSink = new AgentProcessEventSink;
	pSink->AddRef();

	IUnknown* pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	IWbemObjectSink* pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs
	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_Process'"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	// Check for errors.
	if (FAILED(hres))
	{
		printf("ExecNotificationQueryAsync failed "
			"with = 0x%X\n", hres);
		pSvc->Release();
		pLoc->Release();
		pUnsecApp->Release();
		pStubUnk->Release();
		pSink->Release();
		pStubSink->Release();
		CoUninitialize();
		return;
	}

	// Wait for the event
	bool poisonPill = TRUE;
	// set up to query, for thread exit
	while (poisonPill)
	{
		if (pillEventLogContainer.empty())
		{
			// no pill, sleep 10 secs no need for high res timer
			Sleep(10000);
			memoryLog(0, "monitorProcessCreation() poisonPill not set sleeping");
		}
		else
		{
			std::string pill;
			pill = pillEventLogContainer.back();
			if (pill == "exit")
			{
				memoryLog(1, "monitorProcessCreation() poisonPill set to exit WMI event monitor");
				poisonPill = FALSE;
			}
		}

	}


	hres = pSvc->CancelAsyncCall(pStubSink);

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pUnsecApp->Release();
	pStubUnk->Release();
	pSink->Release();
	pStubSink->Release();
	CoUninitialize();

	return;
}

void AgentWMIMonitor::monitorProcessCreation(std::queue<std::string> pillProcessContainer)
{
	/*
	Setup Monitor for event log sub.
	*/
	// Init COM. or re INIT
	std::string tempErr;
	if (!InitializeCOM())
	{
		memoryLog(3, "monitorProcessCreation() The COM library failed");
	}
	memoryLog(3, "monitorProcessCreation() The COM library loaded");
	// Set general COM security levels --------------------------
	if (!initializesSecurityLayer())
	{
		memoryLog(0, "monitorProcessCreation() initializesSecurityLayer failed");
	}
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator *pLoc = NULL;
	HRESULT hres;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (hres = S_OK)
	{
		// all is good
		memoryLog(0, "monitorProcessCreation() An instance of the specified object class was successfully created");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() Failed to create IWbemLocator object.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		return;
	}
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices *pSvc = NULL;

	// Connect to the local root\cimv2 namespace
	// and obtain pointer pSvc to make IWbemServices calls.

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,  // curent context
		NULL,  // current context
		0,
		NULL,
		0,
		0,
		&pSvc
		);
	/*if (hres = WBEM_E_ACCESS_DENIED)
	{
	memoryLog(3, "monitorEventLog() The current or specified user name and password were not valid or authorized to make the connection");
	CoUninitialize();
	pLoc->Release();
	CoUninitialize();
	return;
	}*/
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() Failed to connect to WMI namespace. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	if (hres = WBEM_S_NO_ERROR)
	{
		memoryLog(0, "monitorProcessCreation() Connected to ROOT\\CIMV2 WMI namespace");
	}

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
	if (hres = S_OK)
	{
		memoryLog(0, "monitorProcessCreation() CoSetProxyBlanket security set");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorProcessCreation() CoSetProxyBlanket security failed. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	// Receive event notifications -----------------------------
	// Use an unsecured apartment for security
	IUnsecuredApartment* pUnsecApp = NULL;

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	AgentProcessEventSink* pSink = new AgentProcessEventSink;
	pSink->AddRef();

	IUnknown* pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	IWbemObjectSink* pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs
	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_Process'"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	// Check for errors.
	if (FAILED(hres))
	{
		printf("ExecNotificationQueryAsync failed "
			"with = 0x%X\n", hres);
		pSvc->Release();
		pLoc->Release();
		pUnsecApp->Release();
		pStubUnk->Release();
		pSink->Release();
		pStubSink->Release();
		CoUninitialize();
		return;
	}

	// Wait for the event
	bool poisonPill = TRUE;
	// set up to query, for thread exit
	while (poisonPill)
	{
		if (pillProcessContainer.empty())
		{
			// no pill, sleep 10 secs no need for high res timer
			Sleep(10000);
			memoryLog(0, "monitorProcessCreation() poisonPill not set sleeping");
		}
		else
		{
			std::string pill;
			pill = pillProcessContainer.back();
			if (pill == "exit")
			{
				memoryLog(1, "monitorProcessCreation() poisonPill set to exit WMI event monitor!");
				poisonPill = FALSE;
			}
		}

	}


	hres = pSvc->CancelAsyncCall(pStubSink);

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pUnsecApp->Release();
	pStubUnk->Release();
	pSink->Release();
	pStubSink->Release();
	CoUninitialize();

	return;
}

void AgentWMIMonitor::monitorServiceCreation(std::queue<std::string> pillEventLogContainer)
{
	/*
	Setup Monitor for event log sub.
	*/
	// Init COM. or re INIT
	std::string tempErr;
	if (!InitializeCOM())
	{
		memoryLog(3, "monitorServiceCreation() The COM library failed");
	}
	memoryLog(3, "monitorServiceCreation() The COM library loaded");
	// Set general COM security levels --------------------------
	if (!initializesSecurityLayer())
	{
		memoryLog(0, "monitorServiceCreation() initializesSecurityLayer failed");
	}
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator *pLoc = NULL;
	HRESULT hres;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (hres = S_OK)
	{
		// all is good
		memoryLog(0, "monitorServiceCreation() An instance of the specified object class was successfully created");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorServiceCreation() Failed to create IWbemLocator object.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		return;
	}
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices *pSvc = NULL;

	// Connect to the local root\cimv2 namespace
	// and obtain pointer pSvc to make IWbemServices calls.

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,  // curent context
		NULL,  // current context
		0,
		NULL,
		0,
		0,
		&pSvc
		);
	/*if (hres = WBEM_E_ACCESS_DENIED)
	{
	memoryLog(3, "monitorEventLog() The current or specified user name and password were not valid or authorized to make the connection");
	CoUninitialize();
	pLoc->Release();
	CoUninitialize();
	return;
	}*/
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorServiceCreation() Failed to connect to WMI namespace. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	if (hres = WBEM_S_NO_ERROR)
	{
		memoryLog(0, "monitorServiceCreation() Connected to ROOT\\CIMV2 WMI namespace");
	}

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
	if (hres = S_OK)
	{
		memoryLog(0, "monitorServiceCreation() CoSetProxyBlanket security set");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorServiceCreation() CoSetProxyBlanket security failed. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	// Receive event notifications -----------------------------
	// Use an unsecured apartment for security
	IUnsecuredApartment* pUnsecApp = NULL;

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	AgentServiceEventSink* pSink = new AgentServiceEventSink;
	pSink->AddRef();

	IUnknown* pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	IWbemObjectSink* pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs
	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_Service'"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	// Check for errors.
	if (FAILED(hres))
	{
		printf("ExecNotificationQueryAsync failed "
			"with = 0x%X\n", hres);
		pSvc->Release();
		pLoc->Release();
		pUnsecApp->Release();
		pStubUnk->Release();
		pSink->Release();
		pStubSink->Release();
		CoUninitialize();
		return;
	}

	// Wait for the event
	bool poisonPill = TRUE;
	// set up to query, for thread exit
	while (poisonPill)
	{
		if (pillEventLogContainer.empty())
		{
			// no pill, sleep 10 secs no need for high res timer
			Sleep(10000);
			memoryLog(0, "monitorServiceCreation() poisonPill not set sleeping");
		}
		else
		{
			std::string pill;
			pill = pillEventLogContainer.back();
			if (pill == "exit")
			{
				memoryLog(1, "monitorServiceCreation() poisonPill set to exit WMI event monitor");
				poisonPill = FALSE;
			}
		}

	}


	hres = pSvc->CancelAsyncCall(pStubSink);

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pUnsecApp->Release();
	pStubUnk->Release();
	pSink->Release();
	pStubSink->Release();
	CoUninitialize();

	return;
}

void AgentWMIMonitor::monitorEventLog(std::queue <std::string> pillEventLogContainer)
{
	/*
	Setup Monitor for event log sub.
	*/
	// Init COM. or re INIT
	std::string tempErr;
	if (!InitializeCOM())
	{
		memoryLog(3, "monitorEventLog() The COM library failed");
	}
	memoryLog(3, "monitorEventLog() The COM library loaded");
	// Set general COM security levels --------------------------
	if (!initializesSecurityLayer())
	{
		memoryLog(0, "monitorEventLog() initializesSecurityLayer failed");
	}
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator *pLoc = NULL;
	HRESULT hres;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (hres = S_OK)
	{
		// all is good
		memoryLog(0, "monitorEventLog() An instance of the specified object class was successfully created");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorEventLog() Failed to create IWbemLocator object.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		return;
	}
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices *pSvc = NULL;

	// Connect to the local root\cimv2 namespace
	// and obtain pointer pSvc to make IWbemServices calls.
	
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,  // curent context
		NULL,  // current context
		0,
		NULL,
		0,
		0,
		&pSvc
		);
	/*if (hres = WBEM_E_ACCESS_DENIED)
	{
		memoryLog(3, "monitorEventLog() The current or specified user name and password were not valid or authorized to make the connection");
		CoUninitialize();
		pLoc->Release();
		CoUninitialize();
		return;
	}*/
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorEventLog() Failed to connect to WMI namespace. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		pLoc->Release();
		CoUninitialize();
		return;
	}
	if (hres = WBEM_S_NO_ERROR)
	{
		memoryLog(0, "monitorEventLog() Connected to ROOT\\CIMV2 WMI namespace");
	}

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
	if (hres = S_OK)
	{
		memoryLog(0, "monitorEventLog() CoSetProxyBlanket security set");
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("monitorEventLog() CoSetProxyBlanket security failed. Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return;            
	}
	// Receive event notifications -----------------------------
	// Use an unsecured apartment for security
	IUnsecuredApartment* pUnsecApp = NULL;

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	AgentLogEventSink* pSink = new AgentLogEventSink;
	pSink->AddRef();

	IUnknown* pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	IWbemObjectSink* pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs
	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_NTLogEvent'"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	// Check for errors.
	if (FAILED(hres))
	{
		printf("ExecNotificationQueryAsync failed "
			"with = 0x%X\n", hres);
		pSvc->Release();
		pLoc->Release();
		pUnsecApp->Release();
		pStubUnk->Release();
		pSink->Release();
		pStubSink->Release();
		CoUninitialize();
		return;
	}

	// Wait for the event
	bool poisonPill = TRUE;
	// set up to query, for thread exit
	while (poisonPill)
	{
		if (pillEventLogContainer.empty())
		{
			// no pill, sleep 10 secs no need for high res timer
			Sleep(10000);
			memoryLog(0, "monitorEventLog() poisonPill not set sleeping");
		}
		else 
		{
			std::string pill;
			pill = pillEventLogContainer.back();
			if (pill == "exit")
			{
				memoryLog(1, "monitorEventLog() poisonPill set to exit WMI event monitor");
				poisonPill = FALSE;
			}
		}

	}


	hres = pSvc->CancelAsyncCall(pStubSink);

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pUnsecApp->Release();
	pStubUnk->Release();
	pSink->Release();
	pStubSink->Release();
	CoUninitialize();

	return;

}	

bool AgentWMIMonitor::InitializeCOM()
{
	std::string tempErr;
	HRESULT hres;
	memoryLog(0, "InitializeCOM() Initialize COM");
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (hres = S_OK)
	{
		memoryLog(0, "InitializeCOM() The COM library was initialized successfully on this thread");
		return TRUE;
	}
	if (hres = S_FALSE)
	{
		memoryLog(0, "InitializeCOM() The COM library is already initialized on this thread");
		return TRUE;
	}
	if (FAILED(hres))
	{
		// Initialize COM failed
		auto err = std::to_string(hres);
		tempErr.append("makeToken() Failed to initialize COM library.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		return FALSE;
	}
	// unkown error
	memoryLog(0, "InitializeCOM() The COM library was not initialized! (unknown)");
	return false;
}

bool AgentWMIMonitor::initializesSecurityLayer()
{
	/*
	Calls CoInitializeSecurity() for proccess.
	*/
	std::string tempErr;
	HRESULT hres;
	memoryLog(0, "initializesSecurityLayer() Initialize security");
	hres = CoInitializeSecurity(
		NULL,                        // COM only when a server call
		-1,                          // COM negotiates service
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);
	if (hres = S_OK)
	{
		memoryLog(0, "initializesSecurityLayer() Initialize security success");
		return TRUE;
	}
	if (hres = RPC_E_TOO_LATE)
	{
		memoryLog(0, "initializesSecurityLayer() CoInitializeSecurity has already been called (OK)");
		return TRUE;
	}
	if (FAILED(hres))
	{
		auto err = std::to_string(hres);
		tempErr.append("initializesSecurityLayer() Failed to initialize security.Error code = 0x");
		tempErr.append(err);
		memoryLog(3, tempErr);
		CoUninitialize();
		return FALSE;
		
	}
	// clean up
	CoUninitialize();
	return false;
}
