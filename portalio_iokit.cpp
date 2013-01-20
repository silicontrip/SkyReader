// Code to read and write from USB portal device.
// Tested on DS3 portal and PS3 portal. For XBox see below.
//

#include <stdio.h>
#include <mach/mach_port.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOKitLib.h>


#include "fileio.h"
#include "crypt.h"

#include <assert.h>

#include "portalio.h"


IOUSBDeviceInterface300** g_hPortalHandle = NULL;
bool g_bIsXboxPortal;
mach_port_t masterPort;

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

bool OpenPortalHandleFromGUID(CFUUIDBytes guid, IOUSBDeviceInterface300*** phPortalHandle)
{
	io_iterator_t iterator = 0;
	io_service_t usbRef;
	bool bResult = true;
	SInt32 score;
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	IOUSBConfigurationDescriptorPtr config;
	IOCFPlugInInterface** plugin;
	HIDD_ATTRIBUTES attributes;
	CFMutableDictionaryRef matchingDict = NULL;

	
	ULONG requiredLength=0;
	
	//Set up matching dictionary for class IOUSBDevice and its subclasses 
	matchingDict = IOServiceMatching(kIOUSBDeviceClassName); 
	if (!matchingDict) {
		printf("Couldn’t create a USB matching dictionary\n"); 
		return false;
	}
	
	IOServiceGetMatchingServices(kIOMasterPortDefault,
								 matchingDict, &iterator);
	
	usbRef = IOIteratorNext(iterator);

	
	*phPortalHandle = NULL;

	while (usbRef != 0) {
		IOCreatePlugInInterfaceForService(usbRef, kIOUSBDeviceUserClientTypeID,
										  kIOCFPlugInInterfaceID, &plugin, &score);
		IOObjectRelease(usbRef);
		
		(*plugin)->QueryInterface(plugin,
								  guid,
								  (LPVOID)phPortalHandle);
		
		(*plugin)->Release(plugin);

		
		if(guid == xbox_guid) {
			break; // we are done. xbox_guid does not have HID attributes
		}

		if (*phPortalHandle != NULL)
		{
			UInt16 VendorID, ProductID;
			
			(**phPortalHandle)->GetDeviceVendor(*phPortalHandle, &VendorID);
			(**phPortalHandle)->GetDeviceProduct(*phPortalHandle, &ProductID);
				if (((VendorID == 0x12ba) || (VendorID == 0x54c)) || (VendorID == 0x1430))
				{
					if ((ProductID == 0x150) || (ProductID == 0x967))
					{
						(**phPortalHandle)->USBDeviceOpen(*phPortalHandle);
						ret = (**phPortalHandle)->GetConfigurationDescriptorPtr(*phPortalHandle, 0, &config);
						(**phPortalHandle)->SetConfiguration(*phPortalHandle, config->bConfigurationValue);

						break;  // found the portal. leave the handle open
					}
				}
			}
			(**phPortalHandle)->Release(*phPortalHandle);
			*phPortalHandle = NULL;
		}
		usbRef = IOIteratorNext(iterator);

	}
	IOObjectRelease(iterator);
	return (*phPortalHandle != NULL);
}


bool OpenPortalHandle(IOUSBDeviceInterface300*** phPortalHandle)
{
	bool OK;
	CFUUIDBytes guid;

	// Try to open XBox portal handle first
	OK = OpenPortalHandleFromGUID(xbox_guid, phPortalHandle);
	if(OK) {
		g_bIsXboxPortal = true;
		return OK;
	}

	g_bIsXboxPortal = false;
	guid = CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300);
	OK = OpenPortalHandleFromGUID(guid, phPortalHandle);

    return OK;
}


const int rw_buf_size = 0x21;
typedef struct  {
	unsigned char buf[rw_buf_size];
	UInt16 dwBytesTransferred;
} RWBlock;


// In theory, should be able to use WriteFile to create an asynchronous request using
// an overlapped structure as shown in WriteOverlapped.
// In practice, I couldn't get it to work, had to use output report executed immediately
// for HID portals.
bool WriteHID(HANDLE hPortalHandle, RWBlock *pb) {
	pb->buf[0] = 0; // Use report 0
	return HidD_SetOutputReport(hPortalHandle, pb->buf, 0x21);
}

// Write to the portal using asynchronous IO.
bool WriteOverlapped(IOUSBDeviceInterface300** hPortalHandle, RWBlock *pb) {
	OVERLAPPED ovlr;
	DWORD error_cc;
	DWORD err;
	
	// Must set the Offset and OffsetHigh members of the OVERLAPPED structure to zero.
	memset(&ovlr, 0, sizeof(ovlr));
	ovlr.hEvent = CreateEvent(NULL, true, false, NULL); // write event

	pb->buf[0] = 0; // Use report 0

	error_cc = WriteFile(hPortalHandle, pb->buf, rw_buf_size, NULL, &ovlr);
	
	err = GetLastError();
	if (error_cc == false && err != ERROR_IO_PENDING) {
		fprintf(stderr, "Error in sending command to portal. Error %d.\n", GetLastError());
		return false;
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
		return false;
	}
	*/

	return true;
}

// Send a command to the portal
bool Write(IOUSBDeviceInterface300** hPortalHandle, RWBlock *pb) {
	if(!g_bIsXboxPortal) {
		return WriteHID(hPortalHandle, pb);
	} else {
		return WriteOverlapped(hPortalHandle, pb);
	}
}

#if 0
// Synchronous immediate read
bool Read(IOUSBDeviceInterface300** hPortalHandle, RWBlock *pb) {
	pb->buf[0] = 0; // Use report 0
	return HidD_GetInputReport(hPortalHandle, pb->buf, 0x21);
}
#endif

bool ReadBlock(IOUSBDeviceInterface300** hPortalHandle, unsigned int block, unsigned char data[0x10],
			   bool isNEWskylander) {
   RWBlock req, res;
   bool running = true;
   bool gotData;
   OVERLAPPED ovlr;
   DWORD err;
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
	   memset(&ovlr, 0, sizeof(ovlr));

	   ovlr.hEvent = CreateEvent(NULL, true, false, NULL); // read event

	   int i=0;
	   gotData = false;


	   Write(hPortalHandle, &req);
	   // Don't wait.  Start polling for result immediately

	   for(; i<40; ++i) // try up to 40 reads
	   { 
		   bool b = ReadFile(hPortalHandle, res.buf, rw_buf_size, &(res.dwBytesTransferred), &ovlr);
		   if(!b)
		   { 
			   /* failed to get data immediately*/
			   err = GetLastError();
			   if(err == ERROR_IO_PENDING)
			   { 
				   /* wait for data */
				   b = GetOverlappedResult(hPortalHandle, &ovlr, &res.dwBytesTransferred, true);
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


bool WriteBlock(IOUSBDeviceInterface300** hPortalHandle, unsigned int block, unsigned char data[0x10],
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
	   Sleep(100); // wait for write to take effect.

	   memset(verify, 0xCD, sizeof(verify));

	   OK = ReadBlock(hPortalHandle, block, verify, isNEWskylander); 
	   OK = OK && (memcmp(data, verify, sizeof(verify)) == 0);
	   if(OK) break;
	}

   return OK;
}

// Start portal
void RestartPortal(IOUSBDeviceInterface300** hPortalHandle)
{
	RWBlock req;

	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'R'; 
	Write(hPortalHandle, &req);
}

// Antenna up / activate
void ActivatePortal(IOUSBDeviceInterface300** hPortalHandle)
{
	RWBlock req;

	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'A';
	req.buf[2] = 0x01;
	Write(hPortalHandle, &req);
}

// Set the portal color
void SetPortalColor(IOUSBDeviceInterface300** hPortalHandle, unsigned char r, unsigned char g, unsigned char b)
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
	kern_return_t 	kr;
	
	if(!initialized) 
	{
		printf("Connecting to portal.\n");

		initialized = true;


		// setup hPortalInstance
		bool bResult = true;
		bool bNewSkylander = false;
		
		//Create a master port for communication with the I/O Kit

		kr = IOMasterPort(MACH_PORT_NULL, &masterPort); 
		if (kr || !masterPort) {
			printf("ERR: Couldn’t create a master I/O Kit port(%08x)\n", kr); 
			return -1;
		} 

		
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

#if 0
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

