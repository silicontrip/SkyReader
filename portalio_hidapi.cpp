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
	
	if (hid_write(hPortalHandle, pb->buf, 0x21) != 0)
		throw 6;

}

bool PortalIO::ReadBlock(unsigned int block, unsigned char data[0x10], bool isNEWskylander) throw (int) {
	RWBlock req, res;
	bool gotData;
	unsigned char followup;
	
	if(block >= 0x40) {
		throw 7;
	}
		
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
		
				
		int i=0;
		gotData = false;
		
		
		Write(&req);
		// Don't wait.  Start polling for result immediately
		
		
	//	printf("ReadBlock: %d.\n",block);
		for(i=0;i<40;i++) // try up to 40 reads
		{ 
			int b = hid_read_timeout(hPortalHandle, res.buf, rw_buf_size, TIMEOUT);
			
			if(b<0)
				throw 8;
			
			
			// if (b>=0) fprinthex(stdout,res.buf,19);
			res.dwBytesTransferred = b;
			
			// found wireless USB but portal is not connected
			if (res.buf[0] == 'Z')
				throw 9;
			
				/* has data */
			if(res.buf[0] == 'Q' && res.buf[2] == (unsigned char)block) {
					// Got our query back
				if(res.buf[1] == followup) {						
					/* got the query back with no error */
					memcpy(data, res.buf + 3, 0x10);
					return true;
				}
			}
							
		} /* read loop */
		
		
	} // retries
	
	throw 8;
}

bool PortalIO::WriteBlock(unsigned int block, unsigned char data[0x10], bool isNEWskylander) throw (int) 
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
		req.buf[2] = 0x10;
		req.buf[3] = (unsigned char) block;
		memcpy(req.buf+4, data, 0x10);
		
		try {
		
			Write(&req);
			usleep(100000); // wait for write to take effect.
		
			memset(verify, 0xCD, sizeof(verify));
		
			ReadBlock(block, verify, isNEWskylander); 
			if (memcmp(data, verify, sizeof(verify)) ) { throw 22; }
			return true;
		} catch (int e) {
			;
		}
	}
	throw 6;
}



// Start portal
void PortalIO::RestartPortal() throw (int)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'R'; 
	Write(&req);
}

// Antenna up / activate
void PortalIO::ActivatePortal() throw (int)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'A';
	req.buf[2] = 0x01;
	Write(&req);
}

// Set the portal color
void PortalIO::SetPortalColor(unsigned char r, unsigned char g, unsigned char b) throw (int)
{
	RWBlock req;
	
	memset(req.buf, 0, rw_buf_size);
	req.buf[1] = 'C';
	req.buf[2] = r; // R
	req.buf[3] = g; // G
	req.buf[4] = b; // B
	Write(&req);
}

// Release hPortalInstance
PortalIO::~PortalIO() {
	
	hid_close(hPortalHandle);

}

PortalIO::PortalIO(void) throw (int)
{
		printf("Connecting to portal.\n");
				
		OpenPortalHandle();		
		RestartPortal();
		ActivatePortal();
		
}






