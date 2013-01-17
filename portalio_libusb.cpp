#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <libusb.h>
#include <string.h>


#include "fileio.h"
#include "crypt.h"

libusb_context *g_ctx = NULL;
libusb_device_handle *g_hPortalHandle = NULL;

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