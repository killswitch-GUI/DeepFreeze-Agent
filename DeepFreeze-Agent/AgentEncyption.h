#pragma once
#pragma once
#ifndef AGENTENCYPTION_H
#define AGENTENCYPTION_H
#include "stdafx.h"
#include "WinAES.h"
#include "AgentLog.h"
#include <string>
#include <iostream>

class AgentEncyption
{
public:
	void testEncryption();
	std::string encryptString(std::string plainText, byte * key, byte * iv);
	std::string decryptString(byte *ciphertext, byte *key, byte *iv);
};

#endif

