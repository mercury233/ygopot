#ifndef QRE_H
#define QRE_H

#include <vector>
#include <fstream>
#include <string>

extern std::vector<char> qreBuffer;
extern bool is_qre;
extern bool is_qre_recording;
void savePacket(char* data, int len);
void saveData(char* data, int len);
void saveQre(std::wstring name = L"");
void savePlayers();

typedef struct {
	int data_size;
	int comp_size;
	char props[8];
} CompHeader;

#define QRE_TAG	0x1
#define QRE_COMPRESSED 0x2

#endif