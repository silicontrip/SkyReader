#pragma once

int IsAccessControlBlock(unsigned int blockIndex);
void EncryptTagBlock(unsigned char* blockData, unsigned int blockIndex, unsigned char const* tagBlocks0and1);
void DecryptTagBlock(unsigned char* blockData, unsigned int blockIndex, unsigned char const* tagBlocks0and1);
void EncryptBuffer(unsigned char* buffer);
void DecryptBuffer(unsigned char* buffer);