#ifndef AGENTWMIMONITOR_H
#define AGENTWMIMONITOR_H
#include "stdafx.h"
#include "AgentLog.h"
#include "AgentGlobals.h"
#include "AgentPanic.h"
#include <thread>
#include <queue>

class AgentWMIMonitor: AgentLog, AgentPanic
{
public:
	bool monitorOrchestrator();
	void monitorWMIData(std::vector<json> processrules);
	void monitorEventLog(
		std::queue <std::string> pillEventLogContainer
		);
	void monitorDriverInstall(
		std::queue <std::string> pillEventLogContainer
		);
	void monitorProcessCreation(
		std::queue <std::string> pillEventLogContainer
		);
	void monitorServiceCreation(
		std::queue <std::string> pillEventLogContainer
		);
	bool InitializeCOM();
	bool initializesSecurityLayer();
};

#endif


