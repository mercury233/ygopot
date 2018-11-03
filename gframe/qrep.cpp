#include "qrep.h"
#include "duelclient.h"
#include "../ocgcore/field.h"
#include "game.h"
#include "lzma/LzmaLib.h"
#include <memory>

namespace ygo {

//long QreMode::pduel = 0;
//Replay ReplayMode::cur_replay;
bool QreMode::is_continuing = true;
bool QreMode::is_closing = false;
bool QreMode::is_pausing = false;
bool QreMode::is_paused = false;
bool QreMode::is_swaping = false;
bool QreMode::exit_pending = false;
int QreMode::skip_turn = 0;
wchar_t QreMode::event_string[256];
static std::wstring fileName;

void QreMode::openReplay(const std::wstring& name)
{
	fileName = L"./qre/" + name;
}

bool QreMode::StartReplay(int skipturn) {
	skip_turn = skipturn;
	is_qre = true;
	Thread::NewThread(ReplayThread, 0);
	return true;
}
void QreMode::StopReplay(bool is_exiting) {
	is_pausing = false;
	is_continuing = false;
	is_closing = is_exiting;
	exit_pending = true;
	mainGame->actionSignal.Set();
}
void QreMode::SwapField() {
	if(is_paused)
		mainGame->dField.ReplaySwap();
	else
		is_swaping = true;
}

void QreMode::Pause(bool is_pause, bool is_step) {
	if(is_pause)
		is_pausing = true;
	else {
		if(!is_step)
			is_pausing = false;
		mainGame->actionSignal.Set();
	}
}
//change
int QreMode::ReplayThread(void* param) {
	mainGame->dInfo.isStarted = true;
	//new
	mainGame->showcardcode=0;
	mainGame->dInfo.isReplay = true;

	std::list<std::pair<unsigned short, std::vector<char> > > stream;
	{
		std::ifstream input(fileName, std::ios_base::binary);
		unsigned short len = 0;
		char b = 0;
		char flag = input.get();

		if(flag & QRE_TAG)
		{
			input.read((char*)ygo::mainGame->dInfo.hostname, 40);
			input.read((char*)ygo::mainGame->dInfo.clientname, 40);
			input.read((char*)ygo::mainGame->dInfo.hostname_tag, 40);
			input.read((char*)ygo::mainGame->dInfo.clientname_tag, 40);
			mainGame->dInfo.isTag = true;
		}
		else
		{
			input.read((char*)ygo::mainGame->dInfo.hostname, 40);
			input.read((char*)ygo::mainGame->dInfo.clientname, 40);
		}

		if(flag & QRE_COMPRESSED)
		{
			CompHeader ch;
			input.read((char*)&ch, sizeof(ch));

			std::size_t replay_size = ch.data_size, comp_size = ch.comp_size;

			std::unique_ptr<unsigned char[]> replay_data(new unsigned char[ch.data_size]), comp_data(new unsigned char[ch.comp_size]);
			input.read((char*)comp_data.get(), ch.comp_size);

			if(LzmaUncompress(replay_data.get(), &replay_size, comp_data.get(), &comp_size, (unsigned char*)ch.props, 5) != SZ_OK)
				goto exit_tag;
			
			std::size_t offset = 0;
			unsigned char *data = replay_data.get();
			while(offset < replay_size)
			{
				len = *(reinterpret_cast<unsigned short*>(data + offset));
				offset += 2;
				if(len == 0)
					continue;
				std::vector<char> buf;
				buf.reserve(len);
				for(unsigned int i = 0; i < len; i++)
				{
					buf.push_back(data[offset++]);
				}
				stream.push_back(std::make_pair(len, std::move(buf)));
			}
		}
		else
		{
			while(!input.eof())
			{
				input.read((char*)&len, 2);
				if(len == 0)
					continue;
				std::vector<char> buf;
				buf.reserve(len);
				for(unsigned int i = 0; i < len; i++)
				{
					b = input.get();
					buf.push_back(b);
				}
				stream.push_back(std::make_pair(len, std::move(buf)));
			}
		}
	}

	is_continuing = true;
	exit_pending = false;
	if(skip_turn < 0)
		skip_turn = 0;
	if(skip_turn) {
		mainGame->dInfo.isReplaySkiping = true;
		mainGame->gMutex.Lock();
	} else
		mainGame->dInfo.isReplaySkiping = false;
	//int len = 0;
	while (is_continuing && !exit_pending && !Thread::退出进程) {
		/*int flag = result >> 16;*/
		//if (len > 0) {
			//get_message(pduel, (byte*)engineBuffer);
			//is_continuing = ReplayAnalyze(engineBuffer, len);
		//}
		
		if(stream.empty())
		{
			is_continuing = false;
		}
		else
		{
			if(is_closing) {
				is_continuing = false;
			}
			if(is_swaping) {
				mainGame->gMutex.Lock();
				mainGame->dField.ReplaySwap();
				mainGame->gMutex.Unlock();
				is_swaping = false;
			}
			unsigned short len = stream.front().first;
			char *pdata = stream.front().second.data();
			bool need_wait = true;
			if(*pdata == STOC_GAME_MSG && len > 1) {
				unsigned char msg = *(pdata + 1);
				if(msg == MSG_UPDATE_DATA ||
					msg == MSG_UPDATE_CARD ||
					msg == MSG_SET ||
					msg == MSG_FIELD_DISABLED ||
					msg == MSG_SUMMONING ||
					msg == MSG_SPSUMMONING ||
					msg == MSG_FLIPSUMMONING ||
					msg == MSG_CHAIN_SOLVING ||
					msg == MSG_CHAIN_SOLVED ||
					msg == MSG_CHAIN_END ||
					msg == MSG_RANDOM_SELECTED ||
					msg == MSG_EQUIP ||
					msg == MSG_UNEQUIP ||
					msg == MSG_CARD_TARGET ||
					msg == MSG_CANCEL_TARGET ||
					msg == MSG_BATTLE ||
					msg == MSG_ATTACK_DISABLED ||
					msg == MSG_DAMAGE_STEP_START ||
					msg == MSG_DAMAGE_STEP_END)
				{
					need_wait = false;
				}
				if((msg >= 10 && msg <= 25) || (msg >= 140 && msg <= 143))
				{
					need_wait = false;
					mainGame->waitFrame = 20;
					mainGame->gMutex.Lock();
					mainGame->gMutex.Unlock();
				}
				else
				{
					DuelClient::ClientAnalyze(pdata + 1, len - 1);
				}
			}
			stream.pop_front();
			if(is_pausing && need_wait) {
				is_paused = true;
				mainGame->actionSignal.Reset();
				mainGame->actionSignal.Wait();
				is_paused = false;
			}
		}
	}

exit_tag:
	is_qre = false;
	if(!Thread::退出进程){
	if(mainGame->dInfo.isReplaySkiping) {
		mainGame->dInfo.isReplaySkiping = false;
		mainGame->dField.RefreshAllCards();
		mainGame->gMutex.Unlock();
	}
	if(!is_closing) {
		mainGame->actionSignal.Reset();
		mainGame->gMutex.Lock();
		mainGame->stMessage->setText(dataManager.GetSysString(1501));
		if(mainGame->wCardSelect->isVisible())
			mainGame->HideElement(mainGame->wCardSelect);
		mainGame->PopupElement(mainGame->wMessage);
		mainGame->gMutex.Unlock();
		mainGame->actionSignal.Wait();
		mainGame->gMutex.Lock();
		mainGame->dInfo.isStarted = false;
		mainGame->dInfo.isReplay = false;
		mainGame->gMutex.Unlock();
		mainGame->closeDoneSignal.Reset();
		mainGame->closeSignal.Set();
		mainGame->closeDoneSignal.Wait();
		mainGame->gMutex.Lock();
		mainGame->ShowElement(mainGame->wReplay);
		mainGame->device->setEventReceiver(&mainGame->menuHandler);
		mainGame->gMutex.Unlock();
	}
	}
	Thread::进程数--;
	return 0;
}

}
