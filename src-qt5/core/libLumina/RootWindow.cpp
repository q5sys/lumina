//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootWindow.h"

#include <QDesktopWidget>
#include <QScreen>
#include <QDebug>

#define DEBUG 1

// === PUBLIC ===
RootWindow::RootWindow() : QWidget(0, Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnBottomHint){
  qRegisterMetaType<WId>("WId");
  autoResizeTimer = 0;
  this->setMouseTracking(true);
}

RootWindow::~RootWindow(){

}

void RootWindow::start(){

  if(autoResizeTimer==0){
    autoResizeTimer = new QTimer(this);
      autoResizeTimer->setInterval(100); //1/10 second (collect all nearly-simultaneous signals and compress into a single update)
      autoResizeTimer->setSingleShot(true);
    connect(autoResizeTimer, SIGNAL(timeout()), this, SLOT(ResizeRoot()) );
    connect(QApplication::desktop(), SIGNAL(resized(int)), autoResizeTimer, SLOT(start()) );
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), autoResizeTimer, SLOT(start()) );
  }
  this->show();
  ResizeRoot();
  emit RegisterVirtualRoot(this->winId());
}

// === PRIVATE ===
void RootWindow::updateScreenPixmap(screeninfo *info){
  QPixmap pix(info->area.size());
  if(info->scale == RootWindow::SolidColor){
    QColor color;
    if(info->file.startsWith("rgb(")){
      QStringList colors = info->file.section(")",0,0).section("(",1,1).split(",");
      color = QColor(colors[0].toInt(), colors[1].toInt(), colors[2].toInt());
    }else{
      color = QColor(info->file);
    }
    pix.fill(color);
  }else{
    QPixmap raw(info->file); //load the image from file
    //Now apply the proper aspect ratio as needed
    if(info->scale == RootWindow::Stretch || info->scale == RootWindow::Full || info->scale == RootWindow::Fit){
       Qt::AspectRatioMode armode = Qt::KeepAspectRatio;
       if(info->scale == RootWindow::Stretch ){  armode = Qt::IgnoreAspectRatio; }
      else if(info->scale == RootWindow::Full ){ armode = Qt::KeepAspectRatioByExpanding; }
      if(raw.height()!=info->area.height() && raw.width() !=info->area.width()){
        raw = raw.scaled(info->area.size(), armode);
      }
    }
    //Now calculate offset and draw width/height
    QRect drawRect(0,0, raw.width(), raw.height());
    if(info->scale == RootWindow::Full ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::Fit ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::Center ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::Tile ){
      //Draw the entire area - no offset
      drawRect.setHeight(info->area.height());
      drawRect.setWidth(info->area.width());
    }else if(info->scale == RootWindow::BottomLeft ){
      drawRect.moveTo( 0 , info->area.height() - raw.height() );
    }else if(info->scale == RootWindow::BottomRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), (info->area.height() - raw.height()) );
    }else if(info->scale == RootWindow::BottomCenter ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, info->area.height() - raw.height() );
    }else if(info->scale == RootWindow::TopLeft ){
      drawRect.moveTo( 0, 0 );
    }else if(info->scale == RootWindow::TopRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), 0);
    }else if(info->scale == RootWindow::TopCenter ){
      drawRect.moveTo( (info->area.width() - raw.width())/2, 0);
    }else if(info->scale == RootWindow::CenterLeft ){
      drawRect.moveTo( 0, (info->area.height() - raw.height())/2 );
    }else if(info->scale == RootWindow::CenterRight ){
      drawRect.moveTo( (info->area.width() - raw.width()), (info->area.height() - raw.height())/2 );
    }

  QPainter P(&pix);
    P.setBrush(raw);
    P.setBrushOrigin( drawRect.x(), drawRect.y() );
    P.drawRect( drawRect );
} //end SolidColor Check

  info->wallpaper = pix;
}

RootSubWindow* RootWindow::windowForId(WId id){
  RootSubWindow *tmp = 0;
  for(int i=0; i<WINDOWS.length() && tmp==0; i++){
    if(WINDOWS[i]->id() == id){ tmp = WINDOWS[i]; }
  }
  return tmp;
}

// === PUBLIC SLOTS ===
void RootWindow::ResizeRoot(){
  if(DEBUG){ qDebug() << "Resize Root..."; }
  QList<QScreen*> scrns = QApplication::screens();
  //Update all the screen locations and ID's in the WALLPAPERS list
  QRect fullscreen;
  QStringList valid;
  //Update the size of the rootWindow itself
  for(int i=0; i<scrns.length(); i++){
    if(DEBUG){ qDebug() << " - Found Screen:" << scrns[i]->name() << scrns[i]->geometry(); }
    fullscreen = fullscreen.united(scrns[i]->geometry());
    valid << scrns[i]->name();
    for(int j=0; j<WALLPAPERS.length(); j++){
      if(WALLPAPERS[j].id == scrns[i]->name()){
        QSize oldsize = WALLPAPERS[j].area.size();
        WALLPAPERS[j].area = scrns[i]->geometry();
        if(oldsize != WALLPAPERS[j].area.size()){ updateScreenPixmap(&WALLPAPERS[j]); }
        break;
      }
    }
  }
  //Now clean up any invalid screen info in the WALLPAPERS List
  QStringList invalid;
  for(int i=0; i<WALLPAPERS.length(); i++){
    if(valid.contains(WALLPAPERS[i].id)){
      valid.removeAll(WALLPAPERS[i].id); //Already handled - remove it from the list
    }else{
      invalid << WALLPAPERS[i].id;
      WALLPAPERS.takeAt(i);
      i--;
    }
  }
  //Trigger a repaint and send out any signals
  if(DEBUG){ qDebug() << " - FullScreen Geometry:" << fullscreen; }
  this->setGeometry(fullscreen);
  this->update();
  emit RootResized(fullscreen);
  if(!valid.isEmpty()){ emit NewScreens(valid); }
  if(!invalid.isEmpty()){ emit RemovedScreens(invalid); }
  if(DEBUG){ qDebug() << " - Geom after change:" << this->geometry(); }
}

void RootWindow::ChangeWallpaper(QString id, RootWindow::ScaleType scale, QString file){
  bool found = false;
  for(int i=0; i<WALLPAPERS.length() && !found; i++){
    if(WALLPAPERS[i].id == id){
      WALLPAPERS[i].scale = scale;
      WALLPAPERS[i].file = file;
      updateScreenPixmap(&WALLPAPERS[i]);
      //qDebug() << "   --- Updated Wallpaper:" << WALLPAPERS[i].id << WALLPAPERS[i].file << WALLPAPERS[i].area;
      found = true;
    }
  }
  if(!found){
    ResizeRoot();
    //Need to create a new screeninfo structure
    QList<QScreen*> scrns = QApplication::screens();
    for(int i=0; i<scrns.length(); i++){
      if(scrns[i]->name()==id){
        screeninfo info;
          info.id = id;
	  info.file = file;
          info.scale = scale;
          info.area = scrns[i]->geometry();
         updateScreenPixmap(&info);
         //qDebug() << "   --- Loaded Wallpaper:" << info.id << info.file << info.area;
        WALLPAPERS << info;
        break;
      }
    }
  } //end check for a new id

}

void RootWindow::NewWindow(NativeWindow *win){
  RootSubWindow *subwin = 0;
  //qDebug() << "Got New Window:" << win->property(NativeWindow::Title);
  for(int i=0; i<WINDOWS.length() && subwin==0; i++){
    if(WINDOWS[i]->id() == win->id()){ subwin = WINDOWS[i]; }
  }
  if(subwin==0){
    subwin = new RootSubWindow(this, win);
    connect(win, SIGNAL(WindowClosed(WId)), this, SLOT(CloseWindow(WId)) );
    WINDOWS << subwin;
  }
  CheckWindowPosition(win->id(), true); //first-time run
  //win->setProperty(NativeWindow::Visible, true);
  //win->requestProperty( NativeWindow::Active, true);
  win->requestProperties(QList<NativeWindow::Property>() << NativeWindow::Visible << NativeWindow::Active, QList<QVariant>() << true << true);
}

void RootWindow::CloseWindow(WId win){
  for(int i=0; i<WINDOWS.length(); i++){
    if(WINDOWS[i]->id() == win){ WINDOWS.takeAt(i)->clientClosed(); break; }
  }
}

// === PRIVATE SLOTS ===

// === PROTECTED ===
void RootWindow::paintEvent(QPaintEvent *ev){
  //qDebug() << "RootWindow: PaintEvent:" << ev->rect();  //<< QDateTime::currentDateTime()->toString(QDateTime::ShortDate);
  QWidget::paintEvent(ev);
  bool found = false;
  QPainter painter(this);
  QRect geom = ev->rect();
    geom.adjust(-10,-10,10,10); //give it a few more pixels in each direction to repaint (noticing some issues in Qt 5.7.1)
  for(int i=0; i<WALLPAPERS.length(); i++){
    if(WALLPAPERS[i].area.intersects(geom) ){
      found = true;
      QRect intersect = WALLPAPERS[i].area.intersected(geom);
      painter.drawPixmap( intersect, WALLPAPERS[i].wallpaper, intersect.translated(-WALLPAPERS[i].area.x(), -WALLPAPERS[i].area.y()) );
    }
  }
  painter.end();
  /*if(!found){
    QWidget::paintEvent(ev);
  }*/
}