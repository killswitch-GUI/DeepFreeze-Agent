#pragma once
#ifndef AGENTVMDETECT_H
#define AGENTVMDETECT_H
#include "stdafx.h"

class AgentVMDetect
{ 
public:
	bool AgentCheckVM();
	bool AgentCheckVMWMIPointingDevice();
	bool AgentCheckVMWMISystemEnclosure();
};

#endif


