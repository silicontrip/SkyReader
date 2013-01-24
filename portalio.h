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
	
	bool WriteSkylanderToPortal(unsigned char *encrypted_new_data, unsigned char *encrypted_old_data);
	
	bool ReadBlock (unsigned int block, unsigned char data[0x10], bool isNEWskylander) throw (int); 
	void SetPortalColor(unsigned char r, unsigned char g, unsigned char b) throw (int);
	bool WriteBlock(unsigned int block, unsigned char data[0x10], bool isNEWskylander) throw (int);

	
private:
	
void fprinthex(FILE *f, unsigned char *c, unsigned int n);
void OpenPortalHandle() throw (int);
void Write(RWBlock *pb) throw (int);
void RestartPortal(void) throw (int);
void ActivatePortal(void) throw (int);
void DisconnectPortal(void);
void ConnectToPortal(void) throw (int);

}; 