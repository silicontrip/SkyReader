#pragma once

bool validateChecksum(unsigned char *buffer, int type, int dataArea, bool overwrite);
bool ValidateAllChecksums(unsigned char *buffer, bool overwrite);
unsigned short ComputeCcittCrc16(void const* data, unsigned int bytes);