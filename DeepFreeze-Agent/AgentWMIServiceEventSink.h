#pragma once
// EventSink.h
#ifndef AGENTWMISERVICEEVENTSINK_H
#define AGENTWMISERVICEEVENTSINK_H

#define _WIN32_DCOM
#include <windows.h>
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include "AgentGlobals.h"
#include "AgentHelpers.h"

# pragma comment(lib, "wbemuuid.lib")

class AgentServiceEventSink : AgentHelpers, public IWbemObjectSink
{
	LONG m_lRef;
	bool bDone;

public:
	json processObject;
	AgentServiceEventSink() { m_lRef = 0; }
	~AgentServiceEventSink() { bDone = true; }

	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT
		STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
		);

	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		/* [in] */ LONG lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ BSTR strParam,
		/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
		);
};

#endif    // end of EventSink.h
#pragma once
