#include "qre.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <direct.h>
#include "game.h"
#include "lzma/LzmaLib.h"
#include <memory>
#include <algorithm>

std::vector<char> qreBuffer;
bool is_qre = false;
bool is_qre_recording = false;

void saveData(char* data, int len)
{
	for(int i = 0; i < len; i++)
	{
		qreBuffer.push_back(data[i]);
	}
}

void savePlayers()
{
	is_qre_recording = true;
	char flag = 0;
	if(ygo::mainGame->dInfo.isTag)
	{
		flag |= QRE_TAG;
		qreBuffer.push_back(flag);
		saveData((char*)ygo::mainGame->dInfo.hostname, 40);
		saveData((char*)ygo::mainGame->dInfo.clientname, 40);
		saveData((char*)ygo::mainGame->dInfo.hostname_tag, 40);
		saveData((char*)ygo::mainGame->dInfo.clientname_tag, 40);
	}
	else
	{
		qreBuffer.push_back(flag);
		saveData((char*)ygo::mainGame->dInfo.hostname, 40);
		saveData((char*)ygo::mainGame->dInfo.clientname, 40);
	}
}

void savePacket(char* data, int len)
{
	qreBuffer.push_back(len % 256);
	qreBuffer.push_back(len / 256);
	for(int i = 0; i < len; i++)
	{
		qreBuffer.push_back(data[i]);
	}
}

static void saveData(std::vector<char> &buf, char *data, int len)
{
	for(int i = 0; i < len; i++)
	{
		buf.push_back(data[i]);
	}
}

static void QreCompress()
{
	std::vector<char> buffer;
	buffer.swap(qreBuffer);
	if(buffer.size() == 0)
		return;

	char flag = buffer[0];

	std::vector<char> newBuffer;
	newBuffer.push_back(flag | QRE_COMPRESSED);
	int offset = flag & QRE_TAG ? 160 : 80;
	saveData(newBuffer, buffer.data() + 1, offset);
	
	int bufSize = buffer.size() - offset;
	if(bufSize < 0x5000)
	{
		bufSize = 0x5000;
	}
	
	CompHeader ch;
	std::unique_ptr<unsigned char[]> comp_data(new unsigned char[bufSize]);
	std::size_t comp_size = bufSize, propsize = 8;
	LzmaCompress(comp_data.get(), &comp_size, (unsigned char*)buffer.data() + offset + 1,
		buffer.size() - offset - 1, (unsigned char*)ch.props, &propsize, 5, 1 << 24, 3, 0, 2, 32, 1);

	ch.data_size = buffer.size() - offset - 1;
	ch.comp_size = comp_size;
	saveData(newBuffer, (char*)&ch, sizeof(ch));
	saveData(newBuffer, (char*)comp_data.get(), comp_size);
	qreBuffer.swap(newBuffer);
}

void saveQre(std::wstring name)
{
	is_qre_recording = false;
	std::wstring tmpname;

	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	int year = timeinfo->tm_year + 1900;

	std::stringstream ss;
	ss << std::setfill('0') << std::setw(4) << year;;
	ss << std::setw(2) << 1 + timeinfo->tm_mon << std::setw(2) << timeinfo->tm_mday << "-";
	ss << std::setw(2) << timeinfo->tm_hour << std::setw(2) << timeinfo->tm_min << std::setw(2) << timeinfo->tm_sec << ".qre";

	_mkdir("qre");

	if(name.empty())
	{
		std::string str = "qre/" + ss.str();
		tmpname = std::wstring(str.begin(), str.end());
	}
	else 
	{	
		tmpname = L"qre/" + name + L".qre";
	}

	std::ofstream fs(tmpname, std::ios_base::binary);
	if(!fs.is_open())
	{
		return;
	}

	QreCompress();

	for(auto it = qreBuffer.begin(); it != qreBuffer.end(); ++it)
	{
		fs << *it;
	}
	qreBuffer.clear();
}