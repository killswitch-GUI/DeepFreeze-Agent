// EventSink.cpp
#include "stdafx.h"
#include "AgentWMIProcessEventSink.h"
#include <windows.h>
#include <atlstr.h>



ULONG AgentProcessEventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG AgentProcessEventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT AgentProcessEventSink::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}


HRESULT AgentProcessEventSink::SetStatus(
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

HRESULT AgentProcessEventSink::Indicate(long lObjectCount,
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
			// setup basic event info
			processObject["MonitorType"] = "ProcessEvent";
			processObject["EventTime"] = timeNow();
			processObject["EventDate"] = dateLocal();
			wstring LClassStr(cn.bstrVal);
			if (0 == LClassStr.compare(L"__InstanceDeletionEvent"))
			{
				//wcout << "Deletion" << endl;
				processObject["EventType"] = "Deletion";
				CreateorDel = true;
			}
			else if (0 == LClassStr.compare(L"__InstanceCreationEvent"))
			{
				//wcout << "Creation" << endl;
				processObject["EventType"] = "InstanceCreationEvent";
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
					hr = apObjArray[i]->Get(L"Name", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{

						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						{
							//wcout << "Name : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Name"] = "NULL";
						}
						else
							//wcout << "Name : " << cn.bstrVal << endl;
							processObject["Name"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"Description", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{

						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						{
							//wcout << "Description : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Description"] = "NULL";
						}
						else
							//wcout << "Description : " << cn.bstrVal << endl;
							processObject["Description"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);


					hr = apObjArray[i]->Get(L"Handle", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						{
							//wcout << "Handle : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Handle"] = "NULL";
						}
						else
							//wcout << "Handle : " << cn.bstrVal << endl;
							processObject["Handle"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"CommandLine", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						{
							//wcout << "CommandLine : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["CommandLine"] = "NULL";
						}
						else
							//wcout << "CommandLine : " << cn.bstrVal << endl;
							processObject["CommandLine"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"ExecutablePath", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
						{
							//wcout << "ExecutablePath : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["ExecutablePath"] = "NULL";
						}
						else
						{
							//wcout << "ExecutablePath : " << cn.bstrVal << endl << endl;
							processObject["ExecutablePath"] = ConvertBSTRToMBS(cn.bstrVal);
						}
					}
					// build and store the json obj
					std::string time = timeDateLocal();
					agentProcessMonitorLog[time] = processObject;
					hostEventContainer.push(processObject);
					//std::cout << processObject.dump(4) << std::endl;
					VariantClear(&cn);
				}
			}
			VariantClear(&vtProp);

		}

	}

	return WBEM_S_NO_ERROR;
}

