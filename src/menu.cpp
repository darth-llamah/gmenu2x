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

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "gmenu2x.h"
#include "linkapp.h"
#include "menu.h"
#include "utilities.h"

using namespace std;

Menu::Menu(GMenu2X *gmenu2x, string path) {
	this->gmenu2x = gmenu2x;
	this->path = path;
	iFirstDispSection = 0;

	numRows = 4;
	numCols = 6;

	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath;

	if ((dirp = opendir("sections")) == NULL) return;

	while ((dptr = readdir(dirp))) {
		if (dptr->d_name[0]=='.') continue;
		filepath = (string)"sections/"+dptr->d_name;
		int statRet = stat(filepath.c_str(), &st);
		if (!S_ISDIR(st.st_mode)) continue;
		if (statRet != -1) {
			sections.push_back((string)dptr->d_name);
			linklist ll;
			links.push_back(ll);
		}
	}
	
	closedir(dirp);
	sort(sections.begin(),sections.end(),case_less());
	setSectionIndex(0);
	readLinks();
}

Menu::~Menu() {
	freeLinks();
}

uint Menu::firstDispRow() {
	return iFirstDispRow;
}

/*====================================
   SECTION MANAGEMENT
  ====================================*/
void Menu::freeLinks() {
	for (vector<linklist>::iterator section = links.begin(); section<links.end(); section++)
		for (linklist::iterator link = section->begin(); link<section->end(); link++)
			free(*link);
}

linklist *Menu::sectionLinks(int i) {
	if (i<0 || i>(int)links.size())
		i = selSectionIndex();

	if (i<0 || i>(int)links.size())
		return NULL;

	return &links[i];
}

void Menu::decSectionIndex() {
	setSectionIndex(iSection-1);
}

void Menu::incSectionIndex() {
	setSectionIndex(iSection+1);
}

uint Menu::firstDispSection() {
	return iFirstDispSection;
}

int Menu::selSectionIndex() {
	return iSection;
}

string Menu::selSection() {
	return sections[iSection];
}

void Menu::setSectionIndex(int i) {
	if (i<0)
		i=sections.size()-1;
	else if (i>=(int)sections.size())
		i=0;
	iSection = i;

	if (i>(int)iFirstDispSection+4)
		iFirstDispSection = i-4;
	else if (i<(int)iFirstDispSection)
		iFirstDispSection = i;

	iLink = 0;
	iFirstDispRow = 0;
}

string Menu::sectionPath(int section) {
	if (section<0 || section>(int)sections.size()) section = iSection;
	return "sections/"+sections[section]+"/";
}

/*====================================
   LINKS MANAGEMENT
  ====================================*/
bool Menu::addActionLink(uint section, string title, LinkRunAction action, string description, string icon) {
	cout << "\033[0;34mGMENU2X:\033[0m Adding action link " << sectionLinks(section)->size() << endl;
	if (section>=sections.size()) return false;

	LinkAction *linkact = new LinkAction(gmenu2x,action);
	linkact->setTitle(title);
	linkact->setDescription(description);
	linkact->setIcon(icon);

	sectionLinks(section)->push_back(linkact);
	cout << "\033[0;34mGMENU2X:\033[0m Link added " << sectionLinks(section)->size() << endl;
	return true;
}

bool Menu::addLink(string path, string file, string section) {
	if (section=="")
		section = selSection();
	else if (find(sections.begin(),sections.end(),section)==sections.end()) {
		//section directory doesn't exists
		string sectiondir = "sections/"+section;
		cout << "\033[0;34mGMENU2X:\033[0m mkdir " << sectiondir << endl;
		if (mkdir(sectiondir.c_str(),777)==0)
			sections.push_back(section);
		else
			return false;
	}
	cout << "\033[0;34mGMENU2X:\033[0m addLink section=" << section << " file=" << file << endl;
	if (path[path.length()-1]!='/') path += "/";

	string title = file;
	string::size_type pos = title.rfind(".");
	if (pos!=string::npos && pos>0)
		title = title.substr(0, pos);

	cout << "\033[0;34mGMENU2X:\033[0m Creating link " << title << endl;

	string linkpath = "sections/"+section+"/"+title;
	int x=2;
	while (fileExists(linkpath)) {
		stringstream ss;
		linkpath = "";
		ss << x;
		ss >> linkpath;
		linkpath = "sections/"+section+"/"+title+linkpath; 
		cout << "\033[0;34mGMENU2X:\033[0m linkpath=" << linkpath << endl;
		x++;
	}

	int linkW = 312/numCols;
	if (gmenu2x->font->getTextWidth(title)>linkW) {
		while (gmenu2x->font->getTextWidth(title+"..")>linkW)
			title = title.substr(0,title.length()-1);
		title += "..";
	}

	ofstream f(linkpath.c_str());
	if (f.is_open()) {
		f << "title=" << title << endl;
		if (fileExists(path+title+".png"))
			f << "icon=" << path << title << ".png" << endl;
		f << "exec=" << path << file << endl;
		f.close();

		int isection = find(sections.begin(),sections.end(),section) - sections.begin();
		if (isection>0 && isection<(int)sections.size())
			links[isection].push_back( new LinkApp(gmenu2x, path, linkpath.c_str()) );
	} else {
		cout << "\033[0;34mGMENU2X:\033[0;31m Error while opening the file '" << linkpath << "' for write\033[0m" << endl;
		return false;
	}

	return true;
}

void Menu::deleteSelectedLink() {
	if (selLinkApp()!=NULL) {
		cout << "\033[0;34mGMENU2X:\033[0m Deleting link " << selLink()->getTitle() << endl;
		unlink(selLinkApp()->file.c_str());
		gmenu2x->sc.del(selLink()->getIcon());
		sectionLinks()->erase( sectionLinks()->begin() + selLinkIndex() );
		setLinkIndex(selLinkIndex());
	}
}

void Menu::linkLeft() {
	if (iLink%numCols == 0)
		setLinkIndex( sectionLinks()->size()>iLink+numCols-1 ? iLink+numCols-1 : sectionLinks()->size()-1 );
	else
		setLinkIndex(iLink-1);
}

void Menu::linkRight() {
	if (iLink%numCols == (numCols-1) || iLink == (int)sectionLinks()->size()-1)
		setLinkIndex(iLink-iLink%numCols);
	else
		setLinkIndex(iLink+1);
}

void Menu::linkUp() {
	int l = iLink-numCols;
	if (l<0) {
		int rows = sectionLinks()->size()/numCols+1;
		l = (rows*numCols)+l;
		if (l >= (int)sectionLinks()->size())
			l -= numCols;
	}
	setLinkIndex(l);
}

void Menu::linkDown() {
	uint l = iLink+numCols;
	if (l >= sectionLinks()->size())
		l %= numCols;
	setLinkIndex(l);
}

int Menu::selLinkIndex() {
	return iLink;
}

Link *Menu::selLink() {
	if (sectionLinks()->size()==0) return NULL;
	return sectionLinks()->at(iLink);
}

LinkApp *Menu::selLinkApp() {
	return dynamic_cast<LinkApp*>(selLink());
}

void Menu::setLinkIndex(int i) {
	if (i<0)
		i=sectionLinks()->size()-1;
	else if (i>=(int)sectionLinks()->size())
		i=0;

	if (i>=(int)(iFirstDispRow*numCols+numCols*numRows))
		iFirstDispRow = i/numCols-numRows+1;
	else if (i<(int)(iFirstDispRow*numCols))
		iFirstDispRow = i/numCols;

	iLink = i;
}

void Menu::readLinks() {
	vector<string> linkfiles;
	
	iLink = 0;
	iFirstDispRow = 0;

	DIR *dirp;
	struct stat st;
	struct dirent *dptr;
	string filepath;

	for (uint i=0; i<links.size(); i++) {
		links[i].clear();
		linkfiles.clear();

		if ((dirp = opendir(sectionPath(i).c_str())) == NULL) continue;
	
		while ((dptr = readdir(dirp))) {
			if (dptr->d_name[0]=='.') continue;
			filepath = sectionPath(i)+dptr->d_name;
			int statRet = stat(filepath.c_str(), &st);
			if (S_ISDIR(st.st_mode)) continue;
			if (statRet != -1) {
				linkfiles.push_back(filepath);
			}
		}
		
		sort(linkfiles.begin(), linkfiles.end(),case_less());
		for (uint x=0; x<linkfiles.size(); x++) {
			LinkApp *link = new LinkApp(gmenu2x, path, linkfiles[x].c_str());
			if (link->targetExists())
				links[i].push_back( link );
			else
				free(link);
		}
		
		closedir(dirp);
	}
}
