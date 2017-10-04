#pragma once
#ifndef AGENTTOKEN_H
#define AGENTTOKEN_H
#include "AgentLog.h"
#include "AgentHelpers.h"

class AgentToken: AgentLog, AgentHelpers
{
public:
	bool revertToSelf();
	bool makeToken(LPCWSTR username, LPCWSTR password, LPCWSTR domain);
	std::wstring sidToUser(PTOKEN_USER ti);
	std::wstring enumCurrentToken();
};
#endif