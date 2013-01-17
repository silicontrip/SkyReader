#include "skylander.h"

// Write bytes to buffer
static void UpdateBuf(unsigned char *buffer, int block, int offset, int size, unsigned char val)
{
	unsigned char *ptr = buffer;
	ptr += block * 0x10 + offset;
	for(int i=0; i<size; i++) {
		ptr[i] = val;
	}
}

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


#if 0
// Toy type identifier, see above for location in buffer.
public enum SpyroToyType
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
}

#endif