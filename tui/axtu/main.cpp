#include <stdio.h>
#include <popt.h>

#include <libintl.h>
#include <locale.h>

#include "classTui.h"
#include "classLock.h"
#include "commondef.h"
#include "classRedcastle.h"

#ifndef _
#define _(String) 	gettext(String)
#endif /* _ */

#ifndef PACKAGE
#define PACKAGE		"axtu-tui"
#endif /* PACKAGE */

#ifndef LOCALEDIR
#define LOCALEDIR       "/usr/share/locale"
#endif /* LOCALEDIR */

#define NULL_DEV 	"/dev/null"
#define MAX_ARGUMENTS	3
#define MIN_ARGUMENTS	2

static int nArgUpdate;
static int nArgInstall;
static int nArgErase;
static int nArgForce;

classTui * tui = NULL;

classConfCtl *istConfCtl = NULL;
classConfigParser *istConfigParser = NULL;
classLogger *istLogger = NULL;
classNetwork *istNetwork = NULL;
classRpmEngine *istRpmEngine = NULL;


static struct poptOption optionsTable[] = {

{ "update", (char) 'u', POPT_ARG_NONE,&nArgUpdate,0,

 _("You can bring up to date pre-installed packages"), NULL },

{ "install", (char) 'i', POPT_ARG_NONE,&nArgInstall,0,

 _("You can setup the Not installed packages"), NULL },

{ "erase", (char) 'e', POPT_ARG_NONE,&nArgErase,0,

 _("You can erase already pre-installed packages"), NULL },

{ "force", (char) 'f', POPT_ARG_NONE,&nArgForce,0,

 _("You can execute by force"), NULL },

POPT_TABLEEND

};

/*!
@brief check execute permission for this program(axtu-tui)

@return int - 0:Success 1:Fail
*/
int check_execute_permission(void)
{
	int nResult=1;
	const uid_t nRootPid = 0;

	if(getuid() != nRootPid){
        	printf(_("You need to be root to run TSN Updater.\n"));
		nResult=FAIL_FLAG;
    	}else{
		nResult=SUCCESS_FLAG;
	}
	
	return nResult;
}

/*!
@brief check parameter for this program using popt

@param argc - count of arguments
@param argv - array of arguments
@return int - 0:Success 1:Fail
*/
int check_parameter(const int argc,const char *argv[])
{
	int nResult = SUCCESS_FLAG;
	int option=0;

	if(argc > MAX_ARGUMENTS){
		fprintf(stderr,_("There are too many arguments.\nUsage : %s --help\n"),argv[0]);
		nResult = FAIL_FLAG;
	}else if(argc < MIN_ARGUMENTS){
		fprintf(stderr,_("There are missing option arguments.\nUsage : %s --help\n"),argv[0]);
		nResult = FAIL_FLAG;
	}else{
		poptContext context = poptGetContext((const char*)argv[0],argc,argv,(const struct poptOption* ) &optionsTable,0);
	
		option=poptGetNextOpt(context);
		if(option != -1){
			poptPrintHelp(context,stdout,POPT_ARG_NONE);
			nResult = FAIL_FLAG;
		}else{
			if( (nArgUpdate+nArgInstall+nArgErase) != 1 ){
				fprintf(stderr,_("There are too many arguments.\nUsage : %s --help\n"),argv[0]);
				nResult = FAIL_FLAG;
			}else{ 
				nResult = SUCCESS_FLAG;
			}
		}
   		poptFreeContext(context);
	}

	return nResult;
}

/*!
@brief check execution-mode of updater

@param nMode - reference of Mode 
@return int - 0:Success 1:Fail
*/
int check_mode(int *nMode)
{
	int nResult=SUCCESS_FLAG;

	if(nArgUpdate){
		*nMode=UPDATE_MODE;
	}else if(nArgInstall){
		*nMode=INSTALL_MODE;
	}else if(nArgErase){
		*nMode=ERASE_MODE;
        }else{
		show_err_msg_on_tui(_("Program execution mode is unknown."),NULL);
		nResult=FAIL_FLAG;
	}

	return nResult;
}

/*!
@brief check double execution of this program

@return int - 0:Success 1:Fail
*/
int check_double_execute(void)
{
	int nResult=SUCCESS_FLAG;

	if(!nArgForce){
		
		if (CHSLock::Islock((AXTU_GUI_PID_FILE), (AXTU_GUI_EXE_FILE)) == true)
		{
			nResult=FAIL_FLAG;
		}
		else if (CHSLock::Islock((AXTU_TUI_PID_FILE), (AXTU_TUI_EXE_FILE)) == true)
		{
			nResult=FAIL_FLAG;
		}
		else if (CHSLock::Islock((AXTU_CUI_PID_FILE), (AXTU_CUI_EXE_FILE)) == true)
		{			
			nResult=FAIL_FLAG;
		}
	}

	if(nResult != SUCCESS_FLAG){				
		show_err_msg_on_tui(_("TSN Updater is already running."),NULL);
	}
	else
	{
		if(CHSLock::lock((AXTU_TUI_PID_FILE)) == false)
		{
			show_err_msg_on_tui(_("Internal Error #2, please contact your support provider, or try again later."),NULL);
		}
	}

	return nResult;
}

/*!
@brief redirect Standard-Error Messages

@param szTarget - Target Name to redirect 
@return int - 0:Success 1:Fail
*/
int redirect_stderr(char *szTarget)
{
	int nResult=SUCCESS_FLAG;
	int fdNULL;

        fdNULL = open(szTarget, O_RDWR);

        if (fdNULL == -1) {
                fprintf(stdout, _("%s: failed to open %s\n"), AXTU_TUI_EXE_FILE,szTarget);
		nResult=FAIL_FLAG;
        }

        if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
                close(fdNULL);
                fprintf(stdout, _("%s: failed to duplicate file descriptor\n"), AXTU_TUI_EXE_FILE);
		nResult=FAIL_FLAG;
        }

        if (fdNULL != STDERR_FILENO){
                close(fdNULL);
	}
	
	return nResult;
}

/*!
@brief register signal handler 

@return int - 0:Success 1:Fail
*/
int reg_signal_handler(void)
{
	int nResult=SUCCESS_FLAG;

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_TUI_EXE_FILE, strerror(errno));
		nResult=FAIL_FLAG;
		goto ret_reg_signal;
        }
        if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
                fprintf(stdout, "%s: %s\n", AXTU_TUI_EXE_FILE, strerror(errno));
		nResult=FAIL_FLAG;
		goto ret_reg_signal;
        }
        if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
                fprintf(stdout, "%s: %s\n", AXTU_TUI_EXE_FILE, strerror(errno));
		nResult=FAIL_FLAG;
		goto ret_reg_signal;
        }
        if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
                fprintf(stdout, "%s: %s\n", AXTU_TUI_EXE_FILE, strerror(errno));
		nResult=FAIL_FLAG;
		goto ret_reg_signal;
        }
        if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
                fprintf(stdout, "%s: %s\n", AXTU_TUI_EXE_FILE, strerror(errno));
		nResult=FAIL_FLAG;
		goto ret_reg_signal;
        }

ret_reg_signal:
	return nResult;
}

/*!
@brief initialization routine

@param argc - count of arguments
@param argv - array of arguments
@param nMode - reference of Mode 
@return int - 0:Success 1:Fail
*/
int init(const int argc,const char *argv[],int *nMode,
	classConfCtl *refConfCtl,classConfigParser *refConfigParser,classLogger **refLogger,
	classNetwork **refNetwork,classRpmEngine **refRpmEngine)
{
	int nResult = SUCCESS_FLAG;

	// Confirmation of user permission
	if(check_execute_permission() != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		return nResult;
	}
	// Verification to update parameter
	if(check_parameter(argc,argv) != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		return nResult;
	}
	
	// Decide mode between update and install
	if(check_mode(nMode) != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		goto ret_init;
	}

	// To confirm duplication of executing updater
	if(check_double_execute() != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		return nResult;
	}

	// Register of signal handler
	if(reg_signal_handler() != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		goto ret_init;
	}

	// Redirect STDERR to /dev/null
	if(redirect_stderr(NULL_DEV) != SUCCESS_FLAG){
		nResult = INTENDED_FAIL_FLAG;
		goto ret_init;
	}

	// Verification of confirmation files
	if(refConfCtl->ConfigCheck() != true){
		nResult = INTENDED_FAIL_FLAG;
		goto ret_init;
	}

	// Read configuration
	if(!access(CONFIG_FILE,R_OK)){
		if(!refConfigParser->Read(CONFIG_FILE)){
			show_err_msg_on_tui(ERR_CANNOT_READ_CONF_FILE,CONFIG_FILE);
			nResult = FAIL_FLAG;
			return nResult;
		}
    string strLogPath; 
    strLogPath = refConfigParser->GetOption("main", "logdir");
    if(strLogPath == "") {	
      show_err_msg_on_tui(ERR_BLANK_LOG_DIR,CONFIG_FILE);
      nResult = FAIL_FLAG;
      return nResult;
    }
	}else{
		show_err_msg_on_tui(NOT_FIND_CONF_FILE_MSG,CONFIG_FILE);
		nResult = FAIL_FLAG;
		return nResult;
        }

	// Generate log object
	*refLogger = new classLogger();
	if(*refLogger == NULL){
		nResult = FAIL_FLAG;
		goto ret_init;
	}	

	// verify the space for log directory is available
	if((*refLogger)->CheckLogDirSpace() == false){
                show_err_msg_on_tui(ERR_FOR_NOSPACE,(char*)((*refLogger)->GetLogPath()).c_str());
		nResult = FAIL_FLAG;
		return nResult;
        }

	// Read Cache directory info in configuration file
	// Check the length of directory name
	*refRpmEngine = new classRpmEngine();
	if(*refRpmEngine == NULL){
                nResult = FAIL_FLAG;
                goto ret_init;
        }

	(*refRpmEngine)->ReadCacheDirInfo();
	if(!(*refRpmEngine)->CheckConfigFileLength())
        {
		show_err_msg_on_tui(ERR_DIR_LENGTH_OVER_MSG,NULL);
		nResult = FAIL_FLAG;
		return nResult;
        }

	// Authentication start
	if(*nMode != ERASE_MODE){
		int nCheckResult = FAIL_FLAG;
		*refNetwork = new classNetwork(TEXT);
		nCheckResult = (*refNetwork)->CheckAuthen();
		if (nCheckResult == FAIL_AUTH){
			show_err_msg_on_tui(AUTH_FAIL_MSG,NULL);
			nResult = FAIL_FLAG;
			return nResult;
		}else if(nCheckResult == CANNOT_EXE_AUTH_PROG){
			show_err_msg_on_tui(CANNOT_EXECUTE_AUTH_MSG,NULL);
			nResult = FAIL_FLAG;
			return nResult;
		}else if(nCheckResult == CANCELED_FROM_AUTH_PROG){
			nResult = INTENDED_FAIL_FLAG;
			return nResult;
		}
	}


	if(nResult != SUCCESS_FLAG){
		show_err_msg_on_tui("AXTU initialization is failed.",NULL);
	}
		
ret_init:
	return nResult;
}

int uninit(classLogger **refLogger,classNetwork **refNetwork,classRpmEngine **refRpmEngine)
{
	if(refLogger != NULL){
		delete *refLogger;
		*refLogger = NULL;
	}
	if(refNetwork != NULL){
		delete *refNetwork;
		*refNetwork = NULL;
	}
	if(refRpmEngine != NULL){
		delete *refRpmEngine;
		*refRpmEngine = NULL;
	}
	
	return SUCCESS_FLAG;
}


void ExitFunc (void)
{		
	if(tui != NULL){
		delete tui;
		tui = NULL;
	}				
	uninit(&istLogger,&istNetwork,&istRpmEngine);				
	if(istConfCtl != NULL){
		delete istConfCtl;
		istConfCtl = NULL;
	}
	if(istConfigParser != NULL){
		delete istConfigParser;
		istConfigParser = NULL;
	}	
}

/*!
@brief main fuction

@param argc - count of arguments
@param argv - array of arguments
*/

int main(const int argc,const char *argv[])
{
	if(atexit(ExitFunc) != 0)
	{
		exit(1);
	}	
	classRedcastle * redcastle = new classRedcastle();
	int nStatus = redcastle->GetRCStatus();
	delete redcastle;

	if(nStatus == RC_STATUS_ENABLE)
	{
		show_err_msg_on_tui(_(REDCASTLE_ENABLE_MSG),NULL);

		classLogger * Logger = new classLogger();
		Logger->WriteLog_char(ERROR_LOG,"TUI",REDCASTLE_ENABLE_MSG, NULL);
		delete Logger;

		newtFinished();
                exit(2);
        }
        else if(nStatus == RC_STATUS_WARNING)
        {
		if(show_warn_msg_on_tui(_(REDCASTLE_WARNING_CONTINUE_MSG)) == false)
		{
			newtFinished();
			exit(2);
		}
	}
	int nMode=0;
	int nReturn = SUCCESS_FLAG;
	int nReturnShowUi = SUCCESS_FLAG;

	istConfCtl = new classConfCtl();
	istConfigParser = new classConfigParser();

	nReturn = init(argc,argv,&nMode,istConfCtl,
		istConfigParser,&istLogger,&istNetwork,&istRpmEngine);
	
	if(nReturn != SUCCESS_FLAG){
		uninit(&istLogger,&istNetwork,&istRpmEngine);
	
	        if(istConfCtl != NULL){
			delete istConfCtl;
			istConfCtl = NULL;
		}
		if(istConfigParser != NULL){
			delete istConfigParser;
			istConfigParser = NULL;
		}

		return nReturn;
	}
	
	// related Locale
	setlocale(LC_ALL,"");
    bindtextdomain(PACKAGE,LOCALEDIR);
    textdomain(PACKAGE);



	tui=new classTui(nMode,istConfCtl,istConfigParser,istLogger,istNetwork,istRpmEngine);
	tui->setRedcastleStatus(nStatus);
	if(tui->InitTui() == SUCCESS_FLAG){
		nReturnShowUi = tui->show_ui();

		if(nReturnShowUi == FAIL_FLAG){
			show_err_msg_on_tui(_("Internal error is occurred. This program will be close. You can check in detail from a log file"),(istLogger->GetLogPath()+(string)"/error.log").c_str());
			nReturn = FAIL_FLAG;
		}
	}

	tui->UninitTui();

	if(tui != NULL){
		delete tui;
		tui = NULL;
	}

	uninit(&istLogger,&istNetwork,&istRpmEngine);

	if(istConfCtl != NULL){
		delete istConfCtl;
		istConfCtl = NULL;
	}
	if(istConfigParser != NULL){
		delete istConfigParser;
		istConfigParser = NULL;
	}
	
	return nReturn;
}
