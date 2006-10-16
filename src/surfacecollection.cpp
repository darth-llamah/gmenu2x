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

#include "surfacecollection.h"

using namespace std;

SurfaceCollection::SurfaceCollection(bool defaultAlpha) {
	this->defaultAlpha = defaultAlpha;
}

SurfaceCollection::~SurfaceCollection() {}

void SurfaceCollection::debug() {
	hash_map<string, Surface*>::iterator end = surfaces.end();
	for(hash_map<string, Surface*>::iterator curr = surfaces.begin(); curr != end; curr++){
		cout << "key: " << curr->first << endl;
	}
}

bool SurfaceCollection::exists(string path) {
	return surfaces.find(path) != surfaces.end();
}

Surface *SurfaceCollection::add(Surface *s, string path) {
	cout << "\033[0;34mGMENU2X:\033[0m Adding surface '" << path << "'" << endl;
	surfaces[path] = s;
	return s;
}

Surface *SurfaceCollection::add(string path, bool alpha) {
	cout << "\033[0;34mGMENU2X:\033[0m Adding surface '" << path << "'" << endl;
	Surface *s = new Surface(path,alpha);
	surfaces[path] = s;
	return s;
}

void SurfaceCollection::del(string path) {
	hash_map<string, Surface*>::iterator i = surfaces.find(path);
	if (i != surfaces.end()) {
		free(i->second);
		surfaces.erase(i);
		cout << "\033[0;34mGMENU2X:\033[0m Freed surface '" << path << "'" << endl;
	}
}

void SurfaceCollection::move(string from, string to) {
	del(to);
	surfaces[to] = surfaces[from];
	surfaces.erase(from);
}

Surface *SurfaceCollection::operator[](string key) {
	hash_map<string, Surface*>::iterator i = surfaces.find(key);
	if (i == surfaces.end())
		return add(key, defaultAlpha);
	else
		return i->second;
}
