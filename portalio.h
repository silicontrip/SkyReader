#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <string.h>

// unix io functions
#include <fcntl.h>


#include "hidapi.h"
#include "fileio.h"
#include "crypt.h"

#define rw_buf_size  0x21
#define TIMEOUT 30000
#define SKYLANDER_SIZE 1024
#define MAX_STR 255


typedef struct  {
	unsigned char buf[rw_buf_size];
	int dwBytesTransferred;
} RWBlock;

class PortalIO {

	hid_device *hPortalHandle;
	
public:
	PortalIO() throw (int);
	~PortalIO();
	
	bool WriteSkylanderToPortal(unsigned char *, unsigned char *);
	
	bool ReadBlock (unsigned int , unsigned char [0x10], int ) throw (int); 
	void SetPortalColor(unsigned char , unsigned char , unsigned char ) throw (int);
	bool WriteBlock(unsigned int , unsigned char [0x10], int ) throw (int);

	void flash (void) throw (int);
	
private:
	void OpenPortalHandle() throw (int);
	void Write(RWBlock *) throw (int);
	void RestartPortal(void) throw (int);
	void ActivatePortal(int) throw (int);
	unsigned char PortalStatus() throw (int);
	void DisconnectPortal(void);
	void ConnectToPortal(void) throw (int);
	bool CheckResponse (RWBlock *, char ) throw (int); 

}; 