#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>


#include <libusb.h>
#include <string.h>


#include "fileio.h"
#include "crypt.h"

libusb_context *g_ctx = NULL;
libusb_device_handle *g_hPortalHandle = NULL;

#define OVERLAP_SIZE 8192

bool OpenPortalHandle(libusb_device_handle **phPortalHandle)
{
	int OK;
	libusb_device **list;
	ssize_t i = 0;
	struct libusb_device_descriptor attributes;
	
	*phPortalHandle = NULL;
	
	libusb_init	(&g_ctx);
	libusb_set_debug(g_ctx,0);

	ssize_t cnt = libusb_get_device_list(g_ctx, &list);

	if (cnt < 0) 
		return false;
	
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		libusb_get_device_descriptor(device,&attributes);

		if (((attributes.idVendor == 0x12ba) || (attributes.idVendor == 0x54c)) || (attributes.idVendor == 0x1430))
		{
			if ((attributes.idProduct == 0x150) || (attributes.idProduct == 0x967))
			{
				libusb_ref_device(device);
				libusb_open(device, phPortalHandle);
				break;
			}
		}
	}
	libusb_free_device_list(list, 1);

		//	HidD_GetHidGuid(&guid);
		//	OK = OpenPortalHandleFromGUID(guid, phPortalHandle);
	
    return (*phPortalHandle != NULL);
}


#define rw_buf_size  0x21

typedef struct  {
	unsigned char buf[rw_buf_size];
	unsigned short dwBytesTransferred;
} RWBlock;


// Send a command to the portal
bool Write(libusb_device_handle *hPortalHandle, RWBlock *pb) {
	pb->buf[0] = 0; // Use report 0
//	return HidD_SetOutputReport(hPortalHandle, pb->buf, 0x21);
}

bool ReadBlock(libusb_device_handle *hPortalHandle, unsigned int block, unsigned char data[0x10],
			   bool isNEWskylander) {
	RWBlock req, res;
	bool running = true;
	bool gotData;
	unsigned char ovlr[OVERLAP_SIZE];
	unsigned short err;
	unsigned char followup;
	
	if(block >= 0x40) {
		return false;
	}
	
	printf(".");
	
	for(int retries = 0; retries < 3; retries++)
	{
		// Send query request
		memset(req.buf, 0, rw_buf_size);
		req.buf[1] = 'Q';
		if(isNEWskylander) {
			followup = 0x11;
			if(block == 0) {
				req.buf[2] = 0x21;
			} else {
				req.buf[2] = followup;
			}
		} else {
			followup = 0x10;
			if(block == 0) {
				req.buf[2] = 0x20;
			} else {
				req.buf[2] = followup;
			}
		}
		
		req.buf[3] = (unsigned char)block;
		
		memset(&(res.buf), 0, rw_buf_size);
		
		
		// Must set the Offset and OffsetHigh members of the OVERLAPPED structure to zero.
	//	memset(&ovlr, 0, sizeof(ovlr));
		
	//	ovlr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // read event
		
		int i=0;
		gotData = false;
		
		
		Write(hPortalHandle, &req);
		// Don't wait.  Start polling for result immediately
		
		for(; i<40; ++i) // try up to 40 reads
		{ 
			bool b;
		//	bool b = ReadFile(hPortalHandle, res.buf, rw_buf_size, &(res.dwBytesTransferred), &ovlr);
			if(!b)
			{ 
				/* failed to get data immediately*/
				 // err = GetLastError();
				if(err == 0)
				{ 
					/* wait for data */
				//	b = GetOverlappedResult(hPortalHandle, &ovlr, &res.dwBytesTransferred, TRUE);
					if(!b)
					{ 
						/* wait failed */
						break;
					} 
				} 
				else
				{ 
					/* some other error */
					break;
				} 
			} 
			if(res.dwBytesTransferred > 0)
			{ 
				/* has data */
				if(res.buf[1] == 'Q' && res.buf[3] == (unsigned char)block) {
					// Got our query back
					if(res.buf[2] == followup) {
						/* got the query back with no error */
						gotData = true;
						break;
					}
				}
				
				res.buf[0] = 0; // make sure we are using report 0
			} 
		} /* read loop */
		
		// CloseHandle(ovlr.hEvent);
		
		if(gotData) {
			break;
		}
		
	} // retries
	
	if(gotData) {
		memcpy(data, res.buf + 4, 0x10);
	}
	
	return gotData;
}

bool WriteBlock(libusb_device_handle *hPortalHandle, unsigned int block, unsigned char data[0x10],
				bool isNEWskylander) {
	RWBlock req;
	unsigned char verify[0x10];
	bool OK;
	
	printf(".");
	
	for(int retries=0; retries < 3; retries++) 
	{
		// Write request
		// W	57 10 <block number> <0x10 bytes of data>
		memset(req.buf, 0, rw_buf_size);
		req.buf[1] = 'W';
		req.buf[2] = 0x10;
		req.buf[3] = (unsigned char) block;
		memcpy(req.buf+4, data, 0x10);
		
		Write(hPortalHandle, &req);
		usleep(100000); // wait for write to take effect.
		
		memset(verify, 0xCD, sizeof(verify));
		
		OK = ReadBlock(hPortalHandle, block, verify, isNEWskylander); 
		OK = OK && (memcmp(data, verify, sizeof(verify)) == 0);
		if(OK) break;
	}
	
	return OK;
}



// Start portal
void RestartPortal(libusb_device_handle *hPortalHandle)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'R'; 
	Write(hPortalHandle, &req);
}

// Antenna up / activate
void ActivatePortal(libusb_device_handle *hPortalHandle)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'A';
	req.buf[2] = 0x01;
	Write(hPortalHandle, &req);
}

// Set the portal color
void SetPortalColor(libusb_device_handle *hPortalHandle, unsigned char r, unsigned char g, unsigned char b)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'C';
	req.buf[2] = r; // R
	req.buf[3] = g; // G
	req.buf[4] = b; // B
	Write(hPortalHandle, &req);
}

// Release hPortalInstance
void DisconnectPortal(void) {
	libusb_unref_device(libusb_get_device(g_hPortalHandle));
	libusb_close(g_hPortalHandle);
	libusb_exit	(g_ctx);

}

void ConnectToPortal(void) {
	static bool initialized = false;
	
	if(!initialized) 
	{
		printf("Connecting to portal.\n");
		
		initialized = true;
		
		// Try to stop the SpyroService if we can
		//StopSpyroService();
		
		// setup hPortalInstance
		int bResult = true;
		int bNewSkylander = false;
		
		bResult = OpenPortalHandle(&g_hPortalHandle);
		if(!bResult)
		{
			return;
		}
		
		RestartPortal(g_hPortalHandle);
		ActivatePortal(g_hPortalHandle);
		
		// disconnect on program exit
		atexit(DisconnectPortal);
	}
}

unsigned char *ReadSkylanderFromPortal(void) {
	bool bResult;
	bool bNewSkylander = false;
	unsigned char data[0x10]; 
	unsigned char *buffer;
	unsigned char *ptr;
	
	ConnectToPortal();
	
	printf("Reading Skylander from portal.\n");
	
	// must start with a read of block zero
	bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 
	
	if(!bResult) {
		bNewSkylander = !bNewSkylander;
		bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 
	}
	
	if(!bResult) {
		fprintf(stderr, "\nCould not read Skylander on portal.  Please disconnect/reconnect device.\n");
		return NULL;
	}
	
	// I don't know that we need this, but the web driver sets the color when reading the data
	SetPortalColor(g_hPortalHandle, 0xC8, 0xC8, 0xC8);
	
	buffer = (unsigned char *)malloc(1024);
	if(!buffer) {
		fprintf(stderr, "\nCould not allocate memory to read Skylander data.\n");
		return NULL;
	}
	
	ptr = buffer;
	memcpy(ptr, data, sizeof(data));
	
	for(int block=1; block < 0x40; ++block) {
		ptr += 0x10;
		bResult = ReadBlock(g_hPortalHandle, block, data, bNewSkylander); 
		memcpy(ptr, data, sizeof(data));
	}
	
	printf("\nSkylander read from portal.\n");
	return buffer;
}

bool WriteSkylanderToPortal(unsigned char *encrypted_new_data, unsigned char *encrypted_old_data)
{
	bool bResult;
	bool bNewSkylander = false;
	unsigned char data[0x10]; 
	
	ConnectToPortal();
	
	// must start with a read of block zero
	bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 
	
	if(!bResult) {
		bNewSkylander = !bNewSkylander;
		bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 
		if(!bResult) {
			fprintf(stderr, "Abort before write. Could not read data from Skylander portal.\n");
			return false;
		}
	}
	
	if(encrypted_old_data == NULL) {
		encrypted_old_data = ReadSkylanderFromPortal();
	}
	
	printf("\nWriting Skylander to portal.\n");
	
	for(int i=0; i<2; i++) {
        // two pass write
		// write the access control blocks first
		bool selectAccessControlBlock;
		if(i == 0) {
			selectAccessControlBlock = 1;
		} else {
			selectAccessControlBlock = 0;
		}
		
		for(int block=0; block < 0x40; ++block) {
			bool changed, OK;
			int offset = block * 0x10;
			changed = (memcmp(encrypted_old_data+offset, encrypted_new_data+offset, 0x10) != 0);
			if(changed) {
				if(IsAccessControlBlock(block) == selectAccessControlBlock) {
					OK = WriteBlock(g_hPortalHandle, block, encrypted_new_data+offset, bNewSkylander);
					if(!OK) {
						fprintf(stderr, "Failed to write block %d. Aborting.\n", block);
						return false;
					}
				}
			}
		}
	}
	return true;
}


