#include "data_manager.h"
#include <stdio.h>
#include <fstream>

//new
int bufferGetline(char linebuff[],int buffsize,char** streambuff)
{
	int i=-1;
	while(++i<buffsize)
	{
		if ( **streambuff==13 && *((*streambuff)+1)==10)
		{
			linebuff[i]=0;
			*streambuff+=2;
			break;
		}else{
			linebuff[i]=*((*streambuff)++);
		}
	}
	return i;
}

namespace ygo {

const wchar_t* DataManager::unknown_string = L"???";
wchar_t DataManager::strBuffer[2048];
DataManager dataManager;
byte DataManager::buffer[0x10000];

bool DataManager::LoadDB(const char* file) {
	//change
	sqlite3* pDB;
	if(sqlite3_open(file, &pDB) != SQLITE_OK)
		return Error(pDB);
	sqlite3_stmt* pStmt;
	const char* sql = "select * from datas,texts where datas.id=texts.id";
	if(sqlite3_prepare_v2(pDB, sql, -1, &pStmt, 0) != SQLITE_OK)
		return Error(pDB);
	CardDataC cd;
	CardString cs;
	for(int i = 0; i < 16; ++i) cs.desc[i] = 0;
	int step = 0, len = 0;
	do {
		step = sqlite3_step(pStmt);
		if(step == SQLITE_BUSY || step == SQLITE_ERROR || step == SQLITE_MISUSE)
			return Error(pDB, pStmt);
		else if(step == SQLITE_ROW) {
			cd.code = sqlite3_column_int(pStmt, 0);
			cd.ot = sqlite3_column_int(pStmt, 1);
			cd.alias = sqlite3_column_int(pStmt, 2);
			cd.setcode = sqlite3_column_int64(pStmt, 3);
			cd.type = sqlite3_column_int(pStmt, 4);
			cd.attack = sqlite3_column_int(pStmt, 5);
			cd.defence = sqlite3_column_int(pStmt, 6);
			unsigned int level = sqlite3_column_int(pStmt, 7);
			cd.level = level & 0xff;
			cd.lscale = (level >> 24) & 0xff;
			cd.rscale = (level >> 16) & 0xff;
			cd.race = sqlite3_column_int(pStmt, 8);
			cd.attribute = sqlite3_column_int(pStmt, 9);
			cd.category = sqlite3_column_int(pStmt, 10);
			_datas.insert(std::make_pair(cd.code, cd));
			len = BufferIO::DecodeUTF8((const char*)sqlite3_column_text(pStmt, 12), strBuffer);
			if(len) {
				cs.name = new wchar_t[len + 1];
				memcpy(cs.name, strBuffer, (len + 1)*sizeof(wchar_t));
			} else {
				cs.name = new wchar_t[1];
				cs.name[0] = 0;
			}
			len = BufferIO::DecodeUTF8((const char*)sqlite3_column_text(pStmt, 13), strBuffer);
			if(len) {
				cs.text = new wchar_t[len + 1];
				memcpy(cs.text, strBuffer, (len + 1)*sizeof(wchar_t));
			} else {
				cs.text = new wchar_t[1];
				cs.text[0] = 0;
			}
			for(int i = 14; i < 30; ++i) {
				len = BufferIO::DecodeUTF8((const char*)sqlite3_column_text(pStmt, i), strBuffer);
				if(len) {
					cs.desc[i - 14] = new wchar_t[len + 1];
					memcpy(cs.desc[i - 14], strBuffer, (len + 1)*sizeof(wchar_t));
				} else{
				cs.desc[i - 14] = new wchar_t[1];
				cs.desc[i - 14][0] = 0;
				}
			}
			_strings.insert(std::make_pair(cd.code, cs));
		}
	} while(step != SQLITE_DONE);
	sqlite3_finalize(pStmt);
	sqlite3_close(pDB);
	return true;
}
bool DataManager::LoadStrings(const char* file) {
	FILE* fp = fopen(file, "r");
	if(!fp)
		return false;
	for(int i = 0; i < 2048; ++i)
		_sysStrings[i] = 0;
	char linebuf[256];
	char strbuf[256];
	int value;
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fgets(linebuf, 256, fp);
	while(ftell(fp) < fsize) {
		fgets(linebuf, 256, fp);
		if(linebuf[0] != '!')
			continue;
		sscanf(linebuf, "!%s", strbuf);
		if(!strcmp(strbuf, "system")) {
			sscanf(&linebuf[7], "%d %240[^\n]", &value, strbuf);
			int len = BufferIO::DecodeUTF8(strbuf, strBuffer);
			wchar_t* pbuf = new wchar_t[len + 1];
			wcscpy(pbuf, strBuffer);
			_sysStrings[value] = pbuf;
		} else if(!strcmp(strbuf, "victory")) {
			sscanf(&linebuf[8], "%x %240[^\n]", &value, strbuf);
			int len = BufferIO::DecodeUTF8(strbuf, strBuffer);
			wchar_t* pbuf = new wchar_t[len + 1];
			wcscpy(pbuf, strBuffer);
			_victoryStrings[value] = pbuf;
		} else if(!strcmp(strbuf, "counter")) {
			sscanf(&linebuf[8], "%x %240[^\n]", &value, strbuf);
			int len = BufferIO::DecodeUTF8(strbuf, strBuffer);
			wchar_t* pbuf = new wchar_t[len + 1];
			wcscpy(pbuf, strBuffer);
			_counterStrings[value] = pbuf;
		}
	}
	fclose(fp);
	for(int i = 0; i < 255; ++i)
		myswprintf(numStrings[i], L"%d", i);
	//new
	for(int i = 0; i < 2048; ++i)
		intraStrings[i] = (wchar_t *)unknown_string;
	/*HRSRC hRes = ::FindResource(NULL ,"intra","TXT");
	HGLOBAL hgRes = LoadResource(NULL, hRes);
	LPVOID pRes = LockResource(hgRes);
	char* streambuff=(char*)pRes;
	bufferGetline(linebuf,256,&streambuff);
	while( bufferGetline(linebuf,256,&streambuff) )
	{
		if(linebuf[0] != '!')
			continue;
		sscanf(linebuf, "!%s", strbuf);
		if(!strcmp(strbuf, "string")) {
			sscanf(&linebuf[7], "%d %s", &value, strbuf);
			int len = BufferIO::DecodeUTF8(strbuf, strBuffer);
			wchar_t* pbuf = new wchar_t[len + 1];
			wcscpy(pbuf, strBuffer);
			intraStrings[value] = pbuf;
		}
	}*/
	//

	std::wifstream scConf("setcode.conf");
	scConf.imbue(std::locale("chs"));
	if(scConf.is_open())
	{
		while(!scConf.eof()) {
			std::wstring line;
			std::getline(scConf, line);

			if(line.find(L"0x") != 0)
				continue;
			int sep = line.find_first_of(' ');
			int setcode = wcstol(line.c_str() + 2, NULL, 16);
			std::wstring name = line.substr(sep + 1);
			setcodes.push_back(std::make_pair(setcode, name));
		}
	}

	std::wifstream swConf("summonwords.conf");
	swConf.imbue(std::locale("chs"));
	if (swConf.is_open())
	{
		while (!swConf.eof())
		{
			unsigned int code = 0;
			std::wstring line;
			swConf >> code;
			swConf.get();
			std::getline(swConf, line);
			summonwords[code] = line;
		}
	}
	return true;
}

//new
const wchar_t* DataManager::GetIntraStrings(int code) {
	if(code < 0 || code >= 2048)
		return unknown_string;
	return intraStrings[code];
}

bool DataManager::Error(sqlite3* pDB, sqlite3_stmt* pStmt) {
	BufferIO::DecodeUTF8(sqlite3_errmsg(pDB), strBuffer);
	if(pStmt)
		sqlite3_finalize(pStmt);
	sqlite3_close(pDB);
	return false;
}
bool DataManager::GetData(int code, CardData* pData) {
	auto cdit = _datas.find(code);
	if(cdit == _datas.end())
		return false;
	if(pData)
		*pData = *((CardData*)&cdit->second);
	return true;
}
code_pointer DataManager::GetCodePointer(int code) {
	return _datas.find(code);
}
bool DataManager::GetString(int code, CardString* pStr) {
	auto csit = _strings.find(code);
	if(csit == _strings.end()) {
		pStr->name = (wchar_t*)unknown_string;
		pStr->text = (wchar_t*)unknown_string;
		return false;
	}
	*pStr = csit->second;
	return true;
}
const wchar_t* DataManager::GetName(int code) {
	auto csit = _strings.find(code);
	if(csit == _strings.end())
		return unknown_string;
	if(csit->second.name)
		return csit->second.name;
	return unknown_string;
}
const wchar_t* DataManager::GetText(int code) {
	auto csit = _strings.find(code);
	if(csit == _strings.end())
		return unknown_string;
	if(csit->second.text)
		return csit->second.text;
	return unknown_string;
}
const wchar_t* DataManager::GetDesc(int strCode) {
	if(strCode < 10000)
		return GetSysString(strCode);
	int code = strCode >> 4;
	int offset = strCode & 0xf;
	auto csit = _strings.find(code);
	if(csit == _strings.end())
		return unknown_string;
	if(csit->second.desc[offset])
		return csit->second.desc[offset];
	return unknown_string;
}
const wchar_t* DataManager::GetSysString(int code) {
	if(code < 0 || code >= 2048 || _sysStrings[code] == 0)
		return unknown_string;
	return _sysStrings[code];
}
const wchar_t* DataManager::GetVictoryString(int code) {
	auto csit = _victoryStrings.find(code);
	if(csit == _victoryStrings.end())
		return unknown_string;
	return csit->second;
}
const wchar_t* DataManager::GetCounterName(int code) {
	auto csit = _counterStrings.find(code);
	if(csit == _counterStrings.end())
		return unknown_string;
	return csit->second;
}
const wchar_t* DataManager::GetNumString(int num, bool bracket) {
	if(!bracket)
		return numStrings[num];
	wchar_t* p = numBuffer;
	*p++ = L'(';
	BufferIO::CopyWStrRef(numStrings[num], p, 4);
	*p = L')';
	*++p = 0;
	return numBuffer;
}

const wchar_t* DataManager::FormatLocation(int location, int sequence) {
	if(location == 0x8) {
		if(sequence < 5)
			return GetSysString(1003);
		else if(sequence == 5)
			return GetSysString(1008);
		else
			return GetSysString(1009);
	}
	int filter = 1, i = 1000;
	while(filter != location) {
		filter <<= 1;
		i++;
	}
	if(filter == location)
		return GetSysString(i);
	else
		return unknown_string;
}
const wchar_t* DataManager::FormatAttribute(int attribute) {
	wchar_t* p = attBuffer;
	int filter = 1, i = 1010;
	for(; filter != 0x80; filter <<= 1, ++i) {
		if(attribute & filter) {
			BufferIO::CopyWStrRef(GetSysString(i), p, 16);
			*p = L'|';
			*++p = 0;
		}
	}
	if(p != attBuffer)
		*(p - 1) = 0;
	else
		return unknown_string;
	return attBuffer;
}
const wchar_t* DataManager::FormatRace(int race) {
	wchar_t* p = racBuffer;
	int filter = 1, i = 1020;
	for(; filter != 0x1000000; filter <<= 1, ++i) {
		if(race & filter) {
			BufferIO::CopyWStrRef(GetSysString(i), p, 16);
			*p = L'|';
			*++p = 0;
		}
	}
	if(p != racBuffer)
		*(p - 1) = 0;
	else
		return unknown_string;
	return racBuffer;
}
const wchar_t* DataManager::FormatType(int type) {
	wchar_t* p = tpBuffer;
	int filter = 1, i = 1050;
	for(; filter != 0x2000000; filter <<= 1, ++i) {
		if(type & filter) {
			BufferIO::CopyWStrRef(GetSysString(i), p, 16);
			*p = L'|';
			*++p = 0;
		}
	}
	if(p != tpBuffer)
		*(p - 1) = 0;
	else
		return unknown_string;
	return tpBuffer;
}
int DataManager::CardReader(int code, void* pData) {
	if(!dataManager.GetData(code, (CardData*)pData))
		memset(pData, 0, sizeof(CardData));
	return 0;
}
}