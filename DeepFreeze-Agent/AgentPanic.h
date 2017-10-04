#pragma once
#ifndef AGENTPANIC_H
#define AGENTPANIC_H
#include "stdafx.h"
#include "AgentGlobals.h"
#include "AgentHelpers.h"

class AgentPanic
{
public:
	bool wmiMonitorPanic(json panicTrigger, json eventTrigger);
};

#endif
