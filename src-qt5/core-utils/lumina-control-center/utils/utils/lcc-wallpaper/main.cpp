#include <QTranslator>

#include <QApplication>
#include <QDebug>
#include <QFile>

#include <LuminaOS.h>
#include <LUtils.h>
#include <LuminaThemes.h>


#include "lcc_wallpaper.h"

int  main(int argc, char *argv[]) {
   LTHEME::LoadCustomEnvSettings();
   QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);   
   //Now go ahead and setup the app
   //LuminaThemeEngine theme(&a);
   //Now start the window
   lcc_wallpaper w;
    //QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(updateIcons()) );
   w.show();
   return  a.exec();
}
