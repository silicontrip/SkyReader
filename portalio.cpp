// Code to read and write from USB portal device.
// Tested on DS3 portal and PS3 portal. For XBox see below.
//

// Include Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>


// HID library
extern "C" {
#include <hidsdi.h>
};

// Include WinUSB headers
#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>

#include "fileio.h"
#include "crypt.h"

#include <assert.h>

#include "portalio.h"


// Linked libraries
#pragma comment (lib , "setupapi.lib" )
#pragma comment (lib , "winusb.lib" )
#pragma comment (lib , "hid.lib" )

HANDLE g_hPortalHandle = INVALID_HANDLE_VALUE;
BOOL   g_bIsXboxPortal = FALSE;

BOOL StopSpyroService()
{
	// Shutting down a service requires administrator priviledges.

	SERVICE_STATUS strServiceStatus;
	SC_HANDLE      schManager;
	SC_HANDLE      schService;
	char           *pszServiceName;
	DWORD          dwError;
	if( (schManager = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE,
		SC_MANAGER_ALL_ACCESS )) == NULL ) {
			return FALSE;
	}

	pszServiceName = "SpyroService";
	
	if( (schService = OpenService( schManager, pszServiceName, SERVICE_STOP)) == NULL ) {
		return FALSE;
	}

	if( ControlService( schService, SERVICE_CONTROL_STOP, &strServiceStatus )!=0 ) {
		switch( dwError = GetLastError() ) {
		case ERROR_ACCESS_DENIED:
		case ERROR_DEPENDENT_SERVICES_RUNNING:
		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_PARAMETER:
		case ERROR_INVALID_SERVICE_CONTROL:
		case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
		case ERROR_SERVICE_REQUEST_TIMEOUT:
		default:
			return FALSE;

		case NO_ERROR:
		case ERROR_SERVICE_NOT_ACTIVE:
		case ERROR_SHUTDOWN_IN_PROGRESS:
			return TRUE;
		}
	}
	if( strServiceStatus.dwCurrentState != SERVICE_STOPPED ) return FALSE;
	return TRUE;
}

// Attempted to add support for XBox portal based on 
// XBoxSpyroPortal and XBoxSpyroManager in .NET decompilation of
// FlashPortal.exe in the drivers from spyrowebportaldriver.exe.
// I'm pretty sure the part where I get the handle for the portal is correct,
// however, I doubt the code to talk to it as an HID device will work
// because the XBox driver does not appear to be an HID device.
// May need to set up pipes as seen in the XBoxSpyroPortal class.
// Another approach you could try is to use the WriteFile command
// in the Write function below. The Write function is the only thing
// that actually uses the HID commands.

// XBox Portal GUID
// {B323AD0E-ADA8-4d64-AA53-4B46E5843312}
DEFINE_GUID(xbox_guid, 0xB323AD0E, 0xADA8, 0x4d64, 
			0xAA, 0x53, 0x4B, 0x46, 0xE5, 0x84, 0x33, 0x12);

BOOL OpenPortalHandleFromGUID(GUID guid, PHANDLE phPortalHandle)
{
	int memberIndex = 0;
	BOOL bResult = TRUE;
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
	HIDD_ATTRIBUTES attributes;

	ULONG requiredLength=0;
	
	hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	*phPortalHandle = NULL;

	for(memberIndex = 0;
		SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, memberIndex, &deviceInterfaceData);
		memberIndex++)
	{
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, NULL, 0, &requiredLength, NULL);

		//we got the size, allocate buffer
		pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);
		assert(pInterfaceDetailData);

		//get the interface detailed data
		pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, pInterfaceDetailData, requiredLength, &requiredLength, NULL))
		{
			continue;
		}

		*phPortalHandle = CreateFile (
			pInterfaceDetailData->DevicePath,
			GENERIC_EXECUTE | GENERIC_ALL,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		LocalFree(pInterfaceDetailData); 
		pInterfaceDetailData = NULL;

		if(guid == xbox_guid) {
			break; // we are done. xbox_guid does not have HID attributes
		}

		if (*phPortalHandle != INVALID_HANDLE_VALUE)
		{
			if (HidD_GetAttributes(*phPortalHandle , &attributes))
			{
				if (((attributes.VendorID == 0x12ba) || (attributes.VendorID == 0x54c)) || (attributes.VendorID == 0x1430))
				{
					if ((attributes.ProductID == 0x150) || (attributes.ProductID == 0x967))
					{
						break;  // found the portal. leave the handle open
					}
				}
			}
			CloseHandle(*phPortalHandle);
			*phPortalHandle = NULL;
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return (*phPortalHandle != NULL);
}


BOOL OpenPortalHandle(PHANDLE phPortalHandle)
{
	BOOL OK;
	GUID guid;

	// Try to open XBox portal handle first
	OK = OpenPortalHandleFromGUID(xbox_guid, phPortalHandle);
	if(OK) {
		g_bIsXboxPortal = TRUE;
		return OK;
	}

	g_bIsXboxPortal = FALSE;
	HidD_GetHidGuid(&guid);
	OK = OpenPortalHandleFromGUID(guid, phPortalHandle);

    return OK;
}


const int rw_buf_size = 0x21;
typedef struct  {
	unsigned char buf[rw_buf_size];
	DWORD dwBytesTransferred;
} RWBlock;


// In theory, should be able to use WriteFile to create an asynchronous request using
// an overlapped structure as shown in WriteOverlapped.
// In practice, I couldn't get it to work, had to use output report executed immediately
// for HID portals.
BOOL WriteHID(HANDLE hPortalHandle, RWBlock *pb) {
	pb->buf[0] = 0; // Use report 0
	return HidD_SetOutputReport(hPortalHandle, pb->buf, 0x21);
}

// Write to the portal using asynchronous IO.
BOOL WriteOverlapped(HANDLE hPortalHandle, RWBlock *pb) {
	OVERLAPPED ovlr;
	DWORD error_cc;
	DWORD err;
	
	// Must set the Offset and OffsetHigh members of the OVERLAPPED structure to zero.
	memset(&ovlr, 0, sizeof(ovlr));
	ovlr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // write event

	pb->buf[0] = 0; // Use report 0

	error_cc = WriteFile(hPortalHandle, pb->buf, rw_buf_size, NULL, &ovlr);
	
	err = GetLastError();
	if (error_cc == FALSE && err != ERROR_IO_PENDING) {
		fprintf(stderr, "Error in sending command to portal. Error %d.\n", GetLastError());
		return FALSE;
	}

	//Sleep(2000);

	/*
	// Block until command has been sent
	if(err == ERROR_IO_PENDING) {
		int cnt = 0;
		while(!HasOverlappedIoCompleted(&ovlr) && cnt++ < 10) {
			Sleep(10);
		}
	}

	if(!HasOverlappedIoCompleted(&ovlr)) {
		return FALSE;
	}
	*/

	return TRUE;
}

// Send a command to the portal
BOOL Write(HANDLE hPortalHandle, RWBlock *pb) {
	if(!g_bIsXboxPortal) {
		return WriteHID(hPortalHandle, pb);
	} else {
		return WriteOverlapped(hPortalHandle, pb);
	}
}

#if 0
// Synchronous immediate read
BOOL Read(HANDLE hPortalHandle, RWBlock *pb) {
	pb->buf[0] = 0; // Use report 0
	return HidD_GetInputReport(hPortalHandle, pb->buf, 0x21);
}
#endif

BOOL ReadBlock(HANDLE hPortalHandle, unsigned int block, unsigned char data[0x10],
			   BOOL isNEWskylander) {
   RWBlock req, res;
   BOOL running = TRUE;
   BOOL gotData;
   OVERLAPPED ovlr;
   DWORD err;
   unsigned char followup;

   if(block >= 0x40) {
	   return FALSE;
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
	   memset(&ovlr, 0, sizeof(ovlr));

	   ovlr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // read event

	   int i=0;
	   gotData = FALSE;


	   Write(hPortalHandle, &req);
	   // Don't wait.  Start polling for result immediately

	   for(; i<40; ++i) // try up to 40 reads
	   { 
		   BOOL b = ReadFile(hPortalHandle, res.buf, rw_buf_size, &(res.dwBytesTransferred), &ovlr);
		   if(!b)
		   { 
			   /* failed to get data immediately*/
			   err = GetLastError();
			   if(err == ERROR_IO_PENDING)
			   { 
				   /* wait for data */
				   b = GetOverlappedResult(hPortalHandle, &ovlr, &res.dwBytesTransferred, TRUE);
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
					   gotData = TRUE;
					   break;
				   }
			   }

			   res.buf[0] = 0; // make sure we are using report 0
		   } 
	   } /* read loop */

	   CloseHandle(ovlr.hEvent);

	   if(gotData) {
		   break;
	   }
	   
   } // retries

   if(gotData) {
	   memcpy(data, res.buf + 4, 0x10);
   }

   return gotData;
}


BOOL WriteBlock(HANDLE hPortalHandle, unsigned int block, unsigned char data[0x10],
			   BOOL isNEWskylander) {
   RWBlock req;
   unsigned char verify[0x10];
   BOOL OK;

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
	   Sleep(100); // wait for write to take effect.

	   memset(verify, 0xCD, sizeof(verify));

	   OK = ReadBlock(hPortalHandle, block, verify, isNEWskylander); 
	   OK = OK && (memcmp(data, verify, sizeof(verify)) == 0);
	   if(OK) break;
	}

   return OK;
}

// Start portal
void RestartPortal(HANDLE hPortalHandle)
{
	RWBlock req;

	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'R'; 
	Write(hPortalHandle, &req);
}

// Antenna up / activate
void ActivatePortal(HANDLE hPortalHandle)
{
	RWBlock req;

	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'A';
	req.buf[2] = 0x01;
	Write(hPortalHandle, &req);
}

// Set the portal color
void SetPortalColor(HANDLE hPortalHandle, unsigned char r, unsigned char g, unsigned char b)
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
	CloseHandle(g_hPortalHandle);
}

void ConnectToPortal(void) {
	static bool initialized = false;

	if(!initialized) 
	{
		printf("Connecting to portal.\n");

		initialized = true;

		// Try to stop the SpyroService if we can
		StopSpyroService();

		// setup hPortalInstance
		BOOL bResult = TRUE;
		BOOL bNewSkylander = FALSE;

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

#if 1

// stub for testing
unsigned char *ReadSkylanderFromPortal(void) {
	return ReadSkylanderFile("D:\\Skylanders\\Editor\\Debug\\dspyro.spy");  // emulate portal for testing
}

// stub for testing
void WriteSkylanderToPortal(unsigned char *encrypted_new_data, unsigned char *encrypted_old_data)
{
	WriteSkylanderFile("D:\\Skylanders\\Editor\\Debug\\dspyro_u.spy", encrypted_new_data);  // emulate portal for testing
}

#endif

#if 0 
unsigned char *ReadSkylanderFromPortal(void) {
	BOOL bResult;
	BOOL bNewSkylander = FALSE;
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


BOOL WriteSkylanderToPortal(unsigned char *encrypted_new_data, unsigned char *encrypted_old_data)
{
	BOOL bResult;
	BOOL bNewSkylander = FALSE;
	unsigned char data[0x10]; 
	
	ConnectToPortal();

	// must start with a read of block zero
	bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 

	if(!bResult) {
		bNewSkylander = !bNewSkylander;
		bResult = ReadBlock(g_hPortalHandle, 0, data, bNewSkylander); 
		if(!bResult) {
			fprintf(stderr, "Abort before write. Could not read data from Skylander portal.\n");
			return FALSE;
		}
	}

	if(encrypted_old_data == NULL) {
		encrypted_old_data = ReadSkylanderFromPortal();
	}

	printf("\nWriting Skylander to portal.\n");

	for(int i=0; i<2; i++) {
        // two pass write
		// write the access control blocks first
		BOOL selectAccessControlBlock;
		if(i == 0) {
			selectAccessControlBlock = 1;
		} else {
			selectAccessControlBlock = 0;
		}

		for(int block=0; block < 0x40; ++block) {
			BOOL changed, OK;
			int offset = block * 0x10;
			changed = (memcmp(encrypted_old_data+offset, encrypted_new_data+offset, 0x10) != 0);
			if(changed) {
				if(IsAccessControlBlock(block) == selectAccessControlBlock) {
					OK = WriteBlock(g_hPortalHandle, block, encrypted_new_data+offset, bNewSkylander);
					if(!OK) {
						fprintf(stderr, "Failed to write block %d. Aborting.\n", block);
						return FALSE;
					}
				}
			}
		}
	}
	return TRUE;
}
#endif
