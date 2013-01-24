#pragma once

#include "md5.h"
#include "rijndael.h"
#include "crypt.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

#include <stdio.h>


class Crypt {
	
public:
int IsAccessControlBlock(unsigned int);
void EncryptTagBlock(unsigned char*, unsigned int , unsigned char const* );
void DecryptTagBlock(unsigned char*, unsigned int , unsigned char const* );
void EncryptBuffer(unsigned char* );
void DecryptBuffer(unsigned char* );
	
private:
	int ShouldEncryptBlock(unsigned int );
	void ComputeMD5(unsigned char [16], void const* , unsigned int );
	void ComputeEncryptionKey(unsigned char [16], unsigned char const* , unsigned int );
	void EncryptAES128ECB(unsigned char * , unsigned char const* , unsigned char* );
	void DecryptAES128ECB(unsigned char * , unsigned char const* , unsigned char* );
};