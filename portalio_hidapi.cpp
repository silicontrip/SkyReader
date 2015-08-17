#include "portalio.h"

/*
 Number of possible configurations: 1  Device Class: 0  VendorID: 1430  ProductID: 0150
 Total interface number: 1 ||| Number of alternate settings: 1 | Interface Number: 0 | 
 Number of endpoints:  2
 found an IN End Point 0 with attributes interrupt and address 0x1
 found an OUT End Point 1 with attributes interrupt and address 0x1
 */ 

void PortalIO::OpenPortalHandle() throw (int)
{
	struct hid_device_info *list, *attributes;
	
	wchar_t wstr[MAX_STR];

	hPortalHandle = NULL;

	list = hid_enumerate(0x0, 0x0);
	
	if (!list) {
		throw 4;
	}
	attributes = list;
	while (attributes) {
		if (((attributes->vendor_id == 0x12ba) || (attributes->vendor_id == 0x54c)) || (attributes->vendor_id == 0x1430))
		{
			if ((attributes->product_id == 0x150) || (attributes->product_id == 0x967))
			{
				printf("Found portal usb device\n");
				int err;

				hPortalHandle = hid_open(attributes->vendor_id, attributes->product_id, NULL);
				if (hPortalHandle == NULL)
				{
					printf ("Error communicating with Portal.\n ");
				}
				
				/*
				wstr[0] = 0x0000;
				err = hid_get_manufacturer_string(hPortalHandle, wstr, MAX_STR);
				if (err < 0)
					printf("Unable to read manufacturer string\n");
				printf("Manufacturer String: %ls\n", wstr);
				
				// Read the Product String
				wstr[0] = 0x0000;
				err = hid_get_product_string(hPortalHandle, wstr, MAX_STR);
				if (err < 0)
					printf("Unable to read product string\n");
				printf("Product String: %ls\n", wstr);
				
				// Read the Serial Number String
				wstr[0] = 0x0000;
				err = hid_get_serial_number_string(hPortalHandle, wstr, MAX_STR);
				if (err < 0)
					printf("Unable to read serial number string\n");
				printf("Serial Number String: (%d) %ls", wstr[0], wstr);
				printf("\n");
				*/
				
				break;
			}
		}
		attributes = attributes->next;
	}
	hid_free_enumeration(list);

	if (!hPortalHandle)
		throw 5;
	
}


// Send a command to the portal
void PortalIO::Write(RWBlock *pb) throw (int) {
	pb->buf[0] = 0; // Use report 0
	
	/*
	SkylanderIO *skio;
	skio = new SkylanderIO();
	printf(">>>\n");
	skio->fprinthex(stdout,pb->buf, 0x21);
	delete skio;
	*/
	if (hid_write(hPortalHandle, pb->buf, 0x21) == -1)
		throw 6;

}

bool PortalIO::CheckResponse (RWBlock *res, char expect) throw (int)
{
	
	int b = hid_read_timeout(hPortalHandle, res->buf, rw_buf_size, TIMEOUT);
	
	if(b<0)
		throw 8;
	
	res->dwBytesTransferred = b;

	// this is here to debug the different responses from the portal.
	
	/*
	SkylanderIO *skio;
	skio = new SkylanderIO();
	printf("<<<\n");
	skio->fprinthex(stdout,res->buf, 0x21);
	delete skio;
	*/
	
	// found wireless USB but portal is not connected
	if (res->buf[0] == 'Z')
		throw 9;
	
	// Status says no skylander on portal
	if (res->buf[0] == 'Q' && res->buf[1] == 0)
	throw 11;
	
	
	return   (res->buf[0] != expect);
	
}

bool PortalIO::ReadBlock(unsigned int block, unsigned char data[0x10], int skylander) throw (int) {
	RWBlock req, res;
	bool gotData;
	unsigned char followup;
	
	if(block >= 0x40) {
		throw 7;
	}

	// Send query request
	
	
for(int attempt=0;attempt<15;attempt++)
{
		int i=0;
		gotData = false;
		
		memset(req.buf, 0, rw_buf_size);
		req.buf[1] = 'Q';
		
		followup = 0x10 + skylander ;
		req.buf[2] = followup;
	
		if(block == 0) {
			req.buf[2] = followup + 0x10;
		} 
		
		req.buf[3] = (unsigned char)block;
		
		memset(&(res.buf), 0, rw_buf_size);
		
		
		do { Write(&req); } while (CheckResponse(&res,'Q'));
			
		if(res.buf[0] == 'Q' && res.buf[2] == (unsigned char)block) {
					// Got our query back
			if(res.buf[1] == followup) {						
				/* got the query back with no error */
				memcpy(data, res.buf + 3, 0x10);
				return true;
			}
		}
		
		
	} // retries
	
	throw 8;
}

bool PortalIO::WriteBlock(unsigned int block, unsigned char data[0x10], int skylander) throw (int) 
{
	RWBlock req;
	unsigned char verify[0x10];
	bool OK;
	
	// printf(".");
	
	for(int retries=0; retries < 3; retries++) 
	{
		// Write request
		// W	57 10 <block number> <0x10 bytes of data>
		memset(req.buf, 0, rw_buf_size);
		req.buf[1] = 'W';
		req.buf[2] = 0x10 + skylander;
		req.buf[3] = (unsigned char) block;
		memcpy(req.buf+4, data, 0x10);
		
		try {
		
			Write(&req);
			usleep(100000); // wait for write to take effect.
		
			memset(verify, 0xCD, sizeof(verify));
		
			ReadBlock(block, verify, skylander); 
			if (memcmp(data, verify, sizeof(verify)) ) { throw 22; }
			return true;
		} catch (int e) {
			;
		}
	}
	throw 22;
}


// 
unsigned char PortalIO::PortalStatus() throw (int)
{
	RWBlock req,res;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'S'; 
	do { Write(&req); } while (CheckResponse(&res,'S'));
	
	return res.buf[1];
}


// Start portal
void PortalIO::RestartPortal() throw (int)
{
	RWBlock req,res;
		
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'R'; 
	do { Write(&req); } while (CheckResponse(&res,'R'));
	
}

// Antenna up / activate
void PortalIO::ActivatePortal(int active) throw (int)
{
	RWBlock req,res;

	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'A';
	req.buf[2] = active;
	do { Write(&req); } while (CheckResponse(&res,'A'));
	
}

// Set the portal color
void PortalIO::SetPortalColor(unsigned char r, unsigned char g, unsigned char b) throw (int)
{
	RWBlock req,res;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'C';
	req.buf[2] = r; // R
	req.buf[3] = g; // G
	req.buf[4] = b; // B
	
	// no response for this one.
		Write(&req);
		
}

// Release hPortalInstance
PortalIO::~PortalIO() {
	
	ActivatePortal(0);

	hid_close(hPortalHandle);

}

void PortalIO::flash() throw (int)
{

	for (;;) {
		ActivatePortal(1);
		ActivatePortal(0);
	}
}


PortalIO::PortalIO() throw (int)
{
	//printf("Connecting to portal.\n");
				
	OpenPortalHandle();
	RestartPortal();
	ActivatePortal(1);
	// SetPortalColor(0xC8, 0xC8, 0xC8);

	// printf ("Portal Status: %d\n",PortalStatus());

}






