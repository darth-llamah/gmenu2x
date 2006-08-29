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
#include "inputdialog.h"

using namespace std;

InputDialog::InputDialog(GMenu2X *parent, string text, string startvalue) {
	this->parent = parent;
	this->text = text;
	input = startvalue;
	selCol = 0;
	selRow = 0;

	keyboard.resize(4);
	//keyboard[0] = "ABCDEFGHIJKLM";
	//keyboard[0] = "NOPQRSTUVWXYZ";
	keyboard[0] = "abcdefghijklm";
	keyboard[1] = "nopqrstuvwxyz";
	keyboard[2] = "0123456789 \"'";
	keyboard[3] = "#$%&|?.,:;*+-";
}

InputDialog::~InputDialog() {}

bool InputDialog::exec() {
	bool close = false, ok = true;
	Surface bg("imgs/bg.png");

	SDL_Rect box = {0, 70, 0, parent->font->getHeight()+4};

	Uint32 caretTick, curTick;
	bool caretOn = true;

	boxRGBA(bg.raw, 0, 0, 320, 15, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);
	parent->writeCenter(bg.raw, text, 160, 2);

	//A Button
	filledCircleRGBA(bg.raw, 12, 228, 7, 0,0,0,255);
	parent->writeCenter(bg.raw, "A", 13, 222);
	parent->write(bg.raw, "or", 22, 222);
	//L Button
	filledCircleRGBA(bg.raw, 40, 228, 7, 0,0,0,255);
	parent->writeCenter(bg.raw, "L", 41, 222);
	parent->write(bg.raw, "Backspace", 51, 222);

	//R Button
	filledCircleRGBA(bg.raw, 114, 228, 7, 0,0,0,255);
	parent->writeCenter(bg.raw, "R", 115, 222);
	parent->write(bg.raw, "Space", 125, 222);

	//B Button
	filledCircleRGBA(bg.raw, 168, 228, 7, 0,0,0,255);
	parent->writeCenter(bg.raw, "B", 169, 222);
	parent->write(bg.raw, "Confirm", 179, 222);

	//Y Button
	filledCircleRGBA(bg.raw, 231, 228, 7, 0,0,0,255);
	parent->writeCenter(bg.raw, "Y", 232, 222);
	parent->write(bg.raw, "Shift", 242, 222);

	while (!close) {
		bg.blit(parent->s,0,0);

		box.w = parent->font->getTextWidth(input)+18;
		box.x = 160-box.w/2;
		boxRGBA(parent->s->raw, box.x, box.y, box.x+box.w, box.y+box.h, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);
		rectangleRGBA(parent->s->raw, box.x, box.y, box.x+box.w, box.y+box.h, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);

		parent->write(parent->s->raw, input, box.x+5, box.y+3);

		curTick = SDL_GetTicks();
		if (curTick-caretTick>=600) {
			caretOn = !caretOn;
			caretTick = curTick;
		}

		if (caretOn) boxRGBA(parent->s->raw, box.x+box.w-12, box.y+3, box.x+box.w-4, box.y+box.h-3, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);

		drawVirtualKeyboard();

		parent->s->flip();

#ifdef TARGET_GP2X
		parent->joy.update();
		// LINK NAVIGATION
		if ( parent->joy[GP2X_BUTTON_LEFT ] ) selCol--;
		if ( parent->joy[GP2X_BUTTON_RIGHT] ) selCol++;
		if ( parent->joy[GP2X_BUTTON_UP   ] ) selRow--;
		if ( parent->joy[GP2X_BUTTON_DOWN ] ) selRow++;
		if ( parent->joy[GP2X_BUTTON_A] || parent->joy[GP2X_BUTTON_L] ) input = input.substr(0,input.length()-1);
		if ( parent->joy[GP2X_BUTTON_R    ] ) input += " ";
		if ( parent->joy[GP2X_BUTTON_Y    ] ) {
			if (keyboard[0][0]=='A') {
				keyboard[0] = "abcdefghijklm";
				keyboard[1] = "nopqrstuvwxyz";
			} else {
				keyboard[0] = "ABCDEFGHIJKLM";
				keyboard[1] = "NOPQRSTUVWXYZ";
			}
		}
		if ( parent->joy[GP2X_BUTTON_B] || parent->joy[GP2X_BUTTON_CLICK] ) {
			if (selRow==keyboard.size()) {
				if (selCol==0)
					ok = false;
				close = true;
			} else
				input += keyboard[selRow][selCol];
		}
#else
		while (SDL_PollEvent(&parent->event)) {
			if ( parent->event.type==SDL_KEYDOWN ) {
				if ( parent->event.key.keysym.sym==SDLK_ESCAPE ) { ok = false; close = true; }
				// LINK NAVIGATION
				if ( parent->event.key.keysym.sym==SDLK_LEFT      ) selCol--;
				if ( parent->event.key.keysym.sym==SDLK_RIGHT     ) selCol++;
				if ( parent->event.key.keysym.sym==SDLK_UP        ) selRow--;
				if ( parent->event.key.keysym.sym==SDLK_DOWN      ) selRow++;
				if ( parent->event.key.keysym.sym==SDLK_BACKSPACE ) input = input.substr(0,input.length()-1);
				if ( parent->event.key.keysym.sym==SDLK_LSHIFT    ) {
					if (keyboard[0][0]=='A') {
						keyboard[0] = "abcdefghijklm";
						keyboard[1] = "nopqrstuvwxyz";
					} else {
						keyboard[0] = "ABCDEFGHIJKLM";
						keyboard[1] = "NOPQRSTUVWXYZ";
					}
				}
				if ( parent->event.key.keysym.sym==SDLK_RETURN    ) {
					if (selRow==keyboard.size()) {
						if (selCol==0)
							ok = false;
						close = true;
					} else
						input += keyboard[selRow][selCol];
				}
			}
		}
#endif
	}

	return ok;
}

void InputDialog::drawVirtualKeyboard() {
	//keyboard border
	rectangleRGBA(parent->s->raw, 157-keyboard[0].length()*5, 98, 161+keyboard[0].length()*5, 116+keyboard.size()*15, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);
	uint left = 160-keyboard[0].length()*5;

	if (selCol<0) selCol = keyboard[0].length()-1;
	if (selCol>=(int)keyboard[0].length()) selCol = 0;
	if (selRow<0) selRow = keyboard.size()-1;
	if (selRow>keyboard.size()) selRow = 0;

	//selection
	if (selRow<keyboard.size())
		boxRGBA(parent->s->raw, left+selCol*10-1, 100+selRow*15, left+(selCol+1)*10-1, 113+selRow*15, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);
	else {
		if (selCol>1) selCol = 0;
		if (selCol<0) selCol = 1;
		boxRGBA(parent->s->raw, left+selCol*keyboard[0].length()*5-1, 100+keyboard.size()*15, left+(selCol+1)*keyboard[0].length()*5-1, 114+keyboard.size()*15, parent->colorR,parent->colorG,parent->colorB,parent->alphablend);
	}

	//keys
	for (uint l=0; l<keyboard.size(); l++) {
		string line = keyboard[l];
		for (uint x=0; x<line.length(); x++) {
			string charX = line.substr(x,1);
			int halfW = parent->font->getTextWidth(charX)/2;
			parent->write(parent->s->raw, charX, left+5-halfW+x*10, 100+l*15);
		}
	}

	//Ok/Cancel
	parent->writeCenter(parent->s->raw, "Cancel", (int)(160-keyboard[0].length()*2.5), 101+keyboard.size()*15);
	parent->writeCenter(parent->s->raw, "OK", (int)(160+keyboard[0].length()*2.5), 101+keyboard.size()*15);
}
