#include "qrec.h"
#include "duelclient.h"
#include "game.h"
#include "../ocgcore/duel.h"
#include "../ocgcore/field.h"
#include "../ocgcore/mtrandom.h"

namespace ygo {

long QreConverter::pduel = 0;
Replay QreConverter::cur_replay;
bool QreConverter::is_continuing = true;
bool QreConverter::is_closing = false;
bool QreConverter::is_pausing = false;
bool QreConverter::is_paused = false;
bool QreConverter::exit_pending = false;
wchar_t QreConverter::event_string[256];
std::wstring QreConverter::name;
bool QreConverter::is_converting = false;

static void QreCSavePacket(char* data, int len)
{
	len += 1;
	qreBuffer.push_back(len % 256);
	qreBuffer.push_back(len / 256);
	qreBuffer.push_back(STOC_GAME_MSG);
	saveData(data, len - 1);
}

bool QreConverter::StartReplay(const std::wstring &fileName) {
	if(!is_converting)
	{
		is_converting = true;
	}
	else
	{
		return false;
	}
	name = fileName.substr(0, fileName.length() - 4);
	Thread::NewThread(ReplayThread, 0);
	return true;
}

void QreConverter::StopReplay(bool is_exiting) {
	is_pausing = false;
	is_continuing = false;
	is_closing = is_exiting;
	exit_pending = true;
}

void QreConverter::Pause(bool is_pause, bool is_step) {
	if(is_pause)
		is_pausing = true;
	else {
		if(!is_step)
			is_pausing = false;
	}
}

bool QreConverter::ReadReplayResponse() {
	unsigned char resp[64];
	bool result = cur_replay.ReadNextResponse(resp);
	if(result)
		set_responseb(pduel, resp);
	return result;
}
//change
int QreConverter::ReplayThread(void* param) {
	qreBuffer.clear();
	ReplayHeader rh = cur_replay.pheader;
	mtrandom rnd;
	int seed = rh.seed;
	rnd.reset(seed);
	if(rh.flag & REPLAY_TAG) {
		qreBuffer.push_back(1);
		char hostname[40], clientname[40], clientname_tag[40], hostname_tag[40];
		cur_replay.ReadData(hostname, 40);
		cur_replay.ReadData(hostname_tag, 40);
		cur_replay.ReadData(clientname_tag, 40);
		cur_replay.ReadData(clientname, 40);
		saveData(hostname, 40);
		saveData(clientname, 40);
		saveData(clientname_tag, 40);
		saveData(hostname_tag, 40);
	} else {
		qreBuffer.push_back(0);
		char buf[40];
		cur_replay.ReadData(buf, 40);
		saveData(buf, 40);
		cur_replay.ReadData(buf, 40);
		saveData(buf, 40);
	}
	set_card_reader((card_reader)DataManager::CardReader);
	//new
	set_script_reader((script_reader)DataManager::ygopot_script_reader);
	set_message_handler((message_handler)MessageHandler);
	pduel = create_duel(rnd.rand());
	int start_lp = cur_replay.ReadInt32();
	int start_hand = cur_replay.ReadInt32();
	int draw_count = cur_replay.ReadInt32();
	int opt = cur_replay.ReadInt32();
	set_player_info(pduel, 0, start_lp, start_hand, draw_count);
	set_player_info(pduel, 1, start_lp, start_hand, draw_count);
	//mainGame->dInfo.lp[0] = start_lp;
	//mainGame->dInfo.lp[1] = start_lp;
	//myswprintf(mainGame->dInfo.strLP[0], L"%d", mainGame->dInfo.lp[0]);
	//myswprintf(mainGame->dInfo.strLP[1], L"%d", mainGame->dInfo.lp[1]);
	//mainGame->dInfo.turn = 0;
	//mainGame->dInfo.strTurn[0] = 0;
	int main0, main1, extra0, extra1;

	if(!(opt & DUEL_TAG_MODE)) {
		main0 = cur_replay.ReadInt32();
		for(int i = 0; i < main0; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 0, 0, LOCATION_DECK, 0, 0);
		extra0 = cur_replay.ReadInt32();
		for(int i = 0; i < extra0; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 0, 0, LOCATION_EXTRA, 0, 0);
		main1 = cur_replay.ReadInt32();
		for(int i = 0; i < main1; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 1, 1, LOCATION_DECK, 0, 0);
		extra1 = cur_replay.ReadInt32();
		for(int i = 0; i < extra1; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 1, 1, LOCATION_EXTRA, 0, 0);
	} else {
		main0 = cur_replay.ReadInt32();
		for(int i = 0; i < main0; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 0, 0, LOCATION_DECK, 0, 0);
		extra0 = cur_replay.ReadInt32();
		for(int i = 0; i < extra0; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 0, 0, LOCATION_EXTRA, 0, 0);
		int main = cur_replay.ReadInt32();
		for(int i = 0; i < main; ++i)
			new_tag_card(pduel, cur_replay.ReadInt32(), 0, LOCATION_DECK);
		int extra = cur_replay.ReadInt32();
		for(int i = 0; i < extra; ++i)
			new_tag_card(pduel, cur_replay.ReadInt32(), 0, LOCATION_EXTRA);
		main1 = cur_replay.ReadInt32();
		for(int i = 0; i < main1; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 1, 1, LOCATION_DECK, 0, 0);
		extra1 = cur_replay.ReadInt32();
		for(int i = 0; i < extra1; ++i)
			new_card(pduel, cur_replay.ReadInt32(), 1, 1, LOCATION_EXTRA, 0, 0);
		main = cur_replay.ReadInt32();
		for(int i = 0; i < main; ++i)
			new_tag_card(pduel, cur_replay.ReadInt32(), 1, LOCATION_DECK);
		extra = cur_replay.ReadInt32();
		for(int i = 0; i < extra; ++i)
			new_tag_card(pduel, cur_replay.ReadInt32(), 1, LOCATION_EXTRA);
	}

	qreBuffer.push_back(19);
	qreBuffer.push_back(0);
	qreBuffer.push_back(STOC_GAME_MSG);
	qreBuffer.push_back(MSG_START);
	qreBuffer.push_back(0);
	char lp[4];
	*(reinterpret_cast<int*>(lp)) = start_lp;
	saveData(lp, 4);
	saveData(lp, 4);
	char charC[2];
	*(reinterpret_cast<unsigned short*>(charC)) = main0;
	saveData(charC, 2);
	*(reinterpret_cast<unsigned short*>(charC)) = extra0;
	saveData(charC, 2);
	*(reinterpret_cast<unsigned short*>(charC)) = main1;
	saveData(charC, 2);
	*(reinterpret_cast<unsigned short*>(charC)) = extra1;
	saveData(charC, 2);

	start_duel(pduel, opt);
	ReplayRefreshDeck(0);
	ReplayRefreshDeck(1);
	ReplayRefreshExtra(0);
	ReplayRefreshExtra(1);
	//mainGame->dInfo.isStarted = true;
	//new
	//mainGame->showcardcode=0;
	//mainGame->dInfo.isReplay = true;
	char engineBuffer[0x1000];
	is_continuing = true;
	exit_pending = false;

	int len = 0;
	while (is_continuing && !exit_pending && !Thread::退出进程) {
		int result = process(pduel);
		len = result & 0xffff;
		/*int flag = result >> 16;*/
		if (len > 0) {
			get_message(pduel, (byte*)engineBuffer);
			is_continuing = ReplayAnalyze(engineBuffer, len);
		}
	}
	saveQre(name);
	is_converting = false;
	if(!Thread::退出进程){
		end_duel(pduel);
	}
	Thread::进程数--;
	mainGame->RefreshReplay();
	return 0;
}
bool QreConverter::ReplayAnalyze(char* msg, unsigned int len) {
	char* offset, *pbuf = msg;
	int player, count;
	bool pauseable;
	while (pbuf - msg < (int)len) {
		if(is_closing)
			return false;
		offset = pbuf;
		pauseable = true;
		unsigned char curMsg = BufferIO::ReadUInt8(pbuf);
		switch (curMsg) {
		case MSG_RETRY: {
			return false;
		}
		case MSG_HINT: {
			pbuf += 6;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_WIN: {
			pbuf += 2;
			QreCSavePacket(offset, pbuf - offset);
			return false;
		}
		case MSG_SELECT_BATTLECMD: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 11;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 8 + 2;
			ReplayRefresh();
			return ReadReplayResponse();
		}
		case MSG_SELECT_IDLECMD: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 11 + 3;
			ReplayRefresh();
			return ReadReplayResponse();
		}
		case MSG_SELECT_EFFECTYN: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 8;
			return ReadReplayResponse();
		}
		case MSG_SELECT_YESNO: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 4;
			return ReadReplayResponse();
		}
		case MSG_SELECT_OPTION: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 4;
			return ReadReplayResponse();
		}
		case MSG_SELECT_CARD:
		case MSG_SELECT_TRIBUTE: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 3;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 8;
			return ReadReplayResponse();
		}
		case MSG_SELECT_CHAIN: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += 10 + count * 12;
			return ReadReplayResponse();
		}
		case MSG_SELECT_PLACE:
		case MSG_SELECT_DISFIELD: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 5;
			return ReadReplayResponse();
		}
		case MSG_SELECT_POSITION: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 5;
			return ReadReplayResponse();
		}
		case MSG_SELECT_COUNTER: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 3;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 8;
			return ReadReplayResponse();
		}
		case MSG_SELECT_SUM: {
			pbuf++;
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 6;
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 11;
			return ReadReplayResponse();
		}
		case MSG_SORT_CARD:
		case MSG_SORT_CHAIN: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			return ReadReplayResponse();
		}
		case MSG_CONFIRM_DECKTOP: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_CONFIRM_CARDS: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 7;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_SHUFFLE_DECK: {
			player = BufferIO::ReadInt8(pbuf);
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefreshDeck(player);
			break;
		}
		case MSG_SHUFFLE_HAND: {
			/*int oplayer = */BufferIO::ReadInt8(pbuf);
			int count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 4;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_REFRESH_DECK: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_SWAP_GRAVE_DECK: {
			player = BufferIO::ReadInt8(pbuf);
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefreshGrave(player);
			break;
		}
		case MSG_REVERSE_DECK: {
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_DECK_TOP: {
			pbuf += 6;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_SHUFFLE_SET_CARD: {
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 8;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_NEW_TURN: {
			player = BufferIO::ReadInt8(pbuf);
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_NEW_PHASE: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			break;
		}
		case MSG_MOVE: {
			int pc = pbuf[4];
			int pl = pbuf[5];
			/*int ps = pbuf[6];*/
			/*int pp = pbuf[7];*/
			int cc = pbuf[8];
			int cl = pbuf[9];
			int cs = pbuf[10];
			/*int cp = pbuf[11];*/
			pbuf += 16;
			QreCSavePacket(offset, pbuf - offset);
			if(cl && !(cl & 0x80) && (pl != cl || pc != cc))
				ReplayRefreshSingle(cc, cl, cs);
			break;
		}
		case MSG_POS_CHANGE: {
			pbuf += 9;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_SET: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_SWAP: {
			pbuf += 16;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_FIELD_DISABLED: {
			pbuf += 4;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_SUMMONING: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_SUMMONED: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			break;
		}
		case MSG_SPSUMMONING: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_SPSUMMONED: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			break;
		}
		case MSG_FLIPSUMMONING: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_FLIPSUMMONED: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			break;
		}
		case MSG_CHAINING: {
			pbuf += 16;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_CHAINED: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			break;
		}
		case MSG_CHAIN_SOLVING: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_CHAIN_SOLVED: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			pauseable = false;
			break;
		}
		case MSG_CHAIN_END: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			pauseable = false;
			break;
		}
		case MSG_CHAIN_NEGATED: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_CHAIN_DISABLED: {
			pbuf++;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_CARD_SELECTED:
		case MSG_RANDOM_SELECTED: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 4;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_BECOME_TARGET: {
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 4;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_DRAW: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count * 4;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_DAMAGE: {
			pbuf += 5;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_RECOVER: {
			pbuf += 5;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_EQUIP: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_LPUPDATE: {
			pbuf += 5;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_UNEQUIP: {
			pbuf += 4;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_CARD_TARGET: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_CANCEL_TARGET: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_PAY_LPCOST: {
			pbuf += 5;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_ADD_COUNTER: {
			pbuf += 6;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_REMOVE_COUNTER: {
			pbuf += 6;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_ATTACK: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_BATTLE: {
			pbuf += 26;
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_ATTACK_DISABLED: {
			QreCSavePacket(offset, pbuf - offset);
			pauseable = false;
			break;
		}
		case MSG_DAMAGE_STEP_START: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			pauseable = false;
			break;
		}
		case MSG_DAMAGE_STEP_END: {
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefresh();
			pauseable = false;
			break;
		}
		case MSG_MISSED_EFFECT: {
			pbuf += 8;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_TOSS_COIN: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_TOSS_DICE: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += count;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_ANNOUNCE_RACE: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 5;
			return ReadReplayResponse();
		}
		case MSG_ANNOUNCE_ATTRIB: {
			player = BufferIO::ReadInt8(pbuf);
			pbuf += 5;
			return ReadReplayResponse();
		}
		case MSG_ANNOUNCE_CARD: {
			player = BufferIO::ReadInt8(pbuf);
			return ReadReplayResponse();
		}
		case MSG_ANNOUNCE_NUMBER: {
			player = BufferIO::ReadInt8(pbuf);
			count = BufferIO::ReadInt8(pbuf);
			pbuf += 4 * count;
			return ReadReplayResponse();
		}
		case MSG_CARD_HINT: {
			pbuf += 9;
			QreCSavePacket(offset, pbuf - offset);
			break;
		}
		case MSG_MATCH_KILL: {
			pbuf += 4;
			break;
		}
		case MSG_TAG_SWAP: {
			player = pbuf[0];
			pbuf += pbuf[2] * 4 + pbuf[4] * 4 + 9;
			QreCSavePacket(offset, pbuf - offset);
			ReplayRefreshDeck(player);
			ReplayRefreshExtra(player);
			break;
		}
		}
	}
	return true;
}

static void updateDataPacket(int controler, int location, char *pbuf, int len)
{
	int plen = len + 4;
	qreBuffer.push_back(plen % 256);
	qreBuffer.push_back(plen / 256);
	qreBuffer.push_back(STOC_GAME_MSG);
	qreBuffer.push_back(MSG_UPDATE_DATA);
	qreBuffer.push_back(controler);
	qreBuffer.push_back(location);
	saveData(pbuf, len);
}

void QreConverter::ReplayRefresh(int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_field_card(pduel, 0, LOCATION_MZONE, flag, queryBuffer, 0);
	updateDataPacket(0, LOCATION_MZONE, (char*)queryBuffer, len);

	len = query_field_card(pduel, 1, LOCATION_MZONE, flag, queryBuffer, 0);
	updateDataPacket(1, LOCATION_MZONE, (char*)queryBuffer, len);
	len = query_field_card(pduel, 0, LOCATION_SZONE, flag, queryBuffer, 0);
	updateDataPacket(0, LOCATION_SZONE, (char*)queryBuffer, len);
	len = query_field_card(pduel, 1, LOCATION_SZONE, flag, queryBuffer, 0);
	updateDataPacket(1, LOCATION_SZONE, (char*)queryBuffer, len);
	len = query_field_card(pduel, 0, LOCATION_HAND, flag, queryBuffer, 0);
	updateDataPacket(0, LOCATION_HAND, (char*)queryBuffer, len);
	len = query_field_card(pduel, 1, LOCATION_HAND, flag, queryBuffer, 0);
	updateDataPacket(1, LOCATION_HAND, (char*)queryBuffer, len);
}
void  QreConverter::ReplayRefreshHand(int player, int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_field_card(pduel, player, LOCATION_HAND, flag, queryBuffer, 0);
	updateDataPacket(player, LOCATION_HAND, (char*)queryBuffer, len);
}
void QreConverter::ReplayRefreshGrave(int player, int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_field_card(pduel, player, LOCATION_GRAVE, flag, queryBuffer, 0);
	updateDataPacket(player, LOCATION_GRAVE, (char*)queryBuffer, len);
}
void QreConverter::ReplayRefreshDeck(int player, int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_field_card(pduel, player, LOCATION_DECK, flag, queryBuffer, 0);
	updateDataPacket(player, LOCATION_DECK, (char*)queryBuffer, len);
}
void QreConverter::ReplayRefreshExtra(int player, int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_field_card(pduel, player, LOCATION_EXTRA, flag, queryBuffer, 0);
	updateDataPacket(player, LOCATION_EXTRA, (char*)queryBuffer, len);
}
void QreConverter::ReplayRefreshSingle(int player, int location, int sequence, int flag) {
	unsigned char queryBuffer[0x1000];
	int len = query_card(pduel, player, location, sequence, flag, queryBuffer, 0);
	int plen = len + 5;
	qreBuffer.push_back(plen % 256);
	qreBuffer.push_back(plen / 256);
	qreBuffer.push_back(STOC_GAME_MSG);
	qreBuffer.push_back(MSG_UPDATE_CARD);
	qreBuffer.push_back(player);
	qreBuffer.push_back(location);
	qreBuffer.push_back(sequence);
	saveData((char*)queryBuffer, len);
	//mainGame->dField.UpdateCard(player, location, sequence, (char*)queryBuffer);
}
int QreConverter::MessageHandler(long fduel, int type) {
	if(!enable_log)
		return 0;
	char msgbuf[1024];
	get_log_message(fduel, (byte*)msgbuf);
	if(enable_log == 1) {
		wchar_t wbuf[1024];
		BufferIO::DecodeUTF8(msgbuf, wbuf);
		mainGame->AddChatMsg(wbuf, 9);
	} else if(enable_log == 2) {
		FILE* fp = fopen("error.log", "at");
		if(!fp)
			return 0;
		fprintf(fp, "[Script error:] %s\n", msgbuf);
		fclose(fp);
	}
	return 0;
}

}
