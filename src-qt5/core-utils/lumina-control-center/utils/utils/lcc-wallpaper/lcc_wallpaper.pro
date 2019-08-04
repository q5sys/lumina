include("$${PWD}/../../OS-detect.pri")

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras 


TARGET = lcc-wallpaper

target.path = $${L_BINDIR}

TEMPLATE = app

#include all the special classes from the Lumina tree
include(../../../../core/libLumina/LDesktopUtils.pri) #includes LUtils
include(../../../../core/libLumina/LuminaThemes.pri)

#NO_XINPUT{
  DEFINES+=NO_XINPUT
#}else{
#  include(../../core/libLumina/LInputDevice.pri)
#}

SOURCES += main.cpp \
	   lcc_wallpaper.cpp 

HEADERS  += lcc_wallpaper.h 

FORMS    += lcc_wallpaper.ui 

desktop.files=lcc_wallpaper.desktop
desktop.path=$${L_SHAREDIR}/applications/

INSTALLS += target desktop
