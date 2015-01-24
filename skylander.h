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
	kTfbSpyroTag_ToyType_GIANT = 100,
	kTfbSpyroTag_ToyType_Giant_JetVac = 100,
	kTfbSpyroTag_ToyType_Giant_Swarm = 101,
	kTfbSpyroTag_ToyType_Giant_Crusher = 102,
	kTfbSpyroTag_ToyType_Giant_Flashwing = 103,
	kTfbSpyroTag_ToyType_Giant_HotHead = 104,
	kTfbSpyroTag_ToyType_Giant_HotDog = 105,
	kTfbSpyroTag_ToyType_Giant_Chill = 106,
	kTfbSpyroTag_ToyType_Giant_Thumpback = 107,
	kTfbSpyroTag_ToyType_Giant_PopFizz = 108,
	kTfbSpyroTag_ToyType_Giant_NinJini = 109,
	kTfbSpyroTag_ToyType_Giant_Bouncer = 110,
	kTfbSpyroTag_ToyType_Giant_Sprocket = 111,
	kTfbSpyroTag_ToyType_Giant_TreeRex = 112,
	kTfbSpyroTag_ToyType_Giant_Shroomboom = 113,
	kTfbSpyroTag_ToyType_Giant_EyeBroawl = 114,
	kTfbSpyroTag_ToyType_Giant_FrightRider = 115,
	kTfbSpyroTag_ToyType_EXPANSION = 300,
	kTfbSpyroTag_ToyType_Expansion_Dragon = 300,
	kTfbSpyroTag_ToyType_Expansion_Ice = 0x12d,
	kTfbSpyroTag_ToyType_Expansion_Pirate = 0x12e,
	kTfbSpyroTag_ToyType_Expansion_PVPUnlock = 0x130,
	kTfbSpyroTag_ToyType_Expansion_Undead = 0x12f,
	kTfbSpyroTag_ToyType_Expansion_MirrorOfMystery = 305,
	kTfbSpyroTag_ToyType_Expansion_NightmareExpress = 306,
	kTfbSpyroTag_ToyType_Expansion_SunscraperSpire = 307,
	kTfbSpyroTag_ToyType_Expansion_MidnightMuseum = 308,
	kTfbSpyroTag_ToyType_ITEM = 200,
	kTfbSpyroTag_ToyType_Item_Anvil = 200,
	kTfbSpyroTag_ToyType_Item_CrossedSwords = 0xcb,
	kTfbSpyroTag_ToyType_Item_Hourglass = 0xcc,
	kTfbSpyroTag_ToyType_Item_Regeneration = 0xca,
	kTfbSpyroTag_ToyType_Item_SecretStash = 0xc9,
	kTfbSpyroTag_ToyType_Item_Shield = 0xcd,
	kTfbSpyroTag_ToyType_Item_Sparx = 0xcf,
	kTfbSpyroTag_ToyType_Item_SpeedBoots = 0xce,
	kTfbSpyroTag_ToyType_Item_Cannon = 208,
	kTfbSpyroTag_ToyType_Item_ScorpionStriker = 209,
	kTfbSpyroTag_ToyType_Item_MagicTrap = 210,
	kTfbSpyroTag_ToyType_Item_WaterTrap = 211,
	kTfbSpyroTag_ToyType_Item_AirTrap = 212,
	kTfbSpyroTag_ToyType_Item_TechTrap = 214,
	kTfbSpyroTag_ToyType_Item_FireTrap = 215,
	kTfbSpyroTag_ToyType_Item_EarthTrap = 216,
	kTfbSpyroTag_ToyType_Item_LifeTrap = 217,
	kTfbSpyroTag_ToyType_Item_DarkTrap = 218,
	kTfbSpyroTag_ToyType_Item_LightTrap = 219,
	kTfbSpyroTag_ToyType_Item_KaosTrap = 220,
	kTfbSpyroTag_ToyType_Item_HandOfFate = 230,
	kTfbSpyroTag_ToyType_Item_PiggyBank = 231,
	kTfbSpyroTag_ToyType_Item_RocketRam = 232,
	kTfbSpyroTag_ToyType_Item_TikiSpeaky = 233,
	kTfbSpyroTag_ToyType_LEGENDARY = 400,
	kTfbSpyroTag_ToyType_Legendary_Bash = 0x194,
	kTfbSpyroTag_ToyType_Legendary_ChopChop = 430,
	kTfbSpyroTag_ToyType_Legendary_Spyro = 0x1a0,
	kTfbSpyroTag_ToyType_Legendary_TriggerHappy = 0x1a3,
	kTfbSpyroTag_ToyType_TRAPTEAM = 450,
	kTfbSpyroTag_ToyType_TrapTeam_Gusto = 450,
	kTfbSpyroTag_ToyType_TrapTeam_Thunderbolt = 451,
	kTfbSpyroTag_ToyType_TrapTeam_FlingKong = 452,
	kTfbSpyroTag_ToyType_TrapTeam_Blades = 453,
	kTfbSpyroTag_ToyType_TrapTeam_Wallop = 454,
	kTfbSpyroTag_ToyType_TrapTeam_HeadRush = 455,
	kTfbSpyroTag_ToyType_TrapTeam_FistBump = 456,
	kTfbSpyroTag_ToyType_TrapTeam_RockyRoll = 457,
	kTfbSpyroTag_ToyType_TrapTeam_Wildfire = 458,
	kTfbSpyroTag_ToyType_TrapTeam_KaBoom = 459,
	kTfbSpyroTag_ToyType_TrapTeam_TrailBlazer = 460,
	kTfbSpyroTag_ToyType_TrapTeam_Torch = 461,
	kTfbSpyroTag_ToyType_TrapTeam_SnapShot = 462,
	kTfbSpyroTag_ToyType_TrapTeam_LobStar = 463,
	kTfbSpyroTag_ToyType_TrapTeam_FlipWreck = 464,
	kTfbSpyroTag_ToyType_TrapTeam_Echo = 465,
	kTfbSpyroTag_ToyType_TrapTeam_Blastermind = 466,
	kTfbSpyroTag_ToyType_TrapTeam_Enigma = 467,
	kTfbSpyroTag_ToyType_TrapTeam_DejaVu = 468,
	kTfbSpyroTag_ToyType_TrapTeam_CobraCadabra = 469,
	kTfbSpyroTag_ToyType_TrapTeam_Jawbreaker = 470,
	kTfbSpyroTag_ToyType_TrapTeam_Gearshift = 471,
	kTfbSpyroTag_ToyType_TrapTeam_Chopper = 472,
	kTfbSpyroTag_ToyType_TrapTeam_TreadHead = 473,
	kTfbSpyroTag_ToyType_TrapTeam_Bushwhack = 474,
	kTfbSpyroTag_ToyType_TrapTeam_TuffLuck = 475,
	kTfbSpyroTag_ToyType_TrapTeam_FoodFight = 476,
	kTfbSpyroTag_ToyType_TrapTeam_HighFive = 477,
	kTfbSpyroTag_ToyType_TrapTeam_KryptKing = 478,
	kTfbSpyroTag_ToyType_TrapTeam_ShortCut = 479,
	kTfbSpyroTag_ToyType_TrapTeam_BatSpin = 480,
	kTfbSpyroTag_ToyType_TrapTeam_FunnyBone = 481,
	kTfbSpyroTag_ToyType_TrapTeam_KnightLight = 482,
	kTfbSpyroTag_ToyType_TrapTeam_Spotlight = 483,
	kTfbSpyroTag_ToyType_TrapTeam_KnightMare = 484,
	kTfbSpyroTag_ToyType_TrapTeam_Blackout = 485,
	kTfbSpyroTag_ToyType_PET = 500,
	kTfbSpyroTag_ToyType_Pet_GillGrunt = 0x202,
	kTfbSpyroTag_ToyType_Pet_StealthElf = 0x20e,
	kTfbSpyroTag_ToyType_Pet_Terrafin = 0x1f9,
	kTfbSpyroTag_ToyType_Pet_TriggerHappy = 0x207,
	kTfbSpyroTag_ToyType_Pet_Bop = 502,
	kTfbSpyroTag_ToyType_Pet_Spry = 503,
	kTfbSpyroTag_ToyType_Pet_Hijinx = 504,
	kTfbSpyroTag_ToyType_Pet_Breeze = 506,
	kTfbSpyroTag_ToyType_Pet_Weeruptor = 507,
	kTfbSpyroTag_ToyType_Pet_PetVac = 508,
	kTfbSpyroTag_ToyType_Pet_SmallFry = 509,
	kTfbSpyroTag_ToyType_Pet_Drobit = 510,
	kTfbSpyroTag_ToyType_Pet_Barkley = 540,
	kTfbSpyroTag_ToyType_Pet_Thumpling = 541,
	kTfbSpyroTag_ToyType_Pet_MiniJini = 542,
	kTfbSpyroTag_ToyType_Pet_EyeSmall = 543,
	kTfbSpyroTag_ToyType_SWAPForce = 3000,
	kTfbSpyroTag_ToyType_SWAPForce_Scratch = 3000,
	kTfbSpyroTag_ToyType_SWAPForce_PopThorn = 3001,
	kTfbSpyroTag_ToyType_SWAPForce_SlobberTooth = 3002,
	kTfbSpyroTag_ToyType_SWAPForce_Scorp = 3003,
	kTfbSpyroTag_ToyType_SWAPForce_HogWildFryno = 3004,
	kTfbSpyroTag_ToyType_SWAPForce_Smolderdash = 3005,
	kTfbSpyroTag_ToyType_SWAPForce_BumbleBlast = 3006,
	kTfbSpyroTag_ToyType_SWAPForce_ZooLou = 3007,
	kTfbSpyroTag_ToyType_SWAPForce_DuneBug = 3008,
	kTfbSpyroTag_ToyType_SWAPForce_StarStrike = 3009,
	kTfbSpyroTag_ToyType_SWAPForce_Countdown = 3010,
	kTfbSpyroTag_ToyType_SWAPForce_WindUp = 3011,
	kTfbSpyroTag_ToyType_SWAPForce_RollerBrawl = 3012,
	kTfbSpyroTag_ToyType_SWAPForce_GrimCreeper = 3013,
	kTfbSpyroTag_ToyType_SWAPForce_RipTide = 3014,
	kTfbSpyroTag_ToyType_SWAPForce_PunkShock = 3015,
	kTfbSpyroTag_ToyType_UNKNOWN = -1
} ; 

} ;

