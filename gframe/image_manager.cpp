#include "image_manager.h"
#include "game.h"
#include "os.h"
#include <unordered_set>
namespace ygo {

ImageManager imageManager;

bool ImageManager::Initial() {
	//change
	tUnknown = driver->getTexture(L"textures/unknown.jpg");
	卡背图[0] = tUnknown;
	coverTop[0] = NULL;
	protector[0] = NULL;
	//设定卡背();
	setCovers();
	tAct = driver->getTexture(L"textures/act.png");
	tAttack = driver->getTexture(L"textures/attack.png");
	tChain = driver->getTexture(L"textures/chain.png");
	tNegated = driver->getTexture(L"textures/negated.png");
	tNumber = driver->getTexture(L"textures/number.png");
	tLPBar = driver->getTexture(L"textures/lp.png");
	tLPFrame = driver->getTexture(L"textures/lpf.png");
	tMask = driver->getTexture(L"textures/mask.png");
	tEquip = driver->getTexture(L"textures/equip.png");
	tTarget = driver->getTexture(L"textures/target.png");
	tLim = driver->getTexture(L"textures/lim.png");
	tHand[0] = driver->getTexture(L"textures/f1.jpg");
	tHand[1] = driver->getTexture(L"textures/f2.jpg");
	tHand[2] = driver->getTexture(L"textures/f3.jpg");
	//change
	//tBackGround = driver->getTexture(DataManager::随机取档(L"textures",L"bg.jpg"));
	tField = driver->getTexture(L"textures/field2.png");
	tBlank = driver->getTexture(L"textures/blank.png");
	tWindBG = driver->getTexture(dataManager.randomGet(L"skin\\windowbg", L"png").c_str());

	return true;
}
void ImageManager::SetDevice(irr::IrrlichtDevice* dev) {
	device = dev;
	driver = dev->getVideoDriver();
}
void ImageManager::ClearTexture() {
	for(auto tit = tMap.begin(); tit != tMap.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	for(auto tit = tThumb.begin(); tit != tThumb.end(); ++tit) {
		if(tit->second)
			driver->removeTexture(tit->second);
	}
	tMap.clear();
	tThumb.clear();
}
void ImageManager::RemoveTexture(int code) {
	auto tit = tMap.find(code);
	if(tit != tMap.end()) {
		if(tit->second)
			driver->removeTexture(tit->second);
		tMap.erase(tit);
	}
}

static std::unordered_set<std::wstring> notfounds;

//change
irr::video::ITexture* ImageManager::GetTexture(int code,bool 替代) {
	if(code == 0)
		return tUnknown;
	
	auto tit = tMap.find(code);
	irr::video::ITexture* img = tit==tMap.end() ?NULL :tit->second;
	if(img == NULL) {
		wchar_t file[256],filename[256];
		myswprintf(file, L"pics/%d.jpg", code);
		//change	
		static std::vector<std::wstring> dirlist = dataManager.listDir(L"expansions",L"*",true);
		int dircount = dirlist.size();
		while(--dircount>=0 && img == NULL){
			myswprintf(filename, L"%ls\\%ls",dirlist[dircount].c_str(),file);
			if(notfounds.find(filename) == notfounds.end()) {
				img = driver->getTexture(filename);
				if(img == NULL) {
					notfounds.insert(filename);
				}
			}
		}
		if(img == NULL) {
			if(notfounds.find(file) == notfounds.end()) {
				img = driver->getTexture(file);
				if(img == NULL) {
					notfounds.insert(file);
				}
			}
		}
		tMap[code] = img;
	}
	if(img)
		return img;
	if(替代) {
		return GetTextureThumb(code,false);
	}
	return tUnknown;
}
irr::video::ITexture* ImageManager::GetTextureThumb(int code,bool 替代) {
	if(code == 0)
		return tUnknown;
	auto tit = tThumb.find(code);
	//change
	
	irr::video::ITexture* img = tit==tThumb.end() ?NULL :tit->second;
	if(img == NULL) {
		wchar_t file[256],filename[256];
		myswprintf(file,L"pics/thumbnail/%d.jpg", code);
		static std::vector<std::wstring> dirlist= DataManager::listDir(L"expansions",L"*",true);
		int dircount = dirlist.size();
		while(--dircount>=0 && img == NULL){
			myswprintf(filename, L"%ls\\%ls",dirlist[dircount].c_str(),file);
			if(notfounds.find(filename) == notfounds.end()) {
				img = driver->getTexture(filename);
				if(img == NULL) {
					notfounds.insert(filename);
				}
			}
		}
		if(img == NULL) {
			if(notfounds.find(file) == notfounds.end()) {
				img = driver->getTexture(file);
				if(img == NULL) {
					notfounds.insert(file);
				}
			}
		}
		tThumb[code] = img;
	}
	if(img)
		return img;
	if(替代) {
		return GetTexture(code,false);
	}
	return tUnknown;
		
}
void ImageManager::setBackGround(std::wstring bg)
{
	auto it = imgs.find(bg);
	if(it == imgs.end())
	{
		loadList.insert(std::make_pair(bg, &imageManager.tBackGround));
	}
	else
	{
		imageManager.tBackGround = it->second;
	}
}

void ImageManager::setCovers()
{
	static std::vector<std::wstring> list = DataManager::listDir(L"textures\\cover", L"*.jpg");
	static std::vector<std::wstring> listTop = DataManager::listDir(L"textures\\cover2", L"*.png");
	static std::vector<std::wstring> listPro = DataManager::listDir(L"textures\\protector", L"*.png");

	for(int i = 0; i < 8; i++)
	{
		if(list.size())
		{
			int n = 随机数.取正整数(list.size());
			loadList.insert(std::make_pair(list[n],&卡背图[i]));
			list.erase(list.begin() + n);
		}
		else 
		{
			卡背图[i] = 卡背图[0];
		}

		if(listTop.size())
		{
			int m = 随机数.取正整数(listTop.size());
			loadList.insert(std::make_pair(listTop[m],&coverTop[i]));
			listTop.erase(listTop.begin() + m);
		}
		else 
		{
			coverTop[i] = coverTop[0];
		}
		if(listPro.size())
		{
			int l = 随机数.取正整数(listPro.size());
			loadList.insert(std::make_pair(listPro[l],&protector[i]));
			listPro.erase(listPro.begin() + l);
		}
		else 
		{
			protector[i] = protector[0];
		}
	}
}

void ImageManager::loadImages()
{
	for(auto it = loadList.begin(); it != loadList.end(); ++it)
	{
		irr::video::ITexture* img = driver->getTexture(it->first.c_str());
		imgs.insert(std::make_pair(it->first, img));
		*it->second = img;
	}
	loadList.clear();
}
}
