#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "config.h"
#include "data_manager.h"
#include <unordered_map>

namespace ygo {

class ImageManager {
public:
	bool Initial();
	void SetDevice(irr::IrrlichtDevice* dev);
	void ClearTexture();
	void RemoveTexture(int code);
	//change
	irr::video::ITexture* GetTexture(int code,bool Ìæ´ú=true);
	irr::video::ITexture* GetTextureThumb(int code,bool Ìæ´ú=true);
	//void Éè¶¨¿¨±³();
	void setCovers();
	void setBackGround(std::wstring bg);
	std::map<std::wstring, irr::video::ITexture**> loadList;
	void loadImages();
	
	std::unordered_map<std::wstring, irr::video::ITexture*> imgs;


	std::unordered_map<int, irr::video::ITexture*> tMap;
	std::unordered_map<int, irr::video::ITexture*> tThumb;
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
	irr::video::ITexture* ¿¨±³Í¼[8];
	irr::video::ITexture* coverTop[8];
	irr::video::ITexture* protector[8];
	irr::video::ITexture* tUnknown;
	irr::video::ITexture* tAct;
	irr::video::ITexture* tAttack;
	irr::video::ITexture* tNegated;
	irr::video::ITexture* tChain;
	irr::video::ITexture* tNumber;
	irr::video::ITexture* tLPFrame;
	irr::video::ITexture* tLPBar;
	irr::video::ITexture* tMask;
	irr::video::ITexture* tEquip;
	irr::video::ITexture* tTarget;
	irr::video::ITexture* tLim;
	irr::video::ITexture* tHand[3];
	irr::video::ITexture* tBackGround;
	irr::video::ITexture* tField;
	irr::video::ITexture* tWindBG;
	irr::video::ITexture* tBlank;
};

extern ImageManager imageManager;

}

#endif // IMAGEMANAGER_H
