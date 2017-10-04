#include <string>
#include "stdafx.h"
#include "AgentLog.h"
#include "AgentHelpers.h"
#include "AgentGlobals.h"

void AgentLog::memoryLog(int level, std::string message)
{
	/*
	A agent logger for bedug and critical errors
	This will help debug aggent issue, and allows opperators
	to to query the log when a unkown issue is encountered.

	Error Level:
	0 = debug message
	1 = info message
	2 = warning message
	3 = error message
	4 = critical message
	*/
	int currentLevel = agentJson["agentLogLevel"];
	// build event log structure
	AgentHelpers h;
	std::string time = h.timeDateLocal();
	std::string logLevel = AgentLog::logLevelResolve(level);
	message = "[*] -" + logLevel + "- " + message + " - " + time;
	if (level >= currentLevel)
		std::cout << message << std::endl;
	agentLogArray.push_back(message);
	agentLogJson[time] = message;
}

std::string AgentLog::logLevelResolve(int level)
{
	/*
	Resolves the log int level to a
	named level for the log array.
	*/
	std::string logName;
	if (level == 0)
	{
		logName = "DEBUG";
		return logName;
	}
	if (level == 1)
	{
		logName = "INFO";
		return logName;
	}
	if (level == 2)
	{
		logName = "WARNING";
		return logName;
	}
	if (level == 3)
	{
		logName = "ERROR";
		return logName;
	}
	if (level == 4)
	{
		logName = "CRITICAL";
		return logName;
	}
	else
	{
		logName = "UNKOWN";
	}

}
