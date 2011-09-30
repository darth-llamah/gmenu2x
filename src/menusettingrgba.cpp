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
#include "menusettingrgba.h"
#include "gmenu2x.h"

#include <sstream>

using std::string;
using std::stringstream;
using fastdelegate::MakeDelegate;

MenuSettingRGBA::MenuSettingRGBA(
		GMenu2X *gmenu2x, const string &name,
		const string &description, RGBAColor *value)
	: MenuSetting(gmenu2x,name,description)
{
	edit = false;

	selPart = 0;
	_value = value;
	originalValue = *value;
	this->setR(this->value().r);
	this->setG(this->value().g);
	this->setB(this->value().b);
	this->setA(this->value().a);

	updateButtonBox();
}

void MenuSettingRGBA::draw(int y) {
	this->y = y;
	MenuSetting::draw(y);
	gmenu2x->s->rectangle( 153, y+1, 11, 11, 0,0,0,255 );
	gmenu2x->s->box( 154, y+2, 9, 9, value() );
	gmenu2x->s->write( gmenu2x->font, "R: "+strR, 169, y, ASFont::HAlignLeft, ASFont::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "G: "+strG, 205, y, ASFont::HAlignLeft, ASFont::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "B: "+strB, 241, y, ASFont::HAlignLeft, ASFont::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "A: "+strA, 277, y, ASFont::HAlignLeft, ASFont::VAlignTop );
}

void MenuSettingRGBA::handleTS() {
	if (gmenu2x->ts.pressed()) {
		for (int i=0; i<4; i++) {
			if (i!=selPart && gmenu2x->ts.inRect(166+i*36,y,36,14)) {
				selPart = i;
				i = 4;
			}
		}
	}

	MenuSetting::handleTS();
}

bool MenuSettingRGBA::manageInput(bevent_t *event) {
	if (edit) {
	switch(event->button) {
		case LEFT:
			dec();
			break;
		case RIGHT:
			inc();
			break;
		case ALTLEFT:
			update_value(-10);
			break;
		case ALTRIGHT:
			update_value(10);
			break;
		case ACCEPT:
		case UP:
		case DOWN:
			edit = false;
			updateButtonBox();
			break;
		default:
			return false;
	}
	} else {
		switch(event->button) {
			case LEFT:
				leftComponent();
				break;
			case RIGHT:
				rightComponent();
				break;
			case ACCEPT:
				edit = true;
				updateButtonBox();
				break;
			default:
				return false;
		}
	}
	return true;
}

void MenuSettingRGBA::update_value(int value)
{
	setSelPart(constrain(getSelPart() + value, 0, 255));
}

void MenuSettingRGBA::dec()
{
	update_value(-1);
}

void MenuSettingRGBA::inc()
{
	update_value(+1);
}

void MenuSettingRGBA::leftComponent()
{
	selPart = constrain(selPart-1,0,3);
}

void MenuSettingRGBA::rightComponent()
{
	selPart = constrain(selPart+1,0,3);
}

void MenuSettingRGBA::setR(unsigned short r)
{
	_value->r = r;
	stringstream ss;
	ss << r;
	ss >> strR;
}

void MenuSettingRGBA::setG(unsigned short g)
{
	_value->g = g;
	stringstream ss;
	ss << g;
	ss >> strG;
}

void MenuSettingRGBA::setB(unsigned short b)
{
	_value->b = b;
	stringstream ss;
	ss << b;
	ss >> strB;
}

void MenuSettingRGBA::setA(unsigned short a)
{
	_value->a = a;
	stringstream ss;
	ss << a;
	ss >> strA;
}

void MenuSettingRGBA::setSelPart(unsigned short value)
{
	switch (selPart) {
		default: case 0: setR(value); break;
		case 1: setG(value); break;
		case 2: setB(value); break;
		case 3: setA(value); break;
	}
}

RGBAColor MenuSettingRGBA::value()
{
	return *_value;
}

unsigned short MenuSettingRGBA::getSelPart()
{
	switch (selPart) {
		default: case 0: return value().r;
		case 1: return value().g;
		case 2: return value().b;
		case 3: return value().a;
	}
}

void MenuSettingRGBA::adjustInput()
{
#ifdef PLATFORM_GP2X
    /*
	gmenu2x->input.setInterval(30, ACTION_Y );
	gmenu2x->input.setInterval(30, ACTION_X );
	gmenu2x->input.setInterval(30, ACTION_L );
    */
#endif
}

void MenuSettingRGBA::drawSelected(int y)
{
	int x = 166+selPart*36;
	gmenu2x->s->box( x, y, 36, 14, gmenu2x->skinConfColors[COLOR_SELECTION_BG] );

	MenuSetting::drawSelected(y);
}

bool MenuSettingRGBA::edited()
{
	return originalValue.r != value().r || originalValue.g != value().g || originalValue.b != value().b || originalValue.a != value().a;
}

void MenuSettingRGBA::updateButtonBox()
{
	buttonBox.clear();
	if (edit) {
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/l.png"));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/left.png", gmenu2x->tr["Decrease"]));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/r.png"));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/right.png", gmenu2x->tr["Increase"]));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/accept.png", gmenu2x->tr["Confirm"]));
	} else {
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/left.png"));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/right.png", gmenu2x->tr["Change color component"]));
		buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/accept.png", gmenu2x->tr["Edit"]));
	}
}
