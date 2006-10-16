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
#include <fstream>
#include <sstream>
#include "link.h"
#include "menu.h"
#include "selector.h"

using namespace std;

Link::Link(GMenu2X *gmenu2x) {
	this->gmenu2x = gmenu2x;
	edited = false;
}

void Link::run() {}

string Link::getTitle() {
	return title;
}

void Link::setTitle(string title) {
	this->title = title;
	edited = true;
}

string Link::getDescription() {
	return description;
}

void Link::setDescription(string description) {
	this->description = description;
	edited = true;
}

string Link::getIcon() {
	return icon;
}

void Link::setIcon(string icon) {
	this->icon = icon;
	edited = true;
}
