#pragma once
#ifndef AGENTSHELLEXEC_H
#define AGENTSHELLEXEC_H
#include "stdafx.h"
#include "AgentHelpers.h"
#include "AgentLog.h"
#include "AgentPrivs.h"
/*
*	Shellcode injector adapted from Herzel Levy(Hertz)
*/

class AgentShellExec: AgentHelpers, AgentLog, AgentPrivs
{
public:
	bool injectShellCode(int osLevel, DWORD dwProcId, char shellCode[]);
private:
	HANDLE createRemoteThread(HANDLE hProcess, LPVOID lpRemoteThreadStart, LPVOID lpRemoteCallback);
	HANDLE NtCreateThreadEx(HANDLE hProcess, LPVOID lpRemoteThreadStart, LPVOID lpRemoteCallback);
};

#endif