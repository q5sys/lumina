//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_WALLPAPER_H
#define _LCC_WALLPAPER_H
#include "../../globals.h"

namespace Ui{
	class lcc_wallpaper;
};

class lcc_wallpaper : public QMainWindow{
	Q_OBJECT
public:
    lcc_wallpaper();
	~lcc_wallpaper();

	bool needsScreenSelector(){ return true; }

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
    Ui::lcc_wallpaper *ui;
	int cScreen, cBG; //current screen number/background
	QString DEFAULTBG;
	bool loading;

	QString getColorStyle(QString current, bool allowTransparency);

private slots:
	void updateMenus();
	void deskbgchanged();
	void desktimechanged();
	void deskbgremoved();
	void deskbgadded();
	void deskbgcoloradded();
	void deskbgdiradded();
	void deskbgdirradded();

protected:
	void resizeEvent(QResizeEvent*){
	  deskbgchanged(); //update the wallpaper preview
	}
};
#endif
