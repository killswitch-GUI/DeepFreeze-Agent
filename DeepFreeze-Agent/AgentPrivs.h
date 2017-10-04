#pragma once

#ifndef AGENTPRIVS_H
#define AGENTPRIVS_H
#include "stdafx.h"

class AgentPrivs
{
public:
	bool EnablePrivilege(LPCWSTR lpPrivilegeName);
};

#endif