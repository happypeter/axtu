#include "axtu_setup.h"
#include "classLock.h"
#include "commondef.h"

#define LOCALEDIR	"/usr/share/locale"



int main()
{
	if (getuid() != 0 )
	{
		printf("You need to be root to run TSN Updater setup tool.\n");
		return 1;
	}
	setlocale(LC_ALL,"");
	bindtextdomain("axtu-setup-tui",LOCALEDIR);
	textdomain("axtu-setup-tui");

	bool bAlreadyExecuted = false;
	if(CHSLock::Islock((AXTU_SETUP_PID_FILE), (AXTU_SETUP_EXE_FILE)) == true){
		bAlreadyExecuted=true;		
	}		
	else if(CHSLock::Islock((AXTU_SETUP_TUI_PID_FILE), (AXTU_SETUP_TUI_EXE_FILE)) == true){
		bAlreadyExecuted=true;		
	}
	
	if (bAlreadyExecuted)
	{
		newtInit();
		newtCls();
		newtWinMessage(ERROR_MSG,BTN_OK_MSG,"Updater setup tool is already running.");
		newtFinished();			
		return 2;
	}
	else
	{
		if(CHSLock::lock((AXTU_SETUP_TUI_PID_FILE)) == false)
		{
			newtInit();
			newtCls();
			newtWinMessage(ERROR_MSG,BTN_OK_MSG,"Internal Error #2, please contact your support provider, or try again later.");
			newtFinished();			
			return 2;
		}
	}
			
	
	
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_TUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_TUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_TUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_TUI_EXE_FILE, strerror(errno));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", AXTU_SETUP_TUI_EXE_FILE, strerror(errno));
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {
		fprintf(stdout, "%s: failed to open /dev/null\n", AXTU_SETUP_TUI_EXE_FILE);
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);
		fprintf(stdout, "%s: failed to duplicate file descriptor\n", AXTU_SETUP_TUI_EXE_FILE);
		exit(1);
	}

	if (fdNULL != STDERR_FILENO)
		close(fdNULL);

	classBlacklist *Blacklist=new classBlacklist();
	
	Blacklist->popup_BListSetup();

	delete Blacklist;
}
