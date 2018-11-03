#include "config.h"
#include "deck_con.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "image_manager.h"
#include "game.h"
#include "duelclient.h"
#include <algorithm>

namespace ygo {

bool DeckBuilder::OnEvent(const irr::SEvent& event) {
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		s32 id = event.GUIEvent.Caller->getID();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			case BUTTON_CLEAR_DECK: {
				deckManager.current_deck.main.clear();
				deckManager.current_deck.extra.clear();
				deckManager.current_deck.side.clear();
				break;
			}
			case BUTTON_SORT_DECK: {
				std::sort(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.extra.begin(), deckManager.current_deck.extra.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.side.begin(), deckManager.current_deck.side.end(), ClientCard::deck_sort_lv);
				break;
			}
			case BUTTON_SHUFFLE_DECK: {
				std::random_shuffle(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end());
				break;
			}
			case BUTTON_SAVE_DECK: {
				if(deckManager.SaveDeck(deckManager.current_deck, mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()))) {
					mainGame->stACMessage->setText(dataManager.GetSysString(1335));
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				break;
			}
			case BUTTON_SAVE_DECK_AS: {
				const wchar_t* dname = mainGame->ebDeckname->getText();
				if(*dname == 0)
					break;
				int sel = -1;
				for(size_t i = 0; i < mainGame->cbDBDecks->getItemCount(); ++i) {
					if(!wcscmp(dname, mainGame->cbDBDecks->getItem(i))) {
						sel = i;
						break;
					}
				}
				if(sel >= 0)
					mainGame->cbDBDecks->setSelected(sel);
				else {
					mainGame->cbDBDecks->addItem(dname);
					mainGame->cbDBDecks->setSelected(mainGame->cbDBDecks->getItemCount() - 1);
				}
				if(deckManager.SaveDeck(deckManager.current_deck, dname)) {
					mainGame->stACMessage->setText(dataManager.GetSysString(1335));
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				break;
			}
			case BUTTON_DBEXIT: {
				mainGame->is_building = false;
				mainGame->wDeckEdit->setVisible(false);
				mainGame->wCategories->setVisible(false);
				mainGame->wFilter->setVisible(false);
				mainGame->wCardImg->setVisible(false);
				mainGame->wInfos->setVisible(false);
				mainGame->PopupElement(mainGame->wMainMenu);
				mainGame->device->setEventReceiver(&mainGame->menuHandler);
				mainGame->wACMessage->setVisible(false);
				imageManager.ClearTexture();
				mainGame->scrFilter->setVisible(false);
				if(mainGame->cbDBDecks->getSelected() != -1) {
					BufferIO::CopyWStr(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()), mainGame->gameConf.lastdeck, 64);
				}
				break;
				//del
			}
			case BUTTON_EFFECT_FILTER: {
				mainGame->PopupElement(mainGame->wCategories);
				break;
			}
			case BUTTON_START_FILTER: {
				FilterStart();
				FilterCards();
				mainGame->cbAttribute->setSelected(0);
				mainGame->cbRace->setSelected(0);
				mainGame->cbLimit->setSelected(0);
				mainGame->ebAttack->setText(L"");
				mainGame->ebDefence->setText(L"");
				mainGame->ebStar->setText(L"");
				mainGame->ebCardName->setText(L"");
				filter_effect = 0;
				for(int i = 0; i < 32; ++i)
					mainGame->chkCategory[i]->setChecked(false);
				break;
			}
			case BUTTON_CATEGORY_OK: {
				filter_effect = 0;
				long long filter = 0x1;
				for(int i = 0; i < 32; ++i, filter <<= 1)
					if(mainGame->chkCategory[i]->isChecked())
						filter_effect |= filter;
				mainGame->HideElement(mainGame->wCategories);
				//new
				FilterStart();
				FilterCards();
				break;
			}
			case BUTTON_SIDE_OK: {
				if(deckManager.current_deck.main.size() != pre_mainc || deckManager.current_deck.extra.size() != pre_extrac
				        || deckManager.current_deck.side.size() != pre_sidec) {
					mainGame->env->addMessageBox(L"", dataManager.GetSysString(1410));
					break;
				}
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
				break;
			}
			//new
			case BUTTON_CMD_CardClear: {
				mainGame->wCmdMenu->setVisible(false);
				if(click_pos == 1) {
					deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + click_seq);
				} else if(click_pos == 2) {
					deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + click_seq);
				} else if(click_pos == 3) {
					deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + click_seq);
				}
				break;
			}
			case BUTTON_CMD_CardCopy: {
				mainGame->wCmdMenu->setVisible(false);	
				if(CardLimit(draging_pointer) <= 0)
					break;
				if(click_pos == 1 && deckManager.current_deck.main.size() < 60) {
					deckManager.current_deck.main.insert(deckManager.current_deck.main.begin() + click_seq + 1, draging_pointer);
				} else if(click_pos == 2  && deckManager.current_deck.extra.size() < 15) {
					deckManager.current_deck.extra.insert(deckManager.current_deck.extra.begin() + click_seq + 1, draging_pointer);
				} else if(click_pos == 3  && deckManager.current_deck.side.size() < 15) {
					deckManager.current_deck.side.insert(deckManager.current_deck.side.begin() + click_seq + 1, draging_pointer);
				}
				break;
			}
			case BUTTON_CMD_DeckReturn: {
				mainGame->wCmdMenu->setVisible(false);
				if( deckManager.histroy_deck.size()>1){
					deckManager.histroy_deck.pop_back();
					deckManager.current_deck = *deckManager.histroy_deck.rbegin();
				}
				break;
			}
			case BUTTON_CMD_CardCopyToSide: {
				mainGame->wCmdMenu->setVisible(false);	
				if(CardLimit(draging_pointer) <= 0)
					break;
				if(click_pos == 1 && deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.side.insert(deckManager.current_deck.side.end(), draging_pointer);
				} else if(click_pos == 2  && deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.side.insert(deckManager.current_deck.side.end(), draging_pointer);
				} else if(click_pos == 3) {
					if(draging_pointer->second.type & 0x802040) {
						if(deckManager.current_deck.extra.size() < 15)
							deckManager.current_deck.extra.insert(deckManager.current_deck.extra.end(), draging_pointer);
					} else {
						if(deckManager.current_deck.main.size() < 60)
							deckManager.current_deck.main.insert(deckManager.current_deck.main.end(), draging_pointer);
					}
				}
				break;
			}
			case BUTTON_CMD_CardMoveToSide: {
				mainGame->wCmdMenu->setVisible(false);
				if(click_pos == 1 && deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.side.insert(deckManager.current_deck.side.end(), draging_pointer);
					deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + click_seq);
				} else if(click_pos == 2  && deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.side.insert(deckManager.current_deck.side.end(), draging_pointer);
					deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + click_seq);
				} else if(click_pos == 3) {
					if(draging_pointer->second.type & 0x802040) {
						if(deckManager.current_deck.extra.size() < 15)
							deckManager.current_deck.extra.insert(deckManager.current_deck.extra.end(), draging_pointer);
							deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + click_seq);
					} else {
						if(deckManager.current_deck.main.size() < 60)
							deckManager.current_deck.main.insert(deckManager.current_deck.main.end(), draging_pointer);
							deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + click_seq);
					}
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_SCROLL_BAR_CHANGED: {
			switch(id) {
			case SCROLL_CARDTEXT: {
				u32 pos = mainGame->scrCardText->getPos();
				mainGame->SetStaticText(mainGame->stText, mainGame->stText->getRelativePosition().getWidth()-25, mainGame->textFont, mainGame->showingtext, pos);
				break;
			}
			//new sound
			case SCROLL_SOUND: {
				mainGame->gameConf.soundvolume = (double)mainGame->scrSound->getPos() /100;
				if(mainGame->engineSound)
					mainGame->engineSound->setSoundVolume(mainGame->gameConf.soundvolume);
				break;
			}
			case SCROLL_MUSIC: {
				mainGame->gameConf.musicvolume = (double)mainGame->scrMusic->getPos() /100;
				if(mainGame->engineMusic)
					mainGame->engineMusic->setSoundVolume(mainGame->gameConf.musicvolume);
				break;
			}
			case SCROLL_FONT: {
				mainGame->gameConf.textfontsize = mainGame->scrFont->getPos();
				auto tmp = mainGame->guiFont;
				mainGame->guiFont = irr::gui::CGUITTFont::createTTFont(mainGame->env, 
					mainGame->gameConf.textfont, mainGame->gameConf.textfontsize);
				mainGame->textFont = mainGame->guiFont;
				mainGame->env->getSkin()->setFont(mainGame->guiFont);
				tmp->drop();
				break;
			}
			break;
			}
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			/*
			switch(id) {
			case EDITBOX_KEYWORD: {
				irr::SEvent me;
				me.EventType = irr::EET_GUI_EVENT;
				me.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				me.GUIEvent.Caller = mainGame->btnStartFilter;
				me.GUIEvent.Element = mainGame->btnStartFilter;
				mainGame->device->postEventFromUser(me);
				break;
			}
			}
			*/
			break;
		}
		//new sound
		case irr::gui::EGET_EDITBOX_CHANGED: {
			/*
			switch(id) {
			case EDITBOX_KEYWORD: {
				stringw filter = mainGame->ebCardName->getText();
					FilterStart();
					FilterCards();
				break;
			}
			}
			*/
			if(id==EDITBOX_KEYWORD || id==EDITBOX_Attack || id==EDITBOX_Defence || id==EDITBOX_Star)
			{
				FilterStart();
				FilterCards();
			}
			break;
		}
		case irr::gui::EGET_CHECKBOX_CHANGED:{
			s32 id = event.GUIEvent.Caller->getID();
			switch(id) {
			case CHECKBOX_ENABLE_SOUND:{
				mainGame->gameConf.enablesound = mainGame->chkEnableSound->isChecked();
				break;
			}
			case CHECKBOX_ENABLE_MUSIC:{
				mainGame->gameConf.enablemusic = mainGame->chkEnableMusic->isChecked();
				if(!mainGame->gameConf.enablemusic && mainGame->engineMusic)
					mainGame->engineMusic->stopAllSounds();
				break;
		    }
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch(id) {
			case COMBOBOX_DBLFLIST: {
				filterList = deckManager._lfList[mainGame->cbDBLFList->getSelected()].content;
				break;
			}
			case COMBOBOX_DBDECKS: {
				deckManager.LoadDeck(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()));
				break;
			}
			case COMBOBOX_MAINTYPE: {
				switch(mainGame->cbCardType->getSelected()) {
				case 0: {
					mainGame->cbCardType2->setEnabled(false);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					break;
				}
				case 1: {
					wchar_t syntuner[32];
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(true);
					mainGame->cbAttribute->setEnabled(true);
					mainGame->ebAttack->setEnabled(true);
					mainGame->ebDefence->setEnabled(true);
					mainGame->ebStar->setEnabled(true);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_MONSTER + TYPE_NORMAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1055), TYPE_MONSTER + TYPE_EFFECT);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1056), TYPE_MONSTER + TYPE_FUSION);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1057), TYPE_MONSTER + TYPE_RITUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1063), TYPE_MONSTER + TYPE_SYNCHRO);
					myswprintf(syntuner, L"%ls|%ls", dataManager.GetSysString(1063), dataManager.GetSysString(1062));
					mainGame->cbCardType2->addItem(syntuner, TYPE_MONSTER + TYPE_SYNCHRO + TYPE_TUNER);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1073), TYPE_MONSTER + TYPE_XYZ);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1074), TYPE_MONSTER + TYPE_PENDULUM);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1062), TYPE_MONSTER + TYPE_TUNER);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1061), TYPE_MONSTER + TYPE_DUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1060), TYPE_MONSTER + TYPE_UNION);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1059), TYPE_MONSTER + TYPE_SPIRIT);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1071), TYPE_MONSTER + TYPE_FLIP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1072), TYPE_MONSTER + TYPE_TOON);
					break;
				}
				case 2: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_SPELL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1066), TYPE_SPELL + TYPE_QUICKPLAY);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1067), TYPE_SPELL + TYPE_CONTINUOUS);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1057), TYPE_SPELL + TYPE_RITUAL);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1068), TYPE_SPELL + TYPE_EQUIP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1069), TYPE_SPELL + TYPE_FIELD);
					break;
				}
				case 3: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1080), 0);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1054), TYPE_TRAP);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1067), TYPE_TRAP + TYPE_CONTINUOUS);
					mainGame->cbCardType2->addItem(dataManager.GetSysString(1070), TYPE_TRAP + TYPE_COUNTER);
					break;
				}
				}
			}
			}
			if(id==COMBOBOX_MAINTYPE || id==COMBOBOX_CardType2 || id==COMBOBOX_Race || id==COMBOBOX_Attribute || id==COMBOBOX_Limit || id == COMBOBOX_Setcode)
			{
				FilterStart();
				FilterCards();
			}
		}
		default: break;
		}
		break;
	}
	case irr::EET_MOUSE_INPUT_EVENT: {
		switch(event.MouseInput.Event) {
		case irr::EMIE_LMOUSE_PRESSED_DOWN: {
			is_clicking = false;
			if(mainGame->wCategories->isVisible() || mainGame->HasFocus(EGUIET_LIST_BOX))
				break;
			//new
			if(mainGame->wCmdMenu->isVisible() && !mainGame->wCmdMenu->getRelativePosition().isPointInside(position2di(event.MouseInput.X, event.MouseInput.Y)))
				mainGame->wCmdMenu->setVisible(false);
			if(mainGame->wCmdMenu->isVisible())
				break;
			if( (!hovered_pos || hovered_pos > 4) || hovered_seq == -1) {
				draging_pointer = dataManager._datas.end();
				break;
			}
			click_pos = hovered_pos;
			click_seq = hovered_seq;
			draging_pointer = dataManager.GetCodePointer(hovered_code);
			if(draging_pointer == dataManager._datas.end())
				break;
			is_clicking = true;
			break;
		}
		case irr::EMIE_LMOUSE_LEFT_UP: {
			//new
			if(mainGame->wCmdMenu->isVisible() && !mainGame->wCmdMenu->getRelativePosition().isPointInside(position2di(event.MouseInput.X, event.MouseInput.Y)))
				mainGame->wCmdMenu->setVisible(false);
			if(mainGame->wCmdMenu->isVisible() || mainGame->HasFocus(EGUIET_LIST_BOX))
				break;
			if(!is_draging){
				if(is_clicking && !mainGame->is_siding && click_pos != 4){
					ShowMenu(0x1F,event.MouseInput.X,event.MouseInput.Y);
					is_clicking = false;
				}
				if(mainGame->is_siding)
					is_clicking = false;
				if(!(!mainGame->is_siding && hovered_pos == 4))
					break;
			}
			is_clicking = false;
			if(!mainGame->is_siding) {
				if((hovered_pos == 1 && (draging_pointer->second.type & 0x802040)) || (hovered_pos == 2 && !(draging_pointer->second.type & 0x802040)))
					hovered_pos = 0;
				if((hovered_pos == 1 || (hovered_pos == 0 && click_pos == 1)) && deckManager.current_deck.main.size() < 60) {
					if(hovered_seq == -1)
						deckManager.current_deck.main.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.main.size() && hovered_pos)
						deckManager.current_deck.main.insert(deckManager.current_deck.main.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.main.push_back(draging_pointer);
					is_draging = false;
				} else if((hovered_pos == 2 || (hovered_pos == 0 && click_pos == 2)) && deckManager.current_deck.extra.size() < 15) {
					if(hovered_seq == -1)
						deckManager.current_deck.extra.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.extra.size() && hovered_pos)
						deckManager.current_deck.extra.insert(deckManager.current_deck.extra.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.extra.push_back(draging_pointer);
					is_draging = false;
				} else if((hovered_pos == 3 || (hovered_pos == 0 && click_pos == 3)) && deckManager.current_deck.side.size() < 15) {
					if(hovered_seq == -1)
						deckManager.current_deck.side.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.side.size() && hovered_pos)
						deckManager.current_deck.side.insert(deckManager.current_deck.side.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.side.push_back(draging_pointer);
					is_draging = false;
				} else if (hovered_pos == 4) {
					if(!is_draging) {
						if(draging_pointer == dataManager._datas.end())
							break;
							//change
						if(CardLimit(draging_pointer) <= 0)
							break;
						if((draging_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 15) {
							deckManager.current_deck.extra.push_back(draging_pointer);
						} else if(!(draging_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 60) {
							deckManager.current_deck.main.push_back(draging_pointer);
						}
					} else {
						is_draging = false;
					}
				}
			} else {
				if((hovered_pos == 1 && (draging_pointer->second.type & 0x802040)) || (hovered_pos == 2 && !(draging_pointer->second.type & 0x802040)))
					hovered_pos = 0;
				if(hovered_pos == 4)
					hovered_pos = 0;
				if((hovered_pos == 1 || (hovered_pos == 0 && click_pos == 1)) && deckManager.current_deck.main.size() < 65) {
					if(hovered_seq == -1)
						deckManager.current_deck.main.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.main.size() && hovered_pos)
						deckManager.current_deck.main.insert(deckManager.current_deck.main.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.main.push_back(draging_pointer);
					is_draging = false;
				} else if((hovered_pos == 2 || (hovered_pos == 0 && click_pos == 2)) && deckManager.current_deck.extra.size() < 20) {
					if(hovered_seq == -1)
						deckManager.current_deck.extra.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.extra.size() && hovered_pos)
						deckManager.current_deck.extra.insert(deckManager.current_deck.extra.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.extra.push_back(draging_pointer);
					is_draging = false;
				} else if((hovered_pos == 3 || (hovered_pos == 0 && click_pos == 3)) && deckManager.current_deck.side.size() < 20) {
					if(hovered_seq == -1)
						deckManager.current_deck.side.push_back(draging_pointer);
					else if(hovered_seq < (int)deckManager.current_deck.side.size() && hovered_pos)
						deckManager.current_deck.side.insert(deckManager.current_deck.side.begin() + hovered_seq, draging_pointer);
					else deckManager.current_deck.side.push_back(draging_pointer);
					is_draging = false;
				}
			}
			break;
		}
		case irr::EMIE_RMOUSE_LEFT_UP: {
			is_clicking = false;
			if(mainGame->is_siding) {
				if(is_draging)
					break;
				if(hovered_pos == 0 || hovered_seq == -1)
					break;
				code_pointer rclick_pointer = dataManager.GetCodePointer(hovered_code);
				if(rclick_pointer == dataManager._datas.end())
					break;
				if(hovered_pos == 1) {
					if(deckManager.current_deck.side.size() < 20) {
						deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + hovered_seq);
						deckManager.current_deck.side.push_back(rclick_pointer);
					}
				} else if(hovered_pos == 2) {
					if(deckManager.current_deck.side.size() < 20) {
						deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + hovered_seq);
						deckManager.current_deck.side.push_back(rclick_pointer);
					}
				} else {
					if((rclick_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 20) {
						deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
						deckManager.current_deck.extra.push_back(rclick_pointer);
					}
					if(!(rclick_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 64) {
						deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
						deckManager.current_deck.main.push_back(rclick_pointer);
					}
				}
				break;
			}
			//new
			if(mainGame->wCmdMenu->isVisible()){
				mainGame->wCmdMenu->setVisible(false);
				break;
			}
				
			if(mainGame->wCategories->isVisible() || mainGame->HasFocus(EGUIET_LIST_BOX))
				break;
			if(hovered_pos == 0 || hovered_seq == -1)
			{
				ShowMenu(COMMAND_DeckReturn,event.MouseInput.X,event.MouseInput.Y);
				break;
			}
			
			if(hovered_pos == 1) {
				if(!is_draging && !is_clicking)
					deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + hovered_seq);
				else if(deckManager.current_deck.side.size() < 15) {
					deckManager.current_deck.side.push_back(draging_pointer);
					is_draging = false;
				}
			} else if(hovered_pos == 2) {
				if(!is_draging && !is_clicking)
					deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + hovered_seq);
				else if(deckManager.current_deck.side.size() < 15) {
					deckManager.current_deck.side.push_back(draging_pointer);
					is_draging = false;
				}
			} else if(hovered_pos == 3) {
				if(!is_draging && !is_clicking)
					deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
				else {
					if((draging_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 15) {
						deckManager.current_deck.extra.push_back(draging_pointer);
						is_draging = false;
					} else if(!(draging_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 60) {
						deckManager.current_deck.main.push_back(draging_pointer);
						is_draging = false;
					}
				}
			} else {
				if(is_draging) {
					if(deckManager.current_deck.side.size() < 15) {
						deckManager.current_deck.side.push_back(draging_pointer);
						is_draging = false;
					}
				} else {
					
					code_pointer rclick_pointer = dataManager.GetCodePointer(hovered_code);
					if(rclick_pointer == dataManager._datas.end())
						break;
					if(CardLimit(rclick_pointer) <= 0)
						break;
					if(deckManager.current_deck.side.size() < 20) {
						deckManager.current_deck.side.push_back(rclick_pointer);
					}
					is_draging = false;
				}
			}
			break;
		}
		case irr::EMIE_MOUSE_MOVED: {
			//change
			if(mainGame->window_size.Width == 0 || mainGame->window_size.Height == 0)
			{
				break;
			}
			position2di pos = mainGame->Resize(event.MouseInput.X, event.MouseInput.Y, true);
			position2di mousepos(event.MouseInput.X, event.MouseInput.Y);
			s32 x = pos.X;
			s32 y = pos.Y;
			int pre_code = hovered_code;
			if(x >= 314 && x <= 794 && y >= 164 && y <= 435) {
				int lx = 10, px, py = (y - 164) / 68;
				hovered_pos = 1;
				if(deckManager.current_deck.main.size() > 40)
					lx = (deckManager.current_deck.main.size() - 41) / 4 + 11;
				if(x >= 750)
					px = lx - 1;
				else px = (x - 314) * (lx - 1) / 436;
				if(py*lx + px >= (int)deckManager.current_deck.main.size()) {
					hovered_seq = -1;
					hovered_code = 0;
				} else {
					hovered_seq = py * lx + px;
					hovered_code = deckManager.current_deck.main[hovered_seq]->first;
				}
			} else if(x >= 314 && x <= 794 && y >= 466 && y <= 530) {
				int lx = deckManager.current_deck.extra.size();
				hovered_pos = 2;
				if(lx < 10)
					lx = 10;
				if(x >= 750)
					hovered_seq = lx - 1;
				else hovered_seq = (x - 314) * (lx - 1) / 436;
				if(hovered_seq >= (int)deckManager.current_deck.extra.size()) {
					hovered_seq = -1;
					hovered_code = 0;
				} else {
					hovered_code = deckManager.current_deck.extra[hovered_seq]->first;
				}
			} else if (x >= 314 && x <= 794 && y >= 564 && y <= 628) {
				int lx = deckManager.current_deck.side.size();
				hovered_pos = 3;
				if(lx < 10)
					lx = 10;
				if(x >= 750)
					hovered_seq = lx - 1;
				else hovered_seq = (x - 314) * (lx - 1) / 436;
				if(hovered_seq >= (int)deckManager.current_deck.side.size()) {
					hovered_seq = -1;
					hovered_code = 0;
				} else {
					hovered_code = deckManager.current_deck.side[hovered_seq]->first;
				}
			} else if(x >= 810 && x <= 995 && y >= 165 && y <= 626) {
				hovered_pos = 4;
				hovered_seq = (y - 165) / 66;
				if(mainGame->scrFilter->getPos() + hovered_seq >= (int)results.size()) {
					hovered_seq = -1;
					hovered_code = 0;
				} else {
					hovered_code = results[mainGame->scrFilter->getPos() + hovered_seq]->first;
				}
			} else {
				hovered_pos = 0;
				hovered_code = 0;
			}
			//new
			if(is_clicking && (hovered_pos  != click_pos || hovered_seq != click_seq) && (click_pos != 4 || CardLimit(draging_pointer)) ){
				if(click_pos == 1)
					deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + click_seq);
				else if(click_pos == 2)
					deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + click_seq);
				else if(click_pos == 3)
					deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + click_seq);
				is_draging = true;
				is_clicking = false;
			}
			
			if(is_draging) {
				dragx = mousepos.X;
				dragy = mousepos.Y;
			}
			if(!is_draging && pre_code != hovered_code) {
				if(hovered_code) {
					mainGame->imgCover->setImage(imageManager.protector[0]);
					mainGame->ShowCardInfo(hovered_code);
				}
				if(pre_code)
					imageManager.RemoveTexture(pre_code);
			}	
			break;
		}
		case irr::EMIE_MOUSE_WHEEL: {
			if(!mainGame->scrFilter->isVisible())
				break;
			if(event.MouseInput.Wheel < 0) {
				if(mainGame->scrFilter->getPos() < mainGame->scrFilter->getMax())
					mainGame->scrFilter->setPos(mainGame->scrFilter->getPos() + 1);
			} else {
				if(mainGame->scrFilter->getPos() > 0)
					mainGame->scrFilter->setPos(mainGame->scrFilter->getPos() - 1);
			}
			SEvent e = event;
			e.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			mainGame->device->postEventFromUser(e);
			break;
		}
		default: break;
		}
		break;
	}
	/*
	case irr::EET_KEY_INPUT_EVENT: {
		switch(event.KeyInput.Key) {
		case irr::KEY_KEY_R: {
			if(!event.KeyInput.PressedDown)
				mainGame->textFont->setTransparency(true);
			break;
		}
		case irr::KEY_ESCAPE: {
			mainGame->device->minimizeWindow();
			break;
		}
		default: break;
		}
		break;
	}*/
	default: break;
	}
	//change
	return MenuHandler::myEvent(event);
}
void DeckBuilder::FilterCards() {
	results.clear();
	const wchar_t* pstr = mainGame->ebCardName->getText();
	int trycode = BufferIO::GetVal(pstr);
	if(dataManager.GetData(trycode, 0)) {
		auto ptr = dataManager.GetCodePointer(trycode);	// verified by GetData()
		results.push_back(ptr);
		mainGame->scrFilter->setVisible(false);
		mainGame->scrFilter->setPos(0);
		myswprintf(result_string, L"%d", results.size());
		return;
	}
	if((pstr[0] == L'$' && pstr[1] == 0))
		pstr = 0;

	int setcode_type = mainGame->cbSetcode->getItemData(mainGame->cbSetcode->getSelected());
	auto strpointer = dataManager._strings.begin();
	for(code_pointer ptr = dataManager._datas.begin(); ptr != dataManager._datas.end(); ++ptr, ++strpointer) {
		const CardDataC& data = ptr->second;
		const CardString& text = strpointer->second;
		
		if(data.type & TYPE_TOKEN)
			continue;
		
		if(setcode_type) {
			unsigned long long setcode = data.setcode;
			unsigned long long settype = setcode_type & 0x0fff;
			unsigned long long setsubtype = setcode_type & 0xf000;
			bool found = false;
			while(setcode)
			{
				if((setcode & 0x0fff) == settype &&
						(setsubtype == 0 || (setcode & 0xf000) == setsubtype))
				{
					found = true;
					break;					}
					setcode = setcode >> 16;
				}
			if(!found)
			{
				 continue;
			}
		}
		switch(filter_type) {
		case 1: {
			int type2 = data.type & 0xe03ef1;
			if(!(data.type & TYPE_MONSTER) || (filter_type2 == 0x21 && type2 != 0x21) || (data.type & filter_type2) != filter_type2)
				continue;
			if(filter_race && data.race != filter_race)
				continue;
			if(filter_attrib && data.attribute != filter_attrib)
				continue;
			//change

			const wchar_t* pstr = mainGame->ebAttack->getText();
			int low,high;
			low=DataManager::getRange(pstr,high);
			if(data.attack>high || data.attack<low)
				continue;
			pstr = mainGame->ebDefence->getText();
			low=DataManager::getRange(pstr,high);
			if(data.defence>high || data.defence<low)
				continue;
			pstr = mainGame->ebStar->getText();
			low=DataManager::getRange(pstr,high);
			int cont = 0;
			if(int(data.level)>high || int(data.level)<low)
				cont++;
			if(int(data.lscale)>high || int(data.lscale)<low)
				cont++;
			if(cont == 2)
				continue;
			break;
		}
		case 2: {
			if(!(data.type & TYPE_SPELL))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		case 3: {
			if(!(data.type & TYPE_TRAP))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		}
		if(filter_effect && !(data.category & filter_effect))
			continue;
		if(filter_lm) {
			if(filter_lm <= 3 && (!filterList->count(ptr->first) || (*filterList)[ptr->first] != filter_lm - 1))
				continue;
			if(filter_lm == 4 && data.ot != 1)
				continue;
			if(filter_lm == 5 && data.ot != 2)
				continue;
		}
		//change
		if(pstr) {
			if(pstr[0] == L'$') {
				if(CardNameCompare(text.name, &pstr[1]) == 0)
					continue;
			}
			else {
				if(CardNameCompare(text.name, pstr) == 0 && CardNameCompare(text.text, pstr) == 0)
					continue;
			}
		}
		results.push_back(ptr);
	}

	myswprintf(result_string, L"%d", results.size());
	if(results.size() > 7) {
		mainGame->scrFilter->setVisible(true);
		mainGame->scrFilter->setMax(results.size() - 7);
		mainGame->scrFilter->setPos(0);
	} else {
		mainGame->scrFilter->setVisible(false);
		mainGame->scrFilter->setPos(0);
	}
	std::sort(results.begin(), results.end(), ClientCard::deck_sort_lv);
}

//new
void DeckBuilder::FilterStart(){
	filter_type = mainGame->cbCardType->getSelected();
				filter_type2 = mainGame->cbCardType2->getItemData(mainGame->cbCardType2->getSelected());
				filter_lm = mainGame->cbLimit->getSelected();
				if(filter_type > 1) {
					FilterCards();
					return;
				}
				filter_attrib = mainGame->cbAttribute->getItemData(mainGame->cbAttribute->getSelected());
				filter_race = mainGame->cbRace->getItemData(mainGame->cbRace->getSelected());
}

//new
bool DeckBuilder::CardNameCompare(const wchar_t *sa, const wchar_t *sb)
{
	if (!sa || !sb)
		return false;
	if(!sb[0])
		return true;
	int i = 0;
	int j = 0;
	wchar_t ca;
	wchar_t cb;
	while (sa[i])
	{
		ca = towupper(sa[i]);
		cb = towupper(sb[j]);
		if (ca == ' ') ca = '-';
		if (ca == cb)
		{
			j++;
			if (!sb[j])
				return true;
			if (sb[j]==' ')
				return CardNameCompare(sa+i+1,sb+j+1);
		}
		else
			j = 0;
		i++;
	}
	return false;
}

static bool deckEqual(std::vector<code_pointer> &a, std::vector<code_pointer> &b)
{
	if(a.size() != b.size())
		return false;
	int size = a.size();
	for(int i = 0; i < size; i++) {
		if(a[i] != b[i])
			return false;
	}
	return true;
}

void DeckBuilder::ShowMenu(int flag, int x, int y) {
	mainGame->btnActivate->setVisible(false);
	mainGame->btnSummon->setVisible(false);
	mainGame->btnSPSummon->setVisible(false);
	mainGame->btnMSet->setVisible(false);
	mainGame->btnSSet->setVisible(false);
	mainGame->btnRepos->setVisible(false);
	mainGame->btnAttack->setVisible(false);
	mainGame->btnShowList->setVisible(false);
	//
	int height = 1;
	if(flag & COMMAND_CardCopy && CardLimit(draging_pointer)>0) {
		mainGame->btnCardCopy->setVisible(true);
		mainGame->btnCardCopy->setRelativePosition(position2di(1, height));
		height += 21;
	} else mainGame->btnCardCopy->setVisible(false);
	if(flag & COMMAND_DeckReturn && deckManager.histroy_deck.size()>1) {
		
		bool visible = true;
		

		if((flag & COMMAND_DeckReturn) && (flag & COMMAND_CardClear)) {
			ygo::Deck tmp(deckManager.current_deck), history(*(deckManager.histroy_deck.end() - 2));
			if(click_pos == 1) {
				tmp.main.erase(tmp.main.begin() + click_seq);
			} else if(click_pos == 2) {
 				tmp.extra.erase(tmp.extra.begin() + click_seq);
			} else if(click_pos == 3) {
				tmp.side.erase(tmp.side.begin() + click_seq);
			}
			if(deckEqual(tmp.main, history.main) && deckEqual(tmp.extra, history.extra) && deckEqual(tmp.side, history.side)) {
				visible = false;
			}
		}
		visible = true;
		if(visible) {
			mainGame->btnDeckReturn->setVisible(true);
			mainGame->btnDeckReturn->setRelativePosition(position2di(1, height));
			height += 21;
		} else {
			mainGame->btnDeckReturn->setVisible(false);
			flag -= flag & COMMAND_DeckReturn;
		}
	} else {
		mainGame->btnDeckReturn->setVisible(false);
		flag -= flag & COMMAND_DeckReturn;
	}
	if(flag & COMMAND_CardClear) {
		mainGame->btnCardClear->setVisible(true);
		mainGame->btnCardClear->setRelativePosition(position2di(1, height));
		height += 21;
	} else mainGame->btnCardClear->setVisible(false);
	if(flag & COMMAND_CardCopyToSide && CardLimit(draging_pointer)>0) {
		if(click_pos == 3)
			mainGame->btnCardCopyToSide->setText(L"复制到Main");
		else
			mainGame->btnCardCopyToSide->setText(L"复制到Side");
		mainGame->btnCardCopyToSide->setVisible(true);
		mainGame->btnCardCopyToSide->setRelativePosition(position2di(1, height));
		height += 21;
	} else mainGame->btnCardCopyToSide->setVisible(false);
	if(flag & COMMAND_CardMoveToSide) {
		if(click_pos == 3)
			mainGame->btnCardMoveToSide->setText(L"移动到Main");
		else
			mainGame->btnCardMoveToSide->setText(L"移动到Side");
		mainGame->btnCardMoveToSide->setVisible(true);
		mainGame->btnCardMoveToSide->setRelativePosition(position2di(1, height));
		height += 21;
	} else mainGame->btnCardMoveToSide->setVisible(false);

	if(!flag) {
		mainGame->wCmdMenu->setVisible(false);
		return;
	}
	if(click_pos == 3)
		y -= 50;
	mainGame->wCmdMenu->setVisible(true);
	mainGame->wCmdMenu->setRelativePosition(irr::core::recti(x - 20 , y -10, x + 80, y -10 + height));
	mainGame->env->setFocus(mainGame->wCmdMenu);
}

int DeckBuilder::CardLimit(code_pointer& this_pointer)
{
	unsigned int limitcode = this_pointer->second.alias ? this_pointer->second.alias : this_pointer->first;
	int limit = 3;
	if(mainGame->deckBuilder.filterList->count(limitcode))
		limit = (*mainGame->deckBuilder.filterList)[limitcode];
	for(size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
		if(deckManager.current_deck.main[i]->first == limitcode
		        || deckManager.current_deck.main[i]->second.alias == limitcode)
			limit--;
	for(size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
		if(deckManager.current_deck.extra[i]->first == limitcode
		        || deckManager.current_deck.extra[i]->second.alias == limitcode)
			limit--;
	for(size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
		if(deckManager.current_deck.side[i]->first == limitcode
		        || deckManager.current_deck.side[i]->second.alias == limitcode)
			limit--;
	return limit;
}
}
