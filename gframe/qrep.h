#ifndef QREP_H
#define QREP_H

#include "config.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "qre.h"
#include "../ocgcore/mtrandom.h"
#include <fstream>
#include <string>

namespace ygo {

class QreMode {
private:
	static bool is_continuing;
	static bool is_closing;
	static bool is_pausing;
	static bool is_paused;
	static bool is_swaping;
	static bool exit_pending;
	static int skip_turn;
	static wchar_t event_string[256];
public:
	static bool StartReplay(int skipturn);
	static void StopReplay(bool is_exiting = false);
	static void SwapField();
	static void Pause(bool is_pause, bool is_step);
	static int ReplayThread(void* param);
	static void openReplay(const std::wstring&);
};

}

#endif //REPLAY_MODE_H