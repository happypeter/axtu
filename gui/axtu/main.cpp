/*!
@file main.cpp 
@brief main source file for updater on gui
*/
#include <stdio.h>
#include <qapplication.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qdir.h>
#include <popt.h> 

#include "classRpmEngine.h"
#include "classConfigParser.h"

#include "classLock.h"
#include "classGui.h"

#include "commondef.h"

void TestConfiger();
int TestRpmEngine(int arg, char *argv[]);
void TestRpmHsNetwork();

static int nArgUpdate;
static int nArgInstall;
static int nArgErase;
static int nArgForce;
classGui *gui=NULL;		
QApplication *app=NULL;
static struct poptOption optionsTable[] = {

{ "update", (char) 'u', POPT_ARG_NONE,&nArgUpdate,0,

 "You can update", NULL },

{ "install", (char) 'i', POPT_ARG_NONE,&nArgInstall,0,

 "You can install", NULL },

{ "erase", (char) 'e', POPT_ARG_NONE,&nArgErase,0,

 "You can erase", NULL },

{ "force", (char) 'f', POPT_ARG_NONE,&nArgForce,0,

 "You can execute by force", NULL },


POPT_TABLEEND

};
		
void ExitFunc (void)
{		
	if(gui)
	{
		delete gui;
		gui = NULL;		
	}
	if(app)
	{
		delete app;
		app = NULL;
	}
}


int main(int argc, char *argv[])
{	
	if(atexit(ExitFunc) != 0)
	{
		exit(1);
	}
	// Support multi language.
	// example file name : 
	//     english = ./COMMON/axtu_en_US.qm
	//     korean = ./COMMON/axtu_ko_KR.qm
	//     japan = japanese, ja_JP
	//     korean = ko_KR
	app = new QApplication(argc, argv); 
	QTranslator translator( 0 );
	QStringList fields = QStringList::split( '.', QTextCodec::locale());
	if(translator.load( QString("axtu_") + fields[0] ,  QM_FILE_PATH ) == true)
	{
		app->installTranslator( &translator );
	}
	

	/*!
	 * Confirmation of user permission
	 */
	if (getuid() != 0 )
	{		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QObject::tr("You need to be root to run TSN Updater."));		
		exit(2);
	}
		
	/* !
	 * Verification to update paramete
	 */
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
	if( (nArgUpdate+nArgInstall+nArgErase) > 1)
	{
		fprintf(stderr,"There are too many arguments.\nUsage : %s --help\n",argv[0]);
		exit(2);
	}	
	poptFreeContext(context);
	
	
	/*!
	 * Decide mode between update and install
	 */	
	if(nArgInstall)
	{
		nMode=INSTALL_MODE;
	}
	else if(nArgErase)
	{
		nMode=ERASE_MODE;
	}
	else
	{
		nMode=UPDATE_MODE;
	}

	
	/*!
	 * To confirm duplication of executing update
	 */
	bool bAlreadyExecuted = false;
	if (!nArgForce)
	{
		if (CHSLock::Islock((AXTU_GUI_PID_FILE), (AXTU_GUI_EXE_FILE)) == true)
		{
			bAlreadyExecuted = true;
		}
		else if (CHSLock::Islock((AXTU_TUI_PID_FILE), (AXTU_TUI_EXE_FILE)) == true)
		{
			bAlreadyExecuted = true;
		}
		else if (CHSLock::Islock((AXTU_CUI_PID_FILE), (AXTU_CUI_EXE_FILE)) == true)
		{
			bAlreadyExecuted = true;
		}
	}	

	if(bAlreadyExecuted == true)
	{		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QObject::tr("TSN Updater is already running."));
		exit(2);
	}
	else
	{
		if(CHSLock::lock((AXTU_GUI_PID_FILE)) == false)
		{			
			QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QObject::tr("Internal Error #2, please contact your support provider, or try again later."));
			exit(2);
		}
	}	
		
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: %2").arg(AXTU_GUI_EXE_FILE, QString::fromLocal8Bit(strerror(errno))));
		exit(1);
	}

	/*!
	 * Redirect STDERR to /dev/null
	 */
	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to open /dev/null").arg(AXTU_GUI_EXE_FILE));
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);		
		QMessageBox::critical(0, QObject::tr("Asianux TSN Updater"),  QString("%1: failed to duplicate file descriptor").arg(AXTU_GUI_EXE_FILE));
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);
	
	gui = new classGui(nMode);
	gui->show();
	app->setMainWidget(gui);	
	return app->exec();
}


static void ReadHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	
	printf("p1 = %d%%,   p2 = %d%%,  title = %s,  content = %s\n", p1, p2, msg1, msg2);
}

static void RunCallBack(int p1, int p2, const char * msg1, const char * msg2)
{
	printf("p1 = %d%%,   p2 = %d%%,  title = %s,  content = %s\n", p1, p2, msg1, msg2);
}

int TestRpmEngine(int arg, char *argv[])
{	
	
	classRpmEngine *rpmEngine = new classRpmEngine();
	if(arg == 1)
	{	
		cout << "usage : hsupdater2 /pullpath/aaa.rpm /pullpath/bbb.rpm" << endl; 
		return -1;				    
	}
	else 
	{
		for(int i=1;i<arg;i++)
		{
			if (rpmEngine->AddFile(argv[i], UPDATE, 0) == -1)
			{
				printf("File not found!!\n");
				return -1;
			}
		}	    	  	
	}	  
	
	rpmEngine->ReadRemoteHeaderInfo();
	rpmEngine->ReadLocalHeaderInfo();
	rpmEngine->CreateUpdateInstallList();	
	//Test show udpate, install list
			
	vector <structFileInfo> vectorTest = rpmEngine->GetInstallList();
	vector <structFileInfo>::iterator it;
	for(it=vectorTest.begin();it!=vectorTest.end();it++)
	{
		cout << "Install list " << it->strName << endl;
	}
	
	vector <structFileInfo> vectorTest2 = rpmEngine->GetUpdateList();
	vector <structFileInfo>::iterator it2;
	for(it2=vectorTest2.begin();it2!=vectorTest2.end();it2++)
	{
		cout << "Update list " << it2->strName << endl;
	}	
	
	rpmEngine->SetReadHeadersCallBack(ReadHeadersCallBack);
	rpmEngine->ReadHeaders();
	int result = rpmEngine->Check(); 
	if(result > -1)
	{
		if (result > 0)
		{	
			while(1)
			{
				//test 
				rpmEngine->ChangeGrubToDefaultKernel();
				printf("Dep file is exist! Do you want continue?(yes/no)\n");
				string strInput;
				cin >> strInput;
				if(strInput == "no")
				{
					delete rpmEngine;
					return -1;
				}
				else if(strInput == "yes")
				{
					break;
				}
			}
		}
		rpmEngine->SetRunCallBack(RunCallBack);
		rpmEngine->Run();	  
	}
	
	delete rpmEngine;
	return 0;
}

void TestRpmHsNetwork()
{
/*	classHsNetwork *hsNetwork = new classHsNetwork();	
	delete hsNetwork;
*/
}

void TestConfiger()
{
	classConfigParser * configParser = new classConfigParser();
	configParser->SetConfigFilePath("/etc/hsyum.conf");	
	configParser->Read();
	
	configParser->AddSection("AAA");
	configParser->AddSection("BBB");
	configParser->AddSection("CCC");
	
	configParser->SetOption("AAA", "aaa", "  test");
	configParser->SetOption("AAA", "bbb", "test");
	configParser->SetOption("AAA", "ccc", "  test");
	
	configParser->SetOption("BBB", "aaa", "test");
	configParser->SetOption("BBB", "bbb", "test");
	configParser->SetOption("BBB", "ccc", "test   ");
	
	configParser->SetOption("CCC", "aaa", "tes t");
	configParser->SetOption("CCC", "bbb", "te st");
	configParser->SetOption("CCC", "ccc", "t  est");
	
		
	
	vector<string> sectionVector;
	sectionVector = configParser->GetSections();
	
	vector<string>::iterator i;
	vector<string>::iterator i2;
	for(i=sectionVector.begin();i!=sectionVector.end();i++)
	{	
		vector<string> optionVector;
		optionVector = configParser->GetOptions(*i);
		printf("sections : %s\n", (*i).c_str());
	  for(i2=optionVector.begin();i2!=optionVector.end();i2++)
	   {
	   			configParser->GetOption(*i, *i2);
	   	    printf("    options : %s = %s\n", (*i2).c_str() , configParser->GetOption(*i, *i2).c_str());
	   }	
	}
	
	printf("HasSection Test\n");
	printf("configParser->HasSection(AAA) = %d\n", configParser->HasSection("AAA"));
	printf("configParser->HasSection(XXX) = %d\n", configParser->HasSection("XXX"));
	
		
	printf("HasOption Test\n");
	printf("configParser->HasOption(AAA, aaa) = %d\n", configParser->HasOption("AAA", "aaa"));
	printf("configParser->HasOption(AAA, xxx) = %d\n", configParser->HasOption("XXX", "xxx"));
	
	
	printf("RemoveOption Test\n");
	configParser->RemoveOption("AAA", "aaa");
  configParser->RemoveOption("AAA", "ccc");	
	
	
	sectionVector = configParser->GetSections();
	for(i=sectionVector.begin();i!=sectionVector.end();i++)
	{	
		vector<string> optionVector;
		optionVector = configParser->GetOptions(*i);
		printf("sections : %s\n", (*i).c_str());
	  for(i2=optionVector.begin();i2!=optionVector.end();i2++)
	   {
	   			configParser->GetOption(*i, *i2);
	   	    printf("    options : %s = %s\n", (*i2).c_str() , configParser->GetOption(*i, *i2).c_str());
	   }	
	}
	
	
	printf("RemoveSection Test\n");
	configParser->RemoveSection("CCC");
	sectionVector = configParser->GetSections();
	for(i=sectionVector.begin();i!=sectionVector.end();i++)
	{	
		vector<string> optionVector;
		optionVector = configParser->GetOptions(*i);
		printf("sections : %s\n", (*i).c_str());
	  for(i2=optionVector.begin();i2!=optionVector.end();i2++)
	   {
	   			configParser->GetOption(*i, *i2);
	   	    printf("    options : %s = %s\n", (*i2).c_str() , configParser->GetOption(*i, *i2).c_str());
	   }	
	}
	configParser->Write("/etc/Test.conf");
	//configParser->Write();
	
	delete configParser;	
	
}
