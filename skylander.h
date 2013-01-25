#pragma once

#include <string.h>
#include "crypt.h"
#include "checksum.h"

void MaxStats(unsigned char *buffer, char skill_path);


class Skylander {
	#define SKYLANDER_SIZE 1024
	
	unsigned char data[SKYLANDER_SIZE];
	char name[16];
	int area;
	Checksum crc;
	
public:
	
	Skylander (unsigned char *);
	void initSkylander(unsigned char *);
	unsigned char *getData(); 
	unsigned short getSerial();
	unsigned short getToyType();
	const char * getToyTypeName();
	const char * toyName(int);
	unsigned char * getTradingID();
	unsigned int getXP();
	void setXP(unsigned int);
	unsigned short  getMoney();
	void setMoney(unsigned short);
	unsigned char getArea0Sequence();
	unsigned char getArea1Sequence();
	unsigned short getSkill();
	void setSkillLeft(unsigned short);
	void setSkillRight(unsigned short);
	void setSkill(unsigned short);
	unsigned char getPlatform();
	const char * getPlatformName();
	unsigned short getHat();
	void setHat(unsigned short);
	char * getName();
	unsigned short getHeroPoints();
	void setHeroPoints(unsigned short);
	unsigned int getHeroicChallenges();
	void setHeroicChallenges(unsigned int);

	void setAreaFromSequence();
	
	int getArea();
	void setArea(int);
	
	bool validateChecksum();
	void computeChecksum();

	void UpdateBuf( int , int , int , unsigned char );
	void MaxXP();
	void MaxMoney();
	void MaxHeroPoints();
	void MaxHeroicChallenges();
	void MaxSkills(unsigned char);
	
	void dump(void);

	
private:
	
	unsigned char getByte(int, int);
	unsigned short getShort(int,int);
	void setByte(int,int,unsigned char);
	void setShort(int, int, unsigned short); 

	int getBlockNumberForArea();
	void readName();
	void fprinthex(FILE *, unsigned char *, unsigned int) ;

	
	
enum SpyroToyType
{
	kTfbSpyroTag_ToyType_CHARACTER = 0,
	kTfbSpyroTag_ToyType_Character_Bash = 4,
	kTfbSpyroTag_ToyType_Character_Boomer = 0x16,
	kTfbSpyroTag_ToyType_Character_Camo = 0x18,
	kTfbSpyroTag_ToyType_Character_ChopChop = 30,
	kTfbSpyroTag_ToyType_Character_Cynder = 0x20,
	kTfbSpyroTag_ToyType_Character_DarkSpyro = 0x1c,
	kTfbSpyroTag_ToyType_Character_DinoRang = 6,
	kTfbSpyroTag_ToyType_Character_DoubleTrouble = 0x12,
	kTfbSpyroTag_ToyType_Character_DrillSergeant = 0x15,
	kTfbSpyroTag_ToyType_Character_Drobot = 20,
	kTfbSpyroTag_ToyType_Character_Eruptor = 9,
	kTfbSpyroTag_ToyType_Character_Flameslinger = 11,
	kTfbSpyroTag_ToyType_Character_GhostRoaster = 0x1f,
	kTfbSpyroTag_ToyType_Character_GillGrunt = 14,
	kTfbSpyroTag_ToyType_Character_Hex = 0x1d,
	kTfbSpyroTag_ToyType_Character_Ignitor = 10,
	kTfbSpyroTag_ToyType_Character_LightningRod = 3,
	kTfbSpyroTag_ToyType_Character_PrismBreak = 7,
	kTfbSpyroTag_ToyType_Character_SlamBam = 15,
	kTfbSpyroTag_ToyType_Character_SonicBoom = 1,
	kTfbSpyroTag_ToyType_Character_Spyro = 0x10,
	kTfbSpyroTag_ToyType_Character_StealthElf = 0x1a,
	kTfbSpyroTag_ToyType_Character_StumpSmash = 0x1b,
	kTfbSpyroTag_ToyType_Character_Sunburn = 8,
	kTfbSpyroTag_ToyType_Character_Terrafin = 5,
	kTfbSpyroTag_ToyType_Character_TriggerHappy = 0x13,
	kTfbSpyroTag_ToyType_Character_Voodood = 0x11,
	kTfbSpyroTag_ToyType_Character_Warnado = 2,
	kTfbSpyroTag_ToyType_Character_WhamShell = 13,
	kTfbSpyroTag_ToyType_Character_Whirlwind = 0,
	kTfbSpyroTag_ToyType_Character_WreckingBall = 0x17,
	kTfbSpyroTag_ToyType_Character_Zap = 12,
	kTfbSpyroTag_ToyType_Character_Zook = 0x19,
	kTfbSpyroTag_ToyType_EXPANSION = 300,
	kTfbSpyroTag_ToyType_Expansion_Dragon = 300,
	kTfbSpyroTag_ToyType_Expansion_Ice = 0x12d,
	kTfbSpyroTag_ToyType_Expansion_Pirate = 0x12e,
	kTfbSpyroTag_ToyType_Expansion_PVPUnlock = 0x130,
	kTfbSpyroTag_ToyType_Expansion_Undead = 0x12f,
	kTfbSpyroTag_ToyType_ITEM = 200,
	kTfbSpyroTag_ToyType_Item_Anvil = 200,
	kTfbSpyroTag_ToyType_Item_CrossedSwords = 0xcb,
	kTfbSpyroTag_ToyType_Item_Hourglass = 0xcc,
	kTfbSpyroTag_ToyType_Item_Regeneration = 0xca,
	kTfbSpyroTag_ToyType_Item_SecretStash = 0xc9,
	kTfbSpyroTag_ToyType_Item_Shield = 0xcd,
	kTfbSpyroTag_ToyType_Item_Sparx = 0xcf,
	kTfbSpyroTag_ToyType_Item_SpeedBoots = 0xce,
	kTfbSpyroTag_ToyType_LEGENDARY = 400,
	kTfbSpyroTag_ToyType_Legendary_Bash = 0x194,
	kTfbSpyroTag_ToyType_Legendary_ChopChop = 430,
	kTfbSpyroTag_ToyType_Legendary_Spyro = 0x1a0,
	kTfbSpyroTag_ToyType_Legendary_TriggerHappy = 0x1a3,
	kTfbSpyroTag_ToyType_PET = 500,
	kTfbSpyroTag_ToyType_Pet_GillGrunt = 0x202,
	kTfbSpyroTag_ToyType_Pet_StealthElf = 0x20e,
	kTfbSpyroTag_ToyType_Pet_Terrafin = 0x1f9,
	kTfbSpyroTag_ToyType_Pet_TriggerHappy = 0x207,
	kTfbSpyroTag_ToyType_UNKNOWN = -1
} ; 

} ;
