#include <string>

#include "browsedialog.h"

#include "FastDelegate.h"
#include "filelister.h"
#include "gmenu2x.h"

using namespace fastdelegate;

BrowseDialog::BrowseDialog(GMenu2X *gmenu2x, const string &title,
		const string &subtitle)
	: Dialog(gmenu2x)
	, title(title)
	, subtitle(subtitle)
	, ts_pressed(false)
	, buttonBox(gmenu2x)
{
	IconButton *btn;

	buttonBox.add(new IconButton(gmenu2x, "skin:imgs/buttons/left.png"));
	btn = new IconButton(gmenu2x, "skin:imgs/buttons/cancel.png", gmenu2x->tr["Up one folder"]);
	btn->setAction(MakeDelegate(this, &BrowseDialog::directoryUp));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/accept.png", gmenu2x->tr["Enter folder"]);
	btn->setAction(MakeDelegate(this, &BrowseDialog::directoryEnter));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/start.png", gmenu2x->tr["Confirm"]);
	btn->setAction(MakeDelegate(this, &BrowseDialog::confirm));
	buttonBox.add(btn);

	btn = new IconButton(gmenu2x, "skin:imgs/buttons/select.png", gmenu2x->tr["Exit"]);
	btn->setAction(MakeDelegate(this, &BrowseDialog::quit));
	buttonBox.add(btn);

	iconGoUp = gmenu2x->sc.skinRes("imgs/go-up.png");
	iconFolder = gmenu2x->sc.skinRes("imgs/folder.png");
	iconFile = gmenu2x->sc.skinRes("imgs/file.png");
}

BrowseDialog::~BrowseDialog()
{
}

bool BrowseDialog::exec()
{
	if (!fl)
		return false;

	string path = fl->getPath();
	if (path.empty() || !fileExists(path) || path.compare(0, CARD_ROOT_LEN, CARD_ROOT) != 0)
		setPath(CARD_ROOT);

	fl->browse();

	rowHeight = gmenu2x->font->getHeight()+1; // gp2x=15+1 / pandora=19+1
	numRows = (gmenu2x->resY-gmenu2x->skinConfInt["topBarHeight"]-20)/rowHeight;
	clipRect = (SDL_Rect){0, gmenu2x->skinConfInt["topBarHeight"]+1, gmenu2x->resX-9, gmenu2x->resY-gmenu2x->skinConfInt["topBarHeight"]-25};
	touchRect = (SDL_Rect){2, gmenu2x->skinConfInt["topBarHeight"]+4, gmenu2x->resX-12, clipRect.h};

	selected = 0;
	close = false;
	while (!close) {
		if (gmenu2x->ts.initialized()) gmenu2x->ts.poll();

		paint();

		handleInput();
	}

	return result;
}

BrowseDialog::Action BrowseDialog::getAction(bevent_t *event)
{
	BrowseDialog::Action action;

    switch(event->button) {
        case MENU:
            action = BrowseDialog::ACT_CLOSE;
            break;
        case UP:
            action = BrowseDialog::ACT_UP;
            break;
        case DOWN:
            action = BrowseDialog::ACT_DOWN;
            break;
        case ALTLEFT:
            action = BrowseDialog::ACT_SCROLLUP;
            break;
        case ALTRIGHT:
            action = BrowseDialog::ACT_SCROLLDOWN;
            break;
        case LEFT:
        case CANCEL:
            action = BrowseDialog::ACT_GOUP;
            break;
        case ACCEPT:
            action = BrowseDialog::ACT_SELECT;
            break;
        case SETTINGS:
            action = BrowseDialog::ACT_CONFIRM;
            break;
        default:
            action = BrowseDialog::ACT_NONE;
            break;
    }

	return action;
}

void BrowseDialog::handleInput()
{
	BrowseDialog::Action action;
    bevent_t event;

    do {
        gmenu2x->input.waitForEvent(&event);
    } while (event.state != PRESSED);

    /*
	while(!gmenu2x->input.update())
      usleep(LOOP_DELAY);
      */

	if (ts_pressed && !gmenu2x->ts.pressed()) {
		action = BrowseDialog::ACT_SELECT;
		ts_pressed = false;
	} else {
		action = getAction(&event);
	}

	if (gmenu2x->ts.initialized() && gmenu2x->ts.pressed() && !gmenu2x->ts.inRect(touchRect)) ts_pressed = false;

	if (action == BrowseDialog::ACT_SELECT && (*fl)[selected] == "..")
		action = BrowseDialog::ACT_GOUP;
	switch (action) {
	case BrowseDialog::ACT_CLOSE:
		quit();
		break;
	case BrowseDialog::ACT_UP:
		if (selected == 0)
			selected = fl->size() - 1;
		else
			selected -= 1;
		break;
	case BrowseDialog::ACT_SCROLLUP:
		if (selected <= numRows - 2)
			selected = 0;
		else
			selected -= numRows - 2;
		break;
	case BrowseDialog::ACT_DOWN:
		if (fl->size() - 1 <= selected)
			selected = 0;
		else
			selected += 1;
		break;
	case BrowseDialog::ACT_SCROLLDOWN:
		if (selected+(numRows-2)>=fl->size())
			selected = fl->size()-1;
		else
			selected += numRows-2;
		break;
	case BrowseDialog::ACT_GOUP:
		directoryUp();
		break;
	case BrowseDialog::ACT_SELECT:
		if (fl->isDirectory(selected)) {
			directoryEnter();
			break;
		}
		/* Falltrough */
	case BrowseDialog::ACT_CONFIRM:
		confirm();
		break;
	default:
		break;
	}

	buttonBox.handleTS();
}

#include <iostream>

void BrowseDialog::directoryUp()
{
	string path = fl->getPath();
	string::size_type p = path.rfind("/");

	if (p == path.size() - 1)
		p = path.rfind("/", p - 1);
	
	if (p == string::npos || path.compare(0, 1, "/") != 0 || path.length() < 2) {
		quit();
	} else {
		selected = 0;
		setPath(path.substr(0, p));
	}
}

void BrowseDialog::directoryEnter()
{
	string path = fl->getPath();
	if (path[path.size()-1] != '/')
		path += "/";

	setPath(path + fl->at(selected));

	selected = 0;
}

void BrowseDialog::confirm()
{
	result = true;
	close = true;
}

void BrowseDialog::quit()
{
	result = false;
	close = true;
}

void BrowseDialog::paint()
{
	unsigned int i, iY;
	unsigned int firstElement, lastElement;
	unsigned int offsetY;
	Surface *icon;

	gmenu2x->bg->blit(gmenu2x->s, 0, 0);
	drawTitleIcon("icons/explorer.png", true);
	writeTitle(title);
	writeSubTitle(subtitle);

	buttonBox.paint(5);

	// TODO(MtH): I have no idea what the right value of firstElement would be,
	//            but originally it was undefined and that is never a good idea.
	firstElement = 0;
	if (selected>firstElement+numRows-1)
		firstElement = selected-numRows+1;
	else if (selected < firstElement)
		firstElement = selected;

	//Selection
	iY = selected-firstElement;
	iY = gmenu2x->skinConfInt["topBarHeight"]+1+(iY*rowHeight);
	gmenu2x->s->box(2, iY, gmenu2x->resX-12, rowHeight-1, gmenu2x->skinConfColors[COLOR_SELECTION_BG]);

	lastElement = firstElement + numRows;
	if (lastElement > fl->size())
		lastElement = fl->size();

	offsetY = gmenu2x->skinConfInt["topBarHeight"]+1;

	//Files & Directories
	gmenu2x->s->setClipRect(clipRect);
	for (i = firstElement; i < lastElement; i++) {
		if (fl->isDirectory(i)) {
			if ((*fl)[i]=="..")
				icon = iconGoUp;
			else
				icon = iconFolder;
		} else {
			icon = iconFile;
		}
		icon->blit(gmenu2x->s, 5, offsetY);
		gmenu2x->s->write(gmenu2x->font, (*fl)[i], 24, offsetY + 8, ASFont::HAlignLeft, ASFont::VAlignMiddle);

		if (gmenu2x->ts.initialized() && gmenu2x->ts.pressed() && gmenu2x->ts.inRect(touchRect.x, offsetY + 3, touchRect.w, rowHeight)) {
			ts_pressed = true;
			selected = i;
		}

		offsetY += rowHeight;
	}
	gmenu2x->s->clearClipRect();

	gmenu2x->drawScrollBar(numRows,fl->size(),firstElement,clipRect.y,clipRect.h);
	gmenu2x->s->flip();
}
