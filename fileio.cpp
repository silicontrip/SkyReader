
#include "fileio.h"

void SkylanderIO::fprinthex(FILE *f, unsigned char *c, unsigned int n) {
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
				if (j>=127) j='.';
				fprintf (f,"%c",j);
			} else
				fprintf(f," ");
		}
		fprintf(f,"\n");
	}
}


SkylanderIO::SkylanderIO ()
{
	sky = NULL;
	buffer=new unsigned char [1025];
}

SkylanderIO::~SkylanderIO ()
{
	delete buffer;
}


void SkylanderIO::initWithUnencryptedFile(char * name) throw (int)
{
	if (!sky) {
		ReadSkylanderFile(name);
		sky = new Skylander(buffer);
	}
}

void SkylanderIO::initWithEncryptedFile(char * name) throw (int)
{
	if (!sky) {
		ReadSkylanderFile(name);
		DecryptBuffer(buffer);
		sky = new Skylander(buffer);
	}
}

void SkylanderIO::initWithPortal(int number) throw (int) {
	
	
	if (!sky) {
		//printf("Reading Skylander from portal.\n");
		ReadPortal(buffer,number);
		DecryptBuffer(buffer);
		sky = new Skylander(buffer);
		// printf("\nSkylander read from portal.\n");
		
	}
}


void SkylanderIO::initRawReadPortal(int number) throw (int) {
	
	
	if (!sky) {
		//printf("Reading Skylander from portal.\n");
		ReadPortal(buffer,number);
		sky = new Skylander(buffer);
		// printf("\nSkylander read from portal.\n");
		
	}
}

void SkylanderIO::ReadPortal(unsigned char *s, int number) throw (int) 
{
	
	unsigned char data[0x10]; 
	unsigned char *ptr;
	
	PortalIO *port;
	
	port = new PortalIO();
	
	// must start with a read of block zero
	port->ReadBlock(0, data, number); 
	
	// I don't know that we need this, but the web driver sets the color when reading the data
	port->SetPortalColor(0xC8, 0xC8, 0xC8);
	
	ptr = s;
	memcpy(ptr, data, sizeof(data));
	
	for(int block=1; block < 0x40; ++block) {
		ptr += 0x10;
		port->ReadBlock(block, data, number); 
		memcpy(ptr, data, sizeof(data));
	}
	
	delete port;
}	

bool SkylanderIO::writeSkylanderToPortal(int number) throw (int)
{
	bool bResult;
	unsigned char data[0x10]; 
	
	unsigned char old[1024];
	unsigned char skydata[1024];
	
	Crypt crypt;
	
	if (sky) {
		
		PortalIO *port;
		
		ReadPortal(old,number);
		
		memcpy (skydata,sky->getData(),1024);
		
		EncryptBuffer(skydata);
		
		printf("\nWriting Skylander to portal.\n");
		
		port = new PortalIO();
		
		
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
				int offset = block * BLOCK_SIZE;
				if(crypt.IsAccessControlBlock(block) == selectAccessControlBlock) {
					changed = (memcmp(old + offset, skydata+offset,BLOCK_SIZE) != 0);
					if(changed) {
						port->WriteBlock( block, skydata+offset, number);
					}
				}
			}
		}
		
		return true;
	}
	return false;
}


bool SkylanderIO::RawWriteSkylanderToPortal(int number) throw (int)
{
	bool bResult;
	unsigned char data[0x10]; 
	
	unsigned char old[1024];
	unsigned char skydata[1024];
	
	Crypt crypt;
	
	if (sky) {
		
		PortalIO *port;
		
		ReadPortal(old,number);
		
		memcpy (skydata,sky->getData(),1024);
		
		printf("\nWriting Skylander to portal.\n");
		
		port = new PortalIO();
		
		
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
				int offset = block * BLOCK_SIZE;
				if(crypt.IsAccessControlBlock(block) == selectAccessControlBlock) {
					changed = (memcmp(old + offset, skydata+offset,BLOCK_SIZE) != 0);
					if(changed) {
						port->WriteBlock( block, skydata+offset, number);
					}
				}
			}
		}
		
		return true;
	}
	return false;
}

bool SkylanderIO::writeSkylanderToUnencryptedFile(char *name) throw (int)
{
	if (sky) {
		WriteSkylanderFile(name,sky->getData());
	}
}

bool SkylanderIO::writeSkylanderToEncryptedFile(char *name) throw (int)
{
	if (sky) {
		unsigned char skydata[1024];
		
		memcpy (skydata,sky->getData(),1024);
		EncryptBuffer(skydata);
		WriteSkylanderFile(name,skydata);
	}
	
}

// Encrypt entire buffer of character data
// Buffer is entire 1024 byte block of character data.

void SkylanderIO::EncryptBuffer(unsigned char* buffer) {
	unsigned int blockIndex;
	unsigned char* blockData;
	unsigned char const* tagBlocks0and1;
	
	tagBlocks0and1 = buffer;
	for(blockIndex = 0x08; blockIndex < 0x40; blockIndex++) {
		blockData = buffer + blockIndex * 0x10;
		crypt.EncryptTagBlock(blockData, blockIndex, tagBlocks0and1);
	}
}

// Decrypt entire buffer of character data
// Buffer is entire 1024 byte block of character data.
void SkylanderIO::DecryptBuffer(unsigned char* buffer) {
	unsigned int blockIndex;
	unsigned char* blockData;
	unsigned char const* tagBlocks0and1;
	
	tagBlocks0and1 = buffer;
	for(blockIndex = 0x08; blockIndex < 0x40; blockIndex++) {
		blockData = buffer + blockIndex * 0x10;
		crypt.DecryptTagBlock(blockData, blockIndex, tagBlocks0and1);
	}
}



void SkylanderIO::ReadSkylanderFile(char *name) throw (int)
{
	FILE *file;
	unsigned long fileLen;
	
	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		throw 1;
		//		fprintf(stderr, "Unable to open file %s\n", name);
		//		return NULL;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if(fileLen != 1024){
		throw 2;
	}	
		//Read file contents into buffer
		fread(buffer, fileLen, 1, file);
		fclose(file);
		
	
}

bool SkylanderIO::WriteSkylanderFile(char *name, unsigned char * filedata) throw (int)
{
	FILE *file;
	bool OK = true;
	int count;
	
	file = fopen(name, "wb");
	if (!file)
	{
		throw 1;
		//		fprintf(stderr, "Unable to open file %s for writing.\n", name);
		//		return false;
	}
	
	count = fwrite(filedata, 1024, 1, file);
	if(count < 1) {
		fclose(file);
		throw 3;
		OK = false;
	}
	
	fclose(file);
	return OK;
}

void SkylanderIO::listSkylanders() {

	PortalIO *port;
	Skylander *sky;
	unsigned char data[0x10]; 

				
	port = new PortalIO();
	sky = new Skylander(buffer);
	
	try {
		for (int s=0; s<3; s++) 
		{
			memset(data,0,0x10);
			// must start with a read of block zero

			port->ReadBlock(1, data, s); 

			printf("%0d: %s\n",s,sky->toyName(data[0] + data[1] * 0x100));
		}
		delete sky;
		delete port;

	} catch (int e) {
		delete sky;
		delete port;
		if (e != 8)
			throw e;
	}

}

