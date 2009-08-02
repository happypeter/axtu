#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qapplication.h> 
#include <qtextcodec.h>
#include <qmessagebox.h>
#include <qdir.h>

#include "hsCommon.h"
#include "classLauncher.h"

#define         LAUNCHER_GUI_PID_FILE          "/var/tmp/axtu-launcher.pid"
#define         LAUNCHER_GUI_EXE_FILE          "axtu-launcher"

int main(int argc, char *argv[])
{
	if (getuid() != 0 )
	{		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QObject::tr("You need to be root to run TSN Updater launcher"));
		exit(2);
	}
	
	
	QApplication app(argc, argv);
	//printf("arg[0] : %s,    arg[1] : %s\n", argv[0], argv[1]);	

	// Support multi language.
	// example file name : 
	//     english = ./COMMON/AXInstaller_en_US.qm
	//     korean = ./COMMON/AXInstaller_ko_KR.qm
	//     japen = japanese, ja_JP
	//     korean = ko_KR
	QTranslator translator( 0 );
	QStringList fields = QStringList::split( '.', QTextCodec::locale());
	if (translator.load( QString("axtu_launcher_") + fields[0] ,  QM_FILE_PATH ) == true)
	{
		app.installTranslator( &translator );
	}

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(LAUNCHER_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(LAUNCHER_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(LAUNCHER_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(LAUNCHER_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(LAUNCHER_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to open /dev/null").arg(LAUNCHER_GUI_EXE_FILE));
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to duplicate file descriptor").arg(LAUNCHER_GUI_EXE_FILE));
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);

	classLauncher gui(0, Qt::WStyle_Customize | Qt::WStyle_NormalBorder);
	gui.show();
	app.setMainWidget(&gui);

	return app.exec();
}

