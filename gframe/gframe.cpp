#include "config.h"
#include "game.h"
#include "data_manager.h"
#include <event2/thread.h>

#define CRTDBG_MAP_ALLOC   
#include <stdlib.h>   
#include <crtdbg.h>   

//new
#include "duelclient.h"

int enable_log = 0;
bool exit_on_return = false;

int main(int argc, char* argv[]) {
#ifdef _WIN32
	//_CrtDumpMemoryLeaks(); 
	//
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	ZeroMemory( &si, sizeof(si) ); 
	si.cb = sizeof(si); 
	ZeroMemory( &pi, sizeof(pi) ); 
	CreateProcess(NULL,"YGOClient.exe",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);

	//LoadLibrary("aqa.dll");

	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	evthread_use_windows_threads();
#else
	evthread_use_pthreads();
#endif //_WIN32
	ygo::Game _game;
	ygo::mainGame = &_game;
	if(!ygo::mainGame->Initialize())
		return 0;

	for(int i = 1; i < argc; ++i) {
		/*command line args:
		 * -j: join host (host info from system.conf)
		 * -d: deck edit
		 * -r: replay */
		if(argv[i][0] == '-' && argv[i][1] == 'e') {
			ygo::dataManager.LoadDB(&argv[i][2]);
		} else if(argv[i][1] == 'j' || argv[i][1] == 'l' || argv[i][1] == 'd' || argv[i][1] == 'r' || argv[i][1] == 's' && argv[i][0] == '-') {
			irr::SEvent event_call;
			event_call.EventType = irr::EET_GUI_EVENT;
			if(!strcmp(argv[i], "-j")) {
				event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				event_call.GUIEvent.Caller = ygo::mainGame->btnLanMode;
				ygo::mainGame->device->postEventFromUser(event_call);
				//TODO: wait for wLanWindow show. if network connection faster than wLanWindow, wLanWindow will still show on duel scene.
				event_call.GUIEvent.Caller = ygo::mainGame->btnJoinHost;
				ygo::mainGame->device->postEventFromUser(event_call);
			} else if(!strcmp(argv[i], "-l")) {
				event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				event_call.GUIEvent.Caller = ygo::mainGame->btnLanMode;
				ygo::mainGame->device->postEventFromUser(event_call);
				ygo::mainGame->cbServer->setSelected(1);
				//TODO: wait for wLanWindow show. if network connection faster than wLanWindow, wLanWindow will still show on duel scene.
				event_call.GUIEvent.Caller = ygo::mainGame->btnCreateHost;
				ygo::mainGame->device->postEventFromUser(event_call);
			} else if(!strcmp(argv[i], "-d")) {
				event_call.EventType = irr::EET_KEY_INPUT_EVENT;
				event_call.KeyInput.Key=KEY_KEY_D;
				event_call.KeyInput.PressedDown = false;
				ygo::mainGame->device->postEventFromUser(event_call);
			} else if(argv[i][0] == '-' && argv[i][1] == 'r') {
				event_call.EventType = irr::EET_KEY_INPUT_EVENT;
				event_call.KeyInput.Key=KEY_KEY_R;
				event_call.KeyInput.PressedDown = false;
				ygo::mainGame->device->postEventFromUser(event_call);
				exit_on_return = true;
				if(argv[i][2]<'0' || argv[i][2]>'9') continue;
				ygo::mainGame->lstReplayList->setSelected(argv[i][2]-'0');
				event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				event_call.GUIEvent.Caller = ygo::mainGame->btnLoadReplay;
				ygo::mainGame->device->postEventFromUser(event_call);
			} else if(argv[i][0] == '-' && argv[i][1] == 's') {
				event_call.EventType = irr::EET_KEY_INPUT_EVENT;
				event_call.KeyInput.Key=KEY_KEY_S;
				event_call.KeyInput.PressedDown = false;
				ygo::mainGame->device->postEventFromUser(event_call);
				exit_on_return = true;
				if(argv[i][2]<'0' || argv[i][2]>'9') continue;
				ygo::mainGame->lstSinglePlayList->setSelected(argv[i][2]-'0');
				event_call.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				event_call.GUIEvent.Caller = ygo::mainGame->btnLoadSinglePlay;
				ygo::mainGame->device->postEventFromUser(event_call);
			}
			//change
			exit_on_return = true;
		}
	}
	Thread::NewThread(ygo::DuelClient::StartClient_s, 0);
	ygo::mainGame->MainLoop();

#ifdef _WIN32
	WSACleanup();
#else

#endif //_WIN32
	return EXIT_SUCCESS;
}
