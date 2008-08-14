/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "menusettingbool.h"
#include "utilities.h"
#include <sstream>

using namespace std;
using namespace fastdelegate;

MenuSettingBool::MenuSettingBool(GMenu2X *gmenu2x, string name, string description, bool *value)
	: MenuSetting(gmenu2x,name,description) {
	this->gmenu2x = gmenu2x;
	_value = value;
	originalValue = *value;
	setValue(this->value());

	btnToggle = new IconButton(gmenu2x, "skin:imgs/buttons/b.png");
	btnToggle->setSize(16, 16);
	btnToggle->setAction(MakeDelegate(this, &MenuSettingBool::toggle));
}

void MenuSettingBool::draw(int y) {
	MenuSetting::draw(y);
	gmenu2x->s->write( gmenu2x->font, strvalue, 155, y+6, SFontHAlignLeft, SFontVAlignMiddle );
}

void MenuSettingBool::handleTS() {
	btnToggle->handleTS();
}

void MenuSettingBool::manageInput() {
#ifdef TARGET_GP2X
#include "gp2x.h"
	if ( gmenu2x->joy[GP2X_BUTTON_B] ) toggle();
#else
	if ( gmenu2x->event.key.keysym.sym==SDLK_RETURN ) toggle();
#endif
}

void MenuSettingBool::toggle() {
	setValue(!value());
}

void MenuSettingBool::setValue(bool value) {
	*_value = value;
	strvalue = value ? "ON" : "OFF";
}

bool MenuSettingBool::value() {
	return *_value;
}

void MenuSettingBool::adjustInput() {}

void MenuSettingBool::drawSelected(int) {
	gmenu2x->drawButton(btnToggle, gmenu2x->tr["Switch"], 5);
}

bool MenuSettingBool::edited() {
	return originalValue != value();
}
