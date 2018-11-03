#ifndef DUELCLIENT_H
#define DUELCLIENT_H

#include "config.h"
#include <vector>
#include <deque>
#include <set>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include "network.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "../ocgcore/mtrandom.h"
#include <tuple>

namespace ygo {

class DuelClient {
private:
	static unsigned int connect_state;
	static unsigned char response_buf[64];
	static unsigned char response_len;
	static unsigned int watching;
	static unsigned char selftype;
	static bool is_host;
	static event_base* client_base;
	static bufferevent* client_bev;
	static char duel_client_read[0x2000];
	static char duel_client_write[0x2000];
	static bool is_closing;
	static int select_hint;
	static wchar_t event_string[256];
	static mtrandom rnd;
	//new
	static void ConnectError();
	static event_base* client_base_s;
	static bufferevent* client_bev_s;
	static void addroomtolist(HostPacket* pHP);
	static unsigned short gameport_s;
	static unsigned char create_game_s;
	static bool on_local;
public:
	//new
	static int StartClient_s(void*);
	static void ClientEvent_s(bufferevent *bev, short events, void *ctx);
	static bool StartGame_s();
	static unsigned char connect_state_s;
	//change
	static bool StartClient(unsigned int ip, unsigned short port, unsigned char create_game = 0);
	//change
	static int ConnectTimeout(void* arg);
	
	static void StopClient(bool is_exiting = false);
	static void ClientRead(bufferevent* bev, void* ctx);
	static void ClientEvent(bufferevent *bev, short events, void *ctx);
	static int ClientThread(void* param);
	static void HandleSTOCPacketLan(char* data, unsigned int len);
	static int ClientAnalyze(char* msg, unsigned int len);
	static void SetResponseI(int respI);
	static void SetResponseB(unsigned char* respB, unsigned char len);
	static void SendResponse();
	static void SendPacketToServer(unsigned char proto) {
		char* p = duel_client_write;
		BufferIO::WriteInt16(p, 1);
		BufferIO::WriteInt8(p, proto);
		bufferevent_write(client_bev, duel_client_write, 3);
	}
	template<typename ST>
	static void SendPacketToServer(unsigned char proto, ST& st) {
		char* p = duel_client_write;
		BufferIO::WriteInt16(p, 1 + sizeof(ST));
		BufferIO::WriteInt8(p, proto);
		memcpy(p, &st, sizeof(ST));
		bufferevent_write(client_bev, duel_client_write, sizeof(ST) + 3);
	}
	static void SendBufferToServer(unsigned char proto, void* buffer, size_t len) {
		char* p = duel_client_write;
		BufferIO::WriteInt16(p, 1 + len);
		BufferIO::WriteInt8(p, proto);
		memcpy(p, buffer, len);
		bufferevent_write(client_bev, duel_client_write, len + 3);
	}
	
protected:
	static bool is_refreshing;
	static int match_kill;
	static event* resp_event;
	static std::set<unsigned int> remotes;
public:
	static std::deque<HostPacket> hosts;
	static std::vector<std::pair<std::wstring, std::wstring > > otherServers;
	static std::vector<std::pair<std::wstring, std::wstring > > stickServers;
	static void BeginRefreshHost();
	static int RefreshThread(void* arg);
	static void BroadcastReply(evutil_socket_t fd, short events, void* arg);
	//new
	static bool is_connecting(){
		return connect_state>0;
	}
	static void summonwords(int code);
};

}
#endif //DUELCLIENT_H
