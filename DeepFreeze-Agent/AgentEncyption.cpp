#include "stdafx.h"
#include "base64.h"
#include "AgentHelpers.h"
#include "AgentEncyption.h"


void AgentEncyption::testEncryption()
{
	std::string test = "Test data!";
	WinAES aes;
	byte key[WinAES::KEYSIZE_256];
	byte iv[WinAES::BLOCKSIZE];
	iv[16] = { 0x00 };
	//aes.GenerateRandom(iv, sizeof(iv));
	key[256] = { 0x00 };
	//aes.GenerateRandom(key, sizeof(key));
	std::cout << "data: " << encryptString(test, key, iv) << std::endl;
}

std::string AgentEncyption::encryptString(std::string plainTextInput, byte * key, byte * iv)
{
	/*
	Encrypt a standard agent string and return hex variation.
	*/
	WinAES aes;
	// WinAES aes( NULL, WinAES::THROW_EXCEPTION );
	byte key2[WinAES::KEYSIZE_256] = { 0 };
	byte iv2[WinAES::BLOCKSIZE] = { 0 };
	//aes.GenerateRandom(iv2, sizeof(iv2));
	//aes.GenerateRandom(key2, sizeof(key2));
	// convert for byte char
	char const *plainText = plainTextInput.c_str();
	// pointers
	byte *ciphertext = NULL;
	std::string b64Data;

	try
	{
		// Oops - no iv
		// aes.SetKey( key, sizeof(key) );

		// Oops - no key
		// aes.SetIv( iv, sizeof(iv) );

		// Set the key and IV
		aes.SetKeyWithIv(key2, sizeof(key2), iv2, sizeof(iv2));

		// Done with key material - Microsoft owns it now...
		//SecureZeroMemory(key, sizeof(key));

		size_t psize = 0, csize = 0, rsize = 0;

		psize = strlen(plainText) + 1;
		if (aes.MaxCipherTextSize(psize, csize)) {
			ciphertext = new byte[csize];
		}
		//std::cout << plainText << std::endl;
		if (!aes.Encrypt((byte*)plainText, psize, ciphertext, csize)) {
			std::cerr << "Failed to encrypt plain text" << std::endl;
		}

		// re-syncronize under the key - ok
		// aes.SetIv( iv, sizeof(iv) );

		// get base64 from bytes
		//std::cout << ciphertext << std::endl;
		size_t input_length = sizeof(ciphertext), out_length = 0;
		const char *p = reinterpret_cast<const char*>(ciphertext);
		std::string ss(reinterpret_cast< char const* >(p));
		//std::cout << ss << std::endl;
		static std::string encoded;
		Base64::Encode(ss, &encoded);
		//std::cout << encoded << std::endl;
		b64Data = encoded;
		
	}
	catch (const WinAESException& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	if (NULL != ciphertext) {
		delete[] ciphertext;
		ciphertext = NULL;
	}

	return b64Data;
}

std::string AgentEncyption::decryptString(byte *ciphertext, byte *key, byte *iv)
{
	WinAES aes;
	byte *recovered = NULL;
	size_t csize = 0, rsize = 0, psize = 0;

	aes.SetKeyWithIv(key, sizeof(key), iv, sizeof(iv));

	// Done with key material - Microsoft owns it now...
	SecureZeroMemory(key, sizeof(key));
	SecureZeroMemory(iv, sizeof(iv));

	csize = sizeof(ciphertext); // TODO: do we add one? + 1
	if (aes.MaxPlainTextSize(csize, rsize)) {
		recovered = new byte[rsize];
	}

	if (!aes.Decrypt(ciphertext, csize, recovered, rsize)) {
		std::cerr << "Failed to decrypt cipher text" << std::endl;
	}
	char const *plaintext = NULL;
	if (psize == rsize &&
		0 == memcmp(plaintext, recovered, min(psize, rsize)))
	{
		std::cout << "Recovered plain text" << plaintext << std::endl;
	}
	else
	{
		std::cerr << "Failed to recover plain text" << plaintext << std::endl;
	}

	std::cout << plaintext << std::endl;
	std::string plainText = plaintext;

	if (NULL != recovered) {
		delete[] recovered;
		recovered = NULL;
	}

	return plainText;
}

