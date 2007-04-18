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

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#ifdef TARGET_GP2X
#include "gp2x.h"
#endif
#include "settingsdialog.h"

using namespace std;

SettingsDialog::SettingsDialog(GMenu2X *gmenu2x, string text) {
	this->gmenu2x = gmenu2x;
	this->text = text;
}

SettingsDialog::~SettingsDialog() {
	for (uint i=0; i<voices.size(); i++)
		free(voices[i]);
}

bool SettingsDialog::exec() {
	Surface bg (gmenu2x->wallpaper,false);

	bool close = false;
	uint i, sel = 0, iY, firstElement = 0;
	voices[sel]->adjustInput();

	while (!close) {
		bg.blit(gmenu2x->s,0,0);

		gmenu2x->drawTopBar(gmenu2x->s);
		//link icon
		if (gmenu2x->sc["skin:sections/settings.png"] != NULL)
			gmenu2x->sc["skin:sections/settings.png"]->blit(gmenu2x->s,4,4);
		else
			gmenu2x->sc["icons/generic.png"]->blit(gmenu2x->s,4,4);
		gmenu2x->s->write(gmenu2x->font, text, 40,13, SFontHAlignLeft, SFontVAlignMiddle);
		gmenu2x->drawBottomBar(gmenu2x->s);

		if (sel>firstElement+10) firstElement=sel-10;
		if (sel<firstElement) firstElement=sel;

		//selection
		iY = sel-firstElement;
		iY = 42+(iY*16);
		gmenu2x->s->setClipRect(0,41,311,175);
		if (sel<voices.size())
			gmenu2x->s->box(1, iY, 148, 14, gmenu2x->selectionColor);
		gmenu2x->s->clearClipRect();

		//selected option
		voices[sel]->drawSelected(iY);

		gmenu2x->s->setClipRect(0,41,311,175);
		for (i=firstElement; i<voices.size() && i<firstElement+11; i++) {
			iY = i-firstElement;
			voices[i]->draw(iY*16+42);
		}
		gmenu2x->s->clearClipRect();

		gmenu2x->drawScrollBar(11,voices.size(),firstElement,42,175);

		//description
		gmenu2x->s->write(gmenu2x->font, voices[sel]->description, 40,27, SFontHAlignLeft, SFontVAlignMiddle);

#ifdef TARGET_GP2X
		gmenu2x->joy.update();
		if ( gmenu2x->joy[GP2X_BUTTON_START] ) close = true;
		if ( gmenu2x->joy[GP2X_BUTTON_UP    ] ) {
			if (sel==0)
				sel = voices.size()-1;
			else
				sel -= 1;
			gmenu2x->setInputSpeed();
			voices[sel]->adjustInput();
		}
		if ( gmenu2x->joy[GP2X_BUTTON_DOWN  ] ) {
			sel += 1;
			if (sel>=voices.size()) sel = 0;
			gmenu2x->setInputSpeed();
			voices[sel]->adjustInput();
		}
		voices[sel]->manageInput();
#else
		while (SDL_PollEvent(&gmenu2x->event)) {
			if ( gmenu2x->event.type == SDL_QUIT ) return false;
			if ( gmenu2x->event.type==SDL_KEYDOWN ) {
				if ( gmenu2x->event.key.keysym.sym==SDLK_ESCAPE ) close = true;
				if ( gmenu2x->event.key.keysym.sym==SDLK_UP ) {
					if (sel==0)
						sel = voices.size()-1;
					else
						sel -= 1;
					gmenu2x->setInputSpeed();
					voices[sel]->adjustInput();
				}
				if ( gmenu2x->event.key.keysym.sym==SDLK_DOWN ) {
					sel += 1;
					if (sel>=voices.size()) sel = 0;
					gmenu2x->setInputSpeed();
					voices[sel]->adjustInput();
				}
				voices[sel]->manageInput();
			}
		}
#endif

		gmenu2x->s->flip();
	}

	gmenu2x->setInputSpeed();
	return true;
}

void SettingsDialog::addSetting(MenuSetting* set) {
	voices.push_back(set);
}

bool SettingsDialog::edited() {
	for (uint i=0; i<voices.size(); i++)
		if (voices[i]->edited()) return true;
	return false;
}
