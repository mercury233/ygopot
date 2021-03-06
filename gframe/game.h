#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "client_field.h"
#include "deck_con.h"
#include "menu_handler.h"
#include <unordered_map>
#include <vector>
#include <list>
#include <irrKlang.h>

#include "Movies.h"
#include "CGUIWindowManager.h"
#include "CGUISkinSystem/CGUISkinSystem.h"
#include <sstream>
#include <fstream>
#include <codecvt>
//#pragma comment(lib, "irrKlang.lib")
//#include "CGUISkinSystem/CGUISkinSystem.h"

namespace ygo {

struct Config {
	bool use_d3d;
	unsigned short antialias;
	unsigned short serverport;
	unsigned char textfontsize;
	wchar_t lastip[20];
	wchar_t lastport[10];
	wchar_t nickname[20];
	wchar_t gamename[20];
	wchar_t lastdeck[64];
	wchar_t textfont[256];
	wchar_t numfont[256];
	wchar_t roompass[20];
	//new
	char other[30][256];
	unsigned char othernum;
	//new sound
	bool enablesound;
	double soundvolume;
	bool enablemusic;
	double musicvolume;
	int skin_index;
	int lflist;
	int rule;
	int pkmode;
	int server;
	int gameTimer;
	int enablePriority;
	int noCheckDeck;
	int noShuffleDeck;
	int startLP;
	int startHand;
	int DrawCount;
};

struct DuelInfo {
	bool isStarted;
	bool isReplay;
	bool isReplaySkiping;
	bool isFirst;
	bool isTag;
	bool isSingleMode;
	bool is_shuffling;
	bool tag_player[2];
	int lp[2];
	int turn;
	short curMsg;
	wchar_t hostname[20];
	wchar_t clientname[20];
	wchar_t hostname_tag[20];
	wchar_t clientname_tag[20];
	wchar_t strLP[2][16];
	wchar_t strTurn[8];
	wchar_t* vic_string;
	unsigned char player_type;
	unsigned char time_player;
	unsigned short time_limit;
	unsigned short time_left[2];
};

struct FadingUnit {
	bool signalAction;
	bool isFadein;
	int fadingFrame;
	int autoFadeoutFrame;
	irr::gui::IGUIElement* guiFading;
	irr::core::recti fadingSize;
	irr::core::vector2di fadingUL;
	irr::core::vector2di fadingLR;
	irr::core::vector2di fadingDiff;
};




class Game {

public:
	bool Initialize();
	void MainLoop();
	void BuildProjectionMatrix(irr::core::matrix4& mProjection, f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar);
	void InitStaticText(irr::gui::IGUIStaticText* pControl, u32 cWidth, u32 cHeight, irr::gui::CGUITTFont* font, const wchar_t* text);
	//change
	void SetStaticText(irr::gui::IGUIElement* pControl, u32 cWidth, irr::gui::CGUITTFont* font, const wchar_t* text, u32 pos = 0);
	void RefreshDeck(irr::gui::IGUIComboBox* cbDeck);
	void RefreshReplay();
	void RefreshSingleplay();
	void DrawSelectionLine(irr::video::S3DVertex* vec, bool strip, int width, float* cv);
	void DrawBackGround();
	void DrawCards();
	void DrawCard(ClientCard* pcard);
	void DrawMisc();
	void DrawGUI();
	void DrawSpec();
	void DrawSW();
	void setSW(unsigned int code, unsigned int fadeIn = 20, unsigned int stay = 30, unsigned int fadeOut = 20);
	void ShowElement(irr::gui::IGUIElement* element, int autoframe = 0);
	void HideElement(irr::gui::IGUIElement* element, bool set_action = false);
	void PopupElement(irr::gui::IGUIElement* element, int hideframe = 0);
	void WaitFrameSignal(int frame);
	void DrawThumb(code_pointer cp, position2di pos, std::unordered_map<int, int>* lflist, bool drag = false);
	void DrawDeckBd();
	void LoadConfig();
	void SaveConfig();
	void ShowCardInfo(int code);
	void AddChatMsg(wchar_t* msg, int player);
	void ClearTextures();
	void CloseDuelWindow();

	int LocalPlayer(int player);
	const wchar_t* LocalName(int local_player);
	
	bool HasFocus(EGUI_ELEMENT_TYPE type) const {
		irr::gui::IGUIElement* focus = env->getFocus();
		return focus && focus->hasType(type);
	}

	Mutex gMutex;
	Mutex gBuffer;
	Signal frameSignal;
	Signal actionSignal;
	Signal replaySignal;
	Signal singleSignal;
	Signal closeSignal;
	Signal closeDoneSignal;
	Config gameConf;
	DuelInfo dInfo;
	CMovies *cmovies;
	std::list<FadingUnit> fadingList;
	std::vector<int> logParam;
	std::wstring chatMsg[8];

	int hideChatTimer;
	bool hideChat;
	int chatTiming[8];
	int chatType[8];
	unsigned short linePattern;
	int waitFrame;
	int signalFrame;
	int actionParam;
	const wchar_t* showingtext;
	int showcard;
	int showcardcode;
	int showcarddif;
	int showcardp;
	int is_attacking;
	int attack_sv;
	irr::core::vector3df atk_r;
	irr::core::vector3df atk_t;
	float atkdy;
	int lpframe;
	int lpd;
	int lpplayer;
	int lpccolor;
	wchar_t* lpcstring;
	bool always_chain;
	bool ignore_chain;

	bool is_building;
	bool is_siding;

	ClientField dField;
	DeckBuilder deckBuilder;
	MenuHandler menuHandler;
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
	irr::scene::ISceneManager* smgr;
	irr::scene::ICameraSceneNode* camera;
	//GUI
	irr::gui::IGUIEnvironment* env;
	irr::gui::CGUITTFont* guiFont;
	irr::gui::CGUITTFont* textFont;
	irr::gui::CGUITTFont* numFont;
	irr::gui::CGUITTFont* adFont;
	irr::gui::CGUITTFont* lpcFont;
	std::map<irr::gui::CGUIImageButton*, int> imageLoading;

	//card image
	irr::gui::IGUIStaticText* wCardImg;
	irr::gui::IGUIImage* imgCard;
	irr::gui::IGUIImage* imgCover;
	//hint text
	irr::gui::IGUIStaticText* stHintMsg;
	irr::gui::IGUIStaticText* stTip;
	//infos
	irr::gui::IGUITabControl* wInfos;
	irr::gui::IGUIStaticText* stName;
	irr::gui::IGUIStaticText* stInfo;
	irr::gui::IGUIStaticText* stDataInfo;
	irr::gui::IGUIStaticText* stText;
	irr::gui::IGUIScrollBar* scrCardText;
	irr::gui::IGUICheckBox* chkAutoPos;
	irr::gui::IGUICheckBox* chkRandomPos;
	irr::gui::IGUICheckBox* chkAutoChain;
	irr::gui::IGUICheckBox* chkWaitChain;
	irr::gui::IGUIListBox* lstLog;
	irr::gui::IGUIButton* btnClearLog;
	irr::gui::IGUIButton* btnSaveLog;
	//main menu
	irr::gui::CGUIAdvancedWindow* wMainMenu;
	//irr::gui::IGUIWindow* wMainMenu;
	irr::gui::IGUIButton* btnLanMode;
	irr::gui::IGUIButton* btnServerMode;
	irr::gui::IGUIButton* btnReplayMode;
	irr::gui::IGUIButton* btnTestMode;
	irr::gui::IGUIButton* btnDeckEdit;
	irr::gui::IGUIButton* btnModeExit;
	//lan
	irr::gui::IGUIWindow* wLanWindow;
	irr::gui::IGUIEditBox* ebNickName;
	irr::gui::IGUIListBox* lstHostList;
	irr::gui::IGUIButton* btnLanRefresh;
	irr::gui::IGUIEditBox* ebJoinIP;
	irr::gui::IGUIEditBox* ebJoinPort;
	irr::gui::IGUIEditBox* ebJoinPass;
	irr::gui::IGUIButton* btnJoinHost;
	irr::gui::IGUIButton* btnJoinCancel;
	irr::gui::IGUIButton* btnCreateHost;
	//create host
	irr::gui::IGUIWindow* wCreateHost;
	irr::gui::IGUIComboBox* cbLFlist;
	irr::gui::IGUIComboBox* cbMatchMode;
	irr::gui::IGUIComboBox* cbRule;
	irr::gui::IGUIEditBox* ebTimeLimit;
	irr::gui::IGUIEditBox* ebStartLP;
	irr::gui::IGUIEditBox* ebStartHand;
	irr::gui::IGUIEditBox* ebDrawCount;
	irr::gui::IGUIEditBox* ebServerName;
	irr::gui::IGUIEditBox* ebServerPass;
	irr::gui::IGUICheckBox* chkEnablePriority;
	irr::gui::IGUICheckBox* chkNoCheckDeck;
	irr::gui::IGUICheckBox* chkNoShuffleDeck;
	irr::gui::IGUIButton* btnHostConfirm;
	irr::gui::IGUIButton* btnHostCancel;
	//host panel
	irr::gui::IGUIWindow* wHostPrepare;
	irr::gui::IGUIButton* btnHostPrepDuelist;
	irr::gui::IGUIButton* btnHostPrepOB;
	irr::gui::IGUIButton* btnHostAI;
	irr::gui::IGUIStaticText* stHostPrepDuelist[4];
	irr::gui::IGUICheckBox* chkHostPrepReady[4];
	irr::gui::IGUIButton* btnHostPrepKick[4];
	irr::gui::IGUIComboBox* cbDeckSelect;
	irr::gui::IGUIStaticText* stHostPrepRule;
	irr::gui::IGUIStaticText* stHostPrepOB;
	irr::gui::IGUIButton* btnHostPrepStart;
	irr::gui::IGUIButton* btnHostPrepCancel;
	//replay
	irr::gui::IGUIWindow* wReplay;
	irr::gui::IGUIListBox* lstReplayList;
	irr::gui::IGUIStaticText* stReplayInfo;
	irr::gui::IGUIButton* btnLoadReplay;
	irr::gui::IGUIButton* btnReplayCancel;
	irr::gui::IGUIButton* btnQreConvert;
	irr::gui::IGUIEditBox* ebRepStartTurn;
	//single play
	irr::gui::IGUIWindow* wSinglePlay;
	irr::gui::IGUIListBox* lstSinglePlayList;
	irr::gui::IGUIStaticText* stSinglePlayInfo;
	irr::gui::IGUIButton* btnLoadSinglePlay;
	irr::gui::IGUIButton* btnSinglePlayCancel;
	//hand
	irr::gui::IGUIWindow* wHand;
	irr::gui::IGUIButton* btnHand[3];
	//
	irr::gui::IGUIWindow* wFTSelect;
	irr::gui::IGUIButton* btnFirst;
	irr::gui::IGUIButton* btnSecond;
	//message
	irr::gui::IGUIWindow* wMessage;
	irr::gui::IGUIStaticText* stMessage;
	irr::gui::IGUIButton* btnMsgOK;
	//auto close message
	irr::gui::IGUIWindow* wACMessage;
	irr::gui::IGUIStaticText* stACMessage;
	//yes/no
	irr::gui::IGUIWindow* wQuery;
	irr::gui::IGUIStaticText* stQMessage;
	irr::gui::IGUIButton* btnYes;
	irr::gui::IGUIButton* btnNo;
	//options
	irr::gui::IGUIWindow* wOptions;
	irr::gui::IGUIStaticText* stOptions;
	irr::gui::IGUIButton* btnOptionp;
	irr::gui::IGUIButton* btnOptionn;
	irr::gui::IGUIButton* btnOptionOK;
	//pos selection
	irr::gui::IGUIWindow* wPosSelect;
	irr::gui::CGUIImageButton* btnPSAU;
	irr::gui::CGUIImageButton* btnPSAD;
	irr::gui::CGUIImageButton* btnPSDU;
	irr::gui::CGUIImageButton* btnPSDD;
	//card selection
	irr::gui::IGUIWindow* wCardSelect;
	irr::gui::CGUIImageButton* btnCardSelect[5];
	irr::gui::IGUIStaticText *stCardPos[5];
	irr::gui::IGUIScrollBar *scrCardList;
	irr::gui::IGUIButton* btnSelectOK;
	//card display
	irr::gui::IGUIWindow* wCardDisplay;
	irr::gui::CGUIImageButton* btnCardDisplay[5];
	irr::gui::IGUIStaticText *stDisplayPos[5];
	irr::gui::IGUIScrollBar *scrDisplayList;
	irr::gui::IGUIButton* btnDisplayOK;
	//announce number
	irr::gui::IGUIWindow* wANNumber;
	irr::gui::IGUIComboBox* cbANNumber;
	irr::gui::IGUIButton* btnANNumberOK;
	//announce card
	irr::gui::IGUIWindow* wANCard;
	irr::gui::IGUIEditBox* ebANCard;
	irr::gui::IGUIListBox* lstANCard;
	irr::gui::IGUIButton* btnANCardOK;
	//announce attribute
	irr::gui::IGUIWindow* wANAttribute;
	irr::gui::IGUICheckBox* chkAttribute[7];
	//announce race
	irr::gui::IGUIWindow* wANRace;
	irr::gui::IGUICheckBox* chkRace[24];
	//cmd menu
	irr::gui::IGUIWindow* wCmdMenu;
	irr::gui::IGUIButton* btnActivate;
	irr::gui::IGUIButton* btnSummon;
	irr::gui::IGUIButton* btnSPSummon;
	irr::gui::IGUIButton* btnMSet;
	irr::gui::IGUIButton* btnSSet;
	irr::gui::IGUIButton* btnRepos;
	irr::gui::IGUIButton* btnAttack;
	irr::gui::IGUIButton* btnShowList;
	irr::gui::IGUIButton* btnShuffle;
	//chat window
	irr::gui::IGUIWindow* wChat;
	irr::gui::IGUIListBox* lstChatLog;
	irr::gui::IGUIEditBox* ebChatInput;
	irr::gui::IGUICheckBox* chkIgnore1;
	irr::gui::IGUICheckBox* chkIgnore2;
	//phase button
	irr::gui::IGUIStaticText* wPhase;
	irr::gui::IGUIButton* btnDP;
	irr::gui::IGUIButton* btnSP;
	irr::gui::IGUIButton* btnM1;
	irr::gui::IGUIButton* btnBP;
	irr::gui::IGUIButton* btnM2;
	irr::gui::IGUIButton* btnEP;
	//deck edit
	irr::gui::IGUIStaticText* wDeckEdit;
	irr::gui::IGUIComboBox* cbDBLFList;
	irr::gui::IGUIComboBox* cbDBDecks;
	irr::gui::IGUIButton* btnClearDeck;
	irr::gui::IGUIButton* btnSortDeck;
	irr::gui::IGUIButton* btnShuffleDeck;
	irr::gui::IGUIButton* btnSaveDeck;
	irr::gui::IGUIButton* btnSaveDeckAs;
	irr::gui::IGUIButton* btnDBExit;
	irr::gui::IGUIButton* btnSideOK;
	irr::gui::IGUIEditBox* ebDeckname;
	//filter
	irr::gui::IGUIStaticText* wFilter;
	irr::gui::IGUIScrollBar* scrFilter;
	irr::gui::IGUIComboBox* cbCardType;
	irr::gui::IGUIComboBox* cbCardType2;
	irr::gui::IGUIComboBox* cbRace;
	irr::gui::IGUIComboBox* cbAttribute;
	irr::gui::IGUIComboBox* cbLimit;
	irr::gui::IGUIComboBox* cbSetcode;
	irr::gui::IGUIEditBox* ebStar;
	irr::gui::IGUIEditBox* ebAttack;
	irr::gui::IGUIEditBox* ebDefence;
	irr::gui::IGUIEditBox* ebCardName;
	irr::gui::IGUIButton* btnEffectFilter;
	irr::gui::IGUIButton* btnStartFilter;
	irr::gui::IGUIWindow* wCategories;
	irr::gui::IGUICheckBox* chkCategory[32];
	irr::gui::IGUIButton* btnCategoryOK;
	//replay save
	irr::gui::IGUIWindow* wReplaySave;
	irr::gui::IGUIEditBox* ebRSName;
	irr::gui::IGUIButton* btnRSYes;
	irr::gui::IGUIButton* btnRSNo;
	//replay control
	irr::gui::IGUIStaticText* wReplayControl;
	irr::gui::IGUIButton* btnReplayStart;
	irr::gui::IGUIButton* btnReplayPause;
	irr::gui::IGUIButton* btnReplayStep;
	irr::gui::IGUIButton* btnReplayExit;
	irr::gui::IGUIButton* btnReplaySwap;
	//surrender/leave
	irr::gui::IGUIButton* btnLeaveGame;
	//new
	irr::gui::IGUIComboBox* cbServer;
	irr::gui::IGUIWindow* addWaitingBox(int (*checkfun)(void*),void* checkvar,const wchar_t* text=0,
		bool modal = true, s32 id=-1);
	irr::SIrrlichtCreationParameters params;
	
	//new resize
	irr::core::dimension2d<irr::u32> window_size;
	void OnResize();
	recti Resize(s32 x, s32 y, s32 x2, s32 y2);
	recti Resize(s32 x, s32 y, s32 x2, s32 y2, s32 dx, s32 dy, s32 dx2, s32 dy2);
	position2di Resize(s32 x, s32 y, bool reverse = false);
	recti ResizeWin(s32 x, s32 y, s32 x2, s32 y2, bool chat = false);
	recti ResizeElem(s32 x, s32 y, s32 x2, s32 y2);
	//new deck edit
	irr::gui::IGUIStaticText* stLabel1;
	irr::gui::IGUIStaticText* stLabel2;
	irr::gui::IGUIStaticText* stLabel3;
	irr::gui::IGUIStaticText* stLabel4;
	irr::gui::IGUIStaticText* stLabel5;
	irr::gui::IGUIStaticText* stLabel6;
	irr::gui::IGUIStaticText* stLabel7;
	irr::gui::IGUIStaticText* stLabel8;
	irr::gui::IGUIStaticText* stLabel9;
	irr::gui::IGUIStaticText* stLabel10;
	irr::gui::IGUIStaticText* stLabel11;
	irr::gui::IGUIStaticText* stLabel12;
	irr::gui::IGUIButton* btnCardClear;
	irr::gui::IGUIButton* btnCardCopy;
	irr::gui::IGUIButton* btnDeckReturn;
	irr::gui::IGUIButton* btnCardCopyToSide;
	irr::gui::IGUIButton* btnCardMoveToSide;
	//new sound
	void PlayMusic2(wchar_t* dir,wchar_t* shuffix, bool loop);
	void PlaySound(char* sound);
	void generateTempSkin();
	irrklang::ISoundEngine* engineSound;
	irrklang::ISoundEngine* engineMusic;
	irr::gui::IGUICheckBox* chkEnableSound;
	irr::gui::IGUICheckBox* chkEnableMusic;
	irr::gui::IGUIScrollBar* scrSound;
	irr::gui::IGUIScrollBar* scrMusic;
	irr::gui::IGUIScrollBar* scrFont;
	//new float
	irr::gui::IGUIWindow* wFloat;
	irr::gui::IGUIStaticText* stFloat;
	irr::gui::CGUITTFont* floatFont;
	//new help mode
	irr::gui::IGUIButton* btnHelpMode;
	irr::gui::IGUIWindow* wHelpWindow;
	irr::gui::IGUIListBox* lstHelpList;
	irr::gui::IGUIEditBox* ebHelp;
	irr::gui::CGUITTFont* helpFont;
	irr::gui::IGUIButton* btnHelpCancel;
	irr::gui::IGUIButton* 卡组下载按钮;

	irr::gui::IGUIButton* btnWebSites;
	irr::gui::IGUIWindow* wWebWindow;
	irr::gui::IGUIListBox* lstCataList;
	irr::gui::IGUIListBox* lstWebList;
	irr::gui::IGUIButton* btnWebCancel;

	std::vector<std::pair<std::wstring, std::vector<std::pair<std::wstring, std::wstring> > > > websites;

	CGUISkinSystem *skinSystem;
	CGUIWindowManager* wm;

	bool SWPlaying;

	unsigned int nSWID, nFadeIn, nFadeOut, nStay;
	bool SWChanged;

	unsigned int fadeInAll;
	unsigned int fadeOutAll;
	unsigned int fadeInTime;
	unsigned int stayTime;
	unsigned int fadeOutTime;
	unsigned int SWID;
	unsigned int playID;

	std::unordered_map<unsigned int, irr::video::ITexture*> summonWords;
};

extern Game* mainGame;

}

#define UEVENT_EXIT			0x1
#define UEVENT_TOWINDOW		0x2

#define COMMAND_ACTIVATE	0x0001
#define COMMAND_SUMMON		0x0002
#define COMMAND_SPSUMMON	0x0004
#define COMMAND_MSET		0x0008
#define COMMAND_SSET		0x0010
#define COMMAND_REPOS		0x0020
#define COMMAND_ATTACK		0x0040
#define COMMAND_LIST		0x0080
//new
#define COMMAND_CardClear	0x0001
#define COMMAND_CardCopy	0x0002
#define COMMAND_DeckReturn	0x0004
#define COMMAND_CardCopyToSide	0x008
#define COMMAND_CardMoveToSide	0x010

#define BUTTON_LAN_MODE				100
#define BUTTON_SINGLE_MODE			101
#define BUTTON_REPLAY_MODE			102
#define BUTTON_TEST_MODE			103
#define BUTTON_DECK_EDIT			104
#define BUTTON_MODE_EXIT			105
#define LISTBOX_LAN_HOST			110
#define BUTTON_JOIN_HOST			111
#define BUTTON_JOIN_CANCEL			112
#define BUTTON_CREATE_HOST			113
#define BUTTON_HOST_CONFIRM			114
#define BUTTON_HOST_CANCEL			115
#define BUTTON_LAN_REFRESH			116
#define BUTTON_HP_DUELIST			120
#define BUTTON_HP_OBSERVER			121
#define BUTTON_HP_START				122
#define BUTTON_HP_CANCEL			123
#define BUTTON_HP_KICK				124
#define CHECKBOX_HP_READY			125
#define LISTBOX_REPLAY_LIST			130
#define BUTTON_LOAD_REPLAY			131
#define BUTTON_CANCEL_REPLAY		132
#define EDITBOX_CHAT				140
#define BUTTON_MSG_OK				200
#define BUTTON_YES					201
#define BUTTON_NO					202
#define BUTTON_HAND1				205
#define BUTTON_HAND2				206
#define BUTTON_HAND3				207
#define BUTTON_FIRST				208
#define BUTTON_SECOND				209
#define BUTTON_POS_AU				210
#define BUTTON_POS_AD				211
#define BUTTON_POS_DU				212
#define BUTTON_POS_DD				213
#define BUTTON_OPTION_PREV			220
#define BUTTON_OPTION_NEXT			221
#define BUTTON_OPTION_OK			222
#define BUTTON_CARD_0				230
#define BUTTON_CARD_1				231
#define BUTTON_CARD_2				232
#define BUTTON_CARD_3				233
#define BUTTON_CARD_4				234
#define SCROLL_CARD_SELECT			235
#define BUTTON_CARD_SEL_OK			236
#define BUTTON_CMD_ACTIVATE			240
#define BUTTON_CMD_SUMMON			241
#define BUTTON_CMD_SPSUMMON			242
#define BUTTON_CMD_MSET				243
#define BUTTON_CMD_SSET				244
#define BUTTON_CMD_REPOS			245
#define BUTTON_CMD_ATTACK			246
#define BUTTON_CMD_SHOWLIST			247
#define BUTTON_CMD_SHUFFLE			248
#define BUTTON_ANNUMBER_OK			250
#define BUTTON_ANCARD_OK			251
#define EDITBOX_ANCARD				252
#define LISTBOX_ANCARD				253
#define CHECK_ATTRIBUTE				254
#define CHECK_RACE					255
#define BUTTON_BP					260
#define BUTTON_M2					261
#define BUTTON_EP					262
#define BUTTON_LEAVE_GAME			263
#define BUTTON_CLEAR_LOG			270
#define LISTBOX_LOG					271
#define SCROLL_CARDTEXT				280
#define BUTTON_DISPLAY_0			290
#define BUTTON_DISPLAY_1			291
#define BUTTON_DISPLAY_2			292
#define BUTTON_DISPLAY_3			293
#define BUTTON_DISPLAY_4			294
#define SCROLL_CARD_DISPLAY			295
#define BUTTON_CARD_DISP_OK			296
#define BUTTON_CATEGORY_OK			300
#define COMBOBOX_DBLFLIST			301
#define COMBOBOX_DBDECKS			302
#define BUTTON_CLEAR_DECK			303
#define BUTTON_SAVE_DECK			304
#define BUTTON_SAVE_DECK_AS			305
#define BUTTON_DBEXIT				306
#define BUTTON_SORT_DECK			307
#define BUTTON_SIDE_OK				308
#define BUTTON_SHUFFLE_DECK			309
#define COMBOBOX_MAINTYPE			310
#define BUTTON_EFFECT_FILTER		311
#define BUTTON_START_FILTER			312
#define SCROLL_FILTER				314
#define EDITBOX_KEYWORD				315
#define BUTTON_REPLAY_START			320
#define BUTTON_REPLAY_PAUSE			321
#define BUTTON_REPLAY_STEP			322
#define BUTTON_REPLAY_EXIT			323
#define BUTTON_REPLAY_SWAP			324
#define BUTTON_REPLAY_SAVE			330
#define BUTTON_REPLAY_CANCEL		331
#define LISTBOX_SINGLEPLAY_LIST		350
#define BUTTON_LOAD_SINGLEPLAY		351
#define BUTTON_CANCEL_SINGLEPLAY	352

//new deck
#define COMBOBOX_CardType2			355
#define COMBOBOX_Race				356
#define COMBOBOX_Attribute			357
#define COMBOBOX_Limit				358
#define EDITBOX_Attack 				359
#define EDITBOX_Defence 			360
#define EDITBOX_Star 				361
#define BUTTON_CMD_CardClear		362
#define BUTTON_CMD_CardCopy			363
#define BUTTON_CMD_DeckReturn		364
#define BUTTON_CMD_CardCopyToSide	365
#define BUTTON_CMD_CardMoveToSide	366
#define COMBOBOX_Setcode			367

//new sound
#define CHECKBOX_ENABLE_SOUND		380
#define CHECKBOX_ENABLE_MUSIC		381
//new sound
#define SCROLL_SOUND				382
#define SCROLL_MUSIC				383

//new help
#define BUTTON_HELP_MODE			400
#define LISTBOX_HELP_LIST			401
#define BUTTON_HELP_EXIT			402
#define 卡组下载					403

#define BUTTON_WEBSITES				404
#define LISTBOX_CATA_LIST			405
#define LISTBOX_WEB_LIST			406
#define BUTTON_WEB_CANCEL			407
#define BUTTON_QRE_CONVERT			408
#define BUTTON_HOST_AI				409
#define SCROLL_FONT					410

extern bool delay_swap;
extern int swap_player;
#endif // GAME_H
