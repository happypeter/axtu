/*!
@file classLauncher.h
@brief Class header file for launcher on GUI
*/
#ifndef CLASSLAUNCHER_H_
#define CLASSLAUNCHER_H_

#include "ui/frmlauncher.h"
#include "qprocess.h"
#include <stdlib.h>
#include <qlabel.h>
#include <qbitmap.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qimage.h>
#include <hsCommon.h>

enum { 
	UPDATE_MODE=1,
	INSTALL_MODE,
	ERASE_MODE,
	SETUP_MODE,
};

#define AXTU_LAUNCHER_TITLE tr("Asianux TSN Updater")

#define AXTU_GUI "/usr/share/axtu/gui/axtu-gui"

#define SETUP "/usr/share/axtu/gui/axtu-setup-gui"

#define BG_IMG "/usr/share/axtu/gui/images/Launcher_bg.png"
#define INSTALL_IMG "/usr/share/axtu/gui/images/Icon_install_n.png"
#define UPDATE_IMG "/usr/share/axtu/gui/images/Icon_update_n.png"
#define ERASE_IMG "/usr/share/axtu/gui/images/Icon_erase_n.png"
#define SETUP_IMG "/usr/share/axtu/gui/images/Icon_setup_n.png"

#define INSTALL_O_IMG "/usr/share/axtu/gui/images/Icon_install_o.png"
#define UPDATE_O_IMG "/usr/share/axtu/gui/images/Icon_update_o.png"
#define ERASE_O_IMG "/usr/share/axtu/gui/images/Icon_erase_o.png"
#define SETUP_O_IMG "/usr/share/axtu/gui/images/Icon_setup_o.png"


#define INSTALL_IMG_T "/usr/share/axtu/gui/images/Launcher_install_T.png"
#define UPDATE_IMG_T "/usr/share/axtu/gui/images/Launcher_update_T.png"
#define ERASE_IMG_T "/usr/share/axtu/gui/images/Launcher_erase_T.png"
#define SETUP_IMG_T "/usr/share/axtu/gui/images/Launcher_setup_T.png"

#define BG_COLOR "#C7EEFF"
#define FG_COLOR "#0268A8"
#define FG_TOP_COLOR "#003399"

/*!
@brief Class for GUI-Launcher. 

This program can start update, install, remove wizard.
*/
class myHoverButton : public QButton
{

public:
	myHoverButton(QWidget *parent=0, const char *name=0,	int x=0, int y=0, int number = 0, int nType=0);
	QPixmap  pixmap;
	void leaveEvent(QEvent *);   
	void enterEvent(QEvent *);   
	void paintEvent(QPaintEvent *);
	void mousePressEvent( QMouseEvent *);
	void mouseReleaseEvent( QMouseEvent *);
	//void mouseMoveEvent( QMouseEvent *e );
	void ChangeImage(bool bOn);	
	int keyNumber;  
	int m_nType;
};


class classLauncher : public frmLauncher
{
	Q_OBJECT
public:
	classLauncher(QWidget *parent, WFlags f);
	virtual ~classLauncher();	
	virtual void slotUpdate();
	virtual void slotInstall();
	virtual void slotErase();
	virtual void slotSetup();	
	virtual void windowActivationChange(bool oldActive );	
	
public slots:
	void slotProcessKilled();
private :
	QProcess * m_processUpdate;	
	QProcess * m_processInstall;
	QProcess * m_processErase;
	QProcess * m_processSetup;	
	myHoverButton * btnTUpdate;
	myHoverButton * btnTInstall;
	myHoverButton * btnTErase;
	myHoverButton * btnTSetup;	
	QLabel * img_contain;
	QLabel * img_contain2;
	QLabel * img_contain3;
	QLabel * img_contain4;
	QLabel * img_contain5;
	
	QLabel* img_contain_t;
	QLabel* img_contain_t2;
	QLabel* img_contain_t3;
	QLabel* img_contain_t4;
	

};

#endif /*CLASSLAUNCHER_H_*/

