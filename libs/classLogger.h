/*!
@file classLogger.h
@brief class header file for recording anything in file
*/
#ifndef CLASSLOGGER_H_
#define CLASSLOGGER_H_

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <stdarg.h>
#include "hsCommon.h"
#include "classConfigParser.h"
#include    <sys/stat.h>
#include    <sys/vfs.h>

//! rpm action string to record in file - at update
#define STR_UPDATE_ACTION	"Updated"
//! rpm action string to record in file - at install
#define STR_INSTALL_ACTION	"Installed"
//! rpm action string to record in file - at erase
#define STR_ERASE_ACTION	"Erased"
//! rpm action string to record in file - at update by dependency
#define STR_DEP_UPDATE_ACTION	"Dep_Updated"
//! rpm action string to record in file - at install by dependency
#define STR_DEP_INSTALL_ACTION	"Dep_Installed"
//! rpm action string to record in file - at erase by dependency
#define STR_DEP_ERASE_ACTION	"Dep_Erased"

#define KILO_BYTE 1024
#define MAX_LOG_SIZE 1000 /*1000k*/

#define DEFAULT_LOG_PATH "/var/log"

using namespace std;

/*!
@brief Distinguishable flag the log classify
*/
enum {
        SUCCESS_LOG=1,
        ERROR_LOG,
        DEBUG_LOG
};

/*!
@brief rpm action flag
*/
enum {
        UPDATE_ACTION=1,
        INSTALL_ACTION,
        ERASE_ACTION,	
	DEP_UPDATE_ACTION,
	DEP_INSTALL_ACTION,
	DEP_ERASE_ACTION
};


/*!
@brief Class for Logging

All of the logs record in file.
The log is seperatable. The first is User Log, and second is Program Log(Debug Log).
User Log is related the RPM actions(update, install, erase,...).
The other is able to trace the program.
*/
class classLogger
{

public:
	classLogger(void);
	virtual ~classLogger(void);
//	void Log(const char* msg,bool bShowConsol);
	void RpmLogging(int nAction,const char* szPkgName);
//	void RpmLogging(int nAction,const char* szPkgName,int nFlag);
	void WriteLog_char(const int nFlag,const char *szSubMsg, ...);
	void WriteLog_int(const int nFlag,const char *szMsg,int nArg, ...);
//	void DebugLog_char(const char *szMsg,const char *szSubMsg, ...);
//	void DebugLog_int(const char *szMsg,int nArg, ...);
	string GetErrorFilePath(void);
	string GetDebugFilePath(void);
	string GetSuccessFilePath(void);
	string GetLogPath(void);
	bool CheckLogDirSpace();
	bool check_strlen(char *szStr,int nMax);
  void WriteLog_char_with_linenum(string &errmsg, int linenum, char *msg);
  void WriteLog_syslog(string &msg);
	
private:
	void SetFullPath(void);
	bool Open(int nFlag);
	void Close();
	int Write(const char * strMsg, bool bShowConsol=false);
	char* GetTimeStamp(void);
	
	string m_strLogDir;
	string m_strSuccessLogFilePath;
	string m_strErrorLogFilePath;
	string m_strDebugFilePath;
	ofstream m_fout;

	//! Instance of classConfigParser
	classConfigParser *m_ConfigParser;
};

#endif /*CLASSLOGGER_H_*/
