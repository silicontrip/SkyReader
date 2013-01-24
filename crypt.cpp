
#include "crypt.h"

/*
data encryption
Every block from 0x08 onward (with the exception of the access control blocks) is encrypted using a key unique to that block. 
The algorithm is 128-bit AES, ECB mode and zero-byte padding. As that's a symmetric key algorithm, the same key is used to 
both encrypt and decrypt.

The key itself is the MD5 hash of the following 0x56 bytes:

<first 0x20 bytes of sector 0> <1-byte block index> <0x35-byte constant>

*/


int Crypt::IsAccessControlBlock(unsigned int blockIndex)
{
	return (((blockIndex % 4) != 3) ? 0 : 1);
}

 int Crypt::ShouldEncryptBlock(unsigned int blockIndex)
{
	if ((blockIndex >= 8) && (IsAccessControlBlock(blockIndex) == 0))
	{
		return 1;
	}
	return 0;
}


 void Crypt::ComputeMD5(unsigned char digest[16], void const* bytesIn, unsigned int inputLen)
{
	MD5 md5;
    MD5Open(&md5);
	MD5Digest(&md5, bytesIn, inputLen);
	MD5Close(&md5, digest);
}

/* Compute key used by AES
The key is the MD5 hash of the following 0x56 bytes:

<first 0x20 bytes of sector 0> <1-byte block index> <0x35-byte constant>
*/
 void Crypt::ComputeEncryptionKey(unsigned char keyOut[16], unsigned char const* tagBlocks0and1, unsigned int blockIndex)
{
	// <0x35-byte constant>
	unsigned char hashConst[] = { 
		0x20, 0x43, 0x6F, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x28, 0x43, 0x29, 0x20, 0x32, // Copyright (C) 2
		0x30, 0x31, 0x30, 0x20, 0x41, 0x63, 0x74, 0x69, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x20, // 010 Activision. 
		0x41, 0x6C, 0x6C, 0x20, 0x52, 0x69, 0x67, 0x68, 0x74, 0x73, 0x20, 0x52, 0x65, 0x73, 0x65, 0x72, // All Rights Reser
		0x76, 0x65, 0x64, 0x2E, 0x20};                                                                  // ved.
	const int hashLen = 0x56;
	unsigned char hashBuf[hashLen];
	unsigned char *numPtr = hashBuf;
	memcpy(numPtr, tagBlocks0and1, 0x20);
	numPtr += 0x20;
	numPtr[0] = (unsigned char)blockIndex;
	numPtr += 1;
	memcpy(numPtr, hashConst, 0x35);
	
	//fprinthex(stdout,hashBuf, 0x56);
	
	ComputeMD5(keyOut, hashBuf, 0x56);
}


#define KEYBITS 128
 void Crypt::EncryptAES128ECB(unsigned char * key, unsigned char const* plainTextIn, unsigned char* cipherTextOut)
{
	unsigned long rk[RKLENGTH(KEYBITS)];
	int nrounds;
	nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);
	rijndaelEncrypt(rk, nrounds, plainTextIn, cipherTextOut);
}

 void Crypt::DecryptAES128ECB(unsigned char * key, unsigned char const* cipherTextIn, unsigned char* plainTextOut)
{
	unsigned long rk[RKLENGTH(KEYBITS)];
	int nrounds;
	nrounds = rijndaelSetupDecrypt(rk, key, KEYBITS);
	rijndaelDecrypt(rk, nrounds, cipherTextIn, plainTextOut);
}

void Crypt::EncryptTagBlock(unsigned char* blockData, unsigned int blockIndex, unsigned char const* tagBlocks0and1)
{
	if (ShouldEncryptBlock(blockIndex) != 0)
	{
		unsigned char cipherText[16];
		unsigned char aesKey[16];
		ComputeEncryptionKey(aesKey, tagBlocks0and1, blockIndex);
		EncryptAES128ECB(aesKey, blockData, cipherText);
		memcpy(blockData, cipherText, 16);
	}
}

void Crypt::DecryptTagBlock(unsigned char* blockData, unsigned int blockIndex, unsigned char const* tagBlocks0and1)
{
	if (ShouldEncryptBlock(blockIndex) != 0)
	{
		unsigned char plainText[16];
		unsigned char aesKey[16];
		ComputeEncryptionKey(aesKey, tagBlocks0and1, blockIndex);
		DecryptAES128ECB(aesKey, blockData, plainText);
		memcpy(blockData, plainText, 16);
	}
}

