#pragma once
#ifndef AGENTORCHESTRATOR_H
#define AGENTORCHESTRATOR_H

#include "stdafx.h"
#include "windows.h"
#include <stdio.h>
#include <queue>
#include "AgentGlobals.h"
#include "AgentLog.h"

using namespace std;
using json = nlohmann::json;


class AgentOrchestrator : AgentLog
{
public:
	bool coreOrchestrator();
	bool initzializeEnviroment();
	bool agentSettings(	int agentId, 
						int agentPid, 
						std::wstring agentUser, 
						std::wstring agentDisplayName,
						int agentDelay, 
						int agentJitter, 
						int agentLostLimit, 
						std::wstring  stageServer, 
						std::wstring  stageGetUri, 
						std::wstring  stagePostUri, 
						std::wstring stageKey
						);
	int requestHandler(LPCWSTR stageServer, LPCWSTR stageUri);
	int requestCodeHandler();
	bool enageWMIMonitoring();
	void exitAgentClean();
	std::vector<json> wmiProcessTestRules();
	// vars for threads
	std::queue <std::string> pillProcessContainer; // used for global exit of threads built
	std::queue <std::string> pillEventLogContainer; // used for global exit of threads built
};

#endif