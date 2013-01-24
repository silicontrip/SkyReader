#pragma once

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "crypt.h"


class Checksum {

public:
	bool validateChecksum(unsigned char *, int , int , bool );
	bool ValidateAllChecksums(unsigned char *, bool );
	unsigned short ComputeCcittCrc16(void const *, unsigned int );

protected:
	
	 unsigned short UpdateCcittCrc16(unsigned short , unsigned char );
	 bool getChecksumParameters(int , unsigned int *, unsigned int *, unsigned int *);
	 bool computeChecksum(int, void const *, unsigned short *);

};