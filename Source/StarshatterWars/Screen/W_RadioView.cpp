// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "W_RadioView.h"
#include "../Game/RadioMessage.h"
#include "../Game/RadioTraffic.h"
//#include "QuantumView.h"
//#include "HUDView.h"
#include "../Game/Ship.h"
#include "../Game/Element.h"
#include "../Game/Sim.h"
//#include "Starshatter.h"

//#include "CameraView.h"
//#include "Color.h"
//#include "Window.h"
//#include "Video.h"
//#include "Screen.h"
//#include "DataLoader.h"
//#include "Scene.h"
//#include "FontMgr.h"
//#include "FormatUtil.h"
//#include "Keyboard.h"
//#include "Mouse.h"
#include "../System/Game.h"
#include "../Game/Menu.h"

static Color  hud_color = Color::Black;
static Color  txt_color = Color::White;

// +====================================================================+
//
// RADIO COMMUNICATIONS MENU:
//

static Menu* fighter_menu = 0;
static Menu* starship_menu = 0;
static Menu* target_menu = 0;
static Menu* combat_menu = 0;
static Menu* formation_menu = 0;
static Menu* sensors_menu = 0;
static Menu* mission_menu = 0;
static Menu* wing_menu = 0;
static Menu* elem_menu = 0;
static Menu* control_menu = 0;

static int           starship_page = 0;
static int           num_pages = 0;
const  int           PAGE_SIZE = 9;

static MenuHistory   history;

void
UW_RadioView::Initialize()
{
	static int initialized = 0;
	if (initialized) return;

	target_menu = new Menu(Game::GetText("RadioView.menu.TARGET"));
	target_menu->AddItem(Game::GetText("RadioView.item.attack"), RadioMessage::ATTACK);
	target_menu->AddItem(Game::GetText("RadioView.item.bracket"), RadioMessage::BRACKET);
	target_menu->AddItem(Game::GetText("RadioView.item.escort"), RadioMessage::ESCORT);

	combat_menu = new Menu(Game::GetText("RadioView.menu.COMBAT"));
	combat_menu->AddItem(Game::GetText("RadioView.item.cover"), RadioMessage::COVER_ME);
	combat_menu->AddItem(Game::GetText("RadioView.item.break-attack"), RadioMessage::WEP_FREE);
	combat_menu->AddItem(Game::GetText("RadioView.item.form-up"), RadioMessage::FORM_UP);

	formation_menu = new Menu(Game::GetText("RadioView.menu.FORMATION"));
	formation_menu->AddItem(Game::GetText("RadioView.item.diamond"), RadioMessage::GO_DIAMOND);
	formation_menu->AddItem(Game::GetText("RadioView.item.spread"), RadioMessage::GO_SPREAD);
	formation_menu->AddItem(Game::GetText("RadioView.item.box"), RadioMessage::GO_BOX);
	formation_menu->AddItem(Game::GetText("RadioView.item.trail"), RadioMessage::GO_TRAIL);

	sensors_menu = new Menu(Game::GetText("RadioView.menu.SENSORS"));
	sensors_menu->AddItem(Game::GetText("RadioView.item.emcon-1"), RadioMessage::GO_EMCON1);
	sensors_menu->AddItem(Game::GetText("RadioView.item.emcon-2"), RadioMessage::GO_EMCON2);
	sensors_menu->AddItem(Game::GetText("RadioView.item.emcon-3"), RadioMessage::GO_EMCON3);
	sensors_menu->AddItem(Game::GetText("RadioView.item.probe"), RadioMessage::LAUNCH_PROBE);

	mission_menu = new Menu(Game::GetText("RadioView.menu.MISSION"));
	mission_menu->AddItem(Game::GetText("RadioView.item.skip-navpt"), RadioMessage::SKIP_NAVPOINT);
	mission_menu->AddItem(Game::GetText("RadioView.item.resume"), RadioMessage::RESUME_MISSION);
	mission_menu->AddItem(Game::GetText("RadioView.item.rtb"), RadioMessage::RTB);

	wing_menu = new Menu(Game::GetText("RadioView.menu.WINGMAN"));
	wing_menu->AddMenu(Game::GetText("RadioView.item.target"), target_menu);
	wing_menu->AddMenu(Game::GetText("RadioView.item.combat"), combat_menu);
	wing_menu->AddMenu(Game::GetText("RadioView.item.formation"), formation_menu);
	wing_menu->AddMenu(Game::GetText("RadioView.item.mission"), mission_menu);
	wing_menu->AddMenu(Game::GetText("RadioView.item.sensors"), sensors_menu);

	elem_menu = new Menu(Game::GetText("RadioView.menu.ELEMENT"));
	elem_menu->AddMenu(Game::GetText("RadioView.item.target"), target_menu);
	elem_menu->AddMenu(Game::GetText("RadioView.item.combat"), combat_menu);
	elem_menu->AddMenu(Game::GetText("RadioView.item.formation"), formation_menu);
	elem_menu->AddMenu(Game::GetText("RadioView.item.mission"), mission_menu);
	elem_menu->AddMenu(Game::GetText("RadioView.item.sensors"), sensors_menu);

	control_menu = new Menu(Game::GetText("RadioView.menu.CONTROL"));
	control_menu->AddItem(Game::GetText("RadioView.item.picture"), RadioMessage::REQUEST_PICTURE);
	control_menu->AddItem(Game::GetText("RadioView.item.backup"), RadioMessage::REQUEST_SUPPORT);
	control_menu->AddItem(Game::GetText("RadioView.item.call-inbound"), RadioMessage::CALL_INBOUND);
	control_menu->AddItem(Game::GetText("RadioView.item.call-finals"), RadioMessage::CALL_FINALS);

	fighter_menu = new Menu(Game::GetText("RadioView.menu.RADIO"));
	fighter_menu->AddMenu(Game::GetText("RadioView.item.wingman"), wing_menu);
	fighter_menu->AddMenu(Game::GetText("RadioView.item.element"), elem_menu);
	fighter_menu->AddMenu(Game::GetText("RadioView.item.control"), control_menu);

	starship_menu = new Menu(Game::GetText("RadioView.menu.RADIO"));

	initialized = 1;
}

void
UW_RadioView::Close()
{
	history.Clear();

	delete fighter_menu;
	delete starship_menu;
	delete target_menu;
	delete combat_menu;
	delete formation_menu;
	delete sensors_menu;
	delete mission_menu;
	delete wing_menu;
	delete elem_menu;
	delete control_menu;
}

static bool TargetRequired(const MenuItem* item)
{
	if (item) {
		switch (item->GetData()) {
		case RadioMessage::ATTACK:
		case RadioMessage::BRACKET:
		case RadioMessage::ESCORT:
			return true;

		default:
			//if (item->GetData() == (DWORD)target_menu)
				return true;
		}
	}
	
	return false;
}

// +====================================================================+

UW_RadioView* UW_RadioView::radio_view = 0;
ThreadSync UW_RadioView::sync;

/*UW_RadioView::UW_RadioView(Window* c)
{
	View = c;
	sim = 0;
	ship = 0;
	font = 0;
	dst_elem = 0;
	
	radio_view = this;
	sim = USim::GetSim();

	width = window->Width();
	height = window->Height();
	xcenter = (width / 2.0) - 0.5;
	ycenter = (height / 2.0) + 0.5;
	font = FontMgr::Find("HUD");

	HUDView* hud = HUDView::GetInstance();
	if (hud)
		SetColor(hud->GetTextColor());

	for (int i = 0; i < MAX_MSG; i++)
		msg_time[i] = 0;
}*/


UW_RadioView::~UW_RadioView()
{
	radio_view = 0;
}

void
UW_RadioView::OnWindowMove()
{
	/*width = window->Width();
	height = window->Height();
	xcenter = (width / 2.0) - 0.5;
	ycenter = (height / 2.0) + 0.5;*/
}

// +--------------------------------------------------------------------+

bool
UW_RadioView::Update(USimObject* obj)
{
	if (obj == ship) {
		ship = 0;
		history.Clear();
	}

	return SimObserver::Update(obj);
}

const char*
UW_RadioView::GetObserverName() const
{
	return "RadioView";
}

// +--------------------------------------------------------------------+

void
UW_RadioView::Refresh()
{
	/*sim = USim::GetSim();

	if (!font)
		return;

	font->SetColor(txt_color);
	font->SetAlpha(1);

	if (sim && ship != sim->GetPlayerShip()) {
		ship = sim->GetPlayerShip();
		history.Clear();

		if (ship) {
			if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
				ship = 0;
			}
			else {
				Observe(ship);
			}
		}
	}

	QuantumView* qv = QuantumView::GetInstance();

	if (!qv || !qv->IsMenuShown()) {
		Menu* menu = history.GetCurrent();

		if (menu) {
			Rect menu_rect(width - 115, 10, 115, 12);

			font->DrawText(menu->GetTitle(), 0, menu_rect, DT_CENTER);
			menu_rect.y += 15;

			ListIter<MenuItem> item = menu->GetItems();
			while (++item) {
				if (ship->GetEMCON() < 2 ||
					(TargetRequired(item.value()) && !ship->GetTarget()) ||
					item->GetText().contains("KIA")) {
					item->SetEnabled(false);
					font->SetAlpha(0.35);
				}
				else {
					item->SetEnabled(true);
					font->SetAlpha(1);
				}

				font->DrawText(item->GetText(), 0, menu_rect, DT_LEFT);
				menu_rect.y += 10;
			}
		}
	}

	int message_queue_empty = true;

	// age messages:
	for (int i = 0; i < MAX_MSG; i++) {
		if (msg_time[i] > 0) {
			msg_time[i] -= Game::GUITime();

			if (msg_time[i] <= 0) {
				msg_time[i] = 0;
				msg_text[i] = "";
			}

			message_queue_empty = false;
		}
	}

	if (!message_queue_empty) {
		// advance message pipeline:
		for (int i = 0; i < MAX_MSG; i++) {
			if (msg_time[0] == 0) {
				for (int j = 0; j < MAX_MSG - 1; j++) {
					msg_time[j] = msg_time[j + 1];
					msg_text[j] = msg_text[j + 1];
				}

				msg_time[MAX_MSG - 1] = 0;
				msg_text[MAX_MSG - 1] = "";
			}
		}

		bool hud_off = false;

		if (HUDView::GetInstance())
			hud_off = (HUDView::GetInstance()->GetHUDMode() == HUDView::HUD_MODE_OFF);

		// draw messages:
		if (!hud_off) {
			for (int i = 0; i < MAX_MSG; i++) {
				if (msg_time[i] > 0) {
					Rect msg_rect(0, 95 + i * 10, width, 12);

					if (msg_time[i] > 1)
						font->SetAlpha(1);
					else
						font->SetAlpha(0.5 + 0.5 * msg_time[i]);

					font->DrawText(msg_text[i].data(), msg_text[i].length(), msg_rect, DT_CENTER);
				}
			}

			font->SetAlpha(1);
		}
	}

	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->GetChatMode()) {
		Text chat;

		switch (stars->GetChatMode()) {
		case 1:  chat = "ALL:  ";  break;
		case 2:  chat = "TEAM:  ";  break;
		case 3:  chat = "WING:  ";  break;
		case 4:  chat = "UNIT:  ";  break;
		}

		chat += stars->GetChatText();

		Rect chat_rect(width / 2 - 250, height - 150, 500, 12);
		font->DrawText(chat, 0, chat_rect, DT_LEFT);

		chat_rect.Inflate(2, 2);
		window->DrawRect(chat_rect, hud_color);
	}*/
}

// +--------------------------------------------------------------------+

void
UW_RadioView::SendRadioMessage(UShip* shp, MenuItem* item)
{
	if (!ship || !item) return;
	Element* elem = ship->GetElement();
	if (!elem) return;

	// check destination:
	if (dst_elem) {
		RadioMessage* msg = new RadioMessage(dst_elem, ship, item->GetData());

		if (TargetRequired(item))
			msg->AddTarget(ship->GetTarget());

		RadioTraffic::Transmit(msg);
		dst_elem = 0;
	}

	else if (history.Find(Game::GetText("RadioView.menu.WINGMAN"))) { // wingman menu
		int      index = ship->GetElementIndex();
		int      wing = 0;

		switch (index) {
		case 1:  wing = 2; break;
		case 2:  wing = 1; break;
		case 3:  wing = 4; break;
		case 4:  wing = 3; break;
		}

		if (wing) {
			UShip* dst = elem->GetShip(wing);
			if (dst) {
				RadioMessage* msg = new RadioMessage(dst, ship, item->GetData());

				if (TargetRequired(item))
					msg->AddTarget(ship->GetTarget());

				RadioTraffic::Transmit(msg);
			}
		}
	}

	else if (history.Find(Game::GetText("RadioView.menu.ELEMENT"))) { // element menu
		RadioMessage* msg = new RadioMessage(elem, ship, item->GetData());

		if (TargetRequired(item))
			msg->AddTarget(ship->GetTarget());

		RadioTraffic::Transmit(msg);
	}

	else if (history.Find(Game::GetText("RadioView.menu.CONTROL"))) { // control menu
		RadioMessage* msg = 0;
		UShip* controller = ship->GetController();

		if (controller) {
			msg = new RadioMessage(controller, ship, item->GetData());
			RadioTraffic::Transmit(msg);
		}
	}
}

// +--------------------------------------------------------------------+

void
UW_RadioView::ExecFrame()
{
	/*HUDView* hud = HUDView::GetInstance();
	if (hud) {
		if (txt_color != hud->GetTextColor()) {
			txt_color = hud->GetTextColor();
			SetColor(txt_color);
		}

		hud_color = hud->GetHUDColor();
	}

	static int current_key = 0;

	if (current_key > 0 && Keyboard::KeyDown(current_key))
		return;

	current_key = 0;

	Menu* menu = history.GetCurrent();
	if (menu) {
		int max_items = menu->NumItems();

		if (menu == starship_menu && Keyboard::KeyDown('0')) {
			current_key = '0';
			if (++starship_page >= num_pages)
				starship_page = 0;

			history.Pop();
			history.Push(GetRadioMenu(ship));
		}
		else {
			for (int i = 0; i < max_items; i++) {
				if (Keyboard::KeyDown('1' + i)) {
					current_key = '1' + i;
					MenuItem* item = menu->GetItem(i);
					if (item && item->GetEnabled()) {
						if (item->GetSubmenu()) {
							if (history.GetCurrent() == starship_menu)
								dst_elem = (Element*)item->GetData();

							history.Push(item->GetSubmenu());
						}
						else {
							// execute radio message:
							SendRadioMessage(ship, item);

							// clear radio menu:
							history.Clear();
						}

						break;
					}
				}
			}
		}
	}*/
}

// +--------------------------------------------------------------------+

void
UW_RadioView::SetColor(Color c)
{
	/*HUDView* hud = HUDView::GetInstance();

	if (hud) {
		hud_color = hud->GetHUDColor();
		txt_color = hud->GetTextColor();
	}
	else {
		hud_color = c;
		txt_color = c;
	}*/
}

// +--------------------------------------------------------------------+

bool
UW_RadioView::IsMenuShown()
{
	return false;
	//return history.GetCurrent() != 0;
}

void
UW_RadioView::ShowMenu()
{
	if (!ship) return;

	/*if (!history.GetCurrent()) {
		history.Push(GetRadioMenu(ship));

		for (int i = 0; i < 10; i++) {
			if (Keyboard::KeyDown('1' + i)) {
				// just need to clear the key down flag
				// so we don't process old keystrokes
				// as valid menu picks...
			}
		}
	}*/
}

void
UW_RadioView::CloseMenu()
{
	//history.Clear();
	//dst_elem = 0;
	//starship_page = 0;
	//num_pages = 0;
}

// +--------------------------------------------------------------------+

Menu*
UW_RadioView::GetRadioMenu(UShip* s)
{
	dst_elem = 0;

	/*if (s && sim) {
		if (s->IsStarship()) {
			starship_menu->ClearItems();

			int n = 0;
			int page_offset = starship_page * PAGE_SIZE;

			ListIter<Element> elem = sim->GetElements();

			if (num_pages == 0) {
				while (++elem) {
					if (elem->IsFinished() || elem->IsSquadron() || elem->IsStatic())
						continue;

					if (ship->GetIFF() == elem->GetIFF() && ship->GetElement() != elem.value())
						n++;
				}

				num_pages = (n / PAGE_SIZE) + (n % PAGE_SIZE > 0);
				n = 0;
				elem.reset();
			}

			while (++elem) {
				if (elem->IsFinished() || elem->IsSquadron() || elem->IsStatic())
					continue;

				if (ship->GetIFF() == elem->GetIFF() && ship->GetElement() != elem.value()) {
					if (n >= page_offset && n < page_offset + PAGE_SIZE) {
						char text[64];
						sprintf_s(text, "%d. %s", n + 1 - page_offset, (const char*)elem->Name());

						if (elem->IsActive()) {
							starship_menu->AddMenu(text, elem_menu, (DWORD)elem.value());
						}
						else {
							strcat_s(text, " ");
							strcat_s(text, Game::GetText("RadioView.item.not-avail").data());
							starship_menu->AddItem(text, 0, false);
						}
					}
					n++;
				}
			}

			if (num_pages > 1) {
				char text[64];
				sprintf_s(text, Game::GetText("RadioView.item.next-page").data(), starship_page + 1, num_pages);
				starship_menu->AddItem(text);
			}

			return starship_menu;
		}
		else if (s->IsDropship()) {
			return fighter_menu;
		}
	}*/

	return 0;
}

// +--------------------------------------------------------------------+

void
UW_RadioView::Message(const char* msg)
{
	AutoThreadSync a(sync);

	if (radio_view) {
		int index = -1;

		for (int i = 0; i < MAX_MSG; i++) {
			if (radio_view->msg_time[i] <= 0) {
				index = i;
				break;
			}
		}

		// no space; advance pipeline:
		if (index < 0) {
			for (int i = 0; i < MAX_MSG - 1; i++) {
				radio_view->msg_text[i] = radio_view->msg_text[i + 1];
				radio_view->msg_time[i] = radio_view->msg_time[i + 1];
			}

			index = MAX_MSG - 1;
		}

		radio_view->msg_text[index] = msg;
		radio_view->msg_time[index] = 10;
	}
}

void
UW_RadioView::ClearMessages()
{
	if (radio_view) {
		for (int i = 0; i < MAX_MSG - 1; i++) {
			radio_view->msg_text[i] = Text();
			radio_view->msg_time[i] = 0;
		}
	}
}


