/*!
@file classLauncher.cpp
@brief Class source file for launcher on GUI
*/
#include "classLauncher.h"
#include <qcursor.h>
#include <qmessagebox.h>

myHoverButton::myHoverButton(QWidget *parent, const char *name,  int x, int y, int number, int nType)
							:QButton(parent, name)
{
	
	m_nType = nType;	
	ChangeImage(false);
	// Transparent image.
	/*
	if ( img.hasAlphaBuffer() ) {
		QBitmap bm;
		bm = img.createAlphaMask();
		pixmap.setMask( bm );
		this->setMask( *pixmap.mask() );
	}
	*/  
	this->setGeometry(x, y, pixmap.width(), pixmap.height());
	
	keyNumber = number;
}

void myHoverButton::paintEvent( QPaintEvent *e )
{
	QRect ur = e->rect();            // rectangle to update
		  
	bitBlt( this, ur.left(), ur.top(), 
			&pixmap,  ur.left(), ur.top(), 
			pixmap.width(), pixmap.height());
}

void myHoverButton::leaveEvent( QEvent *e )
{	  
	ChangeImage(false);
	this->update();
	e=NULL;
}

void myHoverButton::enterEvent( QEvent *e )
{		  
	ChangeImage(true);  
	this->update();
	e=NULL;
}

void myHoverButton::mousePressEvent( QMouseEvent *e )
{		  
	ChangeImage(false);  
	this->update();
	QButton::mousePressEvent(e);
}

void myHoverButton::mouseReleaseEvent( QMouseEvent *e )
{	
	ChangeImage(false);
  
	this->update();
	QButton::mouseReleaseEvent(e);
}

void myHoverButton::ChangeImage(bool bOn)
{
	QString fn;
	switch(m_nType)
	{	
		case INSTALL_MODE :
			fn = bOn?INSTALL_O_IMG:INSTALL_IMG;
			break;
		case ERASE_MODE :			
			fn = bOn?ERASE_O_IMG:ERASE_IMG;
			break;
		case SETUP_MODE :			
			fn = bOn?SETUP_O_IMG:SETUP_IMG;
			break;
		default:			
			fn = bOn?UPDATE_O_IMG:UPDATE_IMG;
			break;
	}
	
	QImage img(fn);
	pixmap.convertFromImage(img);
}

/*
void myHoverButton::mouseMoveEvent( QMouseEvent *e )
{
  //printf("In button Event\n");
}
*/


classLauncher::classLauncher(QWidget *parent, WFlags f ):frmLauncher(parent,0,FALSE,f)
{	
	m_processUpdate = NULL; 	
	m_processInstall = NULL;
	m_processErase = NULL;
	m_processSetup = NULL;	
	btnTUpdate = NULL;
	btnTInstall = NULL;
	btnTErase = NULL;
	btnTSetup = NULL;
	img_contain = NULL;
	img_contain2 = NULL;
	img_contain3 = NULL;
	img_contain4 = NULL;
	img_contain4 = NULL;	
	img_contain_t = NULL;
	img_contain_t2 = NULL;
	img_contain_t3 = NULL;
	img_contain_t4 = NULL;
	
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setCaption(AXTU_LAUNCHER_TITLE);
	this->setFixedSize(QSize(690,430));
	this->setPaletteBackgroundPixmap(QPixmap(BG_IMG));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnTUpdate = new myHoverButton( this, "", 5, 5, 1, UPDATE_MODE);	
	btnTUpdate->setGeometry( btnUpdate->frameGeometry() );	
	btnUpdate->setShown(false);
	
	
	btnTInstall = new myHoverButton( this, "", 5, 5, 1, INSTALL_MODE);	
	btnTInstall->setGeometry( btnInstall->frameGeometry() );	
	btnInstall->setShown(false);
		
	btnTSetup = new myHoverButton( this, "", 5, 5, 1, SETUP_MODE);	
	btnTSetup->setGeometry( btnErase->frameGeometry() );	
	btnSetup->setShown(false);
	
	btnTErase = new myHoverButton( this, "", 5, 5, 1, ERASE_MODE);	
	btnTErase->setGeometry(0,0,0,0);	
	btnTErase->setShown(false);	
	btnErase->setShown(false);
	
	m_processUpdate = new QProcess();	
	m_processInstall = new QProcess();
	m_processErase = new QProcess();
	m_processSetup = new QProcess();
	
	img_contain = new QLabel( this, "No Image" );	
	img_contain->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain->setPixmap( NULL );
	img_contain->setMask(img_contain->pixmap()->createHeuristicMask());
	img_contain->setGeometry( labelTop->frameGeometry() );	 
	labelTop->setShown(false);
	img_contain->setPaletteForegroundColor(QColor(FG_TOP_COLOR));
	img_contain->setText(tr("<p align=\"center\"><b><font size=\"+3\">Asianux TSN Updater</font></b></p>"));
	
	img_contain2 = new QLabel( this, "No Image" );
	img_contain2->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	img_contain2->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain2->setPixmap( NULL );
	img_contain2->setMask(img_contain2->pixmap()->createHeuristicMask());
	img_contain2->setGeometry( labelUpdate->frameGeometry() );
	img_contain2->setText(tr("<b>Update installed packages to the latest version</b><br> You can update installed packages on your system from the update server(s). We recommand you to update the packages as often as possible for security reason."));
	
	img_contain3 = new QLabel( this, "No Image" );
	img_contain3->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	img_contain3->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain3->setPixmap( NULL );
	img_contain3->setMask(img_contain3->pixmap()->createHeuristicMask());
	img_contain3->setGeometry( labelInstall->frameGeometry() );
	img_contain3->setText(tr("<b>Install new packages</b><br>You can install additional packages from the update server(s)."));
	
	img_contain5 = new QLabel( this, "No Image" );
	img_contain5->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	img_contain5->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain5->setPixmap( NULL );
	img_contain5->setMask(img_contain5->pixmap()->createHeuristicMask());
	img_contain5->setGeometry( labelErase->frameGeometry() );
	img_contain5->setText(tr("<b>Setup Asianux TSN Updates</b> <br> You can check and modify the Asianux TSN Updater's configuration."));
	
	img_contain4 = new QLabel( this, "No Image" );
	img_contain4->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	img_contain4->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain4->setPixmap( NULL );
	img_contain4->setMask(img_contain4->pixmap()->createHeuristicMask());
	img_contain4->setGeometry(0,0,0,0 );
	img_contain4->setText(tr("<b>Erase packages</b> <br>You can remove packages installed on your system. Please be aware that removing preinstalled packages might cause a serious system crash."));
	img_contain4->setGeometry(0,0,0,0 );
	labelErase->setShown(false);
	labelSetup->setShown(false);
	

	QPixmap pix1(UPDATE_IMG_T);
	img_contain_t = new QLabel( this, "No Image" );	
	img_contain_t->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain_t->setPixmap( pix1 );
	img_contain_t->setMask(img_contain_t->pixmap()->createHeuristicMask());
	img_contain_t->setGeometry(labelUpdateImage->frameGeometry().left(), labelUpdateImage->frameGeometry().top(), pix1.width(),pix1.height());
	labelUpdateImage->setShown(false);	 
	
	QPixmap pix2(INSTALL_IMG_T);
	img_contain_t2 = new QLabel( this, "No Image" );	
	img_contain_t2->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain_t2->setPixmap( pix2 );
	img_contain_t2->setMask(img_contain_t2->pixmap()->createHeuristicMask());
	img_contain_t2->setGeometry(labelInstallImage->frameGeometry().left(), labelInstallImage->frameGeometry().top(), pix2.width(),pix2.height());
	labelInstallImage->setShown(false);
	
	QPixmap pix4(SETUP_IMG_T);
	img_contain_t4 = new QLabel( this, "No Image" );	
	img_contain_t4->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain_t4->setPixmap( pix4 );
	img_contain_t4->setMask(img_contain_t4->pixmap()->createHeuristicMask());
	img_contain_t4->setGeometry(labelEraseImage->frameGeometry().left(), labelEraseImage->frameGeometry().top(), pix4.width(),pix4.height());
	
	QPixmap pix3(ERASE_IMG_T);
	img_contain_t3 = new QLabel( this, "No Image" );	
	img_contain_t3->setBackgroundOrigin( QLabel::ParentOrigin );
	img_contain_t3->setPixmap( pix3 );
	img_contain_t3->setMask(img_contain_t3->pixmap()->createHeuristicMask());
	img_contain_t3->setGeometry(0,0,0,0);
	labelEraseImage->setShown(false);
	labelSetupImage->setShown(false);
		
	
	connect(btnTUpdate, SIGNAL(clicked()), this, SLOT(slotUpdate()));
	connect(btnTInstall, SIGNAL(clicked()), this, SLOT(slotInstall()));
	//connect(btnTErase, SIGNAL(clicked()), this, SLOT(slotErase()));
	connect(btnTSetup, SIGNAL(clicked()), this, SLOT(slotSetup()));
	
	
	connect(m_processUpdate, SIGNAL(processExited()), this, SLOT(slotProcessKilled()));
	connect(m_processInstall, SIGNAL(processExited()), this, SLOT(slotProcessKilled()));
	//connect(m_processErase, SIGNAL(processExited()), this, SLOT(slotProcessKilled()));
	connect(m_processSetup, SIGNAL(processExited()), this, SLOT(slotProcessKilled()));
		
}

classLauncher::~classLauncher()
{	
	if(m_processUpdate) delete m_processUpdate;	
	if(m_processInstall) delete m_processInstall;
	if(m_processErase) delete m_processErase;
	if(m_processSetup) delete m_processSetup;	
	if(btnTUpdate) delete btnTUpdate;
	if(btnTInstall) delete btnTInstall;
	if(btnTErase) delete btnTErase;
	if(btnTSetup) delete btnTSetup;
	if(img_contain) delete img_contain;
	if(img_contain2) delete img_contain2;
	if(img_contain3) delete img_contain3;
	if(img_contain4) delete img_contain4;
	if(img_contain5) delete img_contain5;
	if(img_contain_t) delete img_contain_t;
	if(img_contain_t2) delete img_contain_t2;
	if(img_contain_t3) delete img_contain_t3;
	if(img_contain_t4) delete img_contain_t4;
	
	
}

//! @brief Start update wizard.
void classLauncher::slotUpdate()
{	
	this->setCursor(Qt::WaitCursor);		
	m_processUpdate->clearArguments();
	m_processUpdate->addArgument(AXTU_GUI);	
	m_processUpdate->addArgument("-u");
	if( m_processUpdate->start() == FALSE)
	{
		this->setCursor(Qt::ArrowCursor);
		QMessageBox::critical(this, tr("Error"), tr("Fail to execute program"));
	}	
}

//! @brief Start install wizard.
void classLauncher::slotInstall()
{	
	this->setCursor(Qt::WaitCursor);
	m_processInstall->clearArguments();
	m_processInstall->addArgument(AXTU_GUI);	
	m_processInstall->addArgument("-i");	
	if( m_processInstall->start() == FALSE)
	{
		this->setCursor(Qt::ArrowCursor);
		QMessageBox::critical(this, tr("Error"), tr("Fail to execute program"));
	}
}

//! @brief Start erase wizard.
void classLauncher::slotErase()
{
	
	this->setCursor(Qt::WaitCursor);
	m_processErase->clearArguments();
	m_processErase->addArgument(AXTU_GUI);	
	m_processErase->addArgument("-e");	
	if( m_processErase->start() == FALSE)
	{
		this->setCursor(Qt::ArrowCursor);
		QMessageBox::critical(this, tr("Error"), tr("Fail to execute program"));
	}	
}

//! @brief Start axtu seteup.
void classLauncher::slotSetup()
{
	
	this->setCursor(Qt::WaitCursor);	
	m_processSetup->clearArguments();
	m_processSetup->addArgument(SETUP);	
	if( m_processSetup->start() == FALSE)
	{
		this->setCursor(Qt::ArrowCursor);
		QMessageBox::critical(this, tr("Error"), tr("Fail to execute program"));
	}	
}

//! @brief When other window(updater, setup) will be showed,  This window will be inactive, then this function will be called.
void classLauncher::windowActivationChange(bool oldActive )
{
	if(oldActive == true)
		this->setCursor(Qt::ArrowCursor);
}


//! @brief When any process will be finished, This function wiil be called.
void classLauncher::slotProcessKilled()
{
	this->setCursor(Qt::ArrowCursor);	
}
