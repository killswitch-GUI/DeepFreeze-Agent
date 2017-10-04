#pragma once
#include "json.h"
#include <string>
#include <queue>
using json = nlohmann::json;
extern json agentJson;
extern json agentCodes;
extern json agentSendJson;
extern json agentReturnJson;
extern json agentLogJson;
extern json agentProcessMonitorLog;
extern json agentServiceMonitorLog;
extern std::vector<std::string> agentLogArray;
extern std::queue <json> hostEventContainer;

