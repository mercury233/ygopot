#include "config.h"
#include "menu_handler.h"
#include "netserver.h"
#include "duelclient.h"
#include "deck_manager.h"
#include "replay_mode.h"
#include "single_mode.h"
#include "image_manager.h"
#include "game.h"
#include "Ws2tcpip.h"
#include <exception>
#include "mwin.h"
#include <fstream>
#include <string>
#include "shellapi.h"
#include "qrep.h"
#include "qrec.h"
//new
int ygopro_vsnprintf(char *buf, size_t buflen, const char *format, ...)
{
    if (!buflen)
		return 0;
	int r;
	va_list ap;
	va_start(ap, format);
	r = _vsnprintf(buf, buflen, format, ap);
	va_end(ap);
	buf[buflen-1] = '\0';
	return r;
}
//new
const char* ygo_inet_ntop(int af, const void *src, char *dst, size_t len)
{
	if (af == AF_INET) {
		const struct in_addr *in = (in_addr *)src;
		const unsigned int a = ntohl(in->s_addr);
		int r;
		r = ygopro_vsnprintf(dst, len, "%d.%d.%d.%d",
		    (int)(unsigned char)((a>>24)&0xff),
		    (int)(unsigned char)((a>>16)&0xff),
		    (int)(unsigned char)((a>>8 )&0xff),
		    (int)(unsigned char)((a    )&0xff));
		if (r<0||(size_t)r>=len)
			return NULL;
		else
			return dst;
#ifdef AF_INET6
	} else if (af == AF_INET6) {
		const struct in6_addr *addr = (in6_addr *)src;
		char buf[64], *cp;
		int longestGapLen = 0, longestGapPos = -1, i,
			curGapPos = -1, curGapLen = 0;
		unsigned short words[8];
		for (i = 0; i < 8; ++i) {
			words[i] =
			    (((unsigned short)addr->s6_addr[2*i])<<8) + addr->s6_addr[2*i+1];
		}
		if (words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 &&
		    words[4] == 0 && ((words[5] == 0 && words[6] && words[7]) ||
			(words[5] == 0xffff))) {
			/* This is an IPv4 address. */
			if (words[5] == 0) {
				ygopro_vsnprintf(buf, sizeof(buf), "::%d.%d.%d.%d",
				    addr->s6_addr[12], addr->s6_addr[13],
				    addr->s6_addr[14], addr->s6_addr[15]);
			} else {
				ygopro_vsnprintf(buf, sizeof(buf), "::%x:%d.%d.%d.%d", words[5],
				    addr->s6_addr[12], addr->s6_addr[13],
				    addr->s6_addr[14], addr->s6_addr[15]);
			}
			if (strlen(buf) > len)
				return NULL;
			strncpy(dst, buf, len);
			*(dst+len-1)='\0';
			return dst;
		}
		i = 0;
		while (i < 8) {
			if (words[i] == 0) {
				curGapPos = i++;
				curGapLen = 1;
				while (i<8 && words[i] == 0) {
					++i; ++curGapLen;
				}
				if (curGapLen > longestGapLen) {
					longestGapPos = curGapPos;
					longestGapLen = curGapLen;
				}
			} else {
				++i;
			}
		}
		if (longestGapLen<=1)
			longestGapPos = -1;

		cp = buf;
		for (i = 0; i < 8; ++i) {
			if (words[i] == 0 && longestGapPos == i) {
				if (i == 0)
					*cp++ = ':';
				*cp++ = ':';
				while (i < 8 && words[i] == 0)
					++i;
				--i; /* to compensate for loop increment. */
			} else {
				ygopro_vsnprintf(cp,
								sizeof(buf)-(cp-buf), "%x", (unsigned)words[i]);
				cp += strlen(cp);
				if (i != 7)
					*cp++ = ':';
			}
		}
		*cp = '\0';
		if (strlen(buf) > len)
			return NULL;
		strncpy(dst, buf, len);
		*(dst+len-1)='\0';
		return dst;
#endif
	} else {
		return NULL;
	}
}

//new
bool runexe(LPTSTR name)
{
	HWND hwnd=GetActiveWindow();
	ShowWindow(hwnd,SW_HIDE);
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	ZeroMemory( &si, sizeof(si) ); 
	si.cb = sizeof(si); 
	ZeroMemory( &pi, sizeof(pi) ); 
	if( !CreateProcess(NULL,name,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi ) ) {
		ShowWindow(hwnd,SW_RESTORE);
		return 0; 
	}
	WaitForSingleObject( pi.hProcess, INFINITE ); 
	//
	//
	CloseHandle( pi.hProcess ); 
	CloseHandle( pi.hThread );
	ShowWindow(hwnd,SW_RESTORE);
	return 1;
}
namespace ygo {

bool MenuHandler::OnEvent(const irr::SEvent& event) {
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		irr::gui::IGUIElement* caller = event.GUIEvent.Caller;
		s32 id = caller->getID();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			case BUTTON_MODE_EXIT: {
				mainGame->device->closeDevice();
				break;
			}
			case BUTTON_LAN_MODE: {
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wMainMenu);
				mainGame->ShowElement(mainGame->wLanWindow);
				break;
			}
			case BUTTON_JOIN_HOST: {
			#if WINVER >= 0x0600
				struct addrinfo hints, *servinfo;
				memset(&hints, 0, sizeof(struct addrinfo));
				hints.ai_family = AF_INET;			/* Allow IPv4 or IPv6 */
				hints.ai_socktype = SOCK_STREAM;	/* Datagram socket */
				hints.ai_flags = AI_PASSIVE;		/* For wildcard IP address */
				hints.ai_protocol = 0;				/* Any protocol */
				hints.ai_canonname = NULL;
				hints.ai_addr = NULL;
				hints.ai_next = NULL;
				int status;
				char hostname[100];
				char ip[20];
				const wchar_t* pstr = mainGame->ebJoinIP->getText();
				BufferIO::CopyWStr(pstr, hostname, 100);
				if ((status = getaddrinfo(hostname, NULL, &hints, &servinfo)) == -1) {
					fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
					//error handling
					BufferIO::CopyWStr(pstr, ip, 16);
				} else
					//change
					ygo_inet_ntop(AF_INET, &(((struct sockaddr_in *)servinfo->ai_addr)->sin_addr), ip, 20);
				freeaddrinfo(servinfo);
			#else
				//int status;
				char hostname[100];
				char ip[20];
				const wchar_t* pstr = mainGame->ebJoinIP->getText();
				BufferIO::CopyWStr(pstr, hostname, 100);
				BufferIO::CopyWStr(pstr, ip, 16);
			#endif
				unsigned int remote_addr = htonl(inet_addr(ip));
				unsigned int remote_port = _wtoi(mainGame->ebJoinPort->getText());
				BufferIO::CopyWStr(pstr, mainGame->gameConf.lastip, 20);
				BufferIO::CopyWStr(mainGame->ebJoinPort->getText(), mainGame->gameConf.lastport, 20);
				//change
				if(remote_addr==htonl(SERVER_IP) && remote_port<10000)
				{
					irr::SEvent event_call;
					event_call.EventType = irr::EET_GUI_EVENT;
					event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
					event_call.GUIEvent.Caller = ygo::mainGame->btnCreateHost;
					mainGame->device->postEventFromUser(event_call);
					return false;
				}else
					DuelClient::StartClient(remote_addr, remote_port);
				break;
			}
			case BUTTON_JOIN_CANCEL: {
				mainGame->HideElement(mainGame->wLanWindow);
				mainGame->ShowElement(mainGame->wMainMenu);
				break;
			}
			case BUTTON_LAN_REFRESH: {
				DuelClient::BeginRefreshHost();
				break;
			}
			case BUTTON_CREATE_HOST: {
				mainGame->btnHostConfirm->setEnabled(true);
				mainGame->btnHostCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wLanWindow);
				mainGame->ShowElement(mainGame->wCreateHost);
				/*
				static bool b=false;
				if(b)
					break;
				b=true;
				STARTUPINFO si; 
				PROCESS_INFORMATION pi; 
				ZeroMemory( &si, sizeof(si) ); 
				si.cb = sizeof(si); 
				ZeroMemory( &pi, sizeof(pi) ); 
				CreateProcess(NULL,"YGOUpdater.exe",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
				*/
				break;
			}
			case BUTTON_HOST_CONFIRM: {
				//change
				mainGame->gameConf.lflist = mainGame->cbLFlist->getSelected();
				mainGame->gameConf.rule = mainGame->cbRule->getSelected();
				mainGame->gameConf.pkmode = mainGame->cbMatchMode->getSelected();
				mainGame->gameConf.server = mainGame->cbServer->getSelected();
				mainGame->gameConf.enablePriority = mainGame->chkEnablePriority->isChecked() ? 1 : 0;
				mainGame->gameConf.noCheckDeck = mainGame->chkNoCheckDeck->isChecked() ? 1 : 0;
				mainGame->gameConf.noShuffleDeck = mainGame->chkNoShuffleDeck->isChecked() ? 1 : 0;
				mainGame->gameConf.startLP = _wtoi(mainGame->ebStartLP->getText());
				mainGame->gameConf.startHand = _wtoi(mainGame->ebStartHand->getText());
				mainGame->gameConf.DrawCount = _wtoi(mainGame->ebDrawCount->getText());

				BufferIO::CopyWStr(mainGame->ebServerName->getText(), mainGame->gameConf.gamename, 20);
				unsigned char create_game = mainGame->cbServer->getSelected()+1;
				if( create_game-1==1 )
				{
					if(!NetServer::StartServer(mainGame->gameConf.serverport))
						break;
					if(!DuelClient::StartClient(0x7f000001, mainGame->gameConf.serverport,create_game)) {
						NetServer::StopServer();
						break;
					}
				}else{
					unsigned int remote_addr = htonl(SERVER_IP);
					unsigned int remote_port = SERVER_PORT;
					DuelClient::StartClient(remote_addr, remote_port,create_game);
				}
				break;
			}
			case BUTTON_HOST_CANCEL: {
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wCreateHost);
				mainGame->ShowElement(mainGame->wLanWindow);
				break;
			}
			case BUTTON_HP_DUELIST: {
				DuelClient::SendPacketToServer(CTOS_HS_TODUELIST);
				break;
			}
			case BUTTON_HP_OBSERVER: {
				DuelClient::SendPacketToServer(CTOS_HS_TOOBSERVER);
				break;
			}
			case BUTTON_HOST_AI: {
				ShellExecuteW(NULL, L"open", L"AI.exe", L"-ai", NULL, SW_SHOW);
				break;
			}
			case BUTTON_HP_KICK: {
				int id = 0;
				while(id < 4) {
					if(mainGame->btnHostPrepKick[id] == caller)
						break;
					id++;
				}
				CTOS_Kick csk;
				csk.pos = id;
				DuelClient::SendPacketToServer(CTOS_HS_KICK, csk);
				break;
			}
			case BUTTON_HP_START: {
				if(!mainGame->chkHostPrepReady[0]->isChecked()
				        || !mainGame->chkHostPrepReady[1]->isChecked())
					break;
				DuelClient::SendPacketToServer(CTOS_HS_START);
				break;
			}
			case BUTTON_HP_CANCEL: {
				DuelClient::StopClient();
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wHostPrepare);
				mainGame->ShowElement(mainGame->wLanWindow);
				mainGame->wChat->setVisible(false);
				if(exit_on_return)
					mainGame->device->closeDevice();
				break;
			}
			case BUTTON_REPLAY_MODE: {
				//change
				//if( !runexe("ygopro_vs.exe -r") )
				//{
					irr::SEvent event_call;
					event_call.EventType = irr::EET_KEY_INPUT_EVENT;
					event_call.KeyInput.Key=KEY_KEY_R;
					event_call.KeyInput.PressedDown = false;
					mainGame->device->postEventFromUser(event_call);
				//}
				break;
			}
			case BUTTON_SINGLE_MODE: {
				//change
				//if( !runexe("ygopro_vs.exe -s") )
				//{
					irr::SEvent event_call;
					event_call.EventType = irr::EET_KEY_INPUT_EVENT;
					event_call.KeyInput.Key=KEY_KEY_S;
					event_call.KeyInput.PressedDown = false;
					mainGame->device->postEventFromUser(event_call);
				//}
				break;
			}
			case BUTTON_LOAD_REPLAY: {
				if(mainGame->lstReplayList->getSelected() == -1)
					break;
				std::wstring name(mainGame->lstReplayList->getListItem(mainGame->lstReplayList->getSelected()));
				bool qre = false;
				if(name.find(L".qre") == (name.length() - 4))
					qre = true;
				if(!qre)
				{
					if(!ReplayMode::cur_replay.OpenReplay(name.c_str()))
						break;
				}
				else
				{
					QreMode::openReplay(name);
				}
				//imageManager.设定卡背();
				imageManager.setCovers();
				mainGame->imgCard->setImage(imageManager.卡背图[0]);
				mainGame->imgCover->setImage(imageManager.tBlank);
				mainGame->wCardImg->setVisible(true);
				mainGame->wInfos->setVisible(true);
				mainGame->wReplay->setVisible(true);
				mainGame->stName->setText(L"");
				mainGame->stInfo->setText(L"");
				mainGame->stDataInfo->setText(L"");
				mainGame->stText->setText(L"");
				mainGame->scrCardText->setVisible(false);
				mainGame->wReplayControl->setVisible(true);
				mainGame->btnReplayStart->setVisible(false);
				mainGame->btnReplayPause->setVisible(true);
				mainGame->btnReplayStep->setVisible(false);
				mainGame->wPhase->setVisible(true);
				mainGame->dField.panel = 0;
				mainGame->dField.hovered_card = 0;
				mainGame->dField.clicked_card = 0;
				mainGame->dField.Clear();
				mainGame->HideElement(mainGame->wReplay);
				mainGame->device->setEventReceiver(&mainGame->dField);
				unsigned int start_turn = _wtoi(mainGame->ebRepStartTurn->getText());
				if(start_turn == 1)
					start_turn = 0;
				
				if(!qre)
					ReplayMode::StartReplay(start_turn);
				else
					QreMode::StartReplay(start_turn);
				break;
			}
			case BUTTON_CANCEL_REPLAY: {
				mainGame->HideElement(mainGame->wReplay);
				mainGame->ShowElement(mainGame->wMainMenu);
				break;
			}
			case BUTTON_QRE_CONVERT: {
				if(mainGame->lstReplayList->getSelected() == -1)
					break;
				std::wstring name(mainGame->lstReplayList->getListItem(mainGame->lstReplayList->getSelected()));
				bool qre = false;
				if(name.find(L".qre") == (name.length() - 4))
					qre = true;
				if(!qre)
				{
					if(QreConverter::is_converting)
						break;
					if(!QreConverter::cur_replay.OpenReplay(name.c_str()))
						break;
					QreConverter::StartReplay(name);
				}
			}
			case BUTTON_LOAD_SINGLEPLAY: {
				if(mainGame->lstSinglePlayList->getSelected() == -1)
					break;
				mainGame->singleSignal.SetNoWait(false);
				SingleMode::StartPlay();
				break;
			}
			case BUTTON_CANCEL_SINGLEPLAY: {
				mainGame->HideElement(mainGame->wSinglePlay);
				mainGame->ShowElement(mainGame->wMainMenu);
				break;
			}
			case BUTTON_DECK_EDIT: {
				//change
				//if( !runexe("ygopro_vs.exe -d") )
				//{
					irr::SEvent event_call;
					event_call.EventType = irr::EET_KEY_INPUT_EVENT;
					event_call.KeyInput.Key=KEY_KEY_D;
					event_call.KeyInput.PressedDown = false;
					mainGame->device->postEventFromUser(event_call);
				//}
				break;
			}  
			//new help
			case BUTTON_HELP_MODE: {
				mainGame->HideElement(mainGame->wMainMenu);
				mainGame->lstHelpList->clear();
				std::vector<std::wstring> filelist = DataManager::listDir(L"ygopothelp",L"*.txt");
				int listcount = filelist.size();
				/*
				while(--listcount>=0)
				{
					wchar_t* fptr = filelist[listcount]+10;
					while(*(++fptr) > 0);
					*(fptr-4)=0;
					int i=mainGame->lstHelpList->getItemCount();
					while(--i >= 0 && wcscmp(filelist[listcount]+11,mainGame->lstHelpList->getListItem(i))<0);
					
					mainGame->lstHelpList->insertItem(i+1,filelist[listcount]+11,-1);
				}*/
				while(--listcount>=0)
				{
					std::wstring::iterator fptr = filelist[listcount].begin() + 10;
					while((++fptr) != filelist[listcount].end());
					*(fptr - 4) = 0;
					int i = mainGame->lstHelpList->getItemCount();
					while(--i >= 0 && wcscmp(filelist[listcount].c_str()+11, mainGame->lstHelpList->getListItem(i))<0);
					
					mainGame->lstHelpList->insertItem(i+1,filelist[listcount].substr(11).c_str(), -1);
				}
				mainGame->ShowElement(mainGame->wHelpWindow);
				//
				mainGame->lstHelpList->setSelected(0);
				irr::SEvent event_call;
				event_call.EventType = EET_GUI_EVENT;
				event_call.GUIEvent.EventType = irr::gui::EGET_LISTBOX_CHANGED;
				event_call.GUIEvent.Caller = mainGame->lstHelpList;
				mainGame->device->postEventFromUser(event_call);
				break;
			}
			case BUTTON_WEBSITES: {
				mainGame->HideElement(mainGame->wMainMenu);
				mainGame->lstCataList->clear();
				mainGame->lstWebList->clear();
				mainGame->websites.clear();

				{
					std::wifstream ini("website.ini");
					if(ini.is_open())
					{
						ini.imbue(std::locale("chs"));
						std::wstring line;

						std::wstring cata = L"";
						std::vector<std::pair<std::wstring, std::wstring> > ls;


						while(!ini.eof())
						{
							std::getline(ini, line);
							if(line.length() == 0)
								continue;

							if(line[0] == '[')
							{
								int end = line.find(']');
								if(end == line.npos)
								{
									continue;
								}

								if(!ls.empty())
								{
									std::vector<std::pair<std::wstring, std::wstring> > nls;
									nls.swap(ls);
									mainGame->websites.push_back(std::make_pair(cata, std::move(nls)));
								}

								cata = line.substr(1, end - 1);
								ls.clear();
								continue;
							}

							int sep = line.find('=');
							if(sep == line.npos)
								continue;

							ls.push_back(std::make_pair(line.substr(0, sep), line.substr(sep + 1)));
						}
						if(!ls.empty())
						{
							mainGame->websites.push_back(std::make_pair(cata, std::move(ls)));
						}
					}
				}

				for(auto it = mainGame->websites.begin(); it != mainGame->websites.end(); ++it)
				{
					mainGame->lstCataList->addItem(it->first.c_str());
				}

				mainGame->ShowElement(mainGame->wWebWindow);
				mainGame->lstCataList->setSelected(0);
				irr::SEvent event_call;
				event_call.EventType = EET_GUI_EVENT;
				event_call.GUIEvent.EventType = irr::gui::EGET_LISTBOX_CHANGED;
				event_call.GUIEvent.Caller = mainGame->lstCataList;
				mainGame->device->postEventFromUser(event_call);
				break;
			}
			case BUTTON_HELP_EXIT: {
				mainGame->HideElement(mainGame->wHelpWindow);
				mainGame->ShowElement(mainGame->wMainMenu);
				break;
			}
			//temp
			case 卡组下载: {
				runexe("deckmanager.exe");
				break;
			}
			case BUTTON_WEB_CANCEL: {
				mainGame->HideElement(mainGame->wWebWindow);
				mainGame->ShowElement(mainGame->wMainMenu);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_CHANGED: {
			switch(id) {
			case LISTBOX_LAN_HOST: {
				int sel = mainGame->lstHostList->getSelected();
				if(sel == -1)
					break;
				if(sel >= (DuelClient::hosts.size() + DuelClient::stickServers.size())
					|| sel < DuelClient::stickServers.size())
				{
					int _sel = 0;
					std::wstring p;
					if(sel >= (DuelClient::hosts.size() + DuelClient::stickServers.size()))
					{
						_sel = sel - (DuelClient::hosts.size() + DuelClient::stickServers.size());
						if(_sel > DuelClient::otherServers.size())
						{
							break;
						}
						p = DuelClient::otherServers[_sel].second;
					}
					else
					{
						p = DuelClient::stickServers[sel].second;
					}

					if(p.find(L"ip://") == 0)
					{
						int sep = p.find(':', 5);
						if(sep != p.npos)
						{
							mainGame->ebJoinIP->setText(p.substr(5, sep - 5).c_str());
							mainGame->ebJoinPort->setText(p.substr(sep + 1).c_str());
						}
					}
					else if(p.find(L"http://") == 0)
					{
						ShellExecuteW(NULL, L"open", p.c_str(), NULL, NULL, SW_SHOW);
					}
					else if(p.find(L"run://") == 0)
					{
						std::wstring exe = p.substr(6);
						ShellExecuteW(NULL, L"open", exe.c_str(), NULL, NULL, SW_SHOW);
					}
				}
				else
				{
					int _sel = sel - DuelClient::stickServers.size();
					int addr = DuelClient::hosts[_sel].ipaddr;
					int port = DuelClient::hosts[_sel].port;
					wchar_t buf[20];
					myswprintf(buf, L"%d.%d.%d.%d", addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff);
					mainGame->ebJoinIP->setText(buf);
					myswprintf(buf, L"%d", port);
					mainGame->ebJoinPort->setText(buf);
				}
				break;
			}
			case LISTBOX_REPLAY_LIST: {
				int sel = mainGame->lstReplayList->getSelected();
				if(sel == -1)
					break;

				std::wstring name(mainGame->lstReplayList->getListItem(mainGame->lstReplayList->getSelected()));
				bool qre = false;
				
				std::wstring repinfo;

				if(name.find(L".qre") == (name.length() - 4))
					qre = true;
				if(!qre)
				{
					if(!ReplayMode::cur_replay.OpenReplay(name.c_str()))
						break;
				}
				else
				{
					std::ifstream file(L"./qre/" + name, std::ios_base::binary);
					char isTag = file.get();
					if(isTag & QRE_TAG)
					{
						wchar_t buf[20], buf2[20];
						file.read((char*)buf, 40);
						repinfo.append(buf);
						repinfo.append(L"\n");
						file.read((char*)buf2, 40);
						file.read((char*)buf, 40);
						repinfo.append(buf);
						repinfo.append(L"\nVS\n");
						repinfo.append(buf2);
						repinfo.append(L"\n");
						file.read((char*)buf, 40);
						repinfo.append(buf);
						repinfo.append(L"\n");
					}
					else
					{
						wchar_t buf[20];
						file.read((char*)buf, 40);
						repinfo.append(buf);
						repinfo.append(L"\nVS\n");
						file.read((char*)buf, 40);
						repinfo.append(buf);
						repinfo.append(L"\n");
					}
					mainGame->ebRepStartTurn->setText(L"1");
					mainGame->SetStaticText(mainGame->stReplayInfo, 180, mainGame->guiFont, (wchar_t*)repinfo.c_str());
					break;
				}

				wchar_t infobuf[256];
				if(!is_qre)
				{
					time_t curtime = ReplayMode::cur_replay.pheader.seed;
					tm* st = localtime(&curtime);
					myswprintf(infobuf, L"%d/%d/%d %02d:%02d:%02d\n", st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
					repinfo.append(infobuf);
					wchar_t namebuf[4][20];
					BufferIO::CopyWStr((unsigned short*)&ReplayMode::cur_replay.replay_data[0], namebuf[0], 20);
					BufferIO::CopyWStr((unsigned short*)&ReplayMode::cur_replay.replay_data[40], namebuf[1], 20);
					if(ReplayMode::cur_replay.pheader.flag & REPLAY_TAG) {
						BufferIO::CopyWStr((unsigned short*)&ReplayMode::cur_replay.replay_data[80], namebuf[2], 20);
						BufferIO::CopyWStr((unsigned short*)&ReplayMode::cur_replay.replay_data[120], namebuf[3], 20);
					}
					if(ReplayMode::cur_replay.pheader.flag & REPLAY_TAG)
						myswprintf(infobuf, L"%ls\n%ls\n===VS===\n%ls\n%ls\n", namebuf[0], namebuf[1], namebuf[2], namebuf[3]);
					else
						myswprintf(infobuf, L"%ls\n===VS===\n%ls\n", namebuf[0], namebuf[1]);
					repinfo.append(infobuf);
					mainGame->ebRepStartTurn->setText(L"1");
					mainGame->SetStaticText(mainGame->stReplayInfo, 180, mainGame->guiFont, (wchar_t*)repinfo.c_str());
				}
				break;
			}
			//new
			case LISTBOX_HELP_LIST: {
				int sel = mainGame->lstHelpList->getSelected();
				if(sel == -1)
					break;
				wchar_t fname[256];
				char buffer[0x10000];
				wchar_t textbuffer[0x10000];
				myswprintf(fname, L"ygopothelp\\%ls.txt", mainGame->lstHelpList->getListItem(sel));
				IReadFile* reader=DataManager::fsys->createAndOpenFile(fname);
				uint32 len = reader->getSize();
				if (len > 0xffff)
					return 0;
				reader->read((void*)buffer,len);
				buffer[len]=0;
				BufferIO::DecodeUTF8(buffer,textbuffer);
				mainGame->SetStaticText(mainGame->ebHelp, 370, mainGame->helpFont, textbuffer);
				break;
			}
			case LISTBOX_CATA_LIST: {
				int sel = mainGame->lstCataList->getSelected();
				if(sel == -1 || sel >= mainGame->websites.size())
					break;
				mainGame->lstWebList->clear();
				auto &vec = mainGame->websites[sel].second;
				for(auto it = vec.begin(); it != vec.end(); ++it)
				{
					mainGame->lstWebList->addItem(it->first.c_str());
				}
				mainGame->lstWebList->setSelected(-1);
				break;
			}
			case LISTBOX_WEB_LIST: {
				int sel = mainGame->lstCataList->getSelected();
				int sel2 = mainGame->lstWebList->getSelected();
				if(sel == -1 || sel >= mainGame->websites.size())
					break;
				auto &sites = mainGame->websites[sel].second;
				if(sel2 == -1 || sel2 >= sites.size())
					break;

				std::wstring site = sites[sel2].second;

				ShellExecuteW(NULL, L"open", site.c_str(), NULL, NULL, SW_SHOW); 
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_CHECKBOX_CHANGED: {
			switch(id) {
			case CHECKBOX_HP_READY: {
				if(!caller->isEnabled())
					break;
				mainGame->env->setFocus(mainGame->wHostPrepare);
				if(static_cast<irr::gui::IGUICheckBox*>(caller)->isChecked()) {
					if(mainGame->cbDeckSelect->getSelected() == -1 ||
					        !deckManager.LoadDeck(mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected()))) {
						static_cast<irr::gui::IGUICheckBox*>(caller)->setChecked(false);
						break;
					}
					BufferIO::CopyWStr(mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected()),
					                   mainGame->gameConf.lastdeck, 64);
					char deckbuf[1024];
					char* pdeck = deckbuf;
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.main.size() + deckManager.current_deck.extra.size());
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.side.size());
					for(size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.main[i]->first);
					for(size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.extra[i]->first);
					for(size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.side[i]->first);
					DuelClient::SendBufferToServer(CTOS_UPDATE_DECK, deckbuf, pdeck - deckbuf);
					DuelClient::SendPacketToServer(CTOS_HS_READY);
					mainGame->cbDeckSelect->setEnabled(false);
				} else {
					DuelClient::SendPacketToServer(CTOS_HS_NOTREADY);
					mainGame->cbDeckSelect->setEnabled(true);
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
			case EDITBOX_CHAT: {
				if(mainGame->dInfo.isReplay)
					break;
				const wchar_t* input = mainGame->ebChatInput->getText();
				if(input[0]) {
					unsigned short msgbuf[256];
					if(mainGame->dInfo.isStarted) {
						if(mainGame->dInfo.player_type < 7) {
							if(mainGame->dInfo.isTag && (mainGame->dInfo.player_type % 2))
								mainGame->AddChatMsg((wchar_t*)input, 2);
							else
								mainGame->AddChatMsg((wchar_t*)input, 0);
						} else
							mainGame->AddChatMsg((wchar_t*)input, 10);
					} else
						mainGame->AddChatMsg((wchar_t*)input, 7);
					int len = BufferIO::CopyWStr(input, msgbuf, 256);
					DuelClient::SendBufferToServer(CTOS_CHAT, msgbuf, (len + 1) * sizeof(short));
					mainGame->ebChatInput->setText(L"");
				}
				break;
			}
			}
			break;
		}
		default: break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		StopMovie();
		switch(event.KeyInput.Key) {
		/*change
		case irr::KEY_KEY_R: {
			if(!event.KeyInput.PressedDown)
				mainGame->textFont->setTransparency(true);
			break;
		}
		*/
		case irr::KEY_ESCAPE: {
			mainGame->device->minimizeWindow();
			break;
		}
		default: break;
		}
		break;
	}
	default: break;
	}
	return myEvent(event);
}

//new
bool MenuHandler::myEvent(const irr::SEvent& event){
		irr::gui::IGUIElement* caller = event.GUIEvent.Caller;
		//mainGame->driver->OnResize(mainGame->driver->getScreenSize());
		switch(event.EventType) {
			case irr::EET_GUI_EVENT: {
				s32 id = caller->getID();
				switch(event.GUIEvent.EventType) {
					case irr::gui::EGET_ELEMENT_FOCUSED: {
						break;
					}
					default: break;
				}
				break;
			}
			case irr::EET_KEY_INPUT_EVENT: {
				irr::gui::IGUIElement* foucus = mainGame->env->getFocus();
				if( /*foucus == mainGame->ebNickName ||
					foucus == mainGame->ebJoinIP ||
					foucus == mainGame->ebJoinPort ||
					foucus == mainGame->ebJoinPass ||
					foucus == mainGame->ebTimeLimit ||
					foucus == mainGame->ebStartLP ||
					foucus == mainGame->ebStartHand ||
					foucus == mainGame->ebDrawCount ||
					foucus == mainGame->ebServerName ||
					foucus == mainGame->ebServerPass ||
					foucus == mainGame->ebChatInput ||
					foucus == mainGame->ebDeckname ||
					foucus == mainGame->ebStar ||
					foucus == mainGame->ebAttack ||
					foucus == mainGame->ebDefence ||
					foucus == mainGame->ebCardName ||
					foucus == mainGame->ebRepStartTurn || */
					mainGame->HasFocus(EGUIET_EDIT_BOX) ||
					event.KeyInput.PressedDown
				)break;
				irr::SEvent event_call;
				event_call.EventType = irr::EET_GUI_EVENT;
				event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				if(event.KeyInput.Key==irr::KEY_KEY_J || event.KeyInput.Key==irr::KEY_KEY_D || event.KeyInput.Key==irr::KEY_KEY_R || event.KeyInput.Key==irr::KEY_KEY_S){
					if ( DuelClient::is_connecting() ) break;
					mainGame->HideElement(mainGame->wLanWindow);
					mainGame->HideElement(mainGame->wCreateHost);
					mainGame->HideElement(mainGame->wHostPrepare);
					mainGame->HideElement(mainGame->wReplay);
					mainGame->HideElement(mainGame->wSinglePlay);
					mainGame->HideElement(mainGame->wHelpWindow);
					if(mainGame->is_building){
						event_call.GUIEvent.Caller = mainGame->btnDBExit;
						mainGame->deckBuilder.OnEvent(event_call);
					}
					mainGame->HideElement(mainGame->wMainMenu);
				}
				switch(event.KeyInput.Key) {
					case irr::KEY_KEY_J: {
						event_call.GUIEvent.Caller = ygo::mainGame->btnLanMode;
						mainGame->menuHandler.OnEvent(event_call);
						break;
					}
					case irr::KEY_KEY_D: {
						mainGame->RefreshDeck(mainGame->cbDBDecks);
						if(mainGame->cbDBDecks->getSelected() != -1)
							deckManager.LoadDeck(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()));
						mainGame->is_building = true;
						mainGame->is_siding = false;
						mainGame->wInfos->setVisible(true);
						mainGame->wCardImg->setVisible(true);
						mainGame->wDeckEdit->setVisible(true);
						mainGame->wFilter->setVisible(true);
						mainGame->btnSideOK->setVisible(false);
						mainGame->deckBuilder.filterList = deckManager._lfList[0].content;
						mainGame->cbDBLFList->setSelected(0);
						mainGame->cbCardType->setSelected(0);
						mainGame->cbCardType2->setSelected(0);
						mainGame->cbAttribute->setSelected(0);
						mainGame->cbRace->setSelected(0);
						mainGame->cbSetcode->setSelected(0);
						mainGame->ebAttack->setText(L"");
						mainGame->ebDefence->setText(L"");
						mainGame->ebStar->setText(L"");
						mainGame->cbCardType2->setEnabled(false);
						mainGame->cbAttribute->setEnabled(false);
						mainGame->cbRace->setEnabled(false);
						mainGame->ebAttack->setEnabled(false);
						mainGame->ebDefence->setEnabled(false);
						mainGame->ebStar->setEnabled(false);
						mainGame->deckBuilder.filter_effect = 0;
						mainGame->deckBuilder.result_string[0] = L'0';
						mainGame->deckBuilder.result_string[1] = 0;
						mainGame->deckBuilder.results.clear();
						mainGame->deckBuilder.is_draging = false;
						mainGame->deckBuilder.is_clicking = false;
						mainGame->device->setEventReceiver(&mainGame->deckBuilder);
						for(int i = 0; i < 32; ++i)
							mainGame->chkCategory[i]->setChecked(false);
						break;
					}
					case irr::KEY_KEY_R: {
						mainGame->ShowElement(mainGame->wReplay);
						mainGame->ebRepStartTurn->setText(L"1");
						mainGame->RefreshReplay();
						break;
					}
					case irr::KEY_KEY_S: {
						mainGame->ShowElement(mainGame->wSinglePlay);
						mainGame->RefreshSingleplay();
						break;
					}
					default: break;
				}
				break;
			}
			case irr::EET_MOUSE_INPUT_EVENT: {
				switch(event.MouseInput.Event) {
					case irr::EMIE_LMOUSE_PRESSED_DOWN: {
			
						break;
					}
					case irr::EMIE_LMOUSE_LEFT_UP: {
						break;
					}
					case irr::EMIE_RMOUSE_LEFT_UP: {
						break;
					}
					case irr::EMIE_MOUSE_MOVED: {
						mainGame->wFloat->setVisible(false);
						showfloat(mainGame->ebAttack,L"大于等于：>x\n小于等于：<y\n指定范围：x-y",position2di(event.MouseInput.X, event.MouseInput.Y) );
						showfloat(mainGame->ebStar,L"大于等于：>x\n小于等于：<y\n指定范围：x-y",position2di(event.MouseInput.X, event.MouseInput.Y) );
						showfloat(mainGame->ebDefence,L"大于等于：>x\n小于等于：<y\n指定范围：x-y",position2di(event.MouseInput.X, event.MouseInput.Y) );
						showfloat(mainGame->ebCardName,L"搜索多个关键字：a b c\n只搜索卡名：$name",position2di(event.MouseInput.X, event.MouseInput.Y) );
						showfloat(mainGame->ebJoinIP,L"使用右上角按钮建房",position2di(event.MouseInput.X, event.MouseInput.Y) );
						showfloat(mainGame->btnLanRefresh, L"刷新后，需等待5秒才能点刷新并显示房间", position2di(event.MouseInput.X, event.MouseInput.Y));
						showfloat(mainGame->btnHostAI, L"仅限在本地局域网建立主机时使用，并需关闭防火墙，有问题先查看AILog.txt", 
							position2di(event.MouseInput.X, event.MouseInput.Y));
						break;
					}
					case irr::EMIE_MOUSE_WHEEL: {
						break;
					}
					default: break;
				}
				break;
			}
			default: break;
		}
		return false;
	}

void MenuHandler::showfloat(irr::gui::IGUIElement* thiselement,wchar_t* thistext,position2di mousepos)
{
	irr::gui::IGUIElement* velement=thiselement;
	irr::gui::IGUIElement* felement=mainGame->env->getFocus();
	while(velement)
	{
		if(velement->isVisible())
		{
			velement=velement->getParent();
			if(velement==felement)
				felement=0;
		}
		else
			return;
	}
	if(felement && felement->getAbsolutePosition().isPointInside(position2di(mousepos.X, mousepos.Y)))
		return;
	if(thiselement->getAbsolutePosition().isPointInside(position2di(mousepos.X, mousepos.Y)) )
	{
		position2di posUL(mousepos.X + 200 > irr::s32(mainGame->window_size.Width) ? mainGame->window_size.Width - 200 : mousepos.X , mousepos.Y + 100 > irr::s32(mainGame->window_size.Height) ? mainGame->window_size.Height - 90 : mousepos.Y + 10);
		position2di posLR(mousepos.X + 200 > irr::s32(mainGame->window_size.Width) ? mainGame->window_size.Width : mousepos.X + 200 , mousepos.Y + 100 > irr::s32(mainGame->window_size.Height) ? mainGame->window_size.Height : mousepos.Y + 100);
		mainGame->wFloat->setRelativePosition(irr::core::recti(posUL,posLR));
		mainGame->SetStaticText(mainGame->stFloat, 180, mainGame->floatFont, thistext);
		mainGame->stFloat->setRelativePosition(rect<s32>(10, (80-mainGame->stFloat->getTextHeight())>>2, 190, 90));
		mainGame->wFloat->setVisible(true);
		mainGame->env->setFocus(mainGame->wFloat);
	}
}
////
}