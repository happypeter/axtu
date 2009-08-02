/*!
 * @file axtu-daemon.cpp
 * @brief This daemon works for axtu notifier.
*/
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <classRpmEngine.h>
#include <classNetwork.h>
#include <classLogger.h>
#include <hsCommon.h>
#include "classLock.h"
#include "classConfCtl.h"
#include "commondef.h"

#define HOUR 3600
#define MYSELF_NAME_DAEMON "axtu-daemon"
#define URL_CONFIRM_MSG		_("Check update-server address of your configure file")
#define CONNECT_ERR_MSG		_("Required data were not found on the server.")
#define URL_ERR_MSG			CONNECT_ERR_MSG
#define CONNECT_CONFIRM_MSG	_("Check your network state.")
#define AUTH_ERR_MSG		_("Cannot authenticate from Server.")
#define USERFWRITE_ERR_MSG  _("Can NOT write the file. Please, Check your disk space.")
#define UNKNOWN_ERR_MSG		_("Unknown error is occured.")



static  classConfigParser config;

/*!
 * @brief GetCheckPriod
 * @return second
 * 
 * Get the priod time from configure file.
*/
static int GetCheckPriod()
{
	int nHour = atoi(config.GetOption("main", "alarm_period").c_str());
	
	if (nHour < 1)
		nHour = 1;
	else if (nHour > 72)
		nHour = 72;
	
	return (nHour * HOUR);
}

/*!
 * @brief Check update status.
 * @return Available update packages count.
 * 
 * Return Available update packages count.
 */
static int CheckUpdate()
{
	char strErr[MAX_STRING];
	char strStr[MAX_STRING];
	classLogger m_Logger;
	
	classNetwork * m_Network = NULL;
	m_Network = new classNetwork(TEXT);
	m_Network->SetDontUseUI();
	m_Network->SetDontGetTk();
	if(m_Network->CheckAuthen(false) != SUCC_AUTH)
	{
		return -1;  //Not authentication.
	}

	classConfCtl * ConfCtl=new classConfCtl();
	ConfCtl->ConfigCheck();	
	delete ConfCtl;
	
	if(m_Network->SetDownloadConfig() == false)
	{		
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild to set download config file.", NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
			
		}
		return -1; 
	}
	
	if (access(CONFIG_FILE, F_OK) != 0)
	{
		snprintf(strErr, sizeof(strErr), "%s file is not found. %s will operates abnormally", CONFIG_FILE, MYSELF_NAME_DAEMON);
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, (const char *)strErr, NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		return -1;  //Not authentication.
	}
	// Log dir have not enough space(Under 100k available space)
	if( m_Logger.CheckLogDirSpace() == false)
	{
		snprintf(strErr, sizeof(strErr), "You don't have enough space for log directory(%s).", m_Logger.GetLogPath().c_str());
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, strErr, NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		return -1;  //Not authentication.
	}
	int nHeaderRet = m_Network->GetHeader();
	if(nHeaderRet != NETWORK_RETOK)
	{
		switch (nHeaderRet) {
		case NETWORK_ERR_WRONG_URL:
			snprintf(strErr, sizeof(strErr), "%s", URL_ERR_MSG);
			break;
		case NETWORK_ERR_CONNECT:
			snprintf(strErr, sizeof(strErr), "%s", CONNECT_ERR_MSG);
			break;
		case NETWORK_ERR_AUTH_FAIL:
			snprintf(strErr, sizeof(strErr), "%s", AUTH_ERR_MSG);
			break;
		case NETWORK_ERR_FWRITE:
			snprintf(strErr, sizeof(strErr), "%s", USERFWRITE_ERR_MSG);
			break;
		default:
			snprintf(strErr, sizeof(strErr), "%s", UNKNOWN_ERR_MSG);
			break;
		}
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, strErr, NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		return nHeaderRet;
	}

	classRpmEngine *m_rpmEngine = NULL;
	m_rpmEngine = new classRpmEngine();
	
	if( m_rpmEngine->ReadRemoteHeaderInfo() != 0)
	{
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild :ReadRemoteHeaderInfo()", NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		if(m_rpmEngine)
		{
			delete m_rpmEngine;
			m_rpmEngine=NULL;
		}
		return -1;  //Not authentication.
	}
	
	if(m_rpmEngine->ReadLocalHeaderInfo() != 0)
	{
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild :ReadLocalHeaderInfo()", NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		if(m_rpmEngine)
		{
			delete m_rpmEngine;
			m_rpmEngine=NULL;
		}
		return -1;  //Not authentication.
	}
	if(m_rpmEngine->CreateUpdateInstallList() == false)
	{
		m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild :CreateUpdateInstallList()", NULL);
		if(m_Network)
		{
			delete m_Network;
			m_Network=NULL;
		}
		if(m_rpmEngine)
		{
			delete m_rpmEngine;
			m_rpmEngine=NULL;
		}
		return -1;  //Not authentication.
	}
	
	/*
	vector <structFileInfo> vectorFileInfo;
	vectorFileInfo.clear();
	vectorFileInfo = m_rpmEngine->GetUpdateList();
	int nRet = 0;
	if (config.GetOption("main", "apply_blacklist") == "false")
	{
		nRet = vectorFileInfo.size();
	}
	else
	{
		vector<structFileInfo>::iterator it;
		for(it=vectorFileInfo.begin();it!=vectorFileInfo.end();it++)
		{
			if (it->bBlacklisted == false)
			{
				nRet++;
			}
		}
	}
	*/
	
	int nRet = m_rpmEngine->GetUpdateAvailableCount();
	snprintf(strStr, sizeof(strStr), "%s [Update Count : %d]", "Check update",nRet);	
	m_Logger.WriteLog_char(DEBUG_LOG, MYSELF_NAME_DAEMON, strStr , NULL);
	
	if(m_rpmEngine)
	{
		delete m_rpmEngine;
		m_rpmEngine=NULL;
	}
	if(m_Network)
	{
		delete m_Network;
		m_Network=NULL;
	}
	return nRet;
}

static void DoIt()
{
	classLogger m_Logger;
	if (config.GetOption("main", "alarm") == "true" )
	{
		int nUpdateCount = CheckUpdate();
		if( classRpmEngine::WriteNotifierInfo(nUpdateCount) == false)
		{
			m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild to load the WriteNotifierInfo() function." , NULL);
		}
	}
	else
	{
		if(classRpmEngine::WriteNotifierInfo(-99) == false)
		{
			m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Faild to load the WriteNotifierInfo() function." , NULL);
		}
	}
}


/*!
 * @brief sig_ChangeStatus
 * @return void
 * 
 * This function will be called when other application calls  SIGUSR1 signal.
*/
static void  sig_ChangeStatus(int signo)
{
	if(signo == SIGUSR1)
	{
		DoIt();
	}
}

int main()
{

	pid_t   pid;
	classLogger m_Logger;
	if (getuid() != 0) {
		fprintf(stdout, "%s: You need to be root to run TSN Updater daemon.\n"
			, MYSELF_NAME_DAEMON);
		exit(2);
	}
	
	if (( pid = fork()) < 0)
	{
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}
	// kill process.
	else if(pid != 0)
	{
		exit(0);
	}
	setsid();
	chdir("/");
	umask(0);
	
	
	if (CHSLock::Islock((AXTU_DAEMON_PID_FILE), (AXTU_DAEMON_EXE_FILE)) == true){
		printf("Notifier Daemon is already running.\n");
		exit(2);
	}
	else
	{	
		if(CHSLock::lock((AXTU_DAEMON_PID_FILE)) == false)
		{	
			exit(2);
		}
	}

	
	if (signal(SIGUSR1, sig_ChangeStatus) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_DAEMON, strerror(errno));
		exit(1);
	}

	int fdNULL;
	fdNULL = open("/dev/null", O_RDWR);
	if (fdNULL == -1) {
		fprintf(stdout, "%s: failed to open /dev/null\n", MYSELF_NAME_DAEMON);
		exit(1);
	}

	if (dup2(fdNULL, STDERR_FILENO) != STDERR_FILENO) {
		close(fdNULL);
		fprintf(stdout, "%s: failed to duplicate file descriptor\n", MYSELF_NAME_DAEMON);
		exit(1);
	}
	if (dup2(fdNULL, STDOUT_FILENO) != STDOUT_FILENO) {
		close(fdNULL);
		fprintf(stdout, "%s: failed to duplicate file descriptor\n", MYSELF_NAME_DAEMON);
		exit(1);
	}

	if (fdNULL != STDOUT_FILENO && fdNULL != STDERR_FILENO)
		close(fdNULL);

	classConfCtl * ConfCtl=new classConfCtl();	
	ConfCtl->ConfigCheck();
	delete ConfCtl;
	
	while(1)
	{
		if( config.Read(CONFIG_FILE) == false)
		{				
			m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_DAEMON, "Can not read CONFIG_FILE" , NULL);		
		}
		DoIt();
		int nCheckPriod = GetCheckPriod();

		sleep(nCheckPriod);
	}

	
	return 0;
}

