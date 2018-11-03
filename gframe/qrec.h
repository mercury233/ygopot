#ifndef QREC_H
#define QREC_H

#include "config.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "replay.h"
#include "qre.h"
#include "../ocgcore/mtrandom.h"
#include <string>

namespace ygo {

class QreConverter {
private:
	static long pduel;
	static bool is_continuing;
	static bool is_closing;
	static bool is_pausing;
	static bool is_paused;
	static bool exit_pending;
	static wchar_t event_string[256];
	static std::wstring name;
public:
	static Replay cur_replay;
	
	static bool is_converting;
public:
	static bool StartReplay(const std::wstring &fileName);
	static void StopReplay(bool is_exiting = false);
	static void Pause(bool is_pause, bool is_step);
	static bool ReadReplayResponse();
	static int ReplayThread(void* param);
	static bool ReplayAnalyze(char* msg, unsigned int len);
	
	static void ReplayRefresh(int flag = 0x781fff);
	static void ReplayRefreshHand(int player, int flag = 0x781fff);
	static void ReplayRefreshGrave(int player, int flag = 0x181fff);
	static void ReplayRefreshDeck(int player, int flag = 0x181fff);
	static void ReplayRefreshExtra(int player, int flag = 0x181fff);
	static void ReplayRefreshSingle(int player, int location, int sequence, int flag = 0x781fff);

	static int MessageHandler(long fduel, int type);
};

}

#endif //REPLAY_MODE_H
