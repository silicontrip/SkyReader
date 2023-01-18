//
//  mfrc522.hpp
//  editor
//
//  Created by Peter Lin on 1/15/23.
//

#ifndef mfrc522_h
#define mfrc522_h
#include "fileio.h"

#define SFD_UNAVAILABLE -1

class MFRC522 : public PortIO {
public:
	MFRC522() throw (int);
	~MFRC522();

	bool ReadBlock (unsigned char , unsigned char [0x10], int ) throw (int);
	void SetPortalColor(unsigned char , unsigned char , unsigned char ) throw(int);
	bool WriteBlock(unsigned char , unsigned char [0x10], int ) throw(int);
	void flash (void) throw(int);
	void setName(char *);

private:
	bool OpenPortalHandle() throw (int);
	int OpenPort(const char* portPath, int baudRate);
	bool SerialPortIsOpen();
	ssize_t readSerialData(unsigned char* const rxBuffer, size_t numBytesToReceive);
	ssize_t writeSerialData(const unsigned char* bytesToWrite, size_t numBytesToWrite);
	bool getPrompt(bool send);
	int sfd;
	char *serial;
};

#endif /* mfrc522_hpp */
