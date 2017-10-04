#pragma once
// AgentLog.h
#ifndef AGENTLOG_H
#define AGENTLOG_H
#include <string>
#include "stdafx.h"

class AgentLog
{
public:
	void memoryLog(int level, std::string message);
	std::string logLevelResolve(int level);
};
#endif