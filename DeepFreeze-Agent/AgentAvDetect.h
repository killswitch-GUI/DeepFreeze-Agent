#pragma once
#ifndef AGENTAVDETECT_H
#define AGENTAVDETECT_H
#include "stdafx.h"

class AgentAvDetect
{
public:
	bool EndPoint();
	bool KEndPoint();
	bool ActiveDriver(WCHAR *driver_name);
};

#endif