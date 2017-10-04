// EventSink.cpp
#include "stdafx.h"
#include "AgentWMILogEventSink.h"
#include <windows.h>



ULONG AgentLogEventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG AgentLogEventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT AgentLogEventSink::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}


HRESULT AgentLogEventSink::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
	)
{
	if (lFlags == WBEM_STATUS_COMPLETE)
	{
		printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS)
	{
		printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}    // end of EventSink.cpp

HRESULT AgentLogEventSink::Indicate(long lObjectCount,
	IWbemClassObject **apObjArray)
{
	HRESULT hr = S_OK;
	_variant_t vtProp;

	for (int i = 0; i < lObjectCount; i++)
	{
		bool CreateorDel = false;
		_variant_t cn;
		hr = apObjArray[i]->Get(_bstr_t(L"__Class"), 0, &cn, 0, 0);
		if (SUCCEEDED(hr))
		{
			wstring LClassStr(cn.bstrVal);
			if (0 == LClassStr.compare(L"__InstanceDeletionEvent"))
			{
				wcout << "Deletion" << endl;
				CreateorDel = true;
			}
			else if (0 == LClassStr.compare(L"__InstanceCreationEvent"))
			{
				wcout << "Creation" << endl;
				CreateorDel = true;
			}
			else
			{
				CreateorDel = false;
				//wcout << "Modification " << endl;             
			}
		}
		VariantClear(&cn);

		if (CreateorDel)
		{
			hr = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
			if (!FAILED(hr))
			{
				IUnknown* str = vtProp;
				hr = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast< void** >(&apObjArray[i]));
				if (SUCCEEDED(hr))
				{
					_variant_t cn;
					hr = apObjArray[i]->Get(L"EventCode", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{

						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							wcout << "EventCode : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
						else
							wcout << "EventCode : " << cn.bstrVal << endl;
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"Logfile", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{

						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							wcout << "Logfile : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
						else
							wcout << "Logfile : " << cn.bstrVal << endl;
					}
					VariantClear(&cn);


					hr = apObjArray[i]->Get(L"Message", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							wcout << "Message : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
						else
							wcout << "Message : " << cn.bstrVal << endl;
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"SourceName", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							wcout << "SourceName : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
						else
							wcout << "SourceName : " << cn.bstrVal << endl;
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"Type", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							wcout << "Type : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
						else
							wcout << "Type : " << cn.bstrVal << endl;
					}
					VariantClear(&cn);
				}
			}
			VariantClear(&vtProp);

		}

	}

	return WBEM_S_NO_ERROR;
}

