#pragma once
#ifndef AGENTPS_H
#define AGENTPS_H
#include "AgentLog.h"
#include "AgentGlobals.h"

class AgentPS: AgentLog
{
public:
	json AgentPSEnumID();
	json PSNameAndID(DWORD processID, BOOL isArch64bit);
	int  agentPid();
};
#endif