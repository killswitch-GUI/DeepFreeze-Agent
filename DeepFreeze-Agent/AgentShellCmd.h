#pragma once
#ifndef AGENTSHELLCMD_H
#define AGENTSHELLCMD_H
#include "stdafx.h"
#include "AgentLog.h"
#include <string>

class AgentShellCmd: AgentLog
{
public:
	std::string runShellCmd(std::string command);
};
#endif