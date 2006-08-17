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
#include "utilities.h"

using namespace std;

Link::Link(string path, const char* linkfile) {
	this->path = path;
	file = linkfile;
	wrapper = false;
	dontleave = false;
	setClock(0);

	string line;
	ifstream infile (linkfile, ios_base::in);
	while (getline(infile, line, '\n')) {
		string::size_type position = line.find("=");
		string name = trim(line.substr(0,position));
		string value = trim(line.substr(position+1,line.length()));
		if (name == "title") {
			title = value;
		} else if (name == "description") {
			description = value;
		} else if (name == "icon") {
			if (fileExists(value)) icon = value;
		} else if (name == "exec") {
			exec = value;
			if (icon=="") {
				string::size_type pos = exec.rfind(".");
				string execicon = exec.substr(0,pos)+".png";
				if (fileExists(execicon)) icon = execicon;
			}
		} else if (name == "params") {
			params = value;
		} else if (name == "workdir") {
			workdir = value;
		} else if (name == "wrapper") {
			if (value=="true") wrapper = true;
		} else if (name == "dontleave") {
			if (value=="true") dontleave = true;
		} else if (name == "clock") {
			setClock( atoi(value.c_str()) );
		} else {
			cout << "Unrecognized option: " << name << endl;
			break;
		}
	}

	edited = false;
}

int Link::clock() {
	return iclock;
}

string Link::clockStr() {
	return sclock;
}

void Link::setClock(int mhz) {
	if (mhz<100 || mhz>300) {
		iclock = 0;
		sclock = "Default";
	} else {
		iclock = mhz;
		stringstream ss;
		sclock = "";
		ss << iclock << "MHZ";
		ss >> sclock;
	}

	edited = true;
}

bool Link::targetExists() {
#ifndef TARGET_GP2X
	return true; //For displaying elements during testing on pc
#endif

	string target = exec;
	if (exec!="" && exec[0]!='/' && workdir!="")
		target = workdir + "/" + exec;

	return fileExists(target);
}

bool Link::save() {
	if (!edited) return false;
	cout << "GMENU2X: Saving link " << title << endl;

	ofstream f(file.c_str());
	if (f.is_open()) {
		if (title!=""      ) f << "title="       << title       << endl;
		if (description!="") f << "description=" << description << endl;
		if (icon!=""       ) f << "icon="        << icon        << endl;
		if (exec!=""       ) f << "exec="        << exec        << endl;
		if (params!=""     ) f << "params="      << params      << endl;
		if (workdir!=""    ) f << "workdir="     << workdir     << endl;
		if (iclock!=0      ) f << "clock="       << iclock      << endl;
		if (wrapper        ) f << "wrapper=true"                << endl;
		if (dontleave      ) f << "dontleave=true"              << endl;
		f.close();
		return true;
	} else
		cout << "GMENU2X: Error while opening the file '" << file << "' for write" << endl;
	return false;
}

void Link::run() {
	cout << "GMENU2X: Executing '" << title << "'" << endl;

	//Set correct working directory
	string wd = workdir;
	if (wd=="") {
		string::size_type pos = exec.rfind("/");
		if (pos!=string::npos)
			wd = exec.substr(0,pos);
	}
	if (wd!="") {
		if (wd[0]!='/') wd = path + wd;
		cout << "GMENU2X: chdir '" << wd << "'" << endl;
		chdir(wd.c_str());
	}

	//substitute @ with path
	string::size_type i = params.find("@");
	if (i != string::npos) params.replace(i,1,path);

	//if wrapper put exec in params and wrapper in exec
	if (wrapper) {
		params = exec + " " + params;
		exec = path + "scripts/wrapper.sh";
	}

	//check if we have to quit
	if (dontleave) {
		string command = exec;
		if (params!="") command += " " + params;
		system(command.c_str());
	} else {
		execlp(exec.c_str(),exec.c_str(), params == "" ? NULL : params.c_str() ,NULL);
	}

	//in case execl fails or dontleave
	chdir(path.c_str());
}
