#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include "config.h"

namespace ygo {

class MenuHandler: public irr::IEventReceiver {
public:
	virtual bool OnEvent(const irr::SEvent& event);
	
	//new
	static bool myEvent(const irr::SEvent& event);
	static void showfloat(irr::gui::IGUIElement * thiselement,wchar_t* thistext,position2di mousepos);
};

}

#endif //MENU_HANDLER_H
