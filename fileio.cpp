
#include "fileio.h"

#define DEBUG 1

PortIO::~PortIO() { }

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
	serial = NULL;
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
	
#if DEBUG
printf(">>> SkylanderIO::initWithPortal\n");
#endif
	
	if (!sky) {
		//printf("Reading Skylander from portal.\n");
		ReadPortal(buffer,number);
		DecryptBuffer(buffer);
		sky = new Skylander(buffer);
		// printf("\nSkylander read from portal.\n");
		
	}
#if DEBUG
printf("<<< SkylanderIO::initWithPortal\n");
#endif
}

void SkylanderIO::ReadPortal(unsigned char *s, int number) throw (int) 
{
	
#if DEBUG
printf(">>> SkylanderIO::ReadPortal\n");
#endif
	unsigned char data[0x10]; 
	unsigned char *ptr;
	
	PortIO *port = NULL;
	if (serial) {
		port = new MFRC522();
		port->setName(serial);
	} else {
		port = new PortalIO();
	}
	
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
#if DEBUG
printf("<<< SkylanderIO::ReadPortal\n");
#endif
}	

bool SkylanderIO::writeSkylanderToPortal(int number) throw (int)
{
	bool bResult;
	bool bNewSkylander = false;
	unsigned char data[0x10]; 
	
	unsigned char old[1024];
	unsigned char skydata[1024];
	
	Crypt crypt;
	
	if (sky) {

		ReadPortal(old,number);
		old[0x36] = 0x0f;       // fix access control on first sector
		old[0x37] = 0x0f;
		old[0x38] = 0x0f;

		memcpy (skydata,sky->getData(),1024);
		
		if (sky->validateChecksum()) {
			printf("Encrypting Skylander\n");
			EncryptBuffer(skydata);
		}
		
		printf("\nWriting Skylander to portal.\n");

		PortIO *port = NULL;
		if (serial) {
			port = new MFRC522();
			port->setName(serial);
		} else {
			port = new PortalIO();
		}

		for(int i=0; i<2; i++) {
			// two pass write
			// write the access control blocks first
			bool selectAccessControlBlock;
			if(i == 0) {
				selectAccessControlBlock = 1;
			} else {
				selectAccessControlBlock = 0;
			}
			
			// write in reverse so block 0 is the last block written
			for(int block=0x3F; block >= 0; --block) {
				bool changed, OK;
				int offset = block * BLOCK_SIZE;
				if(crypt.IsAccessControlBlock(block) == selectAccessControlBlock) {
					changed = (memcmp(old + offset, skydata+offset,BLOCK_SIZE) != 0);
					if(changed) {
						port->WriteBlock( block, skydata+offset, bNewSkylander);
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
	return true;
}

bool SkylanderIO::writeSkylanderToEncryptedFile(char *name) throw (int)
{
	if (sky) {
		unsigned char skydata[1024];
		
		memcpy (skydata,sky->getData(),1024);
		EncryptBuffer(skydata);
		WriteSkylanderFile(name,skydata);
	}
	return true;
	
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

	PortIO *port = NULL;
	if (serial) {
		port = new MFRC522();
		port->setName(serial);
	} else {
		port = new PortalIO();
	}
	Skylander *sky;
	unsigned char data[0x10]; 

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

void SkylanderIO::setSerial(char *port) {
	serial = port;
}

