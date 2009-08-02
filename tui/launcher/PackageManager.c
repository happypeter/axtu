/*!
@file PackageManager.c
@brief Launcher on TUI source file
*/
#include "PackageManager.h"

#define LOCALEDIR       "/usr/share/locale"

#define LAUNCHER_TUI_PID_FILE	"/var/tmp/axtu-launcher-tui.pid"
#define LAUNCHER_TUI_EXE_FILE	"axtu-launcher-tui"

/*!
@brief Using popt library Get Option

Popt library do parsing command line options.
The library is included in rpm package.
This function get options for excution mode (ex. update, install, erase)
using popt library
@see popt
@param argc - Arguument count
@param argv - Array of Arguments
*/
void get_option(int argc,char **argv)
{
	int option = 0;

        if(argc > 2 ){
                fprintf(stderr,"%s\nUsage : %s --help\n",	\
					ERROR_MSG_MANY_ARG,argv[0]);
                exit(-1);
        }

        poptContext context = poptGetContext((const char*)argv[0],argc,argv, \
				(const struct poptOption* ) &optionsTable,0);

        option=poptGetNextOpt(context);

        if(option != -1){
                poptPrintHelp(context,stdout,POPT_ARG_NONE);
        	poptFreeContext(context);
		exit(-1);
        }

        if( (nArgUpdate+nArgInstall+nArgErase+nArgSetup) > 1 ){
                fprintf(stderr,"%s\nUsage : %s --help\n",	\
					ERROR_MSG_MANY_ARG,argv[0]);
        	poptFreeContext(context);
                exit(-1);
        }

        poptFreeContext(context);
}

/*!
@brief ShareMemory Check

@return int - get a string in share memory? or Not? : 1 or 0
*/
int shm_check(void)
{
	void *shared_memory = (void *)0;
        struct shared_use_st *shared_stuff;
        int shmid;
	int nRet;

        shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

        if (shmid == -1) {
        	fprintf(stderr, "shmget failed\n");
        	exit(EXIT_FAILURE);
	}

        // make the shared memory accessible to the program
        shared_memory = shmat(shmid, (void *)0, 0);
        if (shared_memory == (void *)-1) {
       		fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
        }

        shared_stuff = (struct shared_use_st *)shared_memory;

#ifdef NDEBUG
        char buffer[1000]={0};
        sprintf(buffer,"echo %s %s >> debug_msg","ShareMemory Contents",shared_stuff->some_text);
        system(buffer);
#endif	/* NDEBUG */

	if(!strncmp(shared_stuff->some_text,SHM_TAG_UPDATE,strlen(SHM_TAG_UPDATE))){
		nArgUpdate=1;
		memset(shared_stuff->some_text,0,strlen(SHM_TAG_UPDATE));
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_INSTALL,strlen(SHM_TAG_INSTALL))){
		nArgInstall=1;
		memset(shared_stuff->some_text,0,strlen(SHM_TAG_INSTALL));
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERASE,strlen(SHM_TAG_ERASE))){
		nArgErase=1;
		memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERASE));
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_WRONG_URL,strlen(SHM_TAG_WRONG_URL))){
		nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_WRONG_URL));
		newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_WRONG_URL);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_NOT_CONNECT,strlen(SHM_TAG_NOT_CONNECT))){
		nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_NOT_CONNECT));
		newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_NOT_CONNECT);
	}else if(!strncmp(shared_stuff->some_text,ERROR_MSG_AUTH_FAIL,strlen(ERROR_MSG_AUTH_FAIL))){
		nError=1;
                memset(shared_stuff->some_text,0,strlen(ERROR_MSG_AUTH_FAIL));
		newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_AUTH_FAIL);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_UNKNOWN,strlen(SHM_TAG_UNKNOWN))){
		nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_UNKNOWN));
		newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_UNKNOWN);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERR_CONFIG,strlen(SHM_TAG_ERR_CONFIG))){
		nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERR_CONFIG));
		newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_CONFIG_FILE);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERR_NOSPACE,strlen(SHM_TAG_ERR_NOSPACE))){
                nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERR_NOSPACE));
                newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_NOSPACE);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERR_CANNOT_EXE_AUTH,strlen(SHM_TAG_ERR_CANNOT_EXE_AUTH))){
                nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERR_CANNOT_EXE_AUTH));
                newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_CANNOT_EXE_AUTH);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERR_DIR_LENGTH_OVER,strlen(SHM_TAG_ERR_DIR_LENGTH_OVER))){
                nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERR_DIR_LENGTH_OVER));
                newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_DIR_LENGTH_OVER);
	}else if(!strncmp(shared_stuff->some_text,SHM_TAG_ERR_CONF_LENGTH_OVER,strlen(SHM_TAG_ERR_CONF_LENGTH_OVER))){
                nError=1;
                memset(shared_stuff->some_text,0,strlen(SHM_TAG_ERR_CONF_LENGTH_OVER));
                newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_CONF_LENGTH_OVER);
        }



	// shared memory is detached
	if (shmdt(shared_memory) == -1) {
	        fprintf(stderr, "shmdt failed\n");
       	        exit(EXIT_FAILURE);
	}

/*
	if(shmctl(shmid,IPC_RMID,0) == -1){
		fprintf(stderr,"shmctl failed\n");
                exit(EXIT_FAILURE);
        }
*/			
	if(nArgUpdate || nArgInstall || nArgErase){
		nRet = 1;
	}else{
		nRet = 0;
	}
	
	return nRet;
}

/*!
@brief unloading newt

@param argForm - Instance of the newtForm component
*/
void newt_unloading(newtComponent argForm)
{
	newtPopHelpLine();
  	newtPopWindow();
  	newtFormDestroy(argForm);
	newtFinished();
}

int main(int argc,char **argv)
{	
	if (getuid() != 0 )
	{
		printf("You need to be root to run TSN Updater launcher.\n");
		return 1;
	}

	newtComponent form;
	newtComponent listbox_Menu;
	newtComponent btn_Next,btn_Cancel;


	int nExitCode = -1;
	int nCheckValue;
	int nRet;

	// related locale
	setlocale(LC_ALL,"");
        bindtextdomain("axtu-launcher-tui",LOCALEDIR);
        textdomain("axtu-launcher-tui");
	
	struct newtExitStruct es;
	get_option(argc,argv);

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MY_NAME, strerror(errno));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MY_NAME, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MY_NAME, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MY_NAME, strerror(errno));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MY_NAME, strerror(errno));
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {
		fprintf(stdout, "%s: failed to open /dev/null\n", MY_NAME);
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);
		fprintf(stdout, "%s: failed to duplicate file descriptor\n", MY_NAME);
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);

	nError = 0;
	newtInit();
	newtCls();

  	newtDrawRootText(0,0,TITLE);

  	newtPushHelpLine(HELP_LINE);
  
	newtCenteredWindow(60,12,MAIN_WIN_TITLE);

	form=newtForm(NULL,NULL,NEWT_FLAG_RETURNEXIT | NEWT_FLAG_NOF12);

	btn_Next=newtButton(15,8,BTN_NEXT_MSG);
	btn_Cancel=newtButton(35,8,BTN_CANCEL_MSG);

	listbox_Menu = newtListbox(2,3,4,NEWT_FLAG_RETURNEXIT);

	newtListboxAppendEntry(listbox_Menu,BTN_UPDATE_MSG,(void*)1);
	newtListboxAppendEntry(listbox_Menu,BTN_INSTALL_MSG,(void*)2);
	//Remove the menu of erase on axtu launcher.  Because some user can eraser important package and can destroy system.
	//newtListboxAppendEntry(listbox_Menu,BTN_ERASE_MSG,(void*)3);
	newtListboxAppendEntry(listbox_Menu,BTN_SETUP_MSG,(void*)4);

  	newtFormAddComponents(form,listbox_Menu,btn_Next,btn_Cancel,NULL);

	if(nArgUpdate || nArgInstall || nArgErase || nArgSetup){
		newtFormSetTimer(form,200);
	}

	do{
    		newtFormRun(form,&es);
		if(es.reason == NEWT_EXIT_COMPONENT && es.u.co != btn_Cancel){
      			nSelect=(int)newtListboxGetCurrent(listbox_Menu);
			newtSuspend();
			nRet = -1;
			switch(nSelect){
				case 1:
					nRet = system(UPDATE_CMD);
					break;
				case 2:
					nRet = system(INSTALL_CMD);
					break;
				case 3:
					nRet = system(ERASE_CMD);
					break;
				case 4:
					nRet = system(SETUP_CMD);
					break;
			}
			newtResume();

			if (nRet == -1 || WEXITSTATUS(nRet) == 127) {
				newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_CANNOT_EXE);
			}

			nCheckValue=0;
			nCheckValue=shm_check();
			
			if(nCheckValue==0 && nError){
				nError=0;
			}else if(nCheckValue && nArgUpdate){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,UPDATE_OPTION,NULL);
			}else if(nCheckValue && nArgInstall){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,INSTALL_OPTION,NULL);
			}else if(nCheckValue && nArgErase){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,ERASE_OPTION,NULL);
			}
		}else if(nArgUpdate || nArgInstall || nArgErase || nArgSetup){
			newtSuspend();
			nRet = -1;
			if(nArgUpdate){
				nRet = system(UPDATE_CMD);
				nArgUpdate=0;
			}else if(nArgInstall){
				nRet = system(INSTALL_CMD);
				nArgInstall=0;
			}else if(nArgErase){
				nRet = system(ERASE_CMD);
				nArgErase=0;
			}else{
				nRet = system(SETUP_CMD);
				nArgSetup=0;
			}
			
			newtResume();

			if (nRet == -1 || WEXITSTATUS(nRet) == 127) {
				newtWinMessage(ERROR_WIN_TITLE,BTN_OK_MSG,ERROR_MSG_CANNOT_EXE);
			}

			nCheckValue=0;
			nCheckValue=shm_check();

			if(nCheckValue && nArgUpdate){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,UPDATE_OPTION,NULL);
			}else if(nCheckValue && nArgInstall){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,INSTALL_OPTION,NULL);
			}else if(nCheckValue && nArgErase){
				newt_unloading(form);
				execl(MY_PATH,MY_NAME,ERASE_OPTION,NULL);
			}
		}else if(es.reason == NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
 			nExitCode=PAGE_EXIT;
		}
	}while(nExitCode < 0);

	newt_unloading(form);
	return 0;
}
