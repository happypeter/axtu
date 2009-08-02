/*!
@file classAuthen.h
@brief Class header file for an authentication 
*/
#ifndef CLASSAUTHEN_H_
#define CLASSAUTHEN_H_

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <fstream>
#include "classLogger.h"

#define MAX_STRING 512
using namespace std;

//! Path string of a authentication program for Graphic User Interface 
const string g_strAuthPathForGui="/usr/share/axtu-authen-client/bin/axtu-authen-client-gui";

//! Path string of a authentication program for Text User Interface 
const string g_strAuthPathForTui="/usr/share/axtu-authen-client/bin/axtu-authen-client-tui";

//! Path string of a authentication program on Command Line
const string g_strAuthPathForCmd="/usr/share/axtu-authen-client/bin/axtu-authen-client-cui";

//! prefix of a file name to save the temporary key
const string strTkPathPrefix="/var/axtu/tmp/axTK";

//! lengthe of the temporary key 
const unsigned int nTkMaxLength=32;

/*! 
@brief User Interface Mode

The GRAPHIC is a Graphic User Interface. and the TEXT is a Text User Interface
*/
typedef enum{
        GRAPHIC=1,
        TEXT
}EXECUTE_MODE;

typedef enum{
	ERR_NOT_EXIST_AUTH_PROG = -2,
	ERR_WHILE_EXECUTE,
	ERR_GET_TK,		// 0
	OK_GET_TK,		// 1
	CANCELED_FROM_AUTH_PROG
}AUTHEN_RESULT;

/*!
@brief Class for authentication

Maybe need this class to do authentication with the server.
And to do authentication, you must install authentication program.
*/
class classAuthen{
public:
	classAuthen(EXECUTE_MODE eMode);
	~classAuthen();
	AUTHEN_RESULT CheckAuthen();
	bool GetAK(string &strAk);
	bool GetTK(string &strTk);
	void SetDontUseUI(void);
        void SetDontGetTk(void);
	bool GetDontGetTk(void);

private:
	bool CheckProgAuth(string strAuthPath);
	void MakeFileName();
	bool SetTk(void);
	void ParseTkTail(void);
	string RemoveSpace(string  strSrc);
	int RunAuthApp(bool bCommand,bool bDontGetTkFlag);
	
	string m_strSavingFileWTk;
	string m_strTk;
	string m_strTkTail;
	bool m_bDontUseUIFlag;
    	bool m_bDontGetTkFlag;
	EXECUTE_MODE m_ExecuteMode;

	//! Instance of classLogger
	classLogger *m_Logger;
};
#endif /*CLASSAUTHEN_H_*/
