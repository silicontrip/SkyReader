#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <iostream> 
#include "checksum.h"
#include "crypt.h"
#include "fileio.h"
#include "portalio.h"
#include "skylander.h"

using namespace std;

void usage()
{
	printf("\n"
		   "Usage:\n"
		   "editor [-i <file>|-p] [-s <skylander>] [-d] [-e] [-o <file>|-P] [-M <money>] [-X experience] ... \n"
		   "\n"
		   "Reading/Writing:\n"
		   "-i <file>\tread skylander data from file, with option to decrypt the data.\n"
		   "-p\t\tread skylander data from portal and decrypt the data.\n"
		   "-K\t\tread skylander data from portal without decrypting\n"
		   "-s <skylander> select which skylander.\n"
		   "-d\t\tdecrypt the data read from the file.\n"
		   "-o <file>\twrite skylander data to <filename>.\n"
		   "-a\t\twrite skylander data to automatic filename.\n"
		   "-W\t\twrite skylander data without encrypting\n"
		   "-P\t\tencrypt and write skylander data to the portal.\n"
		   "-e\t\tencrypt data when writing file.\n"
		   "-D\t\tdump the data of a skylander to the display.\n"
		   "-l\t\tList skylanders on portal.\n"
		   
		   "\nUpgrade:\n"
		   "-M <money>\tupgrade skylander money (max 65,000).\n" 		
		   "-X <xp>\t\tupgrade skylander Experience (level 10 = 33,000).\n" 
		   "-H <hp>\t\tupgrade skylander Hero Points (max 100).\n" 
		   "-C <challenges>\tupgrade skylander challenges.\n"
		   "-L <points>\tupgrade the skillpoints left path. (0 = set path)\n"
		   "-R <points>\tupgrade the skillpoints right path. (0 = set path)\n"
		   "-c\t\tupdate checksums.\n"
		   "\n"
		   "Examples: \n"
		   "editor -p -o spyro.bin\n"
		   "This would save a copy of the figurine to the file dspyro.bak\n"
		   "editor -i spyro.bin -o spyro_upgrade.bin -L 65535 -M 65000 -X 33000 -H 100\n"
		   "upgrade spyro.bin using skills on the LEFT path seen in the character menu\n"
		   "and write it to file spyro_upgrade.bin\n"
		   "\n"
		   "editor -i spyro.bin -P -M 65000 -X 33000\n"
		   "Upgrade skylander, leave skills as is, and write to the portal.\n"
		   "\n"
		   "editor -i spyro.bin -P\n"
		   "Read file from spyro.bin and write it to the portal.\n");
}

void printquad (unsigned int i)
{
	
	printf("%02X ",i & 0xff);
	printf("%02X ",(i & 0xff00 )/ 0x100);
	printf("%02X ",(i & 0xff0000) / 0x10000);
	printf("%02X ",(i & 0xff000000) / 0x1000000);
	printf("\n");
	
}

int main(int argc, char* argv[])
{
	unsigned char *buffer, *original_data;
	bool OK, OK2;
	
	bool encrypt,decrypt,portalIn,portalOut,dump,upgrade,flash,list,autoFile,RawWrite,RawRead;
	
	char * inFile, *outFile;
	
	const static char *legal_flags = "alFWePpKcDo:i:dM:X:H:C:L:R:s:";
	
	encrypt = false;
	decrypt = false;
	portalIn = false;
	portalOut = false;
	upgrade = false;
	dump = false;
	inFile = NULL;
	outFile = NULL;
	flash = false;
	list = false;
	autoFile = false;
	RawWrite = false;
	RawRead = false;
	
	unsigned int money, xp, hp, challenges, skillleft, skillright,skylander_number;
	bool pathleft, pathright;
	
	money = 0;
	xp = 0;
	hp = 0;
	challenges = 0;
	skillleft = 0;
	pathleft = false;
	skillright = 0;
	pathright = false;
	skylander_number = 0;
	
	SkylanderIO *skio;
	Checksum crc;
	
	int k;
	
	while ((k = getopt (argc, argv, legal_flags)) != -1) {
		switch (k) {
			case 'e': encrypt = true; break;
			case 'd': decrypt = true; break;
			case 'P': portalOut = true; break;
			case 'p': portalIn = true; break;
			case 'K': RawRead = true; break;
			case 'D': dump = true; break;
			case 'F': flash = true; break;
			case 'W': RawWrite = true; break;
			case 'i': 
				inFile = new char[strlen(optarg)+1];
				strcpy(inFile,optarg);
				break;
			case 'o':
				outFile = new char[strlen(optarg)+1];
				strcpy(outFile,optarg);
				break;
			case 'a':
				autoFile = true;
				outFile = new char[65];
				strcpy(outFile,"TEMP");
				break;
			case 'M':
				money = atoi(optarg);
				upgrade = true;
				break;
			case 'X':
				xp = atoi(optarg);
				upgrade = true;
				break;
			case 'H':
				hp = atoi(optarg);
				upgrade = true;
				break;
			case 'C':
				challenges = atoi(optarg);
				upgrade = true;
				break;
			case 'L':
				skillleft = atoi(optarg);
				pathleft = true;
				upgrade = true;
				break;
			case 'R':
				skillright = atoi(optarg);
				pathright = true;
				upgrade = true;
				break;
			case 's':
				skylander_number = atoi(optarg);
				break;
			case 'c':
				upgrade = true;
				break;
			case 'l':
				list = true;
				break;
			default:
				usage () ;
				exit (0);
				
		}
	}
	
	try {
		
		// some entertainment.
		if (flash) {
			PortalIO  *pio ;
			pio = new PortalIO();
			
			pio->flash();
			exit (0);
		}
		
		
		if (list)  {
			printf ("Listing Skylanders.\n\n");

			skio = new SkylanderIO();
			skio->listSkylanders();
			
			exit (0);
		}
	
	// validate command line options
	if ( (!inFile && !portalIn && !RawRead) || (inFile && portalIn)) {
		printf ("Must Choose One of: read from file -i <file> or read from portal -p\n");
		usage();
		exit(0);
	}
	
	if (!outFile && !portalOut && !dump && !RawWrite) {
		printf ("Nothing to write. Choose file -o, portal -P or dump -D\n");
		usage();
		exit(0);
	}	
	
	if (portalIn && upgrade) {
		printf ("It is not recommended to upgrade directly from the portal (write to a file first)\n");
		usage();
		exit(0);
	}
	
	if (portalIn && portalOut) {
		printf ("It is not recommended to read and write directly from the portal (write to a file first)\n");
		usage();
		exit(0);
	}
	
		skio = new SkylanderIO();
		
		printf ("====================================================\n\n");
		printf ("Reading Skylander\n\n");
		
		if (portalIn) {
			skio->initWithPortal(skylander_number);
		}
		if (RawRead) {
			skio->initRawReadPortal(skylander_number);
		}
		
		if (inFile) {
			if (decrypt) {
				skio->initWithEncryptedFile(inFile);
			} else {
				skio->initWithUnencryptedFile(inFile);
			}
		}
		
		if(! skio->getSkylander()->validateChecksum()) {
			fprintf(stderr, "Warning. Skylander data read from portal, but checksums are incorrect.  File may be corrupt.\n");
		} 
		/*
		else {
			printf ("Skylander Checksum OK.\n");
		}
		*/
		if (dump) {
			Skylander * sky ;
			sky = skio->getSkylander() ;
			
			printf("Serial Number: %08lX\n",sky->getSerial());
			printf("Toy Type: %s (%d)\n\n",sky->getToyTypeName(),sky->getToyType());
			printf ("Trading ID: ");
			skio->fprinthex(stdout,sky->getTradingID(),8);
			
			//Debugging Use
			printf("Area 0 sequence: %d\n",sky->getArea0Sequence());
			printf("Area 1 sequence: %d\n",sky->getArea1Sequence());
			printf("Area %d selected (higher sequence)\n\n",sky->getArea());
			
			printf("Experience: %d\n",sky->getXP());
			printf("Money: %d\n",sky->getMoney());
			printf("Skills: %04X - %s\n",sky->getSkill(), sky->getPath());
			printf("Platforms: %s\n",sky->getPlatformName());
			printf("Nickname: %s\n",sky->getName());
			printf("Hat: %d\n",sky->getHat());
			printf("Hero Points: %d\n",sky->getHeroPoints());
			printf("Heroic Challenges: %08x\n",sky->getHeroicChallenges());
			printf("\n");
		}
		
		if (upgrade) {
			if (money) { skio->getSkylander()->setMoney(money); }
			if (xp) { skio->getSkylander()->setXP(xp); }
			if (hp) { skio->getSkylander()->setHeroPoints(hp); }
			if (challenges) { skio->getSkylander()->setHeroicChallenges(challenges); }
			if (pathleft) { skio->getSkylander()->setSkillLeft(skillleft); }
			if (pathright) { skio->getSkylander()->setSkillRight(skillright); }
			
			skio->getSkylander()->computeChecksum();
		}
		
		if (outFile || portalOut)  {
			printf ("Writing Skylander.\n");
		}
		if (outFile) {
			if (autoFile) {
				Skylander * sky;
				sky = skio->getSkylander();

				sprintf(outFile, "%s.bin", sky->getToyTypeName());
				printf("Saving to automatic filename: %s\n", outFile);
			}

			if (encrypt) {
				skio->writeSkylanderToEncryptedFile(outFile);
			} else {
				skio->writeSkylanderToUnencryptedFile(outFile);
			}
		}
		if (portalOut) {
			skio->writeSkylanderToPortal(skylander_number);
		}
		if (RawWrite) {
			skio->RawWriteSkylanderToPortal(skylander_number);
		}
		
		delete skio;
		
		
		printf("\nSuccess!\n\n");
		return 0;
		
	} catch (int e) {
		
		switch (e) {
			case 1: printf ("Cannot open File.\n"); break;
			case 2: printf ("Invalid Skylander File.\n"); break;
			case 3: printf ("Cannot write to File.\n"); break;
			case 4: printf ("Unable to get USB Device List.\n"); break;
			case 5: printf ("Cannot Find Portal USB.\n"); break;
			case 6: printf ("Unable to write to Portal.\n"); break;
			case 7: printf ("Invalid Skylander Block.\n"); break;
			case 8: printf ("Unable to read Skylander from Portal.\n"); break;
			case 9: printf ("Wireless portal not connected.\n"); break;
			case 10: printf ("Skylander Write Verify Error.\n"); break;
			case 11: printf ("No Skylander detected on portal.\n"); break;
			default: printf ("Unknown exception: %d.\n",e); break;
		}
		
	}
}
