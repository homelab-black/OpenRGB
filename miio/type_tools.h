#pragma once

#include <cstdint>

int Bcd2Stamp(int y1, int y2, int m, int d, int H, int M, int S);
int Hex2Ascii(const char* hex, unsigned char* ascii);

void Char2Int32LittleEndian(const char* bytes, int position, int* num);
void CharToUint16BigEndian(const unsigned char* bytes, uint16_t* num);
void CharToUint32BigEndian(const unsigned char* bytes, uint32_t* num);
void Uint16ToCharBigEndian(const uint16_t* num, unsigned char* bytes);
void Uint32ToCharBigEndian(const uint32_t *num, unsigned char *bytes);
