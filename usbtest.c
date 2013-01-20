/*
 * gcc -I/opt/local/include/libusb-1.0 -L/opt/local/lib -lusb-1.0 usbtest.c
 */
#include <stdio.h>
#include <libusb.h>

#define PORTAL_VENDOR 0x1430
#define PORTAL_PRODUCT 0x0150
#define BUFFER 128

void fprinthex(FILE *f, char *c, unsigned int n) {
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

const char * endpoint_attribute (int epdesc)
{

	int transfer_type = epdesc & 0x3;
	int sync_type = epdesc & 0xC;
	int usage_type = epdesc & 0x30;
	
	switch (transfer_type) {
		case LIBUSB_TRANSFER_TYPE_CONTROL: return "control";
		case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS: 
			switch (sync_type) {
				case LIBUSB_ISO_SYNC_TYPE_NONE: return "isochronous_none";
				case LIBUSB_ISO_SYNC_TYPE_ASYNC: return "isochronous_asynchronous";
				case LIBUSB_ISO_SYNC_TYPE_ADAPTIVE: return "isochronous_adaptive";
				case LIBUSB_ISO_SYNC_TYPE_SYNC: return "isochronous_synchronous";
			}
			return "isochronous";
		case LIBUSB_TRANSFER_TYPE_BULK: return "bulk";
		case LIBUSB_TRANSFER_TYPE_INTERRUPT: return "interrupt";
	}
	return "UNKNOWN";
	
}

void printdev(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("failed to get device descriptor\n");
		return;
	}
	printf("Number of possible configurations: %d  ",(int)desc.bNumConfigurations);
	printf("Device Class: %d  ",(int)desc.bDeviceClass);
	printf("VendorID: %.4X  ",desc.idVendor);
	printf("ProductID: %.4X\n",desc.idProduct);
	//cout<<"VendorID: "<<desc.idVendor<<"  ";
	//cout<<"ProductID: "<<desc.idProduct<<endl;
	struct libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Total interface number: %d ||| ",(int)config->bNumInterfaces);
	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;
	int i;
	for( i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		printf("Number of alternate settings: %d | ",inter->num_altsetting);
		int j;
		for( j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			printf("Interface Number: %d | interface class: %d | Interface endpoints: %d ",
				   (int)interdesc->bInterfaceNumber,
				   (int)interdesc->bInterfaceClass,
				   (int)interdesc->bNumEndpoints
				   );
		}
	}
	printf("\n");
	libusb_free_config_descriptor(config);
}


void list_endpoints(libusb_device *dev)
{
	struct libusb_config_descriptor *config;
	const struct libusb_interface *inter;
	const struct libusb_interface_descriptor *interdesc;
	const struct libusb_endpoint_descriptor *epdesc;
	int endpoints_no;
	int i;
	
	libusb_get_config_descriptor(dev, 0, &config);
	inter = &config->interface[0];
	interdesc = &inter->altsetting[0];
	endpoints_no=(int)interdesc->bNumEndpoints;
	
	printf("Number of endpoints:  %d\n",endpoints_no);
	for(i=0;i<endpoints_no;i++)
	{
		epdesc = &interdesc->endpoint[i];
		if(epdesc->bEndpointAddress & 0x80)
			printf("found an IN End Point %d with attributes %s and address 0x%x\n",i,endpoint_attribute(epdesc->bmAttributes), epdesc->bEndpointAddress&0x7f);
		else  
			printf("found an OUT End Point %d with attributes %s and address 0x%x\n",i,endpoint_attribute(epdesc->bmAttributes),epdesc->bEndpointAddress);
	}
	libusb_free_config_descriptor(config);
	
	return;
}


int main (int argc, char **argv) 
{

	unsigned char data[BUFFER];
	unsigned short lang;
	int bytes;
	libusb_context *ctx;
	struct libusb_device_descriptor device_descriptor;

	libusb_init	(&ctx);
	libusb_set_debug(ctx,0);

	// discover devices
	libusb_device **list;
	libusb_device *found = NULL;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	ssize_t i = 0;
	int err = 0;
	
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];

		printdev(device);
		
		libusb_device_handle *handle;
		
		err = libusb_open(device, &handle);
		if (!err)
		{
			bytes = libusb_get_string_descriptor(handle, 0,0,data,BUFFER);
			
			lang = data[2]<<8|data[3];
			
			if (bytes>=0) {
				// printf ("Bytes: %d\n",bytes);
				bytes = libusb_get_string_descriptor_ascii(handle,  device_descriptor.iManufacturer,data,BUFFER);
				if (bytes>0) fprinthex(stdout, data,bytes);
				bytes = libusb_get_string_descriptor_ascii(handle, device_descriptor.iProduct,data,BUFFER);
				if (bytes>0) fprinthex(stdout, data,bytes);
				bytes = libusb_get_string_descriptor_ascii(handle, device_descriptor.iSerialNumber,data,BUFFER);
				if (bytes>0) fprinthex(stdout, data,bytes);

			}
			libusb_close(handle);

		}
		
		list_endpoints(device);
		
		printf("\n");
		
		//	printf ("%d %d %d\n",device_descriptor.iManufacturer,device_descriptor.iProduct,device_descriptor.iSerialNumber);

		if (device_descriptor.idVendor == PORTAL_VENDOR && device_descriptor.idProduct == PORTAL_PRODUCT) 
		{
			found = device;
		}
		
	}
	
	if (found) {
	}
	
	libusb_free_device_list(list, 1);
	
	
	
	libusb_exit	(ctx);
	return 0;
}
