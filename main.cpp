#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "checksum.h"
#include "crypt.h"
#include "fileio.h"
#include "portalio.h"
#include "skylander.h"

// Version 2.0


void usage()
{
	printf("\n"
		"Usage:\n"
		"editor <option> <filename>\n"
		"\n"
		"<option>\n"
		"r: read skylader data from portal, save unecrypted data to <filename>.\n"
		"w: write skylader data to portal, read unencrypted <filename> and save\n"
		"   to skylander.\n"
		"u <optional skill path>: upgrade skylander stats.  <path> = - for left\n" 
		"   skill path, + for right skill path, empty = do not upgrade skills.\n"
		"   Original version saved as <filename>.\n"
		"\n"
		"Examples: \n"
		"editor u - dspyro.bak\n"
		"This would save a copy of the figurine to the file dspyro.bak then upgrade\n"
		"it using skills on the LEFT path seen in the character menu.\n"
		"\n"
		"editor u dspyro.bak\n"
		"Upgrade skylander, leave skills as is.\n"
		"\n"
		"editor r dspyro.bak\n"
		"Read figurine, decrypt and save to file dspyro.bak\n");
}


int main(int argc, char* argv[])
{
	unsigned char *buffer, *original_data;
	bool OK, OK2;
	
	switch(*argv[1]) {
		case 'r':
			// read from portal

			if(argc !=3) {
				usage();
				return 1;
			}
			
			buffer = ReadSkylanderFromPortal();
			if(!buffer) {
				fprintf(stderr, "Error. Could not read Skylander from portal.  Check portal connection and make sure "
					"you have installed the portal driver via spyrowebworldportaldriver.exe\n");
				return false;
			}

			DecryptBuffer(buffer);
			OK = ValidateAllChecksums(buffer, false);
			if(!OK) {
				fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  File may be corrupt.\n");
			}

			OK2 = WriteSkylanderFile(argv[2], buffer);
			OK = OK && OK2;

			if(!OK) {
				return 1;
			}

			break;

		case 'w':
			// write to portal

			if(argc !=3) {
				usage();
				return 1;
			}

			buffer = ReadSkylanderFile(argv[2]);  
			if(!buffer) {
				fprintf(stderr, "Error. Could not open Skylander file.\n");
				return 1;
			}

			ValidateAllChecksums(buffer, true);  // recompute checksums for file
			EncryptBuffer(buffer);

			OK = WriteSkylanderToPortal(buffer, NULL);
			if(!OK) {
				return 1;
			}
			
			break;

		case 'u':
			// update 
			if(argc !=4  && argc != 3) {
				usage();
				return 1;
			}

			char *filename;
			char skill_path;

			if(argc == 4) {
				skill_path = *argv[2];
				filename = argv[3];
				if(skill_path != '-' && skill_path != '+') {
					fprintf(stderr, "Error. When you run the upgrade command you must select an upgrade path. "
						"- is left, + is right, <empty> is leave skills unchanged.\n");
					usage();
					return false;
				}
			} else {
				skill_path = 0;
				filename = argv[2];
			}
			
			buffer = ReadSkylanderFromPortal();
			if(!buffer) {
				fprintf(stderr, "Error. Could not read Skylander from portal.  Check portal connection and make sure "
					"you have installed the portal driver via spyrowebworldportaldriver.exe\n");
				return false;
			}

			original_data=(unsigned char *)malloc(1024);
			if(!original_data) {
				fprintf(stderr, "Failed to allocate memory to store Skylander.  Aborting.\n");
				return 1;
			}
			memcpy(original_data, buffer, 1024);

			DecryptBuffer(buffer);
			OK = ValidateAllChecksums(buffer, false);
			if(!OK) {
				fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  Aborting.\n");
				return 1;
			}

			OK = WriteSkylanderFile(filename, buffer);
			if(!OK) {
				fprintf(stderr, "Could not save original version of Skylander to file.  Aborting.\n");
				return 1;
			}

			MaxStats(buffer, skill_path);
			ValidateAllChecksums(buffer, true);  // update checksums

			EncryptBuffer(buffer);

			OK = WriteSkylanderToPortal(buffer, original_data);
			if(!OK) {
				return 1;
			}

			break;
		default:
			usage();
			return 1;
	}

	printf("Success!\n");
	return 0;
}
