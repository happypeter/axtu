/*!
@file classAuthen.cpp
@brief Class source file for an authentication 
*/
#include "classAuthen.h"

//! A constructor
classAuthen::classAuthen(EXECUTE_MODE eMode)
{
	m_Logger = new classLogger();
	m_bDontUseUIFlag=false;
	m_bDontGetTkFlag=false;
	m_ExecuteMode=eMode;
}
//! A destructor
classAuthen::~classAuthen(void)
{	
	delete m_Logger;
}

void classAuthen::SetDontUseUI(void)
{
	m_bDontUseUIFlag=true;
}

void classAuthen::SetDontGetTk(void)
{
	m_bDontGetTkFlag=true;
}

bool classAuthen::GetDontGetTk()
{
        return m_bDontGetTkFlag;
}


/*!
@brief check a authentication from the server

First, it check whether the system have Authentication program or not. 
if exist the authentication program,this method will excute that.
And then, it takes a temporary key that get by a authentication program.
@return bool - authentication is Ok? or Not?
@see CheckProgAuth() SetTk()
*/
AUTHEN_RESULT classAuthen::CheckAuthen()
{
	bool bRet;
	AUTHEN_RESULT nIsSuccess=OK_GET_TK;
	int nResult=0;
	string strCmd;

	MakeFileName();

	nResult = RunAuthApp(m_bDontUseUIFlag,m_bDontGetTkFlag);	

	if((nResult < 0) || (nResult == 1)){
		unlink(m_strSavingFileWTk.c_str());
		m_Logger->WriteLog_char(DEBUG_LOG, "Cannot execute authentication program", NULL);

		nIsSuccess=ERR_WHILE_EXECUTE;
	}else if(nResult == 2){
		// In case of Canceling from user
		nIsSuccess=CANCELED_FROM_AUTH_PROG;
	}else{
		if(!m_bDontGetTkFlag){
			bRet=SetTk();
			if(bRet){
				nIsSuccess=OK_GET_TK;
			}else{
				m_Logger->WriteLog_char(DEBUG_LOG,"Cannot get TK",NULL);
				nIsSuccess=ERR_GET_TK;
			}
		}else{
			nIsSuccess=OK_GET_TK;
		}
	}
	
	unlink(m_strSavingFileWTk.c_str());

	return nIsSuccess;
}

int classAuthen::RunAuthApp(bool bCommand,bool bDontGetTkFlag)
{
	EXECUTE_MODE rMode;
	string strCmdForAddr;
	string strCmdForTk;
	string strCmdPrefix;

	strCmdForAddr = (string)" -a " + (string)NEW_CONFIG_FILE; 
    	strCmdForTk   = (string)" -t " + m_strSavingFileWTk;

	rMode=m_ExecuteMode;


	if(bCommand){
		// Command line mode
		if(!CheckProgAuth(g_strAuthPathForCmd)){
			return ERR_NOT_EXIST_AUTH_PROG;
		}else{
			strCmdPrefix = g_strAuthPathForCmd;
		}

        	if(bDontGetTkFlag){
			strCmdPrefix = strCmdPrefix + strCmdForAddr + (string)" > /dev/null";
		}else{
		    strCmdPrefix = strCmdPrefix + strCmdForAddr + strCmdForTk + (string)" > /dev/null";
		}
	}else{
		if(rMode == GRAPHIC){
			// Gui Mode
			if(CheckProgAuth(g_strAuthPathForGui)){
				strCmdPrefix = g_strAuthPathForGui;
			}else if(CheckProgAuth(g_strAuthPathForTui)){
				strCmdPrefix = g_strAuthPathForTui;
			}else{
				return ERR_NOT_EXIST_AUTH_PROG;
			}
		}else{
			// Tui Mode
			if(!CheckProgAuth(g_strAuthPathForTui)){
				return ERR_NOT_EXIST_AUTH_PROG;
			}else{
				strCmdPrefix = g_strAuthPathForTui;
			}
		}
		strCmdPrefix = strCmdPrefix + strCmdForAddr + strCmdForTk;
	}

#ifdef NDEBUG
	printf("strCmd:%s\n",strCmdPrefix.c_str());
#endif

	int nSystemReturn = 0;

        nSystemReturn = system(strCmdPrefix.c_str());
        nSystemReturn = WEXITSTATUS(nSystemReturn);

        return nSystemReturn;
}

/*!
@brief check whether the system have a authentication program or not.

@return bool - exist a authentication program? or not?
*/
bool classAuthen::CheckProgAuth(string strAuthPath)
{
	if(access(strAuthPath.c_str(), F_OK) == 0){
        	return true;
	}else{
		m_Logger->WriteLog_char(ERROR_LOG,"Cannot find the Program ",strAuthPath.c_str(),NULL);
		return false;
	}
}

/*!
@brief make the file name to be saved a temporary key

The file name to be saved a temporary key is made by combination with pId(process Id)
*/
void classAuthen::MakeFileName()
{
  	char szTkPathSuffix[10]={0};
	pid_t mypid;
  
	mypid=getpid();

	snprintf(szTkPathSuffix, sizeof(szTkPathSuffix) - 1, "%d",mypid);

	m_strSavingFileWTk = strTkPathPrefix + szTkPathSuffix;
}

/*!
@brief retrieve a temporary key from a file

@return retrieve a temporary key? or Not?
*/
bool classAuthen::SetTk(void)
{
	bool bRet;
	ifstream inTkFile;
		
	inTkFile.open(m_strSavingFileWTk.c_str());
	
	if(!inTkFile){
		m_Logger->WriteLog_char(DEBUG_LOG,"Cannot Open Tk file",m_strSavingFileWTk.c_str(),NULL);
		bRet=false;

		goto ret;
	}
	
	inTkFile >> m_strTk;

//	cout<<"TK is "<<m_strTk<<endl;

	inTkFile.close();
	
	if(m_strTk.length() != nTkMaxLength){
		bRet=false;
	}else{
		bRet=true;
	}

	// If TK file is existed,  then delete.  
	remove(	m_strSavingFileWTk.c_str());
ret:
	return bRet;
}

// To remove a white space into the STRING type
string classAuthen::RemoveSpace(string  strSrc)
{
        string strTemp = strSrc;
        string strTarget;

        for(int i=0;i<(int)strTemp.length();i++){
                if(strTemp.at(i) != ' '){
                        strTarget += strTemp.at(i);
                }
        }

        return strTarget;
}

/*!
@brief get temporary key

@param strAk - AuthenticationKey
@param strTk - TemporaryKey
*/
bool classAuthen::GetAK(string &strAk)
{
        ifstream fin;
        char buf[MAX_STRING];
        string strTemp;
        string strId;
        string strValue;

        fin.open(AUTHEN_FILE);

        if (!fin.is_open())
                return false;

        while(fin.getline(buf, MAX_STRING)){
                strTemp = buf;
                int nSplit = strTemp.find("=");

                if(nSplit > -1){
                        strId.assign(strTemp, 0, nSplit);
                        strValue.assign(strTemp,nSplit+1,strTemp.length()-(nSplit+1)) ;
                        strId = RemoveSpace(strId);
                        strId = RemoveSpace(strId);
                        if (strId == "authen_key")
                                strAk=RemoveSpace(strValue);
//			cout << "AK is " << strAk <<endl;
                }
        }

        fin.close();

        if (strAk.length() == 0){
                return false;
	}else{
                return true;
	}
}

bool classAuthen::GetTK(string &strTk)
{
	strTk=m_strTk;
	
	if(strTk.length() == 0){
		return false;
	}else{
		return true;
	}
}
/*!
@brief parse a temporary key 

A temporary key use to repository key.
A head part of the temporary key is a id of the repository, 
the other part is password of the repository.
*/
void classAuthen::ParseTkTail(void)
{
	m_strTkTail=m_strTk.substr(16,31);
//	cout<<"m_strTkTail:"<<m_strTkTail<<endl;
}
