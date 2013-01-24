#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "checksum.h"
#include "crypt.h"
#include "fileio.h"
#include "portalio.h"
#include "skylander.h"

// Version 2.0

void usage();

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
		   "d: dump the data of a skylander to the display.\n"
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

void fprinthex(FILE *f, unsigned char *c, unsigned int n) {
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


int main(int argc, char* argv[])
{
	unsigned char *buffer, *original_data;
	bool OK, OK2;
	
	SkylanderIO *skio;
	Checksum crc;
	
	if (argc<2)
	{
		usage();
		return 1;
	}
	
	try {
	
	switch(*argv[1]) {
		case 'd':
			
			if(argc !=3) {
				usage();
				return 1;
			}			
			
			skio = new SkylanderIO();
			
			skio->initWithUnencryptedFile(argv[2]);
			if(! skio->getSkylander()->validateChecksum()) {
				fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  File may be corrupt.\n");
			}

			Skylander * sky ;
			sky = skio->getSkylander() ;
			
			printf("Serial Number: %X\n",sky->getSerial());
			printf("Toy Type: %s\n",sky->getToyTypeName());
			printf ("trading ID: ");
			fprinthex(stdout,sky->getTradingID(),8);
			
			printf("Area 0 sequence: %d\n",sky->getArea0Sequence());
			printf("Area 1 sequence: %d\n",sky->getArea1Sequence());

			printf("Area %d selected.\n",sky->getArea());
			printf("Experience: %d\n",sky->getXP());
			printf("Money: %d\n",sky->getMoney());

			printf("Skills: %X\n",sky->getSkill());
			printf("Platforms: %s\n",sky->getPlatformName());
			printf("Name: %s\n",sky->getName());

			printf("Hat: %d\n",sky->getHat());
			printf("Hero Points: %d\n",sky->getHeroPoints());

			printf("Heroic Challenges: %x\n",sky->getHeroicChallenges());
			
			delete skio;
			
			break;
		case 'r':
			// read from portal

			if(argc !=3) {
				usage();
				return 1;
			}
			
			skio = new SkylanderIO();

			skio->initWithPortal();

			// DecryptBuffer(buffer);
			if(! skio->getSkylander()->validateChecksum()) {
				fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  File may be corrupt.\n");
			}
			
			skio->writeSkylanderToUnencryptedFile(argv[2]);
			
			delete skio;
			break;

		case 'w':
			// write to portal

			if(argc !=3) {
				usage();
				return 1;
			}

			skio = new SkylanderIO();
			skio->initWithUnencryptedFile(argv[2]);
			skio->getSkylander()->computeChecksum();
			
			skio->writeSkylanderToPortal();
			
			delete skio;
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
			
			skio = new SkylanderIO();
			
			skio -> initWithPortal();
			
			if(! skio->getSkylander()->validateChecksum()) {
				fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  File may be corrupt.\n");
				return 1;
			}
			
			skio->writeSkylanderToUnencryptedFile(filename);

			if (skill_path == '-')
				skio->getSkylander()->MaxSkills(0xFF);
			else 
				skio->getSkylander()->MaxSkills(0xFD);

			skio->getSkylander()->computeChecksum();
			skio->writeSkylanderToPortal();
			
			delete skio;
			
			break;
		default:
			usage();
			return 1;
	}

	printf("Success!\n");
	return 0;
		
	} catch (int e) {
	
		switch (e) {
			case 1: printf ("Cannot open File\n"); break;
			case 2: printf ("Invalid Skylander File\n"); break;
			case 3: printf ("Cannot write to File\n"); break;
			case 4: printf ("Unable to get USB Device List\n"); break;
			case 5: printf ("Cannot Find Portal USB\n"); break;
			case 6: printf ("Unable to write to Portal\n"); break;
			case 7: printf ("Invalid Skylander Block\n"); break;
			case 8: printf ("Unable to read from Portal\n"); break;
			case 9: printf ("Wireless portal not connected\n"); break;
			case 10: printf ("Skylander Write Verify Error\n"); break;
			default: printf ("Unknown exception\n"); break;
		}
		
	}
}
