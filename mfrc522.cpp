//
//  mfrc522.cpp
//  editor
//
//  Created by Peter Lin on 1/15/23.
//

#include <stdio.h>
#include <termios.h>
#include <unistd.h> //write(), read(), close()
#include <fcntl.h>
#include <errno.h> //errno
#include <cstring>
#include "mfrc522.h"

//Returns -1 on failure, with errno set appropriately
ssize_t MFRC522::writeSerialData(const unsigned char* bytesToWrite, size_t numBytesToWrite) {
	ssize_t numBytesWritten = write(sfd, bytesToWrite, numBytesToWrite);
	if (numBytesWritten < 0) {
		printf("Serial port write() failed. Error: %s", strerror(errno));
	}
	return numBytesWritten;
}

//Returns -1 on failure, with errno set appropriately
ssize_t MFRC522::readSerialData(unsigned char* const rxBuffer, size_t numBytesToReceive) {
	ssize_t numBytesRead = 0;
	while (numBytesRead < numBytesToReceive) {
		int l = read(sfd, &rxBuffer[numBytesRead], numBytesToReceive - numBytesRead);
		if (l < 0) {
			printf("Serial port read() failed. Error:%s", strerror(errno));
			break;
		}
		if (l > 0)
			numBytesRead += l;
	}
	return numBytesRead;
}

/*
 * Flush the serial buffer until we get a prompt, optionally
 * send a linefeed to trigger a prompt.
 */
bool MFRC522::getPrompt(bool send) {
	unsigned char out[1];
	unsigned char in[1];
	bool found = false;
	if (send) {
		out[0] = 0x0a;
		writeSerialData(out, 1);
	}
	for (int i = 0; !found && i < 200; i++) {
		if (readSerialData(in, 1) == 1) {
//      printf("%c", data[0]);
			if (in[0] == '$') {
				found = true;
			}
		}
	}
	return found;
}

bool MFRC522::ReadBlock (unsigned char block, unsigned char data[0x10], int skylander) throw(int) {
	// read block from serial port
	if (skylander >= 0) printf("%02X: ", block);
	if (SerialPortIsOpen() || OpenPortalHandle()) {
		unsigned char request[2];
		if (getPrompt(true)) {
			request[0] = 'R';                              // ReadBlock
			request[1] = '0' + block;
			if (writeSerialData(request, 2) == 2) {
				unsigned char buffer[36];
				memset(buffer, 0x0, 36);
				if (readSerialData(buffer, 1) == 1) {
					if (buffer[0] == 'X') {
						printf("No card.\n");
						return false;
					}
				}
				if (readSerialData(&buffer[1], 34) == 34) {     // 32 ascii bytes
					unsigned char c = 0;                      // convert hex string to byte array
					for (int i=0; i<32; i++) {
						c |= buffer[i] - ((buffer[i] >= 'A') ? '7' : '0');
						if (i & 1) {
							data[(i >> 1)] = c;
							if (skylander >= 0) printf("%02X", c);
							c = 0;
						}
						c <<= 4;
					}
					//printf(" %02x %c\n", buffer[34], buffer[34]);
				}
				if (skylander >= 0) printf("\n");
				return true;
			}
		}
	}
	printf("failed.\n");
	return false;
}

void MFRC522::SetPortalColor(unsigned char r, unsigned char g, unsigned char b) throw(int) {
	// does nothing
}

bool MFRC522::WriteBlock(unsigned char block, unsigned char data[0x10], int skylander) throw(int) {
	// write block to serial port
	if (SerialPortIsOpen() || OpenPortalHandle()) {
		unsigned char request[2];
		if (getPrompt(true)) {
			request[0] = 'W';                                 // WriteBlock (use lowecase for testing)
			request[1] = '0' + block;
			if (writeSerialData(request, 2) == 2) {
				unsigned char buffer[36];
				if (readSerialData(buffer, 3) == 3) {
					printf("%c%c%c ", buffer[0], buffer[1], buffer[2]);
					if (buffer[2] == ':') {
						if (writeSerialData(data, 16) == 16) {
							if (readSerialData(buffer, 35) == 35) {   // 32 + nl + lf + result
								for (int i=0; i<34; i++) {
									printf("%c", buffer[i]);
								}
							}
							//printf("result: %c\n", buffer[34]);
							return (buffer[34] == 'O');
						}
					}
				}
			}
		}
	}
	printf("failed.\n");
	return false;
}

void MFRC522::flash (void) throw(int) {
	// does nothing
}

void MFRC522::setName(char *name) {
	serial = name;
}

bool MFRC522::OpenPortalHandle() throw(int) {
	int d = OpenPort(serial, B115200);   // use standard speeds
	if (d > 0) {
		sfd = d;
		// flush the serial port until we get a prompt
		unsigned char data[16];
		if (!getPrompt(false)) {
			getPrompt(true);
		}
		if (ReadBlock(0, data, -1)) {
			return true;
		} else {
			throw 8;
		}
	}
	throw 5;
}

int MFRC522::OpenPort(const char* portPath, int baudRate) {
	if (SerialPortIsOpen()) {
		close(sfd);
	}

	// open serial port
	// Open port, checking for errors
	sfd = open(portPath, (O_RDWR | O_NOCTTY | O_NDELAY));
	if (sfd == SFD_UNAVAILABLE) {
		printf("Unable to open serial port: %s at baud rate: %d\n", portPath, baudRate);
		return sfd;
	}

	//Configure i/o baud rate settings
	struct termios options;
	tcgetattr(sfd, &options);
	cfsetispeed(&options, baudRate);
	cfsetospeed(&options, baudRate);

	//Configure other settings
	//Settings from:
	//  https://github.com/Marzac/rs232/blob/master/rs232-linux.c
	//

	options.c_iflag &= ~(INLCR | ICRNL);
	options.c_iflag |= IGNPAR | IGNBRK;
	options.c_oflag &= ~(OPOST | ONLCR | OCRNL);
	options.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
	options.c_cflag |= CLOCAL | CREAD | CS8;
	options.c_lflag &= ~(ICANON | ISIG | ECHO);
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN]  = 0;

	//Apply settings
	//TCSANOW vs TCSAFLUSH? Was using TCSAFLUSH; settings source above
	//uses TCSANOW.
	if (tcsetattr(sfd, TCSANOW, &options) < 0) {
		printf("Error setting serial port attributes.\n");
		close(sfd);
		return -2; //Using negative value; -1 used above for different failure
	}

	//printf("Serial port opened: %d\n", sfd);
	return sfd;
}

bool MFRC522::SerialPortIsOpen() {
	return sfd != SFD_UNAVAILABLE;
}

MFRC522::MFRC522() throw(int) {
	sfd = SFD_UNAVAILABLE;
}

MFRC522::~MFRC522() {
	if (SerialPortIsOpen()) {
		// close serial port
		close(sfd);
	}
}
