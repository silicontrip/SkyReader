#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "fileio.h"

#pragma warning(disable : 4996)  // disable warnings about fopen being deprecated.

unsigned char* ReadSkylanderFile(char *name)
{
	FILE *file;
	unsigned char *buffer = NULL;
	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s\n", name);
		return NULL;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	if(fileLen != 1024){
		fprintf(stderr, "Error. File %s must be 1024 bytes long.  "
			"Are you sure this is a skylander file?\n", name);
		return NULL;
	}

	//Allocate memory
	buffer=(unsigned char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Could not allocate memory to read file.\n");
		fclose(file);
		return NULL;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	return buffer;
}

bool WriteSkylanderFile(char *name, unsigned char *buffer){
	FILE *file;
	bool OK = true;
	int count;

	file = fopen(name, "wb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s for writing.\n", name);
		return false;
	}

	count = fwrite(buffer, 1024, 1, file);
	if(count < 1) {
		OK = false;
	}

	fclose(file);
	return OK;
}
