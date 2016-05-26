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
	unsigned long getSerial();
	unsigned short getToyType();
	const char * getToyTypeName();
	const char * toyName(unsigned short);
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
	const char * getPath();
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

} ;
