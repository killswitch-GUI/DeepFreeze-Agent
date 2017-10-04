// EventSink.cpp
#include "stdafx.h"
#include "AgentWMIServiceEventSink.h"
#include <windows.h>



ULONG AgentServiceEventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG AgentServiceEventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT AgentServiceEventSink::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}


HRESULT AgentServiceEventSink::SetStatus(
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

HRESULT AgentServiceEventSink::Indicate(long lObjectCount,
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
			processObject["MonitorType"] = "ServiceEvent";
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
				processObject["EventType"] = "ModificationEvent";
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
							//wcout << "Name : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Name"] = "NULL";
						else
							//wcout << "Name : " << cn.bstrVal << endl;
							processObject["Name"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"Description", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{

						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "Description : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Description"] = "NULL";
						else
							wcout << "Description : " << cn.bstrVal << endl;
							processObject["Description"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);


					hr = apObjArray[i]->Get(L"Caption", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "Caption : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Caption"] = "NULL";
						else
							//wcout << "Caption : " << cn.bstrVal << endl;
							processObject["Caption"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"DisplayName", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "DisplayName : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["DisplayName"] = "NULL";
						else
							//wcout << "DisplayName : " << cn.bstrVal << endl;
							processObject["DisplayName"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"PathName", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "PathName : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["PathName"] = "NULL";
						else
							//wcout << "PathName : " << cn.bstrVal << endl;
							processObject["PathName"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"ServiceType", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "ServiceType : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["ServiceType"] = "NULL";
						else
							//wcout << "ServiceType : " << cn.bstrVal << endl;
							processObject["ServiceType"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"StartName", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "StartName : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["StartName"] = "NULL";
						else
							//wcout << "StartName : " << cn.bstrVal << endl;
							processObject["StartName"] = ConvertBSTRToMBS(cn.bstrVal);
					}

					hr = apObjArray[i]->Get(L"StartMode", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "ServiceType : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["StartMode"] = "NULL";
						else
							//wcout << "ServiceType : " << cn.bstrVal << endl;
							processObject["StartMode"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"InstallDate", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "ServiceType : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["InstallDate"] = "NULL";
						else
							//wcout << "ServiceType : " << cn.bstrVal << endl;
							processObject["InstallDate"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"Started", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "ServiceType : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["Started"] = "NULL";
						else
							//wcout << "ServiceType : " << cn.bstrVal << endl;
							processObject["Started"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);

					hr = apObjArray[i]->Get(L"SystemName", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						if ((cn.vt == VT_NULL) || (cn.vt == VT_EMPTY))
							//wcout << "ServiceType : " << ((cn.vt == VT_NULL) ? "NULL" : "EMPTY") << endl;
							processObject["SystemName"] = "NULL";
						else
							//wcout << "ServiceType : " << cn.bstrVal << endl;
							processObject["SystemName"] = ConvertBSTRToMBS(cn.bstrVal);
					}
					VariantClear(&cn);








					// build and store the json obj
					std::string time = timeDateLocal();
					agentServiceMonitorLog[time] = processObject;
					hostEventContainer.push(processObject);
					std::cout << processObject.dump(4) << std::endl;
					VariantClear(&cn);
				}
			}
			VariantClear(&vtProp);

		}

	}

	return WBEM_S_NO_ERROR;
}

