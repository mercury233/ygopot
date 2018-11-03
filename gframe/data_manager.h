#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "config.h"
#include "sqlite3.h"
#include "client_card.h"
#include <unordered_map>
#include <string>
#include <sstream>

namespace ygo {

class DataManager {
public:
	DataManager(): _datas(8192), _strings(8192) {}
	bool LoadDB(const char* file);
	bool LoadStrings(const char* file);
	bool Error(sqlite3* pDB, sqlite3_stmt* pStmt = 0);
	bool GetData(int code, CardData* pData);
	code_pointer GetCodePointer(int code);
	bool GetString(int code, CardString* pStr);
	const wchar_t* GetName(int code);
	const wchar_t* GetText(int code);
	const wchar_t* GetDesc(int strCode);
	const wchar_t* GetSysString(int code);
	const wchar_t* GetVictoryString(int code);
	const wchar_t* GetCounterName(int code);
	const wchar_t* GetNumString(int num, bool bracket = false);
	const wchar_t* FormatLocation(int location, int sequence);
	const wchar_t* FormatAttribute(int attribute);
	const wchar_t* FormatRace(int race);
	const wchar_t* FormatType(int type);

	std::unordered_map<unsigned int, CardDataC> _datas;
	std::unordered_map<unsigned int, CardString> _strings;
	std::unordered_map<unsigned int, wchar_t*> _counterStrings;
	std::unordered_map<unsigned int, wchar_t*> _victoryStrings;

	std::unordered_map<unsigned int, std::wstring> summonwords;
	std::list<std::pair<int, std::wstring>> setcodes;

	wchar_t* _sysStrings[2048];
	wchar_t numStrings[256][4];
	wchar_t numBuffer[6];
	wchar_t attBuffer[128];
	wchar_t racBuffer[128];
	wchar_t tpBuffer[128];
	//new
	wchar_t* intraStrings[2048];
	const wchar_t* GetIntraStrings(int code);
	static byte buffer[0x10000];

	static wchar_t strBuffer[2048];
	static const wchar_t* unknown_string;
	static int CardReader(int, void*);
	//new file system
	static irr::io::IFileSystem* fsys;

	static std::vector<std::wstring> listDir(const wchar_t* dirName, const wchar_t* fileName,bool subdir=false)
	{
		std::vector<std::wstring> ret;
		WIN32_FIND_DATAW fileInfo;
		std::wstring buff(dirName);
		buff.append(L"\\");
		buff.append(fileName);
		HANDLE handle = FindFirstFileW(buff.c_str(), &fileInfo);
		if (handle != INVALID_HANDLE_VALUE)
			do {
				if(!subdir || (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&(wcscmp(fileInfo.cFileName,L".") != 0) && (wcscmp(fileInfo.cFileName,L"..") != 0))
				{
					std::wstring buff2(dirName);
					buff2.append(L"\\");
					buff2.append(fileInfo.cFileName);
					ret.push_back(buff2);
				}
			} while (FindNextFileW(handle, &fileInfo));
		return std::move(ret);
	}

	static std::wstring randomGet(const wchar_t* dir, const wchar_t* shuffix) 
	{
		std::wstring pat(L"*.");
		pat.append(shuffix);
		std::vector<std::wstring> fileList = listDir(dir, pat.c_str());
		if(fileList.empty())
			return L"";
		return fileList[随机数.取正整数(fileList.size())];
	}

	static int getRange(const wchar_t* pstr,int& high){
		if(*pstr == 0){
			high = 0x7fffffff;
			return 0x80000000;
		}
		else if(*pstr == L'=') {
			high = BufferIO::GetVal(pstr + 1);
			return high;
		} else if(*pstr >= L'0' && *pstr <= L'9') {
			const wchar_t* pstrh=pstr+1;
			while(*pstrh >= L'0' && *pstrh <= L'9')
				pstrh++;
			if(*pstrh == L'-'){
				high = BufferIO::GetVal(pstrh + 1);
				return BufferIO::GetVal(pstr);
			}else
			{
				high = BufferIO::GetVal(pstr);
				return high;
			}
		} else if(*pstr == L'>') {
			high = 0x7fffffff;
			return BufferIO::GetVal(pstr + 1);
		} else if(*pstr == L'<') {
			high = BufferIO::GetVal(pstr + 1);
			return -0x7fffffff;
		} else if(*pstr == L'?') {
			high = -1;
			return 0x80000000;
		} else{
			high = 0x7fffffff;
			return 0x80000000;
		}
	}
	static byte* ygopot_script_reader(const char* script_name, int* slen){
		wchar_t file[256],fileex[256];
		if(script_name[0] == '.')
			script_name+=2;
		BufferIO::DecodeUTF8(script_name, file);

		IReadFile* scriptreader=NULL;
		std::vector<std::wstring> dirlist= listDir(L"expansions",L"*",true);
		int dircount = dirlist.size();
		while(--dircount>=0 && scriptreader==NULL){
			myswprintf(fileex, L"%ls\\%ls",dirlist[dircount].c_str(),file);
			scriptreader=fsys->createAndOpenFile(fileex);

		}
		if(scriptreader==NULL){
			scriptreader=fsys->createAndOpenFile(file);
		}
		if(scriptreader==NULL)
			return 0;
		uint32 len = scriptreader->getSize();
		if (len > 0xffff)
			return 0;
		buffer[len]=0;
		scriptreader->read((void*)buffer,len);
		*slen = len;
		return buffer;
	}
	
};

extern DataManager dataManager;

}

#endif // DATAMANAGER_H
