#pragma once
// AgentSink.h
#ifndef AGENTSEND_H
#define AGENTSEND_H
#include <string>
#include "stdafx.h"
#include "AgentGlobals.h"


class AgentSend
{
public:
	bool resetSendJson();
	void setAgentCodes();
	bool sendMessage(
		int requestCode, 
		std::string message, 
		std::string type = "string"
		);
	bool sendCheckinMessage();
};

#endif    // end of AgentSink.h