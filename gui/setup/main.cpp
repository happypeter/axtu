#include <stdio.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <popt.h>

#include "classRpmEngine.h"
#include "classConfigParser.h"

#include "classLock.h"
#include "classSetup.h"

#include "commondef.h"

static int nArgServer;
static int nArgPath;
static int nArgAlarm; 
static int nArgLog;
static int nArgBlacklist;
static struct poptOption optionsTable[] = {

{ "server", (char) 's', POPT_ARG_NONE,&nArgServer,0,

 "You can setup server", NULL },

{ "path", (char) 'p', POPT_ARG_NONE,&nArgPath,0,

 "You can setup path", NULL },

{ "alarm", (char) 'a', POPT_ARG_NONE,&nArgAlarm,0,

 "You can setup alarm", NULL },
 
{ "blacklist", (char) 'b', POPT_ARG_NONE,&nArgBlacklist,0,
	
 "You can setup blacklist", NULL },
 
 { "log", (char) 'l', POPT_ARG_NONE,&nArgLog,0,

 "You can view log", NULL },



POPT_TABLEEND

};

int main(int argc, char *argv[])
{
	// Support multi language.
	// example file name :
	//     korean = ./qm/axtu_setup_ko.qm
	//     japen = ./qm/axtu_setup_jp.qm	
	QApplication * app = new QApplication(argc, argv);
	
	QTranslator translator( 0 );
	QStringList fields = QStringList::split( '.', QTextCodec::locale());
	if(translator.load( QString("axtu_setup_") + fields[0] ,  QM_FILE_PATH ) == true)
	{
		app->installTranslator( &translator );
	}

	if (getuid() != 0 )
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QObject::tr("You need to be root to run TSN Updater setup tool."));		
		exit(2);		
	}

	int option=0;
	int nMode=0;
	poptContext context = poptGetContext((const char*)argv[0],argc,(const char**)argv,(const struct poptOption* ) &optionsTable,0);

	option=poptGetNextOpt(context);
	if(option != -1)
	{
		poptPrintHelp(context,stdout,POPT_ARG_NONE);
		poptFreeContext(context);
		exit(2);
	}
	if( (nArgServer + nArgPath + nArgAlarm + nArgBlacklist + nArgLog) > 1 )
	{
		fprintf(stderr,"There are too many arguments.\nUsage : %s --help\n",argv[0]);
		exit(2);
	}	
	poptFreeContext(context);	
	if(nArgPath){
		nMode=PATH_SETUP;
	}else if(nArgAlarm){
		nMode=ALARM_SETUP;
	}else if(nArgBlacklist){
		nMode=BLACKLIST_SETUP;
	}else if(nArgLog){
		nMode=LOG_SETUP;
	}else{
		nMode=SERVER_SETUP;
	}
	
	bool bAlreadyExecuted = false;
	if(CHSLock::Islock((AXTU_SETUP_PID_FILE), (AXTU_SETUP_EXE_FILE)) == true){
		bAlreadyExecuted=true;		
	}
	else if(CHSLock::Islock((AXTU_SETUP_TUI_PID_FILE), (AXTU_SETUP_TUI_EXE_FILE)) == true){
		bAlreadyExecuted=true;		
	}
		
	if (bAlreadyExecuted)
	{
		QMessageBox::critical(0, QObject::tr("Setup - Asianux TSN Updater"),  QObject::tr("Updater setup tool is already running."));
		exit(2);
	}
	else
	{
		if(CHSLock::lock((AXTU_SETUP_PID_FILE)) == false)
		{
			QMessageBox::critical(0, QObject::tr("Setup - Asianux TSN Updater"),  QObject::tr("Internal Error #2, please contact your support provider, or try again later."));
			exit(2);			
		}
	}

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) 
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_SETUP_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) 
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_SETUP_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) 
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_SETUP_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) 
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_SETUP_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) 
	{
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_SETUP_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));		
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to open /dev/null").arg(AXTU_SETUP_EXE_FILE));
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to duplicate file descriptor").arg(AXTU_SETUP_EXE_FILE));
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);

	classSetup setup(nMode,0,Qt::WStyle_Customize | Qt::WStyle_NormalBorder);
	setup.show();
	app->setMainWidget(&setup);

	return app->exec();	
}


