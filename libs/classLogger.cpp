/*!
@file classLogger.cpp
@brief class source file for recording anything in file
*/
#include "classLogger.h"
#include <syslog.h>
#include <sstream>
#include "message.h"
#include "trace.h"

//! A Constructor
classLogger::classLogger(void)
{
	m_ConfigParser = new classConfigParser();
	SetFullPath();
}

//! A destructor
classLogger::~classLogger()
{
	delete m_ConfigParser;
}

/*!
@brief make full path for logging

It read a log directory path from configure file 
And then concatenate a log directory and a log file name.
*/
void classLogger::SetFullPath(void)
{
	m_strLogDir=GetLogPath();

	m_strLogDir=m_ConfigParser->StripRString(m_strLogDir,'/');

	m_strSuccessLogFilePath.assign(m_strLogDir + "/" + SUCCESS_LOG_FILE);
	m_strErrorLogFilePath.assign(m_strLogDir + "/" + ERROR_LOG_FILE);
	m_strDebugFilePath.assign(m_strLogDir + "/" + DEBUG_LOG_FILE);
}
/*
void classLogger::Log(const char* msg,bool bShowConsol)
{
	Open(DEBUG_LOG);
	Write(msg,bShowConsol);
	Close();
}
*/

/*!
@brief Open a log file

@param nFlag a log classify(error or success or debug)
@return bool - Open? or Not?
*/
bool classLogger::Open(int nFlag)
{
	switch(nFlag){
		case SUCCESS_LOG:
			m_ConfigParser->_mkdir(m_strLogDir.c_str());
			m_fout.open(m_strSuccessLogFilePath.c_str(),ios::app);
			break;
		case ERROR_LOG:
			m_ConfigParser->_mkdir(m_strLogDir.c_str());
			m_fout.open(m_strErrorLogFilePath.c_str(),ios::app);
			break;
		case DEBUG_LOG:
			m_ConfigParser->_mkdir(m_strLogDir.c_str());
			m_fout.open(m_strDebugFilePath.c_str(),ios::app);
			break;
		default:
			return false;
	}	

	if (m_fout.is_open()) return true;

	return false;
}

/*!
@brief write a log file

@param szMsg - message string to record in file
@param bShowConsole - if bShowConsole is ture, the message is shown on console
*/
int classLogger::Write(const char *szMsg, bool bShowConsol)
{
	char szLogMsg[MAX_STRING]={0};
	char szDate[18]={0};
	int nSize=0;

	snprintf(szDate, sizeof(szDate), "%s", GetTimeStamp());
	snprintf(szLogMsg, MAX_STRING, "%s", szMsg);
	nSize = strlen(szLogMsg);

	if (szLogMsg[nSize-1] == '\n')
		szLogMsg[nSize-1] = '\0';

	m_fout << szDate << " " << szLogMsg << endl;
	
	if (bShowConsol == true){
		cout << szLogMsg << endl;
	}
	
	return 0;
}

/*! 
@brief read a log directory path from configure file.

@return log directory path
*/
string classLogger::GetLogPath(void)
{
	string strLogPath;

	if (m_ConfigParser->Read(CONFIG_FILE) == false)
		return DEFAULT_LOG_PATH;

	strLogPath=m_ConfigParser->GetOption("main","logdir");

	if (strLogPath.length() == 0)
		return DEFAULT_LOG_PATH;

	if (access(strLogPath.c_str(), F_OK) == 0) 
		return strLogPath;

	if (m_ConfigParser->_mkdir(strLogPath.c_str()))
		return strLogPath;

	return DEFAULT_LOG_PATH;
}

/*!
@brief get current time 

@return char* - current time string
*/
char* classLogger::GetTimeStamp(void)
{
	time_t curtime;
        struct tm *loctime;
        static char datedt[22];

        curtime=time(NULL);
        loctime=localtime(&curtime);

        strftime(datedt,22,"%m/%d/%y %T",loctime);

        return datedt;
}

//! close a file
void classLogger::Close()
{
	m_fout.close();
}	

/*!
@brief record the user log(rpm log)

rpm related logs(install, erase, ...) are written according to a fixed form.
@param nAction - rpm action number (ex. install, erase, update ...)
@param szPkgName - package name that occured rpm action
*/
void classLogger::RpmLogging(int nAction,const char* szPkgName)
{	
	// argument check
	if(szPkgName == NULL) {
		return;
	}

	switch(nAction){
		case UPDATE_ACTION:
			WriteLog_char(SUCCESS_LOG,STR_UPDATE_ACTION,szPkgName,NULL);
			break;
		case INSTALL_ACTION:
			WriteLog_char(SUCCESS_LOG,STR_INSTALL_ACTION,szPkgName,NULL);
			break;
		case ERASE_ACTION:
			WriteLog_char(SUCCESS_LOG,STR_ERASE_ACTION,szPkgName,NULL);
			break;
		case DEP_UPDATE_ACTION:
			WriteLog_char(SUCCESS_LOG,STR_DEP_UPDATE_ACTION,szPkgName,NULL);
			break;
		case DEP_INSTALL_ACTION:		    
			WriteLog_char(SUCCESS_LOG,STR_DEP_INSTALL_ACTION,szPkgName,NULL);
			break;
		case DEP_ERASE_ACTION:			
			WriteLog_char(SUCCESS_LOG,STR_DEP_ERASE_ACTION,szPkgName,NULL);
			break;
		default:
			{
				char sztmp[128];
				snprintf(sztmp, sizeof(sztmp), "unknown RPM action(%d)", nAction);
				WriteLog_char(ERROR_LOG, sztmp, szPkgName, NULL);
			}
			return;
	}	
}

bool classLogger::check_strlen(char *szStr,int nMax)
{
	if (strlen(szStr) >= nMax)
		return false;
	else
		return true;
}

//void classLogger::DebugLog_char(const char *szMsg,const char *szSubMsg, ...)

/*!
@brief record the log

This method is able to record any log(string oriented)
@param nFlag - UserLog? or DebugLog?
@param szSubMsg - any log of string type
*/
void classLogger::WriteLog_char(const int nFlag,const char *szSubMsg, ...)
{	
	va_list ap;
	char *s;

	char szBuffer[MAX_STRING]={0};	
	
	if(szSubMsg != NULL){
		va_start(ap,szSubMsg);

		snprintf(szBuffer, MAX_STRING, "%s", szSubMsg);
		    
		while((s=va_arg(ap,char*))){
			if(!check_strlen(szBuffer,MAX_STRING)){
				break;
			}
			sprintf(szBuffer,"%s %s",szBuffer,s);		
		}
		va_end(ap);
	}

	if(!Open(nFlag)){
		return;
	}
	Write(szBuffer);
	Close();
}

//void classLogger::DebugLog_int(const char *szMsg,int nArg, ...)

/*!
@brief record the log

This method is able to record any log(integer oriented)
@param nFlag - UserLog? or DebugLog?
@param szMsg - description string about "nArg"
@param nArg - any log of integer type
*/
void classLogger::WriteLog_int(const int nFlag,const char *szMsg,int nArg, ...)
{
	va_list ap;
	int d;

	char szBuffer[MAX_STRING]={0};
	
	snprintf(szBuffer, MAX_STRING, "%s", szMsg);

	if(&nArg != NULL){
		va_start(ap,nArg);
	
		sprintf(szBuffer,"%s %d",szBuffer,nArg);
	
		while((d=va_arg(ap,int))){
			if(!check_strlen(szBuffer,MAX_STRING)){
				break;
			}
			sprintf(szBuffer,"%s %d",szBuffer,d);
		}
	
		va_end(ap);
	}

	if(!Open(nFlag)){
		return;
	}
	Write(szBuffer);
	Close();
}

/*! 
@brief get file(the file is recorded Error message)'s path

@return string - ErrorLog file's path
*/
string classLogger::GetErrorFilePath(void)
{
	return m_strErrorLogFilePath;
}

/*! 
@brief get file(the file is recorded Debug message)'s path

@return string - DebugLog file's path
*/
string classLogger::GetDebugFilePath(void)
{
	return m_strDebugFilePath;
}

/*! 
@brief get file(the file is recorded Rpm action message)'s path

@return string - RpmLog file's path
*/
string classLogger::GetSuccessFilePath(void)
{
	return m_strSuccessLogFilePath;
}


/*! 
@brief Check available log dir space

@return bool - true : wriete log available   false : or not  
*/
bool classLogger::CheckLogDirSpace()
{	
	long lSize;
	struct statfs lstatfs;	

	if(access(m_strLogDir.c_str(), F_OK) == 0)
	{
		statfs(m_strLogDir.c_str(), &lstatfs);
		lSize = lstatfs.f_bavail * (lstatfs.f_bsize/KILO_BYTE);
		//printf("Size = %ld\n", lSize);
	}
	else
	{
		return false;
	}
	//printf("Size ::::::::::::: %ld\n", MP->size.avail);
	if (lSize > MAX_LOG_SIZE/*100k*/)
	{
		return true;
	}
	else
	{
		return false;
	}	
}

void classLogger::WriteLog_char_with_linenum(string &errmsg, int linenum, char *msg)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  stringstream oss;
  string s = " line: ";

  errmsg = msg + s;
  oss << linenum;
  errmsg += oss.str();
  // cerr << errmsg << endl;
  printf("%s\n", errmsg.c_str());

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif
}

void classLogger::WriteLog_syslog(string &msg)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  openlog(NULL, LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_ERR, "%s", msg.c_str());
  closelog();

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif
}
