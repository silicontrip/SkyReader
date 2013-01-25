#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "skylander.h"
#include "portalio.h"
#include "crypt.h"

#define BLOCK_SIZE 16

class SkylanderIO {
	
	Skylander *sky;
	unsigned char * buffer;
	Crypt crypt;
	
public:
	SkylanderIO();
	~SkylanderIO();
	void fprinthex(FILE *, unsigned char *, unsigned int);
	void initWithUnencryptedFile(char *) throw (int);
	void initWithEncryptedFile(char *) throw (int);
	void initWithPortal(int) throw (int);
	
	void listSkylanders();

	Skylander * getSkylander() { return sky; }
	
//	void setSkylander(Skylander *);
	
	bool writeSkylanderToPortal(int) throw (int);
	bool writeSkylanderToUnencryptedFile(char *) throw (int);
	bool writeSkylanderToEncryptedFile(char *) throw (int);
	
	enum IOException {
		Success,
		CannotOpenFile,
		InvalidFile,
		CannotWriteFile,
		UnableToGetUSBDevices,
		UnableToFindPortal,
		UnableToWriteToPortal,
		InvalidSkylanderBlock,
		UnableToReadFromPortal,
		PortalNotReady,
		WriteVerifyError,
		
	};
protected:

	void ReadSkylanderFile(char *name) throw (int);
	bool WriteSkylanderFile(char *name, unsigned char *) throw (int);
	void ReadPortal(unsigned char *, int) throw (int);
	
	void EncryptBuffer(unsigned char* );
	void DecryptBuffer(unsigned char* );

};