#include "stdafx.h"
#include "AgentSleep.h"
#include <windows.h>


//  Dmember function
void AgentSleep::sleepAgent(int time, int jitter)
{
	int second = 1000;
	time = second * time;
	if (jitter > 0)
		jitter = jitter * second;
	int n = jitter * 2;
	jitter = rand() % n + (-jitter);
	time = time + jitter;
	Sleep(time);
}