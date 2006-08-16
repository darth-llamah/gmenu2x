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

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>

//for statfs
#include <sys/vfs.h>
#include <errno.h>

#ifdef TARGET_GP2X
#include "gp2x.h"
#include <sys/fcntl.h> //for battery
#endif

#include "SFont.h"
#include "surface.h"
#include "utilities.h"
#include "gmenu2x.h"

using namespace std;

int main(int argc, char *argv[]) {
	GMenu2X app(argc,argv);
	return 0;
}

GMenu2X::GMenu2X(int argc, char *argv[]) {
	path = getExePath();

	//fork to set clock in background
	if (!fork()) {
		setClock(200);
		exit(0);
	}

	//Screen
	cout << "GMENU2X: Initializing screen..." << endl;
	if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK)<0 ) {
		fprintf(stdout, "Could not initialize SDL: %s\n", SDL_GetError());
		SDL_Quit();
	}
	s = new Surface(320,240);

	font = new SFont( sc["imgs/font.png"]->raw );

	//Menu structure handler
	menu = new Menu(path);
	for (uint i=0; i<menu->sections.size(); i++) {
		string sectionIcon = "sections/"+menu->sections[i]+".png";
		if (fileExists(sectionIcon))
			sc.add(sectionIcon);
	}

	initBG();

	//Events
#ifdef TARGET_GP2X
	startquit = argc>1 && string(argv[1])=="--startquit";

	joy.init(0);
	joy.setInterval(150);
	joy.setInterval(30,  GP2X_BUTTON_VOLDOWN);
	joy.setInterval(30,  GP2X_BUTTON_VOLUP  );
	joy.setInterval(500, GP2X_BUTTON_SELECT );
	joy.setInterval(1000,GP2X_BUTTON_B      );
	joy.setInterval(300, GP2X_BUTTON_L      );
	joy.setInterval(300, GP2X_BUTTON_R      );
#else
	SDL_EnableKeyRepeat(1,150);
#endif

	main();

	exit(0);
}

int GMenu2X::main() {
	bool quit = false;
	int x,y,ix,iy, offset = menu->links.size()>24 ? 0 : 4;
	uint i;
	long tickBattery = -30000, tickNow;
	string batteryStatus;

	while (!quit) {
		//Background
		sc["imgs/bg.png"]->blit(s,0,0);

		//Sections
		if (menu->firstDispSection()>0)
			sc["imgs/left.png"]->blit(s,1,16);
		if (menu->firstDispSection()+5<menu->sections.size())
			sc["imgs/right.png"]->blit(s,311,16);
		for (i=menu->firstDispSection(); i<menu->sections.size() && i<menu->firstDispSection()+5; i++) {
			string sectionIcon = "sections/"+menu->sections[i]+".png";
			x = (i-menu->firstDispSection())*60+24;
			if (menu->selSectionIndex()==(int)i)
				boxRGBA(s->raw, x-14, 0, x+46, 40, 255, 255, 255, ALPHABLEND);
			if (sc.exists(sectionIcon))
				sc[sectionIcon]->blit(s,x,0);
			writeCenter( s->raw, menu->sections[i], x+16, 27 );
		}

		//Links
		for (i=menu->firstDispRow()*6; i<(menu->firstDispRow()*6)+24 && i<menu->links.size(); i++) {
			int ir = i-menu->firstDispRow()*6;
			x = (ir%6)*52+offset;
			y = (ir/6+1)*41-1;
			ix = x+10;
			iy = y+1;

			if (menu->selLink()==menu->links[i])
				sc["imgs/selection.png"]->blit(s,x,y);

			if (menu->links[i]->icon != "")
				sc[menu->links[i]->icon]->blit(s,ix,iy);
			else
				sc["icons/generic.png"]->blit(s,ix,iy);

			writeCenter( s->raw, menu->links[i]->title, ix+16, iy+29 );
		}
		drawScrollBar();

		if (menu->selLink()!=NULL) {
			writeCenter( s->raw, menu->selLink()->description, 160, 207 );
			write ( s->raw, menu->selLink()->clockStr(), cpuX, 223 );
		}
		
		//battery
		tickNow = SDL_GetTicks();
		//check battery status every 30 seconds
		if (tickNow-tickBattery >= 30000) {
			tickBattery = tickNow;
			stringstream ss;
			ss << getBatteryLevel();
			ss << "%";
			ss >> batteryStatus;
			cout << "GMENU2X: Battery level " << batteryStatus << endl;
		}
		write( s->raw, batteryStatus, batX, 223 );

#ifdef TARGET_GP2X
		joy.update();
		if ( joy[GP2X_BUTTON_START ] && startquit ) quit = true;
		if ( joy[GP2X_BUTTON_SELECT] ) options();
		// LINK NAVIGATION
		if ( joy[GP2X_BUTTON_LEFT ] ) menu->linkLeft();
		if ( joy[GP2X_BUTTON_RIGHT] ) menu->linkRight();
		if ( joy[GP2X_BUTTON_UP   ] ) menu->linkUp();
		if ( joy[GP2X_BUTTON_DOWN ] ) menu->linkDown();
		// CLOCK DOWN
		if ( joy[GP2X_BUTTON_VOLDOWN] && !joy[GP2X_BUTTON_VOLUP] ) {
			if (menu->selLink()->clock()==0)
				menu->selLink()->setClock(200);
			else if (menu->selLink()->clock()>100)
				menu->selLink()->setClock( menu->selLink()->clock()-1 );
		}
		// CLOCK UP
		if ( joy[GP2X_BUTTON_VOLUP] && !joy[GP2X_BUTTON_VOLDOWN] ) {
			if (menu->selLink()->clock()==0)
				menu->selLink()->setClock(200);
			else if (menu->selLink()->clock()<300)
				menu->selLink()->setClock( menu->selLink()->clock()+1 );
		}
		if ( joy[GP2X_BUTTON_VOLUP] && joy[GP2X_BUTTON_VOLDOWN] ) {
			menu->selLink()->setClock(0);
		}
		if ( joy[GP2X_BUTTON_L     ] ) {
			menu->decSectionIndex();
			offset = menu->links.size()>24 ? 0 : 4;
		}
		if ( joy[GP2X_BUTTON_R     ] ) {
			menu->incSectionIndex();
			offset = menu->links.size()>24 ? 0 : 4;
		}
		if ( joy[GP2X_BUTTON_B     ] && menu->selLink()!=NULL ) runLink();
#else
		while (SDL_PollEvent(&event)) {
			if ( event.type == SDL_QUIT ) quit = true;
			if ( event.type==SDL_KEYDOWN ) {
				if ( event.key.keysym.sym==SDLK_ESCAPE ) quit = true;
				if ( event.key.keysym.sym==SDLK_SPACE  ) options();
				// LINK NAVIGATION
				if ( event.key.keysym.sym==SDLK_LEFT   ) menu->linkLeft();
				if ( event.key.keysym.sym==SDLK_RIGHT  ) menu->linkRight();
				if ( event.key.keysym.sym==SDLK_UP     ) menu->linkUp();
				if ( event.key.keysym.sym==SDLK_DOWN   ) menu->linkDown();
				// CLOCK DOWN
				if ( event.key.keysym.sym==SDLK_z      ) {
					if (menu->selLink()->clock()==0)
						menu->selLink()->setClock(200);
					else if (menu->selLink()->clock()>100)
						menu->selLink()->setClock( menu->selLink()->clock()-1 );
				}
				// CLOCK UP
				if ( event.key.keysym.sym==SDLK_x      ) {
					if (menu->selLink()->clock()==0)
						menu->selLink()->setClock(200);
					else if (menu->selLink()->clock()<300)
						menu->selLink()->setClock( menu->selLink()->clock()+1 );
				}
				if ( event.key.keysym.sym==SDLK_q      ) {
					menu->decSectionIndex();
					offset = menu->links.size()>24 ? 0 : 4;
				}
				if ( event.key.keysym.sym==SDLK_w      ) {
					menu->incSectionIndex();
					offset = menu->links.size()>24 ? 0 : 4;
				}
				if ( event.key.keysym.sym==SDLK_RETURN && menu->selLink()!=NULL ) runLink();
			}
		}
#endif

		s->flip();
	}

	return -1;
}

int GMenu2X::options() {
	Surface bg = *s;
	//Darken background
	boxRGBA(bg.raw, 0, 0, 320, 240, 0,0,0,ALPHABLEND);

	vector<MenuOption*> options;
	options.push_back( new MenuOption(this, "Add link in '"+menu->selSection()+"'", &GMenu2X::fileBrowser) );

	if (menu->selLink()!=NULL)
		options.push_back( new MenuOption(this, "Delete link '"+menu->selLink()->title+"'", 0) );

	bool close = false;
	uint i, sel = 0;

	int h = font->getHeight();
	SDL_Rect box;
	box.h = (h+2)*options.size()+8;
	box.w = 0;
	for (i=0; i<options.size(); i++) {
		int w = font->getTextWidth(options[i]->text);
		if (w>box.w) box.w = w;
	}
	box.w += 23;
	box.x = 160 - box.w/2;
	box.y = 120 - box.h/2;

	SDL_Rect selbox = {box.x+4, 0, box.w-8, h+2};

	while (!close) {
		bg.blit(s,0,0);
		SDL_FillRect( s->raw, &box, SDL_MapRGB(s->format(),255,255,255) );
		selbox.y = box.y+4+(h+2)*sel;
		SDL_FillRect( s->raw, &selbox, SDL_MapRGB(s->format(),160,160,160) );
		rectangleColor( s->raw, box.x+2, box.y+2, box.x+box.w-3, box.y+box.h-3, SDL_MapRGB(s->format(),80,80,80) );
		for (i=0; i<options.size(); i++) {
			write( s->raw, options[i]->text, box.x+12, box.y+5+(h+2)*i );
		}

#ifdef TARGET_GP2X
		joy.update();
		if ( joy[GP2X_BUTTON_SELECT] ) close = true;
		if ( joy[GP2X_BUTTON_UP    ] ) sel = max(0, sel-1);
		if ( joy[GP2X_BUTTON_DOWN  ] ) sel = min(options.size()-1, sel+1);
		if ( joy[GP2X_BUTTON_B     ] ) {
			//StatusFcn action = opts[i].action;
			//if (action!=0)
			//	(*action)();
			//StatusFcn action = &GMenu2X::fileBrowser;
			//(*action)();
			fileBrowser();
		}
#else
		while (SDL_PollEvent(&event)) {
			if ( event.type == SDL_QUIT ) return -1;
			if ( event.type==SDL_KEYDOWN ) {
				if ( event.key.keysym.sym==SDLK_ESCAPE ) close = true;
				if ( event.key.keysym.sym==SDLK_UP ) sel = max(0, sel-1);
				if ( event.key.keysym.sym==SDLK_DOWN ) sel = min(options.size()-1, sel+1);
				if ( event.key.keysym.sym==SDLK_RETURN ) options[i]->exec();
			}
		}
#endif

		s->flip();
	}

	return 0;
}

int GMenu2X::fileBrowser() {
	bool close = false;
	
	while (!close) {
		sc["imgs/bg.png"]->blit(s,0,0);
		writeCenter(s->raw,"File Browser",160,1);
	
		s->flip();
	}
	
	return -1;
}

GMenu2X::~GMenu2X() {
	free(menu);
	free(s);
	free(font);
}

unsigned short GMenu2X::getBatteryLevel() {
#ifdef TARGET_GP2X
	int devbatt = open ("/dev/batt", O_RDONLY);
	if (devbatt<0) return 0;
	
	int battval = 0;
	unsigned short cbv;
	int v;

	for (int i = 0; i < BATTERY_READS; i ++) {
		if (read (devbatt, &cbv, 2) == 2)
			battval += cbv;
	}
 	close(devbatt);
 	
 	battval /= BATTERY_READS;
 	battval -= 534; //534 = 2.0v (0%) , 745 = 2.6v (100%)
 	if (battval<0) battval = 0;
 	battval = battval*100/211; //745-534=211
 	if (battval>100) battval = 100;
 	
 	return battval;
#else
	return 100;
#endif
}

void GMenu2X::initBG() {
	//Top Bar
	boxRGBA(sc["imgs/bg.png"]->raw, 0, 0, 320, 40, 255,255,255,ALPHABLEND);
	//Bottom Bar
	boxRGBA(sc["imgs/bg.png"]->raw, 0, 220, 320, 240, 255,255,255,ALPHABLEND);

	Surface sd("imgs/sd.png");
	Surface cpu("imgs/cpu.png");
	Surface battery("imgs/battery.png");
	string df = getDiskFree();

	sd.blit( sc["imgs/bg.png"], 3, 222 );
	write( sc["imgs/bg.png"]->raw, df, 22, 223 );
	batX = 27+font->getTextWidth(df);
	battery.blit( sc["imgs/bg.png"], batX, 222 );
	batX += 19;
	cpuX = batX+5+font->getTextWidth("100%");
	cpu.blit( sc["imgs/bg.png"], cpuX, 222 );
	cpuX += 19;
}

void GMenu2X::drawScrollBar() {
	if (menu->links.size()<=24) return;

	rectangleRGBA(s->raw, 312, 43, 317, 202, 255,255,255,ALPHABLEND);
	//internal bar total height = 157
	//bar size
	int bs = 157 * 3 / (int)ceil(menu->links.size()/6);
	//bar y position
	int by = 157 * menu->firstDispRow() / (int)ceil(menu->links.size()/6);
	by = 45+by;
	if (by+bs>200) by = 200-bs;

	boxRGBA(s->raw, 314, by, 315, by+bs, 255,255,255,ALPHABLEND);
}

void GMenu2X::write(SDL_Surface *s, string text, int x, int y) {
	font->write(s,text.c_str(),x,y);
}

void GMenu2X::writeCenter(SDL_Surface *s, string text, int x, int y) {
	font->writeCenter(s,text.c_str(),x,y);
}

void GMenu2X::drawRun() {
	//Darkened background
	boxRGBA(s->raw, 0, 0, 320, 240, 0,0,0,ALPHABLEND);

	string text = "Launching "+menu->selLink()->title;
	int textW = font->getTextWidth(text);
	int boxW = 62+textW;
	int halfBoxW = boxW/2;

	//outer box
	SDL_Rect r = {158-halfBoxW, 97, halfBoxW*2+5, 47};
	SDL_FillRect(s->raw, &r, SDL_MapRGB(s->format(),255,255,255));
	//draw inner rectangle
	rectangleColor(s->raw, 160-halfBoxW, 99, 160+halfBoxW, 141, SDL_MapRGB(s->format(),80,80,80));

	int x = 170-halfBoxW;
	if (menu->selLink()->icon!="")
		sc[menu->selLink()->icon]->blit(s,x,104);
	else
		sc["icons/generic.png"]->blit(s,x,104);
	write( s->raw, text, x+42, 114 );
	s->flip();
}

void GMenu2X::setClock(int mhz) {
	string command = "";
	stringstream ss;
	ss << mhz;
	ss >> command;
	cout << "GMENU2X: Setting clock speed at " << command << "MHZ" << endl;
	command = path + "scripts/cpuspeed.sh " + command;
	cout << "GMENU2X: " << command << endl;
#ifdef TARGET_GP2X
	system(command.c_str());
#endif
}

void GMenu2X::runLink() {
	drawRun();
#ifndef TARGET_GP2X
	//delay for testing
	SDL_Delay(2000);
#endif
	bool sync = false;
	for (uint i=0; i<menu->links.size(); i++)
		if (menu->links[i]->save())
			sync = true;
	if (sync)
		system("sync");
	if (menu->selLink()->clock()>0)
		setClock(menu->selLink()->clock());
	menu->selLink()->run();
}

string GMenu2X::getExePath() {
	stringstream ss;
	ss << "/proc/" << getpid() << "/exe";
	string p;
	ss >> p;
	char buf[255];
	int l = readlink(p.c_str(),buf,255);
	p = buf;
	p = p.substr(0,l);
	l = p.rfind("/");
	return p.substr(0,l+1);
}

string GMenu2X::getDiskFree() {
	stringstream ss;
	string df = "";
	struct statfs b;

#ifdef TARGET_GP2X
	int ret = statfs("/mnt/sd", &b);
#else
	int ret = statfs("/mnt/usb", &b);
#endif
	if (ret==0) {
		ss << b.f_bfree*b.f_bsize/1048576 << "/" << b.f_blocks*b.f_bsize/1048576 << "MB";
		ss >> df;
	} else cout << "GMENU2X: statfs failed with error '" << strerror(errno) << "'" << endl;
	return df;
}
