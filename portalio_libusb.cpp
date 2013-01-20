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

const char * libusb_error(int err) {

	switch(err)
	{
		case 0: return "SUCCESS";
		case LIBUSB_ERROR_IO: return "IO error";
		case LIBUSB_ERROR_INVALID_PARAM: "Invalid Parameter";
		case LIBUSB_ERROR_ACCESS: "Access denied";
		case LIBUSB_ERROR_NO_DEVICE: return "No such device";
		case LIBUSB_ERROR_NOT_FOUND: return "Entity not found";
		case LIBUSB_ERROR_BUSY: return "Resource busy";
		case LIBUSB_ERROR_TIMEOUT: return "Operation timed out";
		case LIBUSB_ERROR_PIPE: return "PIPE";
		case LIBUSB_ERROR_INTERRUPTED: "call interrupted";
		case LIBUSB_ERROR_OVERFLOW: return "OVERFLOW";
		case LIBUSB_ERROR_NO_MEM: return "Insufficient memory";
		case LIBUSB_ERROR_NOT_SUPPORTED: return "Operation not supported";
		case LIBUSB_ERROR_OTHER: return "Other error";
		default: return "UNKNOWN";
	}
	
	
}

void fprinthex(FILE *f, unsigned char *c, unsigned int n) {
	unsigned int h,i;
	unsigned char j;
	
	
	for (h=0; h<n; h+=16) {
		
		fprintf (f,"%04x: ",h);
		
		for (i=0; i<16; i++) {
			if (i+h < n) 
				fprintf (f,"%02x ",*(c+i+h) & 0xff);
			else
				fprintf (f,"   ");
		}
		for (i=0; i<16; i++) {
			if (i+h < n) { 
				j = *(c+i+h);	
				if (j<32) j='.';
				if (j>127) j='.';
				fprintf (f,"%c",j);
			} else
				fprintf(f," ");
		}
		fprintf(f,"\n");
	}
}


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
				printf("Found portal usb device\n");
				int err;
				libusb_ref_device(device);
				err= libusb_open(device, phPortalHandle);
				printf ("usb open: %s\n",libusb_error(err));

				err = libusb_claim_interface(*phPortalHandle, 0);
				printf ("claim interface: %s\n",libusb_error(err));

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
	int dwBytesTransferred;
} RWBlock;

/*
 Number of possible configurations: 1  Device Class: 0  VendorID: 1430  ProductID: 0150
 Total interface number: 1 ||| Number of alternate settings: 1 | Interface Number: 0 | 
 Number of endpoints:  2
 found an IN End Point 0 with attributes interrupt and address 0x1
 found an OUT End Point 1 with attributes interrupt and address 0x1
*/ 

// Send a command to the portal
bool Write(libusb_device_handle *hPortalHandle, RWBlock *pb) {
	
	int transferred;
	int err;
	
	printf("Write\n");
	
	pb->buf[0] = 0; // Use report 0
	
	fprinthex(stdout,pb->buf,0x21);
	
	err = libusb_interrupt_transfer (hPortalHandle, 0x1, pb->buf, 0x21, &transferred, 30000);

	
	printf("Write, %d bytes transferred (err = %s)\n",transferred,libusb_error(err));

	return err == 0;
	
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
	
	printf("ReadBlock\n");

	
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
			int b;
		//	bool b = ReadFile(hPortalHandle, res.buf, rw_buf_size, &(res.dwBytesTransferred), &ovlr);
			b = libusb_interrupt_transfer (hPortalHandle, 0x81, res.buf, rw_buf_size, &(res.dwBytesTransferred), 30000);
			
			
			if (b>=0) fprinthex(stdout,res.buf,res.dwBytesTransferred);

			
			if(b<0)
			{ 
				
				printf("error reading from usb handle: %s\n",libusb_error(b));
				
				/* failed to get data immediately*/
				 // err = GetLastError();
				if(b == 0)
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
	libusb_release_interface(g_hPortalHandle,0);
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


