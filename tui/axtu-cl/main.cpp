#include <sys/types.h> 
#include <unistd.h>
#include <sys/stat.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <classConfCtl.h>
#include <classRpmEngine.h>
#include <classNetwork.h>
#include <classLogger.h>
#include <hsCommon.h>
#include <popt.h> 
#include "classConfCtl.h"
#include "classLock.h"
#include "commondef.h"
#include "classRedcastle.h"
#include "trace.h"

#define MYSELF_NAME_CUI "axtu-cl"

#define URL_CONFIRM_MSG		_("Check update-server address of your configure file")
#define CONNECT_ERR_MSG		_("Cannot connect with Update Server.")
#define URL_ERR_MSG		CONNECT_ERR_MSG
#define CONNECT_CONFIRM_MSG	_("Check your network state.")
#define AUTH_ERR_MSG		_("Cannot authenticate from Server.")
#define USERFWRITE_ERR_MSG  _("Can NOT write the file. Please, Check your disk space.")
#define USERCANCEL_ERR_MSG  _("Download is canceled.")
#define UNKNOWN_ERR_MSG		_("Unknown error is occured.")
#define USERFWRITE_ERR_MSG  _("Can NOT write the file. Please, Check your disk space.")

static vector <string> vectorInputPackages;
static classConfigParser config;
static classRpmEngine *m_rpmEngine = NULL;
static classLogger *m_Logger = NULL;
static classNetwork  *m_Network = NULL;

static int nArgYes;
static int nArgBacklist;
static int nArgConflict;
static const char * szArgFile;
static int nArgCheck;
static int nArgDown;
static int nArgTest;
static int nArgRandomMin;
static int nArgDelayMin;
static int nArgHelp;
//static int nArgForce;

char strTemp[MAX_STRING];
static int nStatus;

static struct poptOption optionsTable[] = {
	
 { "yes", (char) 'y', POPT_ARG_NONE,&nArgYes,0,
 "You can update without yes/no question.", NULL },
 
 { "blacklist", (char) 'b', POPT_ARG_NONE,&nArgBacklist,0,
 "You can update even if there are blacklisted files.", NULL },
 
 { "conflict", (char) 'c', POPT_ARG_NONE,&nArgConflict,0,
 "You can update even if there are conficted files.", NULL },
 
 { "file", (char)NULL, POPT_ARG_STRING,&szArgFile,0,
 "You can update by the list in the file.", NULL },  
  
  { "check", (char) 'k', POPT_ARG_NONE,&nArgCheck,0,
 "You can update check that update package exist or not.", NULL },
 
 { "download", (char)'d', POPT_ARG_NONE,&nArgDown,0,
 "You can download only update available packages.", NULL },
  
 { "test", (char) 't', POPT_ARG_NONE,&nArgTest,0,
 "You can update test.", NULL },  
 
 { "random", (char)NULL, POPT_ARG_INT,&nArgRandomMin,0,
 "Sets the maximum amount of time axtu will wait before performing a command.", NULL },
 
 { "delay", (char)NULL, POPT_ARG_INT,&nArgDelayMin,0,
 "You can update after the delay time.", NULL },
 
 { "help", (char) 'h', POPT_ARG_NONE,&nArgHelp,0,
 //"You can show axtu-cl help.", NULL }, 
 "You can show " MYSELF_NAME_CUI " help.", NULL },

 
/*
{ "force", (char) 'f', POPT_ARG_NONE,&nArgForce,0,

 "You can update by nodep, force option.", NULL }, 
*/

POPT_TABLEEND

};

void ExitFunc (void)
{
	if(m_rpmEngine)
	{
		delete m_rpmEngine;
		m_rpmEngine = NULL;
	}
	if(m_Logger)
	{
		delete m_Logger;
		m_Logger = NULL;
	}
	if(m_Network)
	{
		delete m_Network;
		m_Network = NULL;
	}	
}
static void Exit(int num)
{
	if(unlink(AXTU_CUI_PID_FILE) != 0)
	{
		exit(1);
	}
	exit(num);
}
static bool show_NetworkErrorDialog(int nErr)
{
	
	
	bool bRet=false;
	switch(nErr){
	        case NETWORK_ERR_WRONG_URL:	        				
	        				snprintf(strTemp, sizeof(strTemp), "%s", URL_ERR_MSG);	        				
									bRet = false;  
	                break;
	        case NETWORK_ERR_CONNECT:
	        				snprintf(strTemp, sizeof(strTemp), "%s", CONNECT_ERR_MSG);
									bRet = false;             
	                break;
	        case NETWORK_ERR_AUTH_FAIL:
	        				snprintf(strTemp, sizeof(strTemp), "%s", AUTH_ERR_MSG);	        				
									bRet = false;                
	                break;
	        case NETWORK_ERR_UNKNOWN:
	        				snprintf(strTemp, sizeof(strTemp), "%s", UNKNOWN_ERR_MSG);	        					        				
									bRet = false;	                
	                break;	
	        case NETWORK_ERR_USERCANCEL:
	        				snprintf(strTemp, sizeof(strTemp), "%s", USERCANCEL_ERR_MSG);
	        				bRet = false;	                
	                break;
					case NETWORK_ERR_FWRITE:
									snprintf(strTemp, sizeof(strTemp), "%s", USERFWRITE_ERR_MSG);
	        				bRet = false;
	                break;
	        default:
	        				bRet = true;
	                break;
	}
	if(bRet == false)
	{
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp, NULL);
		printf("%s\n", strTemp);
	}
	return bRet;
}
	
//! @Bring the update package name from file.
static bool LoadInputFile(const char * strPath)
{
	
	char strMsg[256];
	if (access(strPath, F_OK) != 0)
	{
		snprintf(strMsg, sizeof(strMsg), "%s file do not exist.", strPath);
		printf("%s\n", strMsg); 
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strMsg , NULL);
    return false;              
	}
	char strBuf[MAX_STRING];
	ifstream fin;
	fin.open(strPath);	
	while (fin.getline(strBuf, sizeof(strBuf)) > 0 )
	{			
		vectorInputPackages.push_back(strBuf);		
	}	
	fin.close();
	return true;
} 


//! @Select random min in nMaxMin.
static int GetRandomSecond(int nMaxMin)
{
		int nMaxSecond = nMaxMin * 60;
    srand( time(NULL) );
    int nSecond = (int)(rand() % nMaxSecond) ;  //random is not use.
    return nSecond;
}

//! @brief Show complete message
static void WriteSuccessLog(int nErr)
{		
	if (nErr > 0)
	{
		//printf("axtu-cl updated following packages :\n");
		printf("%s updated following packages :\n", MYSELF_NAME_CUI);
		vector <structAddedFile> vectorTemp; 
		vectorTemp = m_rpmEngine->GetAddedFile( UPDATE | REQDEP |OTHERDEP , vectorTemp);
		
		vector <structAddedFile>::iterator it;
		int nIndex = 1;
		for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
		{					 
			string strTemp;			
			strTemp = m_rpmEngine->GetFullFileNameFromPath(it->strFile, strTemp);
			if (it->nType == UPDATE)  // User selected case
			{	 	
				if(it->nUpgrade)
				{
					m_Logger->RpmLogging(UPDATE_ACTION,strTemp.c_str());
				}
				else
				{
					m_Logger->RpmLogging(INSTALL_ACTION,strTemp.c_str());
				}
			}
			else  // Dependency case.
			{	
				if(it->nUpgrade)
				{
					m_Logger->RpmLogging(DEP_UPDATE_ACTION,strTemp.c_str() );
				}
				else
				{
					m_Logger->RpmLogging(DEP_INSTALL_ACTION,strTemp.c_str() );
				}
			}	
			printf("\t%s\n", strTemp.c_str());
		}
	}	
}


//! Find same package name in input packages that is from arguments. 
static bool CheckPackage(structFileInfo fileInfo)
{	
	bool bFind = (vectorInputPackages.size()==0)?true:false;
	//printf("bFind = %d\n", bFind);
	vector<string>::iterator it;
	for(it=vectorInputPackages.begin();it!=vectorInputPackages.end();it++)
	{
		if(fileInfo.strName == *it)
		{
			bFind = true;
			break;						
		}
	}

  if (fileInfo.strName == INCMP_RPM_NAME) {
    return true;
  }
	
	if (bFind)
	{	
		if (fileInfo.bIncompatible == true)
		{
			return false;
		}

		if (fileInfo.bBlacklisted == false)
		{
			return true;
		}
		else
		{
			if( nArgBacklist)
			{
				return true;
			} 
			else
			{
				return false;
			}
		}
	}

	return false;
}


//! return : Available update packages count.
static int DoUpdate()
{

        char strDubugMsg[MAX_STRING];
	m_Network->SetDontUseUI();
	
	int nResult=m_Network->CheckAuthen(false);
        if (nResult == FAIL_AUTH){
		snprintf(strTemp, sizeof(strTemp), "Authentication failed.");
                printf("%s\n", strTemp);
                m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);
                return 0;
	}else if(nResult == CANNOT_EXE_AUTH_PROG){
		snprintf(strTemp, sizeof(strTemp), "Cannot execute the authentication program.");
                printf("%s\n", strTemp);
                m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);
                return 0;
	}
	
	classConfCtl * m_ConfCtl=new classConfCtl();
	m_ConfCtl->ConfigCheck();
	delete m_ConfCtl;
	
	if(m_Network->SetDownloadConfig() == false)
	{
		snprintf(strTemp, sizeof(strTemp), "%s  file is not correct. Please modify this file.", CONFIG_FILE);
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI, strTemp, NULL);
		Exit(0);
	}
	if (config.Read(CONFIG_FILE) == false)
	{			
		printf("%s\n", "Failed to set up the download option configuration.");
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI, "Failed to set up the download option configuration.", NULL);
		Exit(0);
		
	}
    
	int nHeaderRet = m_Network->GetHeader();
	if(nHeaderRet != NETWORK_RETOK)
	{
		show_NetworkErrorDialog(nHeaderRet);
		return nHeaderRet;
	}
	    
	m_rpmEngine->SetIgnoreSelfUpdate(false);
	m_rpmEngine->ReadRemoteHeaderInfo();
	m_rpmEngine->ReadLocalHeaderInfo();
	m_rpmEngine->ReadIncmplistInfo(INCMP_CONFIG_FILE);
	m_rpmEngine->CreateUpdateInstallList();
	
	
	vector <structFileInfo> vectorUpdateFileInfo;
	vectorUpdateFileInfo.clear();
	vectorUpdateFileInfo = m_rpmEngine->GetUpdateList();
	
	// check if a new update package exist or not.
	
	if(nArgCheck)
	    {
			snprintf(strTemp, sizeof(strTemp), "There are ( %d ) update available packages.", vectorUpdateFileInfo.size());    		
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);    		
			return -4;
	    } 
	  
	vector <structFileInfo> vectorInstallFileInfo;
	vectorInstallFileInfo.clear();
	vectorInstallFileInfo = m_rpmEngine->GetInstallList();   
	
	m_Network->ClearPackages();
	nHeaderRet = m_Network->GetHeaders(vectorUpdateFileInfo, vectorInstallFileInfo);    
	if(nHeaderRet != NETWORK_RETOK)
	    {
		show_NetworkErrorDialog(nHeaderRet);
		return nHeaderRet;
	    }	
	
	m_rpmEngine->SetNetwork(m_Network);
	m_rpmEngine->ReadHeaders();
	int nRet = 0;
	string strUrlFullPath, strFullPath;
	
	
	vector<structFileInfo>::iterator it;
	for(it=vectorUpdateFileInfo.begin();it!=vectorUpdateFileInfo.end();it++)
	    {	       
	  	if (CheckPackage(*it) == false)
			continue;
	  	strFullPath = m_rpmEngine->GetFullPathFile(0,  false, it->strName, it->strVersion , it->strRelease, it->strArch, "");
		strUrlFullPath = m_rpmEngine->GetFullPathFile(0,  false, it->strName, it->strVersion , it->strRelease, it->strArch, "", true);
			
		//cout << "test strFullPath : " << strFullPath << endl;
		//cout << "test strUrlFullPath : " << strUrlFullPath << endl << endl;
		m_rpmEngine->AddFile(strFullPath.c_str(), UPDATE, 1);
		m_Network->AddPackage(strFullPath, strUrlFullPath);	
		  
		nRet++;
	    }
	    	    		
		
	if (nRet == 0)
	{
		snprintf(strTemp, sizeof(strTemp), "%s", "There are no packages available to update.");    		
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);    		
		return -1;
	}

// According to Bug#4094 policy. I removed following function.
// But I think may be It can be used in future. So I just commented following function. 
/* 
	#if defined(__i386__) || defined(__x86_64__) || defined(__ia64__)	
	if(nStatus == RC_STATUS_WARNING)
	{
		if(m_rpmEngine.CheckKmodRedcastle() == false)
		{
			string strKmodName;
			printf("%s\n\n", REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG);		
			m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI , REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG, NULL);
			vector <string> vectorTemp = m_rpmEngine.GetAddedKmodRedcastleFile();
			vector <string>::iterator it;		
	
			string  strName, strVer, strRel, strArch;
			for(it=vectorTemp.begin();it!=vectorTemp.end();it++)
			{
				m_rpmEngine.stripNVRA(*it, &strName, &strVer, &strRel, &strArch);
		                strKmodName = strName + "-" +  strVer + "-" + strRel + "." + strArch;
				printf("    -%s\n", strKmodName.c_str());
				m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI , "    -",strKmodName.c_str(), NULL);
			}
			if(m_rpmEngine.RemoveKernelAndKmodRedcastle(0) == false)
			{
				m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI , "Failed to remove kernel and redcastle.", NULL);
	                	return -1;
			}
			vector <structAddedFile> vectorForCount;
			vectorForCount = m_rpmEngine.GetAddedFile( UPDATE, vectorForCount);
			if((int)vectorForCount.size() < 1)
			{
				printf("%s\n", "There are no packages available to update!");
				m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI , "There are no packages available to update!", NULL);
	                	
				return -1;
			}
		}
		else
		{
			vector <string> vectorTemp = m_rpmEngine.GetAddedKmodRedcastleFile();
	                vector <string>::iterator it;
	                int nIndex = 1;
	                for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
	                {
				string  strName1, strVer1, strRel1, strArch1;			
				m_rpmEngine.stripNVRA(*it, &strName1, &strVer1, &strRel1, &strArch1);
				strUrlFullPath = m_rpmEngine.GetFullPathFile(0,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "", true);
				strFullPath = m_rpmEngine.GetFullPathFile(0,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "");
				if(m_rpmEngine.AddFile(strFullPath.c_str(), REQDEP, 0) != 0)
				{	
					snprintf(strDubugMsg, sizeof(strDubugMsg), "Failed to load m_rpmEngine.AddFile(%s) function.", strFullPath.c_str());
					m_Logger.WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI, strDubugMsg, NULL);
					printf("%s\n", strDubugMsg);
					return -1;
				}
				m_Network.AddPackage(strFullPath, strUrlFullPath);
				snprintf(strDubugMsg, sizeof(strDubugMsg), "Add install : %s", strFullPath.c_str());
				m_Logger.WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, (const char *)strDubugMsg, NULL);
	
	                }
		}
	}
#endif	
*/
		
	nHeaderRet = m_Network->GetPackages();
	if(nHeaderRet != NETWORK_RETOK)
	{
		show_NetworkErrorDialog(nHeaderRet);
	return nHeaderRet;
	}
	
	if(nArgDown)
	{
		if(vectorInputPackages.size()==0)
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "All update available packages are downloaded.");
		}
		else
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "User selected packages are downloaded.");					    		
		}
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);    		
		return -5;			
	}
		
	
	int result;
	
	if(nArgBacklist)
	{						
		result = m_rpmEngine->Check(true);
	}
	else
	{
							
		result = m_rpmEngine->Check();
	}		
		
	if (result >= 0)
	{			
		//Check OK
	}
	else
	{			
		if(result == -9 || result == -6)
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "Cannot solve dependency problem.");	
		}
		else if(result = -8)
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "Blacklist or IncompatibleList is added while finding dependency. ");							
		}
		else if(result = -10)
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "Network error.");				
		}
		else
		{
			snprintf(strTemp, sizeof(strTemp), "%s", "Unknown error.");				
		}
					
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI, strTemp, NULL);
		
		return -3;
	}
	
	
	if(nArgTest)
	{			
		snprintf(strTemp, sizeof(strTemp), "%s", "Update test has been finished sucessfully.  You will can update some packages.");
		printf("%s\n", strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);
		return -2;
	}
	
	if(nArgConflict)
	{			
		result = m_rpmEngine->Run(true);
	}
	else
	{			
		result = m_rpmEngine->Run();		
	}
	
	
	if(result ==0) // success
	{				
		snprintf(strTemp, sizeof(strTemp), "%s has finished successfully. See success.log file in %s directory", MYSELF_NAME_CUI, m_Logger->GetLogPath().c_str());
	}		
	else if(result == -11)
	{	
		snprintf(strTemp, sizeof(strTemp), "%s has failed. There are no space for installation.", MYSELF_NAME_CUI);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_CUI, strTemp, NULL);
		return -11;			
	}
	else					// fail
	{	
		snprintf(strTemp, sizeof(strTemp), "%s has failed. See error.log file in %s directory", MYSELF_NAME_CUI, m_Logger->GetLogPath().c_str());
	}
	
	printf("\n\n%s\n", strTemp);
	m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);    
	return nRet;
}



int main(int argc, char *argv[])
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif	
	if(atexit(ExitFunc) != 0)
	{
		exit(1);
	}
	classRedcastle * redcastle = new classRedcastle();
        nStatus = redcastle->GetRCStatus();
        delete redcastle;

        if(nStatus == RC_STATUS_ENABLE)
        {
		printf("\n");
		printf("************************************************\n");
		printf("%s\n", REDCASTLE_ENABLE_MSG);
		printf("************************************************\n");
		printf("\n");
                Exit(2);
        }
        else if(nStatus == RC_STATUS_WARNING)
        {
		printf("\n");
		printf("************************************************\n");
		printf("%s\n", REDCASTLE_WARNING_MSG);
		printf("************************************************\n");
		printf("\n");
				Exit(2);
        }

	bool bAlreadyExecuted = false;
	
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
	
	if(bAlreadyExecuted)
	{
		printf("TSN Updater is already running.\n");
		Exit(2);
	}
	else
	{
		if(CHSLock::lock((AXTU_CUI_PID_FILE)) == false)
		{
			printf("Internal Error #2, please contact your support provider, or try again later.\n");
			Exit(2);
		}
	}
	
	
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_CUI, strerror(errno));
		Exit(1);
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_CUI, strerror(errno));
		Exit(1);
	}
	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_CUI, strerror(errno));
		Exit(1);
	}
	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_CUI, strerror(errno));
		Exit(1);
	}
	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		fprintf(stdout, "%s: %s\n", MYSELF_NAME_CUI, strerror(errno));
		Exit(1);
	}

	m_rpmEngine = new classRpmEngine();
	m_Logger = new classLogger();
	m_Network = new classNetwork();
	
	classConfCtl * m_ConfCtl=new classConfCtl();
	m_ConfCtl->ConfigCheck();
	delete m_ConfCtl;
	
	if (access(CONFIG_FILE, F_OK) != 0)
	{
		snprintf(strTemp, sizeof(strTemp), "%s file is not found. %s will operates abnormally", CONFIG_FILE, MYSELF_NAME_CUI);
		printf("%s: %s\n", MYSELF_NAME_CUI, strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, (const char *)strTemp, NULL);
		Exit(0);
	}
	
	if( m_Logger->CheckLogDirSpace() == false)
	{
		snprintf(strTemp, sizeof(strTemp), "You don't have enough space for log directory(%s).", m_Logger->GetLogPath().c_str());
		printf("%s: %s\n", MYSELF_NAME_CUI, strTemp);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);
		Exit(0);
	}
	
	
	nArgYes=0;
	nArgBacklist=0;
	nArgConflict=0;
	nArgCheck=0;
	nArgDown=0;
	nArgTest=0;
	nArgRandomMin=0;
	nArgDelayMin=0;
	szArgFile=0;
	nArgHelp=0;
		
		
	if (getuid() != 0 )
    {		
		printf("%s: You need to be root to run TSN Updater.\n", MYSELF_NAME_CUI);
		Exit(1);
    }
    
	printf("************************************************\n");	
	printf("Start %s [Asianux TSN Updater - Command Line].\n", MYSELF_NAME_CUI);
	
	int option=0;	
	poptContext context = poptGetContext((const char*)argv[0],argc,(const char**)argv,(const struct poptOption* ) &optionsTable,0);
	option=poptGetNextOpt(context);
	if(option != -1 || nArgHelp)
	{
		poptPrintHelp(context,stdout,POPT_ARG_NONE);
		poptFreeContext(context);
		Exit(0);
    }

	if(nArgDelayMin)
 	{
 		sleep(nArgDelayMin * 60); 		
 	}
 	
 	if (nArgRandomMin)
	{		
		sleep(GetRandomSecond(nArgRandomMin));
	}
    	
  for(int i=1;i<argc;i++)
    {
		if(argv[i][0] != '-')
		{
    	//cout << argv[i] << ", ";  //test
    	vectorInputPackages.push_back(argv[i]);
		}
    }
  if(szArgFile)
	{			
		if (LoadInputFile(szArgFile) == false)
		{
			Exit(0);
		}
		
	}
	 
	// No question.
	if(nArgYes == 0)
	{
		if(nArgCheck == 0)
		{			
				
			char cChar;
		  char buf[256];	  
		  			
			
		  if(vectorInputPackages.size() != 0)
		    {
		    if(nArgDown)
			    {
					printf("Do you want to download user selected packages now? (y/n) ");
			    }
			  else
			    {
			    printf("Do you want to update user selected packages now? (y/n) ");
			    }
		    }
		  else
		    {
		    if(nArgDown)
			    {
		    	printf("Do you want to download all available packages now? (y/n) ");
			    }
			  else
			    {
			    printf("Do you want to update all available packages now? (y/n) ");			    	
			    }
			}		    
		    
		  fgets(buf, sizeof(buf), stdin);
		  cChar = buf[0];
		  
		  //if (cChar == 'n')
		  if (cChar != 'Y' && cChar != 'y')
		  {
			  Exit(0);
		  }
		}
	}
	
	
	//printf("axtu-cl update mode :\n");
	printf("%s update mode :\n", MYSELF_NAME_CUI);
	if(nArgBacklist)
		printf("\tBlacklist mode : %s\n", nArgBacklist?"On":"Off");
	if(nArgConflict)
		printf("\tConflict mode : %s\n", nArgConflict?"On":"Off");
	if(szArgFile)
		printf("\tInput file name : %s\n", szArgFile);
	if(nArgCheck)
		printf("\tUpdate check mode : %s\n", nArgCheck?"On":"Off");
	if(nArgDown)
		printf("\tDownload mode : %s\n", nArgDown?"On":"Off");
	if(nArgTest)
		printf("\tUpdate test mode : %s\n", nArgTest?"On":"Off");
	if(nArgRandomMin)
		printf("\tRandon range minutes : %d min\n", nArgRandomMin);
	if(nArgDelayMin)
		printf("\tDelay minutes : %d min\n", nArgDelayMin);
  
	
	
	if(!m_rpmEngine->CheckConfigFileLength())
	{
	        printf("%s: The maximum length of the combined path and file name is 255 characters. \nPlease modify the entry.\n", MYSELF_NAME_CUI);
	        Exit(0);
	}
	
	int nUpdateCount = DoUpdate();		
	WriteSuccessLog(nUpdateCount);
	//printf("End axtu-cl.\n");
	printf("End %s.\n", MYSELF_NAME_CUI);
	printf("************************************************\n");
	
  // Restart myself if updated myself
  if (m_rpmEngine->CheckSelfUpdate() == true) {
    // It can't because it is static variable.
    // delete resource;
	ExitFunc();
    unlink(AXTU_CUI_PID_FILE);
    printf("Restart myself\n");
    execvp(argv[0], argv);
  }
  
#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  Exit(0);	
}

