#include "skylander.h"

/*
 character data contents
 Even though there are two "data areas" (headers at blocks 0x08 and 0x24, data starts at blocks 0x09 and 0x25), some data is stored outside of the area, so here's a breakdown of the whole 1KB:
 
 Block   Block   Offset  Size     Description
 Area 0  Area 1          (bytes)
 0x00    N/A     0x00    0x02    Unique serial number for the toy.
 0x00    N/A     0x04    0x0E    Unknown.
 0x01    N/A     0x00    0x02    Identifier for the character/toy type. In the dump above, you can see it's 0E 00 (Little Endian), or 0x000E (Gill Grunt).
 0x01    N/A     0x04    0x08    Trading card ID.
 0x01    N/A     0x0C    0x02    Unknown. Zeroes for me.
 0x01    N/A     0x0E    0x02    Type 0 CRC16 checksum.
 0x08    0x24    0x00    0x03    24-bit experience/level value. Maximum unknown. Set this really high to max out the level.
 0x08    0x24    0x03    0x02    16-bit money value. Maximum 65000. Set it higher and the game rounds down to 65000.
 0x08    0x24    0x05    0x02    Unknown.
 0x08    0x24    0x07    0x02    Unknown. Zeroes for me.
 0x08    0x24    0x09    0x01    8-bit sequence value for this data area. I'm not totally sure how it works yet, but I think the area with the higher value is the "primary" one at the moment.
 0x08    0x24    0x0A    0x02    Type 3 CRC16 checksum.
 0x08    0x24    0x0C    0x02    Type 2 CRC16 checksum.
 0x08    0x24    0x0E    0x02    Type 1 CRC16 checksum.
 0x09    0x25    0x00    0x04    Skills given by Fairy. Bit 7 = path chosen. FD0F = Left, FF0F = Right
 0x09    0x25    0x01    0x02    Unknown. Zeroes for me.
 0x09    0x25    0x03    0x01    8-bit value, bitmap of platforms the character has touched. Bit 0 is the Wii and bit 1 is the Xbox 360, evidently.
 0x09    0x25    0x04    0x02    ID of hat the character is currently wearing.
 0x09    0x25    0x06    0x02    Unknown. Zeroes for me.
 0x09    0x25    0x08    0x08    Unknown. I've seen FF BF 1B 7F FF 2F B9 7E and FF 83 EE 7E FF 19 30 7F.
 0x0A    0x26    0x00    0x10    First half of Unicode name of character, zero-terminated, maximum 14 characters.
 0x0C    0x28    0x00    0x10    Second half of Unicode name of character, zero-terminated, maximum 14 characters.
 0x0D    0x29    0x00    0x0A    Unknown.
 0x0D    0x29    0x0A    0x02    16-bit hero points value. Maximum 100.
 0x0D    0x29    0x0C    0x03    Unknown. Zeroes for me.
 0x0D    0x29    0x0E    0x01    Unknown. 01 for me.
 0x10    0x2C    0x00    0x0C    Unknown. Zeroes for me.
 0x10    0x2C    0x0C    0x04    32 bit flag value indicating heroic challenges completed.
 
 */

void Skylander::fprinthex(FILE *f, unsigned char *c, unsigned int n) {
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

void Skylander::dump(void)
{
	fprinthex(stdout,data,SKYLANDER_SIZE);
}

Skylander::Skylander(unsigned char *in)
{
	initSkylander(in);
}

void Skylander::initSkylander(unsigned char *in)
{
	memcpy (data,in,SKYLANDER_SIZE);
	setAreaFromSequence();
	readName();
}	

unsigned char *Skylander::getData() { return data; }


unsigned char Skylander::getByte (int block, int offset) { return data[block * 16 + offset]; }
unsigned short Skylander::getShort (int block, int offset) { return data[block * 16 + offset] + data[block * 16 + offset + 1] * 0x100; }
void Skylander::setByte(int block, int offset, unsigned char b) { data[block * 16 + offset] = b; }
void Skylander::setShort(int block, int offset, unsigned short b) 
{ 
	data[block * 16 + offset] = b & 0xff;
	data[block * 16 + offset + 1] = (b & 0xff00) / 0x100;

}


void Skylander::readName()
{

	int block = getBlockNumberForArea() + 2;
	for (int i=0; i<15; i++)
	{
		int offset = (i * 2) & 0xf; 
		if (i == 8) { block += 2; }
		
		unsigned short utf = getShort(block,offset);
	
		name[i] = utf & 0xff;
		
	}
	
}

void Skylander::setAreaFromSequence() 
{
	area = 0;
	if (getArea0Sequence() < getArea1Sequence()) { area = 1;}
}
int Skylander::getBlockNumberForArea() { return area * 28 + 8; }

// should validate for 0 or 1
void Skylander::setArea(int a) {  if (a == 1 || a == 0) {area = a;} }
int Skylander::getArea() { return area; }
unsigned short Skylander::getSerial() { return getShort(0,0); }

const char * Skylander::toyName(int toy) {
	
	switch (toy) {
		case kTfbSpyroTag_ToyType_Character_Bash: return "Bash";
		case kTfbSpyroTag_ToyType_Character_Boomer: return "Boomer";
		case kTfbSpyroTag_ToyType_Character_Camo: return "Camo";
		case kTfbSpyroTag_ToyType_Character_ChopChop: return "ChopChop";
		case kTfbSpyroTag_ToyType_Character_Cynder: return "Cynder";
		case kTfbSpyroTag_ToyType_Character_DarkSpyro: return "DarkSpyro";
		case kTfbSpyroTag_ToyType_Character_DinoRang: return "DinoRang";
		case kTfbSpyroTag_ToyType_Character_DoubleTrouble: return "DoubleTrouble";
		case kTfbSpyroTag_ToyType_Character_DrillSergeant: return "DrillSergeant";
		case kTfbSpyroTag_ToyType_Character_Drobot: return "Drobot";
		case kTfbSpyroTag_ToyType_Character_Eruptor: return "Eruptor";
		case kTfbSpyroTag_ToyType_Character_Flameslinger: return "Flameslinger";
		case kTfbSpyroTag_ToyType_Character_GhostRoaster: return "GhostRoaster";
		case kTfbSpyroTag_ToyType_Character_GillGrunt: return "GillGrunt";
		case kTfbSpyroTag_ToyType_Character_Hex: return "Hex";
		case kTfbSpyroTag_ToyType_Character_Ignitor: return "Ignitor";
		case kTfbSpyroTag_ToyType_Character_LightningRod: return "LightningRod";
		case kTfbSpyroTag_ToyType_Character_PrismBreak: return "PrismBreak";
		case kTfbSpyroTag_ToyType_Character_SlamBam: return "SlamBam";
		case kTfbSpyroTag_ToyType_Character_SonicBoom: return "SonicBoom";
		case kTfbSpyroTag_ToyType_Character_Spyro: return "Spyro";
		case kTfbSpyroTag_ToyType_Character_StealthElf: return "StealthElf";
		case kTfbSpyroTag_ToyType_Character_StumpSmash: return "StumpSmash";
		case kTfbSpyroTag_ToyType_Character_Sunburn: return "Sunburn";
		case kTfbSpyroTag_ToyType_Character_Terrafin: return "Terrafin";
		case kTfbSpyroTag_ToyType_Character_TriggerHappy: return "TriggerHappy";
		case kTfbSpyroTag_ToyType_Character_Voodood: return "Voodood";
		case kTfbSpyroTag_ToyType_Character_Warnado: return "Warnado";
		case kTfbSpyroTag_ToyType_Character_WhamShell: return "WhamShell";
		case kTfbSpyroTag_ToyType_Character_Whirlwind: return "Whirlwind";
		case kTfbSpyroTag_ToyType_Character_WreckingBall: return "WreckingBall";
		case kTfbSpyroTag_ToyType_Character_Zap: return "Zap";
		case kTfbSpyroTag_ToyType_Character_Zook: return "Zook";
		case kTfbSpyroTag_ToyType_Expansion_Dragon: return "Dragon";
		case kTfbSpyroTag_ToyType_Expansion_Ice: return "Ice";
		case kTfbSpyroTag_ToyType_Expansion_Pirate: return "Pirate";
		case kTfbSpyroTag_ToyType_Expansion_PVPUnlock: return "PVPUnlock";
		case kTfbSpyroTag_ToyType_Expansion_Undead: return "Undead";
		case kTfbSpyroTag_ToyType_Item_Anvil: return "Anvil";
		case kTfbSpyroTag_ToyType_Item_CrossedSwords: return "CrossedSwords";
		case kTfbSpyroTag_ToyType_Item_Hourglass: return "Hourglass";
		case kTfbSpyroTag_ToyType_Item_Regeneration: return "Regeneration";
		case kTfbSpyroTag_ToyType_Item_SecretStash: return "SecretStash";
		case kTfbSpyroTag_ToyType_Item_Shield: return "Shield";
		case kTfbSpyroTag_ToyType_Item_Sparx: return "Sparx";
		case kTfbSpyroTag_ToyType_Item_SpeedBoots: return "SpeedBoots";
		case kTfbSpyroTag_ToyType_LEGENDARY: return "LEGENDARY";
		case kTfbSpyroTag_ToyType_Legendary_Bash: return "Bash";
		case kTfbSpyroTag_ToyType_Legendary_ChopChop: return "ChopChop";
		case kTfbSpyroTag_ToyType_Legendary_Spyro: return "Spyro";
		case kTfbSpyroTag_ToyType_Legendary_TriggerHappy: return "TriggerHappy";
		case kTfbSpyroTag_ToyType_PET: return "PET";
		case kTfbSpyroTag_ToyType_Pet_GillGrunt: return "GillGrunt";
		case kTfbSpyroTag_ToyType_Pet_StealthElf: return "StealthElf";
		case kTfbSpyroTag_ToyType_Pet_Terrafin: return "Terrafin";
		case kTfbSpyroTag_ToyType_Pet_TriggerHappy: return "TriggerHappy";
		default: return "UNKNOWN";
	}
}	

unsigned short Skylander::getToyType() { return getShort(1,0);	 }
const char * Skylander::getToyTypeName() { return toyName(getToyType()); }

unsigned char * Skylander::getTradingID()
{
	return data + 20;	
}

unsigned int Skylander::getXP()
{	
	int block = getBlockNumberForArea();	
	return getByte(block,0) + getByte(block,1) * 0x100 + getByte(block,2) * 0x10000;
}

void Skylander::setXP(unsigned int xp)
{

	if (xp < 0x1000000 )
	{
		int block = getBlockNumberForArea();	

		setByte(block,0, xp & 0xff);
		setByte(block,1, (xp & 0xff00) / 0x100);
		setByte(block,2, (xp & 0xff0000) / 0x10000);
	}
	
}


unsigned short Skylander::getMoney()
{
	int block = getBlockNumberForArea();
	return getShort(block, 3);
}

void Skylander::setMoney(unsigned short money)
{
	int block = getBlockNumberForArea();
	setShort(block,3,money);
}

// apparently the largest here gives the area.
unsigned char Skylander::getArea0Sequence() { return getByte(8, 9); }
unsigned char Skylander::getArea1Sequence() { return getByte(24, 9); }

unsigned short Skylander::getSkill() {
	int block = getBlockNumberForArea();
	return getShort(block+1, 0);
}

void Skylander::setSkillLeft(unsigned short skill) 
{

	skill = skill | 0x2;
	setSkill(skill);
	
}

void Skylander::setSkillRight(unsigned short skill) 
{
	
	skill = skill & 0xfffd;
	setSkill(skill);
	
}

	
void Skylander::setSkill(unsigned short skill) 
{
	int block = getBlockNumberForArea();
	setShort(block+1, 0, skill);
}

unsigned char Skylander::getPlatform() 
{
	int block = getBlockNumberForArea();
	return getByte(block+1, 3);
}

const char * Skylander::getPlatformName()
{

	if ( getPlatform() & 1 == 1) return "Wii";
	if ( getPlatform() & 2 == 2) return "Xbox 360";

	return "UNKNOWN";
	
}

unsigned short Skylander::getHat()
{
	int block = getBlockNumberForArea();
	return getShort(block+1, 4);
}

void Skylander::setHat(unsigned short hat)
{
	int block = getBlockNumberForArea();
	return setShort(block+1, 4,hat);
}


char * Skylander::getName()
{
	return name;
}

	
unsigned short Skylander::getHeroPoints()
{
	int block = getBlockNumberForArea();
	return getShort(block+5, 0xA);
}

void Skylander::setHeroPoints(unsigned short hp)
{
	int block = getBlockNumberForArea() + 5;
	setShort(block,0xA,hp);
}

unsigned int Skylander::getHeroicChallenges() 
{
	int block = getBlockNumberForArea() + 8;
	return getByte(block , 0xc) + getByte(block , 0xd) * 0x100  + getByte(block , 0xe) * 0x10000 + getByte(block , 0xf) * 0x1000000;
}


void Skylander::setHeroicChallenges(unsigned int hc) 
{
	int block = getBlockNumberForArea() + 8 ;
	setByte(block , 0xc, hc & 0xff);
	setByte(block , 0xd, (hc & 0xff00) / 0x100);
	setByte(block , 0xe, (hc & 0xff0000) / 0x10000);
	setByte(block , 0xf, (hc & 0xff000000) / 0x1000000);
}


void Skylander::UpdateBuf( int block, int offset, int size, unsigned char val)
{
	unsigned char *ptr = data;
	ptr += block * 0x10 + offset;
	for(int i=0; i<size; i++) {
		ptr[i] = val;
	}
}


bool Skylander::validateChecksum()
{
	// want to put the checksum files into a class.
	return crc.ValidateAllChecksums(data,false);
}

void Skylander::computeChecksum()
{
	// want to put the checksum files into a class.
	 crc.ValidateAllChecksums(data,true);
}

	
// Write bytes to buffer
static void UpdateBuf(unsigned char *buffer, int block, int offset, int size, unsigned char val)
{
	unsigned char *ptr = buffer;
	ptr += block * 0x10 + offset;
	for(int i=0; i<size; i++) {
		ptr[i] = val;
	}
}


void Skylander::MaxXP()
{
	UpdateBuf( 0x08, 0x00, 0x03, 0xFF);  // Max XP, data area 0
	UpdateBuf( 0x24, 0x00, 0x03, 0xFF);  // Max XP, data area 1
}

void Skylander::MaxMoney()
{
	UpdateBuf( 0x08, 0x03, 0x02, 0xFF);  // Max money, data area 0
	UpdateBuf( 0x24, 0x03, 0x02, 0xFF);  // Max money, data area 1
}

void Skylander::MaxHeroPoints()
{
	UpdateBuf( 0x0D, 0x0A, 0x01, 0x64);  // Max hero points, data area 0
	UpdateBuf( 0x29, 0x0A, 0x01, 0x64);  // Max hero points, data area 1
}

void Skylander::MaxHeroicChallenges()
{
	UpdateBuf( 0x0D, 0x06, 0x04, 0xFF);  // Mark all heroic challenges as complete, data area 0
	UpdateBuf( 0x29, 0x06, 0x04, 0xFF);  // Mark all heroic challenges as complete, data area 1
}

void Skylander::MaxSkills(unsigned char path)
{
	
	if (path == 0xFF || path == 0xFD) 
	{
		// Upgrade all skills, data area 0
		UpdateBuf( 0x09, 0x00, 0x01, path);
		UpdateBuf( 0x09, 0x01, 0x01, 0x0F);  
	
		// Upgrade all skills, data area 1
		UpdateBuf( 0x25, 0x00, 0x01, path);  
		UpdateBuf( 0x25, 0x01, 0x01, 0x0F); 
	}
}	
	
	
// Update Skylander character with common upgrades
void MaxStats(unsigned char *buffer, char skill_path) {
	unsigned char path;

	//  UpdateBuf(buffer, block, offset, size, val)

	UpdateBuf(buffer, 0x08, 0x00, 0x03, 0xFF);  // Max XP, data area 0
	UpdateBuf(buffer, 0x24, 0x00, 0x03, 0xFF);  // Max XP, data area 1

	UpdateBuf(buffer, 0x08, 0x03, 0x02, 0xFF);  // Max money, data area 0
	UpdateBuf(buffer, 0x24, 0x03, 0x02, 0xFF);  // Max money, data area 1

	UpdateBuf(buffer, 0x0D, 0x0A, 0x01, 0x64);  // Max hero points, data area 0
	UpdateBuf(buffer, 0x29, 0x0A, 0x01, 0x64);  // Max hero points, data area 1

	UpdateBuf(buffer, 0x0D, 0x06, 0x04, 0xFF);  // Mark all heroic challenges as complete, data area 0
	UpdateBuf(buffer, 0x29, 0x06, 0x04, 0xFF);  // Mark all heroic challenges as complete, data area 1

	if(skill_path != 0) {
		if(skill_path == '-') {
			// Choose left skills upgrade path
			path = 0xFD;
		} else {
			// Choose right skills upgrade path
			path = 0xFF;
		}
	    
		// Upgrade all skills, data area 0
		UpdateBuf(buffer, 0x09, 0x00, 0x01, path);
		UpdateBuf(buffer, 0x09, 0x01, 0x01, 0x0F);  

		// Upgrade all skills, data area 1
		UpdateBuf(buffer, 0x25, 0x00, 0x01, path);  
		UpdateBuf(buffer, 0x25, 0x01, 0x01, 0x0F); 
	}
}
