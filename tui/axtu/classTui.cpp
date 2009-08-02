/*!
@file classTui.cpp
@brief Class source file for Text User Interface of Updater
*/
#include "classTui.h"
#include "commondef.h"
#include "classRedcastle.h"
#include "classConfigParser.h"

static classLogger *m_refLogger;

newtComponent g_label_TotalProgressMsg;
newtComponent g_label_CurrentProgressMsg;
newtComponent g_scale_TotalProgress;
newtComponent g_scale_CurrentProgress;
newtComponent g_form_Progress;
newtComponent btn_Cancel;
struct newtExitStruct g_es_Progress;

/*! 
@brief show error window on TUI using newt library

If both of messages are NULL, you cannot see the messages.
@param szMsg_1 - First message for error
@param szMsg_2 - Second message for error
*/
void show_err_msg_on_tui(const char *szMsg_1,const char *szMsg_2)
{
        char szBuffer[MAX_STRING]={0};

        if(!(szMsg_1 == NULL && szMsg_2 == NULL)){
		if(szMsg_1 == NULL){
                	snprintf(szBuffer, MAX_STRING, "%s",szMsg_2);
		}else if(szMsg_2 == NULL){
                	snprintf(szBuffer, MAX_STRING, "%s",szMsg_1);
		}else{	
                	snprintf(szBuffer, MAX_STRING, "%s : %s",szMsg_1,szMsg_2);
        	}
        	newtInit();
        	newtCls();
        	newtDrawRootText(0,0,TITLE);
        	newtWinMessage(ERROR_MSG,BTN_OK_MSG,szBuffer);
        	newtFinished();
	}
}


/*!
@brief 

This Method return a file name 
from the path string
*/
void pathFilter(const char *szArg,char *szTarget)
{
        int i,j,k=0;

        // find '/'
        for(i=strlen(szArg);i >= 0;i--){
                if(szArg[i]=='/')       break;
        }

        // setting the value of j
        if(i != 0){
                j=i+1;
        }else{
                j=0;
        }

        // fill the data
        for(;j <= (int)strlen(szArg);j++){
                if((szArg[j] == '.') && (szArg[j+1] == 'r') && (szArg[j+2] == 'p') && (szArg[j+3] == 'm')){
                        break;
                }else{
                        szTarget[k]=szArg[j];
                }
        	k++;
        }
        szTarget[k]='\0';
}

/*!
@brief check all package list

This method present '*' into Pkglist-Checkbox 
when user check a AllCheck-Checkbox
*/
static void AllCheckCallback(newtComponent co,void *data)
{
	struct callbackInfo *cbi=(struct callbackInfo*)data;
	int i=0;
	
	if(*cbi->state == ' '){
		for(i=0;i<(*cbi).figure;i++){ 
			if(newtCheckboxGetValue((*cbi).en[i]) != 'b' && newtCheckboxGetValue((*cbi).en[i]) != 'i'){				
				newtCheckboxSetValue((*cbi).en[i],' ');
			}
		}
	}else{
		for(i=0;i< (*cbi).figure;i++){
			if(newtCheckboxGetValue((*cbi).en[i]) != 'b' && newtCheckboxGetValue((*cbi).en[i]) != 'i'){
				newtCheckboxSetValue((*cbi).en[i],'*');
			}
		}
	}
	newtRefresh();
}

/*!
@brief A constructor

@param nMode - Program mode ex) update or install or erase
*/
classTui::classTui(int nMode,
		classConfCtl *refConfCtl,
		classConfigParser *refConfigParser,
		classLogger *refLogger,
		classNetwork *refNetwork,
		classRpmEngine *refRpmEngine)
{		
	switch (nMode) {
		case INSTALL_MODE:
			break;
		case ERASE_MODE:
			break;
		case UPDATE_MODE:
			break;
		default:
			nMode = UPDATE_MODE;
			break;
	}

	m_refRpmEngine = refRpmEngine;
	m_refNetwork = refNetwork;

	m_refLogger = refLogger;
	m_refConfigParser = refConfigParser;
	m_refConfCtl = refConfCtl;

	set_ProgramMode((PROGRAM_MODE)nMode);
}

//! @brief A destructor
classTui::~classTui(void)
{	
	UninitTui();
} 

/* 
@brief get rpm-header files

get rpm-header files (*.hdr) using the classNetwork from server
@return int - 0:Success 1:Fail
@see classNetwork
*/
int classTui::GetHeader(void)
{
	int nRet=0;
	int nResult = SUCCESS_FLAG;

	nRet = m_refNetwork->GetHeader();
	if(nRet != NETWORK_RETOK)
	{
		switch(nRet){
			case NETWORK_ERR_FWRITE:
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,FWRITE_ERR_MSG,"GetHeader()",NULL);
				show_err_msg_on_tui(FWRITE_ERR_MSG,NULL);
				nResult = FAIL_FLAG;
	                        break;
			default:
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,CONNECT_ERR_MSG,"GetHeader()",NULL);
				show_err_msg_on_tui(CONNECT_ERR_MSG,CONNECT_CONFIRM_MSG);
				nResult = FAIL_FLAG;
				break;
		}
	}else{
		nResult = SUCCESS_FLAG;
	}
	
	return nResult;
}

/*!
@brief initializer of classTui 

@return int - 0:Success 1:Fail
*/
int classTui::InitTui(void)
{
	int nRet = 0;
	int nResult = SUCCESS_FLAG;

	if(get_ProgramMode() != ERASE_MODE){
		if(m_refNetwork->SetDownloadConfig() == false)
		{
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Failed to set up the download option configuration.",NULL);
			nResult = FAIL_FLAG;
		}
		nRet = GetHeader();
		if(nRet != SUCCESS_FLAG){
			nResult = FAIL_FLAG;
		}else{
			nResult = SUCCESS_FLAG;
			m_bIsHeaderWork = false;
		}
	}else{
		nResult = SUCCESS_FLAG;
	}
		
	return nResult;
}

/*!
@brief uninitializer of classTui 
*/
void classTui::UninitTui(void)
{	
	newtFinished();
	if(access(AXTU_TUI_PID_FILE, F_OK) == 0 && unlink(AXTU_TUI_PID_FILE) != 0)
	{		
		exit(1);
	}		
}

/*!
If there is not a process of launcher, it can just restart by itself.
But, there is a process of launcher, it write some contents share-memory.
And then, the launcher take some action.

@param szArg - string to write in share-memory
@return bool - Success or not
*/
bool classTui::restarter_core(char *szArg)
{
	PROGRAM_MODE pMode=get_ProgramMode();

	if(strlen(szArg) <  2){
		if(pMode == UPDATE_MODE){
			if(execl(TUI_UPDATER_PATH,TUI_UPDATER_NAME,OPTION_UPDATE,OPTION_FORCE,NULL) == -1){
				return false;
			}
		}else if(pMode == INSTALL_MODE){
			if(execl(TUI_UPDATER_PATH,TUI_UPDATER_NAME,OPTION_INSTALL,OPTION_FORCE,NULL) == -1){
				return false;
			}
		}else if(pMode == ERASE_MODE){
			if(execl(TUI_UPDATER_PATH,TUI_UPDATER_NAME,OPTION_ERASE,OPTION_FORCE,NULL) == -1){
				return false;
			}
		}
	}else{
	    	void *shared_memory = (void *)0;
	    	struct shared_use_st *shared_stuff;
	    	char buffer[BUFF_SZ]={0,};
	    	int shmid;
	
                if(!strncmp(szArg,"network",strlen("network"))){
                        if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
                }else if(!strncmp(szArg,"config",strlen("config"))){
                        if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
		}else if(!strncmp(szArg,SHM_TAG_ERR_NOSPACE,strlen(SHM_TAG_ERR_NOSPACE))){
			if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
		}else if(!strncmp(szArg,SHM_TAG_ERR_CANNOT_EXE_AUTH,strlen(SHM_TAG_ERR_CANNOT_EXE_AUTH))){
                        if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
		}else if(!strncmp(szArg,SHM_TAG_ERR_DIR_LENGTH_OVER,strlen(SHM_TAG_ERR_DIR_LENGTH_OVER))){
                        if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
		}else if(!strncmp(szArg,SHM_TAG_ERR_CONF_LENGTH_OVER,strlen(SHM_TAG_ERR_CONF_LENGTH_OVER))){
                        if(strncpy(buffer,szArg,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
                }else if(pMode == UPDATE_MODE){
                        if(strncpy(buffer,SHM_TAG_UPDATE,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
                }else if(pMode == INSTALL_MODE){
                        if(strncpy(buffer,SHM_TAG_INSTALL,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}	
                }else if(pMode == ERASE_MODE){
                        if(strncpy(buffer,SHM_TAG_ERASE,BUFF_SZ-1) == NULL){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
				return false;
			}
                }else{
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Unknown argument in \"shm_write()\"",NULL);
			return false;
		}
		
	    	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	
	    	if (shmid == -1) {
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot get share memory","shmget failed",NULL);
			return false;
    		}
	
	    	shared_memory = shmat(shmid, (void *)0, 0);
	    	if (shared_memory == (void *)-1) {
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot attach share memory","shmat failed",NULL);
			return false;
	    	}

		shared_stuff = (struct shared_use_st *)shared_memory;
		if(strncpy(shared_stuff->some_text, buffer, BUFF_SZ-1) == NULL){
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot copy the string","strncpy()",NULL);
			return false;
		}

    		if (shmdt(shared_memory) == -1){
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot detach share memory","shmdt failed",NULL);
			return false;
    		}
	}

	return true;
}


/*
@brief check the contents in share-memory

@return bool - Success or not
*/
bool classTui::shm_check(void)
{
        void *shared_memory = (void *)0;
        struct shared_use_st *shared_stuff;
        int shmid;
        bool bRet=false;

        shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

        if (shmid == -1) {
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot get share memory","shmget failed",NULL);
		return false;
        }

        // make the shared memory accessible to the program
        shared_memory = shmat(shmid, (void *)0, 0);
        if (shared_memory == (void *)-1) {
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot attach share memory","shmat failed",NULL);
		return false;
        }

        shared_stuff = (struct shared_use_st *)shared_memory;
//      shared_stuff->written_by_you = 0;

        if(!strncmp(shared_stuff->some_text,"saved",strlen("saved"))){
                memset(shared_stuff->some_text,0,strlen("saved"));
                bRet=true;
        }else if(!strncmp(shared_stuff->some_text,"canceled",strlen("canceled"))){
                memset(shared_stuff->some_text,0,strlen("canceled"));
                bRet=false;
        }

        // shared memory is detached
        if (shmdt(shared_memory) == -1) {
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot detach share memory","shmdt failed",NULL);
		return false;
        }

/*
        if(shmctl(shmid,IPC_RMID,0) == -1){
                fprintf(stdout,"shmctl failed\n");
		return false;
        }
*/
        return bRet;
}


/*!
@brief let this program restart

@param pArg - A flag whether restart or not
@return int - 0:Success 1:Fail
*/
int classTui::updater_restart(void)
{
	char szCmd[MAX_STRING]={0};
	char szPid[10]={0};
	FILE *fp = NULL;
	bool bRet=false;
	int nResult = SUCCESS_FLAG;

	if(snprintf(szCmd,sizeof(szCmd),"pidof %s",TUI_LOADER_PATH) > 0){
		fp = popen(szCmd,"r");
		if(fp == NULL){
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot open named-pipe","popen error",NULL);
			nResult = FAIL_FLAG;
		}else{
			fread(szPid,1,5,fp);
			pclose(fp);

			bRet=restarter_core(szPid);
			if(!bRet){
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot restart","restarter_core()",NULL);
				nResult = FAIL_FLAG;
			}
		}
	}else{
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot work fucntion","snprintf()",NULL);
		nResult = FAIL_FLAG;
	}

	return nResult;
}

/*
This Method set the inner value - "m_ProgramMode"

@param mode - This is one of values in ( UPDATE_MODE, INSTALL_MODE, ERASE_MODE )
*/
void classTui::set_ProgramMode(PROGRAM_MODE mode)
{
	switch (mode) {
		case INSTALL_MODE:	break;
		case ERASE_MODE:	break;
		case UPDATE_MODE:
	default:
		mode = UPDATE_MODE;
		break;
	}

	m_ProgramMode = mode;
}


PROGRAM_MODE classTui::get_ProgramMode(void)
{
	return m_ProgramMode;
}

/*!
@brief CALLBACK Function getting the rpm-headers

@param p1 - total progress percentage 
@param p2 - current progress percentage 
@param msg1 - total progress message
@param msg2 - current progress message
@see classNetwork
*/
void classTui::GetHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{	
	if(g_es_Progress.reason == g_es_Progress.NEWT_EXIT_COMPONENT && g_es_Progress.u.co == btn_Cancel){		
		Exit(1);
	}
	// argument check
	if( p1 > 100 || p1 < 0 || p2 > 100 || p2 < 0 )
	{
		return;	
	}
	if(msg1 == NULL){
		msg1 = "";
	}
	if(msg2 == NULL){
		msg2 = "";
	}
	
	newtLabelSetText(g_label_TotalProgressMsg,msg1);
	newtLabelSetText(g_label_CurrentProgressMsg,msg2);
	
	newtScaleSet(g_scale_TotalProgress,p1);
	newtScaleSet(g_scale_CurrentProgress,p2);
        newtFormSetTimer(g_form_Progress,50);
	newtRefresh();
	newtFormRun(g_form_Progress,&g_es_Progress);

#ifdef NDEBUG	
	m_refLogger->WriteLog_char(DEBUG_LOG,"TUI GetHeadersCallBack",msg1,",",msg2,NULL);
	m_refLogger->WriteLog_int(DEBUG_LOG,"TUI GetHeadersCallBack",p1,p2,NULL);
#endif /* NDEBUG */
}

/*!
@brief CALLBACK Function getting the rpm files

@param p1 - total progress percentage 
@param p2 - current progress percentage 
@param msg1 - total progress message
@param msg2 - current progress message
@see classNetwork
*/
void classTui::GetPackagesCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	if(g_es_Progress.reason == g_es_Progress.NEWT_EXIT_COMPONENT && g_es_Progress.u.co == btn_Cancel){		
		Exit(1);
	}
	// argument check
	if( p1 > 100 || p1 < 0 || p2 > 100 || p2 < 0 )
	{
		return;	
	}
	string strTotalMsg;
	
	if(msg1 == NULL){
		msg1 = "";
	}
	if(msg2 == NULL){
		msg2 = "";
	}
	
	newtLabelSetText(g_label_TotalProgressMsg,msg1);
	newtLabelSetText(g_label_CurrentProgressMsg,msg2);
	
	newtScaleSet(g_scale_TotalProgress,p1);
	newtScaleSet(g_scale_CurrentProgress,p2);
        newtFormSetTimer(g_form_Progress,50);
	newtRefresh();
	newtFormRun(g_form_Progress,&g_es_Progress);
	
#ifdef NDEBUG	
	m_refLogger->WriteLog_char(DEBUG_LOG,"TUI GetPackagesCallBack",msg1,",",msg2,NULL);
	m_refLogger->WriteLog_int(DEBUG_LOG,"TUI GetPackagesCallBack",p1,p2,NULL);
#endif /* NDEBUG */
}

/*!
@brief Common CALLBACK Function

@param p1 - total progress percentage 
@param p2 - current progress percentage 
@param msg1 - total progress message
@param msg2 - current progress message
@see classRpmEngine
*/
void classTui::CommonCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
        char *szTotalMsg = NULL;

        szTotalMsg=(char*)calloc(sizeof(char),BUFF_SZ);
	if(szTotalMsg == NULL){
		fprintf(stdout,"TUI - %s\n",strerror(errno));
                return;
        }

        // argument check
        if( p1 > 100 || p1 < 0 || p2 > 100 || p2 < 0 )
        {
                return;
        }

        if(msg1 == NULL){
                msg1 = "";
        }
        if(msg2 == NULL){
                msg2 = "";
        }else{
//                pathFilter(msg2,szTotalMsg);
        }

        newtLabelSetText(g_label_TotalProgressMsg,msg1);
        newtLabelSetText(g_label_CurrentProgressMsg,msg2);

        newtScaleSet(g_scale_TotalProgress,p1);
        newtScaleSet(g_scale_CurrentProgress,p2);
        newtFormSetTimer(g_form_Progress,50);
	    newtRefresh();        

#ifdef NDEBUG	
	m_refLogger->WriteLog_char(DEBUG_LOG,"TUI CommonCallBack",msg1,",",msg2,NULL);
	m_refLogger->WriteLog_int(DEBUG_LOG,"TUI CommonCallBack",p1,p2,NULL);
#endif /* NDEBUG */

	if(szTotalMsg != NULL){
		free(szTotalMsg);
		szTotalMsg = NULL;
	}
}

/*!
@brief CALLBACK Function reading rpm-headers

@param p1 - total progress percentage 
@param p2 - current progress percentage 
@param msg1 - total progress message
@param msg2 - current progress message
@see classRpmEngine
*/
void classTui::ReadHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{	
	char *szTotalMsg = NULL;
	char *szCurMsg = NULL;

	szTotalMsg=(char*)calloc(sizeof(char),BUFF_SZ);
	if(szTotalMsg == NULL){
		fprintf(stdout,"TUI - %s\n",strerror(errno));
                return;
        }

	szCurMsg=(char*)calloc(sizeof(char),BUFF_SZ);
        if(szCurMsg == NULL){
		fprintf(stdout,"TUI - %s\n",strerror(errno));
		free(szTotalMsg);
		szTotalMsg = NULL;
                return;
        }

	// argument check
	if( p1 > 100 || p1 < 0 || p2 > 100 || p2 < 0 )
	{
		return;	
	}

	if(msg1 == NULL || strlen(msg1) == 0){
 		strncpy(szTotalMsg,_("Total progress"),BUFF_SZ);
	}else{
                strncpy(szTotalMsg,msg1,BUFF_SZ);
	}

	if(msg2 == NULL || strlen(msg2) == 0){
		strncpy(szCurMsg,_("Current progress"),BUFF_SZ);
	}else{
		pathFilter(msg2,szCurMsg);
	}	
	
	newtLabelSetText(g_label_TotalProgressMsg,szTotalMsg);	
	newtLabelSetText(g_label_CurrentProgressMsg,szCurMsg);
	newtScaleSet(g_scale_TotalProgress,p1);	
	newtScaleSet(g_scale_CurrentProgress,p2);
	newtFormSetTimer(g_form_Progress,50);        
	newtRefresh();
		
	
#ifdef NDEBUG
	m_refLogger->WriteLog_char(DEBUG_LOG,"TUI ReadHeadersCallBack",szTotalMsg,",",szCurMsg,NULL);
	m_refLogger->WriteLog_int(DEBUG_LOG,"TUI ReadHeadersCallBack",p1,p2,NULL);
#endif /* NDEBUG */

	if(szTotalMsg != NULL){
		free(szTotalMsg);
		szTotalMsg = NULL;
	}

	if(szCurMsg != NULL){
		free(szCurMsg);
		szCurMsg = NULL;
	}
}

/*!
@brief CALLBACK Function runing the rpm-actions(update/install/erase)

@param p1 - total progress percentage 
@param p2 - current progress percentage 
@param msg1 - total progress message
@param msg2 - current progress message
@see classRpmEngine
*/
void classTui::RunCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{	
	char *szTotalMsg = NULL;
	char *szCurMsg = NULL;

	szTotalMsg=(char*)calloc(sizeof(char),BUFF_SZ);
	if(szTotalMsg == NULL){
		fprintf(stdout,"TUI - %s\n",strerror(errno));
                return;
        }

	szCurMsg=(char*)calloc(sizeof(char),BUFF_SZ);
	if(szCurMsg == NULL){
		fprintf(stdout,"TUI - %s\n",strerror(errno));
		if(szTotalMsg != NULL){
                	free(szTotalMsg);
			szTotalMsg = NULL;
		}
                return;
        }

	// argument check
	if( p1 > 100 || p1 < 0 || p2 > 100 || p2 < 0 )
	{
		return;	
	}
			
        if(msg1 == NULL || strlen(msg1) == 0){
                strncpy(szTotalMsg,_("Total progress"),BUFF_SZ);
        }else{
                strncpy(szTotalMsg,msg1,BUFF_SZ);
        }

        if(msg2 == NULL || strlen(msg2) == 0){
                strncpy(szCurMsg,_("Current progress"),BUFF_SZ);
        }else{
                pathFilter(msg2,szCurMsg);
        }

	newtLabelSetText(g_label_TotalProgressMsg,szTotalMsg);
	newtLabelSetText(g_label_CurrentProgressMsg,szCurMsg);

	newtScaleSet(g_scale_TotalProgress,p1);
	newtScaleSet(g_scale_CurrentProgress,p2);
        newtFormSetTimer(g_form_Progress,50);
	newtRefresh();	
	
#ifdef NDEBUG	
	m_refLogger->WriteLog_char(DEBUG_LOG,"TUI RunCallBack",szTotalMsg,",",szCurMsg,NULL);
	m_refLogger->WriteLog_int(DEBUG_LOG,"TUI RunCallBack",p1,p2,NULL);
#endif /* NDEBUG */

	if(szTotalMsg != NULL){
		free(szTotalMsg);
        	szTotalMsg = NULL;
	}
	if(szCurMsg != NULL){
        	free(szCurMsg);
        	szCurMsg = NULL;
	}
}

/*! 
@brief Set Rpm command for RpmEngine

This method set command for some rpm actions
The actions are 1 and 0
 1 will be update and install rpms
 0 will be erase rpms

@return NextPage - next page
@see PAGE
*/
PAGE classTui::set_RpmCommand(PROGRAM_MODE mode)
{
	PAGE NextPage = PAGE_EXIT;

        if(mode == UPDATE_MODE){
                m_refRpmEngine->SetCommand(1);
				m_refRpmEngine->ReadCacheDirInfo();
                NextPage = UPDATE_METHOD_PAGE;
        }else if(mode == INSTALL_MODE){
                m_refRpmEngine->SetCommand(1);
				m_refRpmEngine->ReadCacheDirInfo();
                NextPage = UPDATE_INSTALL_LIST_PAGE;
        }else if(mode == ERASE_MODE){
                 m_refRpmEngine->SetCommand(0);
                 NextPage = ERASE_LIST_PAGE;
        }

	return NextPage;
}
void classTui::struct2str(vector <structAddedFile> vectorOrig,vector <string> &vectorTarget)
{
	vector <structAddedFile>::iterator it;
	string strTmp;
		
	// argument check
	if(vectorOrig.size() == 0){
		return;
	}
	
	vectorTarget.clear();
	
	for(it=vectorOrig.begin();it!=vectorOrig.end();it++) 
	{
		string  strName, strVer, strRel, strArch;
		m_refRpmEngine->stripNVRA(it->strFile, &strName, &strVer, &strRel, &strArch);
	
		strTmp=strName + "-" + strVer + "-" + strRel + "." + strArch;
		vectorTarget.push_back(strTmp);
	}

}
// This Method convert from "structFileInfo" to "string"
// Becase the space is limited in console mode 
// and newt library do not support a seperated list
// so we don't show seperated package information
void classTui::struct2str(vector <structFileInfo> vectorOrig,vector <string> &vectorTarget)
{
	vector <structFileInfo>::iterator it;
	string strTmp;
		
	// argument check
	if(vectorOrig.size() == 0){
		return;
	}
	
	vectorTarget.clear();
	
	for(it=vectorOrig.begin();it!=vectorOrig.end();it++) 
	{
		strTmp=it->strName + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch;
		vectorTarget.push_back(strTmp);
	}
}

// This Method convert from convert from "structHeaderInfo" to "string"
// Becase the space is limited in console mode 
// and newt library do not support a seperated list
// so we don't show seperated package information
void classTui::struct2str(set <structHeaderInfo, DereferenceLess> setOrig,vector <string> &vectorTarget)
{
	set <structHeaderInfo, DereferenceLess>::iterator it;
	string strTmp;
		
	// argument check
	if(setOrig.size() == 0){
		return;
	}
	
	vectorTarget.clear();
	
	for(it=setOrig.begin();it!=setOrig.end();it++) 
	{
		strTmp=it->strNVRA;
		vectorTarget.push_back(strTmp);
	}
}

/*!
@brief This method set the title of window
*/
void classTui::set_WinTitle(void)
{
	PROGRAM_MODE pMode=get_ProgramMode();

	switch(pMode){
  		case UPDATE_MODE:
  			newtCenteredWindow(70,15,UPDATE_WIN_TITLE);
  			break;
  		case INSTALL_MODE:
  			newtCenteredWindow(70,15,INSTALL_WIN_TITLE);
  			break;
  		case ERASE_MODE:
  			newtCenteredWindow(70,15,ERASE_WIN_TITLE);
  			break;
		default:
  			newtCenteredWindow(70,15,_("Untitled"));
  			break;
	}
}

/*!
@brief This method make Count string of Update Pkg

@param nWhatKine - 1: To update package except blacklist   2: Blacklisted package
@param nCount - Count of packages
*/
string classTui::make_CountMsg(int nWhatKind,int nCount)
{
	// argument check
	if(nCount < 0){
		return (string)"-1";
	}
	if(!((nWhatKind == 1) || (nWhatKind == 2))){
		return (string)"-1";
	}

	ostringstream osstreamOutput;

	if(nWhatKind == 1){
		osstreamOutput<<TOTAL_COUNT_MSG<<nCount;
#ifdef NDEBUG
		m_refLogger->WriteLog_int(DEBUG_LOG,"TUI amount of the Update Package",nCount,NULL);
#endif /* NDEBUG */
	}else{
		osstreamOutput<<BLACKLISTED_COUNT_MSG<<nCount;
#ifdef NDEBUG
		m_refLogger->WriteLog_int(DEBUG_LOG,"TUI amount of the Blacklisted Package",nCount,NULL);
#endif /* NDEBUG */
		
	}
	return osstreamOutput.str();
}

/*!
This method initialize vectors to need whole process.
It reads "*header.info" file and local rpm information
And check if there are updated packages for this program.
We call this routine as "CheckSelfUpdate()"

@return VECTOR_LOAD_STATE -  Result of loading information
*/
VECTOR_LOAD_STATE classTui::vectorLoad(void)
{
	bool bRet = true;
	VECTOR_LOAD_STATE stateResult = SUCCESS_LOAD;
	PROGRAM_MODE pMode=get_ProgramMode();

	if(pMode != ERASE_MODE){
		m_refRpmEngine->ClearAddedFiles();
		m_refRpmEngine->SetReadRemoteHeaderInfoCallBack(ReadHeadersCallBack);
		m_refRpmEngine->ReadRemoteHeaderInfo();
		m_refRpmEngine->SetReadLocalHeaderInfoCallBack(ReadHeadersCallBack);
		m_refRpmEngine->ReadLocalHeaderInfo();
		// We don't care  incompatible packages when INCMP_CONFIG_FILE file is not exist.
		if(access(INCMP_CONFIG_FILE, F_OK) == 0)
		{
			if(m_refRpmEngine->ReadIncmplistInfo(INCMP_CONFIG_FILE) == false)
			{
				m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI, "Failed to read : ",INCMP_CONFIG_FILE, NULL);								
				stateResult = FAIL_LOAD;
				goto ret;
			}
		}
				
		m_refRpmEngine->SetCreateUpdateInstallListCallBack(CommonCallBack);
		bRet=m_refRpmEngine->CreateUpdateInstallList();
		if(!bRet){
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot Create UpdateInstallList",NULL);
			stateResult = FAIL_LOAD;
			goto ret;
		}

		if(m_refRpmEngine->CheckSelfUpdate()){
#ifdef NDEBUG
			m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Find Self Update Pkgs",NULL);
#endif /* NDEBUG */
			m_nUpdateMethod = SELF_UPDATE;
			bRet=popup_SelfUpdateMsg();
			if(!bRet){
				stateResult = CANCEL_SELF_UPDATE;
				goto ret;
			}
		}
			
		m_refNetwork->ClearPackages();
	
		m_struct_vectorSelList.clear();
		m_struct_vectorDoneList.clear();
		m_string_vectorDepList.clear();
		
		m_struct_vectorUpdateList.clear();		
		m_struct_vectorUpdateList=m_refRpmEngine->GetUpdateList();
		
		m_string_vectorUpdateList.clear();
		struct2str(m_struct_vectorUpdateList,m_string_vectorUpdateList);
	
		m_struct_vectorInstallList.clear();
		m_struct_vectorInstallList=m_refRpmEngine->GetInstallList();
		
		m_string_vectorInstallList.clear();
		struct2str(m_struct_vectorInstallList,m_string_vectorInstallList);
		
		m_struct_setEraseList.clear();
		m_struct_setEraseSelList.clear();
		m_struct_setEraseDoneList.clear();
		m_struct_setEraseList=m_refRpmEngine->GetInstalledList();
	}else{
                m_refRpmEngine->ClearAddedFiles();                                		
                m_refRpmEngine->SetReadLocalHeaderInfoCallBack(ReadHeadersCallBack);
                m_refRpmEngine->ReadLocalHeaderInfo();

                m_struct_vectorSelList.clear();
                m_struct_vectorDoneList.clear();
                m_string_vectorDepList.clear();

                m_struct_vectorUpdateList.clear();
                m_string_vectorUpdateList.clear();
                m_struct_vectorInstallList.clear();
                m_string_vectorInstallList.clear();

                m_struct_setEraseList.clear();
                m_struct_setEraseSelList.clear();
                m_struct_setEraseDoneList.clear();
                m_struct_setEraseList=m_refRpmEngine->GetInstalledList();
	}

ret:
	return stateResult;
}

// This method get the added list by dependency
vector <string> classTui::GetDepList(vector <string> &str_vectorArg)
{
	vector <structAddedFile>::iterator it;
	vector <structAddedFile> vectorByDepArch;
	
	char *szTemp = NULL;
	
	szTemp=(char*)calloc(sizeof(char),BUFF_SZ);
	if(szTemp == NULL){
                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                return str_vectorArg;
	}

	vectorByDepArch=m_refRpmEngine->GetAddedFile(REQDEP | OTHERDEP,vectorByDepArch);

	for(it=vectorByDepArch.begin();it != vectorByDepArch.end();it++)
	{
#ifdef NDEBUG
		m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"vectorByDepArch",it->strFile,NULL);
#endif /* NDEBUG */
		memset(szTemp,0,BUFF_SZ);
		pathFilter(it->strFile,szTemp);
#ifdef NDEBUG
		m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Dependency Pkg",szTemp,NULL);
#endif /* NDEBUG */
		str_vectorArg.push_back((string)szTemp);
	}
	
	if(szTemp != NULL){
		free(szTemp);
		szTemp = NULL;
	}

	return str_vectorArg;
}

/*! 
@brief manage whloe screen process

@return int - 0:Success 1:Fail -1:retry
*/
int classTui::show_ui(void)
{
	int nResult = SUCCESS_FLAG;
	PAGE pExitCode=INIT_VALUE;

	// Session buffer
	// A seesion means a page when you run this program
	// The session buffer has some elements, such as selected items
	struct structSessionBuffer *r_SessionBuf=NULL;

	// get current program mode - update or install or erase
	PROGRAM_MODE pMode=get_ProgramMode();
	
	r_SessionBuf=(structSessionBuffer*)malloc(sizeof(struct structSessionBuffer));
	if(r_SessionBuf == NULL){
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
  		return FAIL_FLAG;
	}else{
		memset(r_SessionBuf,0,sizeof(struct structSessionBuffer));
	}

	// Initialize Actions(ex.Update or Install or Erase Actions) Flag
	// If this flag is set, it needs to call "vectorLoad()"
	m_bIsAction=false;
	
	// send a command to RpmEngine
	pExitCode = set_RpmCommand(pMode);
	
	// Main-loop of UI
	while(pExitCode != PAGE_EXIT)
	{	
		switch(pExitCode){
			case INIT_VALUE:
				pExitCode = PAGE_EXIT;
				nResult = FAIL_FLAG;
				break;
			case PAGE_BACK:
				if(!m_stackBuffer.empty()){
					memset(r_SessionBuf,0,sizeof(structSessionBuffer));
					memcpy(r_SessionBuf,&m_stackBuffer.top(),sizeof(structSessionBuffer));
					m_stackBuffer.pop();
				}
				// get previous page
				pExitCode=r_SessionBuf->SessionId;
				break;
			case UPDATE_METHOD_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter UPDATE_METHOD_PAGE",NULL);
#endif /* NDEBUG */
				// get out of all data of m_stackBuffer
				while(!m_stackBuffer.empty()){
		  			m_stackBuffer.pop();
		  			memset(r_SessionBuf,0,sizeof(structSessionBuffer));
				}
				m_nUpdateMethod = TOTAL_UPDATE;
				pExitCode=show_UpdateMethodPage(r_SessionBuf);
				break;
			case UPDATE_INSTALL_PROGRESS_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter UPDATE_INSTALL_PROGRESS_PAGE",NULL);
#endif /* NDEBUG */
				pExitCode=show_UpdateInstallProgressPage();
				m_bIsAction=false;
				break; 
			case ERASE_PROGRESS_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter ERASE_PROGRESS_PAGE",NULL);
#endif /* NDEBUG */
				pExitCode=show_EraseProgressPage();
				m_bIsAction=false;
				break; 
			case UPDATE_INSTALL_LIST_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter UPDATE_INSTALL_LIST_PAGE",NULL);
#endif /* NDEBUG */
				if(pMode == INSTALL_MODE){
					while(!m_stackBuffer.empty()){
			  			m_stackBuffer.pop();
			  			memset(r_SessionBuf,0,sizeof(structSessionBuffer));
					}
				}
				pExitCode=show_UpdateInstallListPage();
				break;
			case ERASE_LIST_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter ERASE_LIST_PAGE",NULL);
#endif /* NDEBUG */
				while(!m_stackBuffer.empty()){
		  			m_stackBuffer.pop();
		  			memset(r_SessionBuf,0,sizeof(structSessionBuffer));
				}
				pExitCode=show_EraseListPage();
				break;
			case DONE_OK_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter DONE_OK_PAGE",NULL);
#endif /* NDEBUG */
				pExitCode=show_DonePage(true);
				break;
			case DONE_FAIL_PAGE:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"Enter DONE_FAIL_PAGE",NULL);
#endif /* NDEBUG */
				pExitCode=show_DonePage(false);
				break;
			case PROGRAM_RESTART:
#ifdef NDEBUG
				m_refLogger->WriteLog_char(DEBUG_LOG,MYSELF_NAME_TUI,"PROGRAM_RESTART",NULL);
#endif /* NDEBUG */
				goto restart;
				break;
			default:
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Unknown Page",NULL);
				break;
		}
	}

restart:
	newtFinished();
	if(r_SessionBuf != NULL){
		free(r_SessionBuf);
		r_SessionBuf=NULL;
	}

	if((nResult != FAIL_FLAG) && (pExitCode == PROGRAM_RESTART)){
		nResult = updater_restart();
		if(nResult != SUCCESS_FLAG){
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot restart the AXTU",NULL);
			show_err_msg_on_tui(_("Cannot restart the AXTU."),NULL);
		}
	}
	
	return nResult;
}

/*!
@brief show a loading page of this program.

should be loaded some information when execute this program. 
the information is something about local rpmdb, server rpm info,... 
@return bool - information loading ok? or not?
*/
VECTOR_LOAD_STATE classTui::show_LoadPage(void)
{
        newtComponent form_Progress;
        newtComponent label_ProgressMsg;

        struct structSessionBuffer sBuffer;
        int bRet;

        string strFullPath;
        string strUrlFullPath;

        vector<structFileInfo>::iterator it;
        vector <structFileInfo> vectorTemp;
        vector <structFileInfo> vectorUpdate;
        vector <structFileInfo> vectorInstall;

	VECTOR_LOAD_STATE loadState = SUCCESS_LOAD;


        vectorUpdate = m_refRpmEngine->GetUpdateList();
        vectorInstall = m_refRpmEngine->GetInstallList();


        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(HELP_LINE);
        set_WinTitle();

        label_ProgressMsg=newtLabel(1,1,PROGRESS_PAGE_LABEL);

        g_label_TotalProgressMsg=newtLabel(1,4,TOTAL_PROGRESS_MSG);
        g_scale_TotalProgress=newtScale(5,5,60,100);
        g_label_CurrentProgressMsg=newtLabel(1,8,CURRENT_PROGRESS_MSG);
        g_scale_CurrentProgress=newtScale(5,9,60,100);

        form_Progress = newtForm(NULL,NULL,NEWT_FLAG_NOF12);

        newtFormAddComponents(form_Progress,    \
                        g_label_TotalProgressMsg,g_scale_TotalProgress, \
                        g_label_CurrentProgressMsg,g_scale_CurrentProgress,     \
                        NULL);

        btn_Cancel = newtButton(50,11,BTN_CANCEL_MSG);

        g_form_Progress=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

        newtFormAddComponents(g_form_Progress,label_ProgressMsg,     \
                       form_Progress,NULL);
        newtRefresh();
        newtFormSetTimer(g_form_Progress,50);

        newtFormRun(g_form_Progress,&g_es_Progress);

	loadState = vectorLoad();
	if(loadState == FAIL_LOAD){
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"vectorLoad Fail",NULL);
		loadState = FAIL_LOAD;
                newtWinMessage(ERROR_MSG,BTN_OK_MSG,CREATELIST_FAIL_MSG);
	}

	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(g_form_Progress);

	return loadState;
}

void classTui::FilterBlacklist(vector <structFileInfo> &vectorFiltered, vector <structFileInfo> &vectorBlacklist, vector <structFileInfo> &vectorIncompatible)
{
	vector<structFileInfo>::iterator it;
	vectorFiltered.clear();
	vectorBlacklist.clear();
	vectorIncompatible.clear();

	for(it=m_struct_vectorUpdateList.begin();it!=m_struct_vectorUpdateList.end();it++){
		if(it->bIncompatible)
		{
			vectorIncompatible.push_back(*it);			
		}
		else if(it->bBlacklisted)
		{
			vectorBlacklist.push_back(*it);
		}
		else
		{		
			vectorFiltered.push_back(*it);
		}
	}

}

/*!
@brief show the page to select the Update Method - total or custom

@param structBuffer - Screen Buffer
@return int - next page number
@see structSessionBuffer
*/
PAGE classTui::show_UpdateMethodPage(struct structSessionBuffer *structBuffer)
{
        newtComponent form;
        newtComponent label_IsPkgMsg,label_TotalCount,label_BlacklistedCount;
        newtComponent label_SelectMethodMsg;
        newtComponent listbox_Menu;
        newtComponent listbox_form;
        newtComponent btn_Next,btn_BList;
	newtComponent btn_ViewUpdatePkgs;
        
        PAGE pExitCode=INIT_VALUE;
        struct structSessionBuffer sBuffer;
        struct newtExitStruct es;
        int nUpdatePkgCount = 0,nBlacklistedpkgCount = 0;
	vector <string> string_vectorFilteredList;
	vector <string> string_vectorIncompatibleList;
	VECTOR_LOAD_STATE vectorState;

	if(!m_bIsAction){
		vectorState = show_LoadPage();
		if(vectorState == FAIL_LOAD){
			pExitCode=PAGE_EXIT;
			goto ret;
		}else if(vectorState == CANCEL_SELF_UPDATE){
			pExitCode=show_DonePage(USER_CANCEL);
			goto ret;
		}else{
			m_bIsAction=true;
		}
		if(m_nUpdateMethod == SELF_UPDATE){
			return UPDATE_INSTALL_LIST_PAGE;
		}
	}

        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(HELP_LINE);
	set_WinTitle();

	FilterBlacklist(m_struct_vectorFilteredList, m_struct_vectorBlackList, m_struct_vectorIncompatibleList);
	struct2str(m_struct_vectorFilteredList,string_vectorFilteredList);
	struct2str(m_struct_vectorIncompatibleList,string_vectorIncompatibleList);	

	nUpdatePkgCount = m_struct_vectorFilteredList.size() + m_struct_vectorIncompatibleList.size();
	nBlacklistedpkgCount = m_struct_vectorBlackList.size();
	if(nBlacklistedpkgCount < 0){
		nBlacklistedpkgCount = 0;
	}

	if(nUpdatePkgCount == 0){
        	label_IsPkgMsg=newtLabel(1,1,NOT_EXIST_UPDATE_LIST);
	}else if(nUpdatePkgCount > 0){
		label_IsPkgMsg=newtLabel(1,1,EXIST_UPDATE_LIST);
	}
	
	label_TotalCount=newtLabel(1,2,(make_CountMsg(1,nUpdatePkgCount)).c_str());
	label_BlacklistedCount=newtLabel(1,3,(make_CountMsg(2,nBlacklistedpkgCount)).c_str());
   	label_SelectMethodMsg=newtLabel(1,5,UPDATE_METHOD_SEL_MSG);

  	btn_ViewUpdatePkgs = newtCompactButton(25,2,BTN_VIEW_LIST_MSG);

	listbox_Menu = newtListbox(5,7,2,NEWT_FLAG_RETURNEXIT);

	newtListboxAppendEntry(listbox_Menu,TOTAL_UPDATE_MENU,(void*)1);
	newtListboxAppendEntry(listbox_Menu,CUSTOM_UPDATE_MENU,(void*)2);

	newtListboxSetCurrent(listbox_Menu,(structBuffer->nData)-1);

        listbox_form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(listbox_form,listbox_Menu,NULL);
        newtFormSetBackground(listbox_form,NEWT_COLORSET_CHECKBOX);

        btn_Next = newtButton(10,11,BTN_NEXT_MSG);
        btn_BList = newtButton(25,11,BTN_BLACKLIST_MSG);
        btn_Cancel = newtButton(50,11,BTN_CANCEL_MSG);

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_IsPkgMsg,label_TotalCount,	\
				label_BlacklistedCount,		\
        			label_SelectMethodMsg,listbox_form,btn_Next, \
                		btn_BList,btn_Cancel,btn_ViewUpdatePkgs,NULL);

        do{
                newtFormRun(form,&es);

	  	if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_ViewUpdatePkgs){
    			popup_UpdateList(string_vectorFilteredList, string_vectorIncompatibleList);
                }else if((es.reason == es.NEWT_EXIT_COMPONENT) && (es.u.co != btn_Cancel) && (es.u.co != btn_BList)){
                        m_nUpdateMethod=(long)newtListboxGetCurrent(listbox_Menu);
                        switch(m_nUpdateMethod){
                                case TOTAL_UPDATE:
	       				sBuffer.nData=m_nUpdateMethod;
					if(m_struct_vectorFilteredList.size()==0){
//						popup_UpdateList(string_vectorFilteredList);
						if(m_struct_vectorIncompatibleList.size() > 0)
							popup_ConfirmWin_For_Incompatible(m_struct_vectorIncompatibleList);
						pExitCode=show_DonePage(NONE_AVAILABLE_PKG);
					}else{
						pExitCode=UPDATE_INSTALL_LIST_PAGE;
					}
                                        break;
                                case CUSTOM_UPDATE:
	       				sBuffer.nData=m_nUpdateMethod;
	                                pExitCode=UPDATE_INSTALL_LIST_PAGE;
                                        break;
                        }
			sBuffer.SessionId=UPDATE_METHOD_PAGE;
			m_stackBuffer.push(sBuffer);
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_BList){
                        newtSuspend();
			system(TUI_SETUP_PATH);
			if(shm_check()){
				m_bIsAction=false;
				pExitCode=UPDATE_METHOD_PAGE;
			}
			newtResume();
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
			pExitCode=PAGE_EXIT;
       		}
        }while(pExitCode < 0);
        
	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(form);
ret:    
        return pExitCode;
}

/*!
@brief show Update or Install List

@return int - next page number
*/
PAGE classTui::show_UpdateInstallListPage(void)
{
	newtComponent form;
	newtComponent label_ListMsg;
	newtComponent form_List;
	newtComponent form_AllCheck;
	newtComponent chklist_AllCheck;
	newtComponent *chklist_Item = NULL;
	newtComponent blank;
	newtComponent btn_Prev,btn_Next,btn_Reload,btn_BList;
	newtComponent scrollbar;
	newtComponent curr;
	newtGrid grid,hSubgrid,buttons;
	struct newtExitStruct es;

	char prgAllSelResults;
	char *prgResults = NULL;
	int nSize=0;
	int nCount=0;
	int nSelCount=0;
	PAGE pExitCode = INIT_VALUE;

	struct structSessionBuffer sBuffer;
	struct callbackInfo IsCheck;

	vector<string>::iterator it1;
	vector<structFileInfo>::iterator it2;
	vector<string> string_vectorTemp;
	vector<structFileInfo> struct_vectorTemp;

	VECTOR_LOAD_STATE vectorState;
	PROGRAM_MODE pMode=get_ProgramMode();
	


	if(!m_bIsAction){
                vectorState = show_LoadPage();
                if(vectorState == FAIL_LOAD){
                        pExitCode=PAGE_EXIT;
                        return pExitCode;
                }else if(vectorState == CANCEL_SELF_UPDATE){
                        pExitCode=show_DonePage(USER_CANCEL);
                        return pExitCode;
                }else{
                        m_bIsAction=true;
                }
	}

	if(!m_bIsHeaderWork){
		if(!show_HeaderWorkProgressPage()){
			pExitCode=PAGE_BACK;
			goto ret;
		}else{
			m_bIsHeaderWork = true;
		}
	}

        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(F1_HELP_LINE);

	// Make the message
  	if(pMode == UPDATE_MODE){
  		nSize=(int)m_string_vectorUpdateList.size();
  		string_vectorTemp=m_string_vectorUpdateList;
  		struct_vectorTemp=m_struct_vectorUpdateList;

  		if(nSize != 0){
  			label_ListMsg=newtLabel(-1,-1,UPDATE_EXIST_MSG);
  		}else{
  			label_ListMsg=newtLabel(-1,-1,UPDATE_NO_EXIST_MSG);
  		}
  	}else if(pMode == INSTALL_MODE){
  		nSize=(int)m_string_vectorInstallList.size();
  		string_vectorTemp=m_string_vectorInstallList;
  		struct_vectorTemp=m_struct_vectorInstallList;

  		if(nSize != 0){
  			label_ListMsg=newtLabel(-1,-1,INSTALL_EXIST_MSG);
  		}else{
  			label_ListMsg=newtLabel(-1,-1,INSTALL_NO_EXIST_MSG);
  		}
  	}

	chklist_Item = (newtComponent*)calloc(sizeof(newtComponent),nSize);
	if(chklist_Item == NULL){
                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                newtPopHelpLine();
                pExitCode = INIT_VALUE;
		goto ret;
        }

	prgResults=(char*)calloc(sizeof(char),nSize);
	if(prgResults == NULL){
                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                newtPopHelpLine();
                pExitCode = INIT_VALUE;
		goto ret;
        }

	scrollbar=newtVerticalScrollbar(-1,-1,7,NEWT_COLORSET_CHECKBOX,NEWT_COLORSET_ACTCHECKBOX);
	
	form_List = newtForm(scrollbar,NULL,NEWT_FLAG_NOF12);

	newtFormSetBackground(form_List,NEWT_COLORSET_CHECKBOX);
	newtFormSetHeight(form_List,7);

	for(it1=string_vectorTemp.begin(),it2=struct_vectorTemp.begin();	\
					it1 != string_vectorTemp.end();it1++,it2++)
	{
		if(it2->bIncompatible && (pMode == UPDATE_MODE)) {
			chklist_Item[nCount]=newtCheckbox(-1,4+nCount,it1->c_str(),'i',"ii",NULL);
		}
		else if(it2->bBlacklisted && (pMode == UPDATE_MODE)){
			chklist_Item[nCount]=newtCheckbox(-1,4+nCount,it1->c_str(),'b',"bb",NULL);
		}
		else{
			chklist_Item[nCount]=newtCheckbox(-1,4+nCount,it1->c_str(),' ',NULL,&prgResults[nCount]);
		}
		newtFormAddComponent(form_List,chklist_Item[nCount]);
   		nCount++;
   	}

        newtFormSetWidth(form_List,60);
	newtFormSetBackground(form_List,NEWT_COLORSET_CHECKBOX);

        blank=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormSetWidth(blank,2);
        newtFormSetHeight(blank,7);
        newtFormSetBackground(blank,NEWT_COLORSET_CHECKBOX);

        hSubgrid=newtGridHCloseStacked(NEWT_GRID_COMPONENT,form_List,    \
                                        NEWT_GRID_COMPONENT,blank,      \
                                        NEWT_GRID_COMPONENT,scrollbar,NULL);

	// AllCheckbox
        form_AllCheck=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        chklist_AllCheck=newtCheckbox(-1,0,SEL_ALL_ITEM_MSG,' ',NULL,&prgAllSelResults);
        newtFormAddComponents(form_AllCheck,label_ListMsg,chklist_AllCheck,NULL);
	
	IsCheck.state=&prgAllSelResults;
	IsCheck.en=chklist_Item;
	IsCheck.figure=nCount;
	newtComponentAddCallback(chklist_AllCheck,AllCheckCallback,&IsCheck);

	// Buttons
	if(pMode == UPDATE_MODE){
  		buttons=newtButtonBar(BTN_PREV_MSG,&btn_Prev,BTN_NEXT_MSG,&btn_Next,BTN_BLACKLIST_MSG,	\
				&btn_BList,BTN_RELOAD_MSG,&btn_Reload,BTN_CANCEL_MSG,&btn_Cancel,NULL);
	}else{
  		buttons=newtButtonBar(BTN_NEXT_MSG,&btn_Next,BTN_RELOAD_MSG,&btn_Reload,	\
								BTN_CANCEL_MSG,&btn_Cancel,NULL);
	}

	// Make the grid
  	grid = newtGridBasicWindow(form_AllCheck,hSubgrid,buttons);

  	form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

  	newtGridAddComponentsToForm(grid,form,1);

	if(pMode == UPDATE_MODE){
  		newtGridWrappedWindow(grid,UPDATE_WIN_TITLE);
	}else if(pMode == INSTALL_MODE){
		newtGridWrappedWindow(grid,INSTALL_WIN_TITLE);
	}

	newtGridFree(grid,1);
	newtFormSetTimer(form,BUFF_SZ);
	newtFormAddHotKey(form,NEWT_KEY_F1);
	newtFormSetCurrent(form,form_List);

	do{
		if((pMode == UPDATE_MODE) && (m_nUpdateMethod == TOTAL_UPDATE)	&& (nSize != 0)){
			if(m_struct_vectorIncompatibleList.size() > 0)
				popup_ConfirmWin_For_Incompatible(m_struct_vectorIncompatibleList);
			if(popup_ConfirmWin(m_struct_vectorFilteredList)){
				pExitCode=UPDATE_INSTALL_PROGRESS_PAGE;
			}else{
				m_struct_vectorFilteredList.clear();
                        	pExitCode=PAGE_BACK;
                        }
			break;
		}else if(m_nUpdateMethod == SELF_UPDATE){
			pExitCode=UPDATE_INSTALL_PROGRESS_PAGE;
			break;
		}else{
  			newtFormRun(form,&es);
		}
 
		if(es.reason == es.NEWT_EXIT_HOTKEY){
			if(es.u.key == NEWT_KEY_F1){
				curr = newtFormGetCurrent(form);
				if(curr == form_List){
					for(nCount=0;nCount < nSize;nCount++){
						curr = newtFormGetCurrent(form_List);
						if(curr == chklist_Item[nCount]){
							structRPMInfo *result = NULL;
                					result = m_refRpmEngine->FindHeaderFromRemote(			  \
								(char *)(struct_vectorTemp[nCount].strName).c_str(), \
								(char *)(struct_vectorTemp[nCount].strVersion).c_str(),  \
								(char *)(struct_vectorTemp[nCount].strRelease).c_str(),result);
							
							if(result != NULL){
								popup_PkgSummary((struct_vectorTemp[nCount].strName).c_str(),result->summary,result->shortDesp);
							}
							break;
						}
					}
				}
			}
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Next){
			nSelCount=0;
			for(nCount = 0;nCount < nSize;nCount++){
				if(prgResults[nCount] == '*')	nSelCount++;
  			}
  			
  			if(nSelCount == 0){
				newtWinMessage(WARNING_MSG,BTN_OK_MSG,ERR_NO_SELECT_MSG);
  				continue;
  			}else{
				nSelCount=0;
				for(nCount = 0;nCount < nSize;nCount++){
					if(prgResults[nCount] == '*'){
						m_struct_vectorSelList.push_back(struct_vectorTemp[nCount]);
						nSelCount++;
					}
				}

				if(popup_ConfirmWin(m_struct_vectorSelList)){
  					sBuffer.SessionId=UPDATE_INSTALL_LIST_PAGE;
					pExitCode=UPDATE_INSTALL_PROGRESS_PAGE;
					m_stackBuffer.push(sBuffer);
				}else{
					m_struct_vectorSelList.clear();
				}
  			}
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_BList){
                        newtSuspend();
			int nSystemResult;
			nSystemResult = system(TUI_SETUP_PATH);
			newtResume();
			if(nSystemResult == 0){
				if(shm_check()){
					m_bIsAction=false;
					pExitCode=UPDATE_INSTALL_LIST_PAGE;
				}
			}else{
				m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
				newtWinMessage(ERROR_MSG,BTN_OK_MSG,_("Cannot execute the setup program for blacklist."));
			}
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Reload){
			m_bIsAction=false;
			pExitCode=UPDATE_INSTALL_LIST_PAGE;
    		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
    			pExitCode=PAGE_EXIT;
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && pMode == UPDATE_MODE){
			if(es.u.co == btn_Prev){
				pExitCode=PAGE_BACK;
			}
		}
	}while(pExitCode < 0);

	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(form);

ret:
	if(chklist_Item != NULL){
		free(chklist_Item);
		chklist_Item = NULL;
	}

	if(prgResults != NULL){
		free(prgResults);
		prgResults = NULL;
	}

  	return pExitCode;
}

/*!
@brief show installed-list for ErasePage

@return int - next page number
*/
PAGE classTui::show_EraseListPage(void)
{
	newtComponent form;
	newtComponent label_ListMsg;
	newtComponent form_List;
	newtComponent *chklist_Item = NULL;
	newtComponent blank;
	newtComponent btn_Next,btn_Reload;
	newtComponent scrollbar;
	newtComponent curr;
	newtGrid grid,hSubgrid,buttons;
	Header header;
	
	char *prgResults = NULL;
	struct structSessionBuffer sBuffer;
	struct newtExitStruct es;
	int nSize=0;
	int nCount=0;
	PAGE pExitCode = INIT_VALUE;
	int nSelCount=0;
	
	const char *description = NULL;
   	const char *summary = NULL;
   
   	string strNameTemp,strVerTemp,strRelTemp,strArchTemp;
	set <structHeaderInfo, DereferenceLess>::iterator erase_it;
	VECTOR_LOAD_STATE vectorState;
	
	if(!m_bIsAction){
                vectorState = show_LoadPage();
                if(vectorState == FAIL_LOAD){
                        pExitCode=PAGE_EXIT;
			return pExitCode;
                }else if(vectorState == CANCEL_SELF_UPDATE){
                        pExitCode=show_DonePage(USER_CANCEL);
			return pExitCode;
                }else{
                        m_bIsAction=true;
                }
	}

        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(F1_HELP_LINE);
  
	nSize=(int)m_struct_setEraseList.size();
	
	if(nSize != 0){
		label_ListMsg=newtLabel(-1,-1,ERASE_EXIST_MSG);
	}else{
		label_ListMsg=newtLabel(-1,-1,ERASE_NO_EXIST_MSG);
	}


	chklist_Item = (newtComponent*)calloc(sizeof(newtComponent),nSize);
        if(chklist_Item == NULL){
                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                newtPopHelpLine();
                pExitCode = INIT_VALUE;
		return pExitCode;
        }

        prgResults=(char*)calloc(sizeof(char),nSize);
        if(prgResults == NULL){
                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                newtPopHelpLine();
		if(chklist_Item != NULL){
                	free(chklist_Item);
			chklist_Item = NULL;
        	}
                pExitCode = INIT_VALUE;
		return pExitCode;
        }

	scrollbar=newtVerticalScrollbar(-1,-1,7,NEWT_COLORSET_CHECKBOX,NEWT_COLORSET_ACTCHECKBOX);
	
	form_List = newtForm(scrollbar,NULL,NEWT_FLAG_NOF12);

	newtFormSetBackground(form_List,NEWT_COLORSET_CHECKBOX);
	newtFormSetHeight(form_List,7);
	newtFormSetWidth(form_List,60);

	for(erase_it=m_struct_setEraseList.begin();erase_it != m_struct_setEraseList.end();erase_it++)
	{
		chklist_Item[nCount]=newtCheckbox(-1,3+nCount,(erase_it->strNVRA).c_str(),' ',NULL,&prgResults[nCount]);
		newtFormAddComponent(form_List,chklist_Item[nCount]);
   		nCount++;
   	}

	blank=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
	newtFormSetWidth(blank,2);
	newtFormSetHeight(blank,7);
	newtFormSetBackground(blank,NEWT_COLORSET_CHECKBOX);
	
	hSubgrid=newtGridHCloseStacked(NEWT_GRID_COMPONENT,form_List,	\
					NEWT_GRID_COMPONENT,blank,	\
  					NEWT_GRID_COMPONENT,scrollbar,NULL);
	
	// Buttons
 	buttons=newtButtonBar(BTN_NEXT_MSG,&btn_Next,BTN_RELOAD_MSG,&btn_Reload,BTN_CANCEL_MSG,&btn_Cancel,NULL);

	// Make the grid
	grid = newtGridBasicWindow(label_ListMsg,hSubgrid,buttons);

  	form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

  	newtGridAddComponentsToForm(grid,form,1);

  	newtGridWrappedWindow(grid,ERASE_WIN_TITLE);

	newtGridFree(grid,1);
	newtFormSetTimer(form,BUFF_SZ);
	newtFormAddHotKey(form,NEWT_KEY_F1);
	set <structHeaderInfo, DereferenceLess>::iterator it;
	do{
  		newtFormRun(form,&es);
  	
		if(es.reason == es.NEWT_EXIT_HOTKEY){
			if(es.u.key == NEWT_KEY_F1){
				curr = newtFormGetCurrent(form_List);
				
				it = m_struct_setEraseList.begin();
				for(nCount=0;nCount<nSize;nCount++)
				{
					if(curr == chklist_Item[nCount])	break;
					it++;
				}
									
            			m_refRpmEngine->stripNVRA((it->strNVRA).c_str(), &strNameTemp, &strVerTemp, &strRelTemp, &strArchTemp);
				header = m_refRpmEngine->FindHeaderFromLocal((it->strNVRA).c_str(), header);
            			headerGetEntry(header, RPMTAG_DESCRIPTION, NULL, (void **)&description, NULL);
            			headerGetEntry(header, RPMTAG_SUMMARY, NULL, (void **)&summary, NULL);
            			popup_PkgSummary(strNameTemp.c_str(),summary,description);
			}
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Next){

			nSelCount=0;

			for(nCount = 0;nCount < nSize;nCount++){
				if(prgResults[nCount] == '*')	nSelCount++;
  			}
  			
  			if(nSelCount == 0){
				newtWinMessage(WARNING_MSG,BTN_OK_MSG,ERR_NO_SELECT_MSG);
  				continue;
  			}else{
				nSelCount=0;
				for(nCount=0,it=m_struct_setEraseList.begin();it!=m_struct_setEraseList.end();it++,nCount++){
					if(prgResults[nCount] == '*'){
						m_struct_setEraseSelList.insert(*it);
						nSelCount++;
					}
				}

				if(popup_ConfirmWin(m_struct_setEraseSelList)){
  					sBuffer.SessionId=ERASE_LIST_PAGE;
					pExitCode=ERASE_PROGRESS_PAGE;
					m_stackBuffer.push(sBuffer);
				}else{
					m_struct_setEraseSelList.clear();
//					pExitCode=PAGE_BACK;
				}
  			}
		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Reload){
			m_bIsAction=false;
			pExitCode=ERASE_LIST_PAGE;
    		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
    			pExitCode=PAGE_EXIT;
        	}
	}while(pExitCode < 0);

	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(form);

	if(chklist_Item != NULL){
                free(chklist_Item);
		chklist_Item = NULL;
        }
        if(prgResults != NULL){
                free(prgResults);
		prgResults = NULL;
        }

  	return pExitCode;
}

/*! 
@brief Download and get the *.hdr files

show Rpm-Header(*.hdr) related actions to get a package description
@return bool - Success or not
*/
bool classTui::show_HeaderWorkProgressPage(void)
{
	newtComponent form_Progress;
	newtComponent label_ProgressMsg;
	
	vector <structFileInfo> vectorTemp;
	vector<structFileInfo>::iterator it;
	vector <structFileInfo> vectorUpdate;
	vector <structFileInfo> vectorInstall;

	string strFullPath;
	string strUrlFullPath;

	struct structSessionBuffer sBuffer;
	char buffer[1000]={0};
	PAGE pExitCode=INIT_VALUE;
	int nRet;
	bool bRet;



	vectorUpdate = m_refRpmEngine->GetUpdateList();
  	vectorInstall = m_refRpmEngine->GetInstallList();
			
        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(HELP_LINE);
        set_WinTitle();
	
	label_ProgressMsg=newtLabel(1,1,PROGRESS_PAGE_LABEL);
	
  	g_label_TotalProgressMsg=newtLabel(1,4,TOTAL_PROGRESS_MSG);
  	g_scale_TotalProgress=newtScale(5,5,60,100);
  
  	g_label_CurrentProgressMsg=newtLabel(1,8,CURRENT_PROGRESS_MSG);
  	g_scale_CurrentProgress=newtScale(5,9,60,100);
  
  	form_Progress = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
  	newtFormAddComponents(form_Progress,	\
   			g_label_TotalProgressMsg,g_scale_TotalProgress,	\
  			g_label_CurrentProgressMsg,g_scale_CurrentProgress,	\
  			NULL);
  	

  	btn_Cancel = newtButton(50,11,BTN_CANCEL_MSG);

  	g_form_Progress=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

  	newtFormAddComponents(g_form_Progress,label_ProgressMsg,     \
  	                form_Progress,btn_Cancel,NULL);

  	newtRefresh();
  	newtFormSetTimer(g_form_Progress,50);
  
  	newtFormRun(g_form_Progress,&g_es_Progress);
  
	// Set the CallBack Functions  	
	m_refNetwork->SetGetHeadersCallBack(GetHeadersCallBack);
	m_refRpmEngine->SetReadHeadersCallBack(ReadHeadersCallBack);
		
	// Set current Network class to rpmEngine.
	m_refRpmEngine->SetNetwork(m_refNetwork);
	
	/*
	if(m_refRpmEngine->CheckSelfUpdate() == false){
		newtSuspend();
		m_refNetwork->CheckAuthen();
		m_refConfCtl->ConfigCheck();
		newtResume();
	}
	*/

	//m_refNetwork->SetDownloadConfig();

	// Get *.hdr files
	nRet=m_refNetwork->GetHeaders(vectorUpdate, vectorInstall);
#ifdef NDEBUG
	m_refLogger->WriteLog_int(DEBUG_LOG,"Download",nRet,NULL);
#endif /* NDEBUG */

	if(nRet != NETWORK_RETOK){
		show_NetworkErrorDialog(nRet);
		bRet=false;
		goto ret;
	}else{
		bRet=true;
	}
			
	// Read *.hdr files
	nRet = m_refRpmEngine->ReadHeaders();

	do{
		newtFormRun(g_form_Progress,&g_es_Progress);
		if(g_es_Progress.reason == g_es_Progress.NEWT_EXIT_COMPONENT && g_es_Progress.u.co == btn_Cancel){
			pExitCode=PAGE_BACK;
		}

		if(nRet==0){
			pExitCode=PAGE_EXIT;
		}
	}while(pExitCode < 0);
ret:
	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(g_form_Progress);
	
	return bRet;
}

bool classTui::popup_ConfirmWinRemoveKernel(vector <string> struct_vectorArg)
{

	string strMsg;
	strMsg += REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG;
        strMsg += "\n\n";

	vector <string> string_vectorSelList;
	string_vectorSelList = struct_vectorArg;
	string  strName, strVer, strRel, strArch;
	string strKmodName;
	vector<string>::iterator it;
        for(it=string_vectorSelList.begin();it != string_vectorSelList.end();it++)
	{
		m_refRpmEngine->stripNVRA(*it, &strName, &strVer, &strRel, &strArch);
		strKmodName = strName + "-" +  strVer + "-" + strRel + "." + strArch;

		strMsg += "    -";
		strMsg += strKmodName;
		strMsg += "\n";
	}
	

	
	newtWinMessage(WARNING_MSG,BTN_OK_MSG,(char *)strMsg.c_str());

	return true;	
}
bool classTui::popup_ConfirmWin(vector <structFileInfo> struct_vectorArg)
{
	vector<string>::iterator it;
	vector <string> string_vectorSelList;

        newtComponent listbox_SelList;
        newtComponent label_SelMsg1,label_SelMsg2;
        newtComponent btn_Yes,btn_No;
        newtComponent form;
	newtComponent checklist_ShowDepDlg;
        struct newtExitStruct es;
        PAGE pExitCode=INIT_VALUE;
        bool bRetVal;
	char prgSelResult;

        newtCenteredWindow(60,15,CONFIRM_WIN_TITLE);

        label_SelMsg1=newtLabel(1,1,CONFIRM_MSG);
        label_SelMsg2=newtLabel(1,2,CONTINUE_MSG);
        listbox_SelList=newtListbox(1,3,5,NEWT_FLAG_SCROLL);
        newtListboxSetWidth(listbox_SelList,56);

	struct2str(struct_vectorArg,string_vectorSelList);

        for(it=string_vectorSelList.begin();it != string_vectorSelList.end();it++){
                newtListboxAppendEntry(listbox_SelList,(*it).c_str(),NULL);
        }

	checklist_ShowDepDlg = newtCheckbox(1,9,NEED_OR_NOT_DEP_MSG,' ',NULL,&prgSelResult);

        btn_Yes = newtButton(15,11,BTN_YES_MSG);
        btn_No = newtButton(35,11,BTN_NO_MSG);

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_SelMsg1,label_SelMsg2,listbox_SelList, \
                checklist_ShowDepDlg,btn_Yes,btn_No,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Yes){
			if(prgSelResult == '*'){
				m_bShowDependencyMsg = false;
			}else{
				m_bShowDependencyMsg = true;
			}
                        bRetVal=true;   pExitCode=PAGE_EXIT;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_No){
                        bRetVal=false;  pExitCode=PAGE_EXIT;
                }
        }while(pExitCode < PAGE_EXIT);

        newtPopWindow();
        newtFormDestroy(form);

        return bRetVal;
}

void classTui::popup_ConfirmWin_For_Incompatible(vector <structFileInfo> struct_vectorArg)
{
	vector<string>::iterator it;
	vector <string> string_vectorSelList;

        newtComponent listbox_SelList;
        newtComponent label_SelMsg1,label_SelMsg2,label_SelMsg3,label_SelMsg4;
        newtComponent btn_Ok;
        newtComponent form;	
        struct newtExitStruct es;
        PAGE pExitCode=INIT_VALUE;
        bool bRetVal;
	char prgSelResult;

        newtCenteredWindow(60,15,CONFIRM_WIN_TITLE);

        label_SelMsg1=newtLabel(1,1,COMFIRM_FOR_INCOMPATIBLE_MSG1);
        label_SelMsg2=newtLabel(1,2,COMFIRM_FOR_INCOMPATIBLE_MSG2);
        label_SelMsg3=newtLabel(1,3,COMFIRM_FOR_INCOMPATIBLE_MSG3);        
        label_SelMsg4=newtLabel(1,4,COMFIRM_FOR_INCOMPATIBLE_MSG4);
        listbox_SelList=newtListbox(1,6,5,NEWT_FLAG_SCROLL);
        newtListboxSetWidth(listbox_SelList,56);

	struct2str(struct_vectorArg,string_vectorSelList);

        for(it=string_vectorSelList.begin();it != string_vectorSelList.end();it++){
                newtListboxAppendEntry(listbox_SelList,(*it).c_str(),NULL);
        }	

        btn_Ok = newtButton(/*15*/25,11,BTN_OK_MSG);        

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_SelMsg1,label_SelMsg2,label_SelMsg3,label_SelMsg4,listbox_SelList, \
                btn_Ok,NULL);

        do{
                newtFormRun(form,&es);

         if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Ok){			
                        pExitCode=PAGE_EXIT;
         	}
        }while(pExitCode < PAGE_EXIT);

        newtPopWindow();
        newtFormDestroy(form);
}

bool classTui::popup_ConfirmWin(set <structHeaderInfo, DereferenceLess> struct_setArg)
{
        vector<string>::iterator it;
        vector <string> string_vectorSelList;

        newtComponent listbox_SelList;
        newtComponent label_SelMsg1,label_SelMsg2;
        newtComponent btn_Yes,btn_No;
        newtComponent form;
        newtComponent checklist_ShowDepDlg;
        struct newtExitStruct es;
        PAGE pExitCode=INIT_VALUE;
        bool bRetVal;
        char prgSelResult;

        newtCenteredWindow(60,15,CONFIRM_WIN_TITLE);

        label_SelMsg1=newtLabel(1,1,CONFIRM_MSG);
        label_SelMsg2=newtLabel(1,2,CONTINUE_MSG);
        listbox_SelList=newtListbox(1,3,5,NEWT_FLAG_SCROLL);
        newtListboxSetWidth(listbox_SelList,56);

        struct2str(struct_setArg,string_vectorSelList);

        for(it=string_vectorSelList.begin();it != string_vectorSelList.end();it++){
                newtListboxAppendEntry(listbox_SelList,(*it).c_str(),NULL);
        }

        checklist_ShowDepDlg = newtCheckbox(1,9,NEED_OR_NOT_DEP_MSG,' ',NULL,&prgSelResult);

        btn_Yes = newtButton(15,11,BTN_YES_MSG);
        btn_No = newtButton(35,11,BTN_NO_MSG);

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_SelMsg1,label_SelMsg2,listbox_SelList, \
                checklist_ShowDepDlg,btn_Yes,btn_No,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Yes){
                        if(prgSelResult == '*'){
                                m_bShowDependencyMsg = false;
                        }else{
                                m_bShowDependencyMsg = true;
                        }
                        bRetVal=true;   pExitCode=PAGE_EXIT;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_No){
                        bRetVal=false;  pExitCode=PAGE_EXIT;
                }
        }while(pExitCode < PAGE_EXIT);

        newtPopWindow();
        newtFormDestroy(form);

        return bRetVal;
}

/*
//! pop-up a dialog to confirm actions(update,install,erase)
bool classTui::popup_ConfirmWin(void)
{
	int nRet=0;
	bool bRet=false;
	PROGRAM_MODE pMode=get_ProgramMode();

	switch(pMode){
		case UPDATE_MODE:
			nRet=newtWinChoice(CONFIRM_WIN_TITLE,BTN_YES_MSG,BTN_NO_MSG,UPDATE_CONFIRM_MSG);
			break;
		case INSTALL_MODE:
			nRet=newtWinChoice(CONFIRM_WIN_TITLE,BTN_YES_MSG,BTN_NO_MSG,INSTALL_CONFIRM_MSG);
			break;
		case ERASE_MODE:
			nRet=newtWinChoice(CONFIRM_WIN_TITLE,BTN_YES_MSG,BTN_NO_MSG,ERASE_CONFIRM_MSG);
			break;
	}
	
	if(nRet == 1){
		bRet=true;
	}else{
		bRet=false;
	}

	return bRet;
}
*/
/*!
@brief Error handler for getting packages

@param nErr - network error number
*/
void classTui::show_NetworkErrorDialog(int nErr)
{
	switch(nErr){
	        case NETWORK_ERR_WRONG_URL:
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,URL_ERR_MSG,NULL);
	                newtWinMessage(ERROR_MSG,BTN_OK_MSG,URL_ERR_MSG);
	                break;
	        case NETWORK_ERR_CONNECT:
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,CONNECT_ERR_MSG,NULL);
	                newtWinMessage(ERROR_MSG,BTN_OK_MSG,CONNECT_ERR_MSG);
	                break;
	        case NETWORK_ERR_AUTH_FAIL:
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,AUTH_ERR_MSG,NULL);
	                newtWinMessage(ERROR_MSG,BTN_OK_MSG,AUTH_ERR_MSG);
	                break;
		case NETWORK_ERR_FWRITE:
			m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,FWRITE_ERR_MSG,NULL);
                        newtWinMessage(ERROR_MSG,BTN_OK_MSG,FWRITE_ERR_MSG);
                        break;
	        case NETWORK_ERR_UNKNOWN:
			m_refLogger->WriteLog_int(ERROR_LOG,"TUI - Unknown network error",nErr,NULL);
	                newtWinMessage(ERROR_MSG,BTN_OK_MSG,UNKNOWN_ERR_MSG);
	                break;
	        default:
			m_refLogger->WriteLog_int(ERROR_LOG,"TUI - Unknown network error",nErr,NULL);
	                break;
	}
}
/*!
@brief show Progress-page for Update or Install the packages

@return int - next page number
*/
PAGE classTui::show_UpdateInstallProgressPage(void)
{
	newtComponent form_Progress;
	newtComponent label_ProgressMsg;
	
	struct structSessionBuffer sBuffer;
	PAGE pExitCode=INIT_VALUE;
	int nRet;
	bool bAct=true;
	int nIsUpdate=0;
	int nSelfUpdateCount=0;
	int nDownload=0;
	string strFullPath;
	string strUrlFullPath;
	vector <structFileInfo> vectorTemp;
	vector<structFileInfo>::iterator it;
	vector <structFileInfo> vectorUpdate;
	vector <structFileInfo> vectorInstall;
	vector <string>::iterator strIt;
	VECTOR_LOAD_STATE vectorState;
        char strDubugMsg[MAX_STRING];
	PROGRAM_MODE pMode=get_ProgramMode();

	if((pMode == UPDATE_MODE && m_nUpdateMethod == TOTAL_UPDATE)	\
						|| m_nUpdateMethod == SELF_UPDATE){
		if(!m_bIsAction){
			vectorState = show_LoadPage();
                	if(vectorState == FAIL_LOAD){
                        	pExitCode=PAGE_EXIT;
                        	goto ret;
                	}else if(vectorState == CANCEL_SELF_UPDATE){
                        	pExitCode=show_DonePage(USER_CANCEL);
				goto ret;
                	}else{
                        	m_bIsAction=true;
                	}
		}
	}
	
        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(HELP_LINE);
        set_WinTitle();
	
	label_ProgressMsg=newtLabel(1,1,PROGRESS_PAGE_LABEL);
	
  	g_label_TotalProgressMsg=newtLabel(1,4,TOTAL_PROGRESS_MSG);
  	g_scale_TotalProgress=newtScale(5,5,60,100);
  
  	g_label_CurrentProgressMsg=newtLabel(1,8,CURRENT_PROGRESS_MSG);
  	g_scale_CurrentProgress=newtScale(5,9,60,100);
 
  	form_Progress = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
  	newtFormAddComponents(form_Progress,	\
   			g_label_TotalProgressMsg,g_scale_TotalProgress,	\
  			g_label_CurrentProgressMsg,g_scale_CurrentProgress,	\
  			NULL);

  	btn_Cancel = newtButton(50,11,BTN_CANCEL_MSG);

  	g_form_Progress=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

	newtFormAddComponents(g_form_Progress,label_ProgressMsg,     \
		form_Progress,btn_Cancel,NULL);
	
  
  	newtRefresh();
  	newtFormSetTimer(g_form_Progress,50);
  
  	newtFormRun(g_form_Progress,&g_es_Progress);
  
	// Set the CallBack Functions
	m_refNetwork->SetGetPackagesCallBack(GetPackagesCallBack);
		
	vectorTemp.clear();
	if(m_nUpdateMethod != SELF_UPDATE){
		if(pMode == UPDATE_MODE){
			nIsUpdate = 0;
			if(m_nUpdateMethod == TOTAL_UPDATE){
				vectorTemp=m_struct_vectorFilteredList;
			}else if(m_nUpdateMethod == CUSTOM_UPDATE){
				vectorTemp=m_struct_vectorSelList;
			}
		}else if(pMode == INSTALL_MODE){
			nIsUpdate = 1;
			vectorTemp=m_struct_vectorSelList;
		}
		for(it=vectorTemp.begin();it != vectorTemp.end();it++){

 			strFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,	\
				false,(it->strName).c_str(),(it->strVersion).c_str(),	\
 				 (it->strRelease).c_str(),(it->strArch).c_str(), "");

			strUrlFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,	\
				false,(it->strName).c_str(),(it->strVersion).c_str(),	\
 				 (it->strRelease).c_str(),(it->strArch).c_str(), "",true);
  	
			m_refNetwork->AddPackage(strFullPath, strUrlFullPath);
		
			if( m_refRpmEngine->AddFile(strFullPath.c_str(), UPDATE,nIsUpdate?0:1) < 0){
				cout<<strFullPath<<" file is not found!"<<endl;
			}
		}
	}else{
		while(nSelfUpdateCount < 2){
			if(nSelfUpdateCount == 0){
				nIsUpdate = 0;
				vectorTemp = m_struct_vectorUpdateList;
			}else{
				nIsUpdate = 1;
	                	vectorTemp = m_struct_vectorInstallList;
			}	

                	for(it=vectorTemp.begin();it != vectorTemp.end();it++){

                        	strFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,   \
                                	false,(it->strName).c_str(),(it->strVersion).c_str(),   \
                                 	(it->strRelease).c_str(),(it->strArch).c_str(), "");

                        	strUrlFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,        \
                                	false,(it->strName).c_str(),(it->strVersion).c_str(),   \
                                 	(it->strRelease).c_str(),(it->strArch).c_str(), "",true);

                        	m_refNetwork->AddPackage(strFullPath, strUrlFullPath);

                        	if( m_refRpmEngine->AddFile(strFullPath.c_str(), UPDATE , nIsUpdate?0:1) < 0){
					m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"File is not found ",strFullPath.c_str(),NULL);
                        	}
			}
			nSelfUpdateCount++;
		}
	}

#if defined(__i386__) || defined(__x86_64__) || defined(__ia64__)	
	if(m_nRedcastleStatus == RC_STATUS_WARNING)
	{
		if(m_refRpmEngine->CheckKmodRedcastle() == false)
		{
			
			vector <string> vectorTemp = m_refRpmEngine->GetAddedKmodRedcastleFile();
			popup_ConfirmWinRemoveKernel(vectorTemp);
			int nTypeForURL;
			if(pMode == UPDATE_MODE)
			{	
				nTypeForURL = 0;
			}
			else
			{
				nTypeForURL = 1;
			}
			if(m_refRpmEngine->RemoveKernelAndKmodRedcastle(nTypeForURL) == false)
			{
				m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI , "Failed to remove kernel and redcastle.", NULL);
				pExitCode = PAGE_EXIT;
	                	goto ret;
			}
			vector <structAddedFile> vectorForCount;
			vectorForCount = m_refRpmEngine->GetAddedFile( UPDATE, vectorForCount);
			if((int)vectorForCount.size() < 1)
			{
				if(pMode == UPDATE_MODE)
		                {
					newtWinMessage(WARNING_MSG,BTN_OK_MSG, "There are no packages available to update!");
	                	}
		                else
	        	        {
					newtWinMessage(WARNING_MSG,BTN_OK_MSG, "There are no packages available to install!");
	        	        }
				pExitCode = PAGE_BACK;
		                goto ret;
			}
		}
		else
		{
			vector <string> vectorTemp = m_refRpmEngine->GetAddedKmodRedcastleFile();
	                vector <string>::iterator it;
	                int nIndex = 1;
	                for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
	                {
				string  strName1, strVer1, strRel1, strArch1;			
				m_refRpmEngine->stripNVRA(*it, &strName1, &strVer1, &strRel1, &strArch1);
				strUrlFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "", true);
				strFullPath = m_refRpmEngine->GetFullPathFile(nIsUpdate,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "");
				if(m_refRpmEngine->AddFile(strFullPath.c_str(), REQDEP, 0) != 0)
				{	
					snprintf(strDubugMsg, sizeof(strDubugMsg), "Failed to load m_refRpmEngine->AddFile(%s) function.", strFullPath.c_str());
					m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI, strDubugMsg, NULL);
					newtWinMessage(ERROR_MSG,BTN_OK_MSG, strDubugMsg);
					pExitCode = PAGE_EXIT;
		                        goto ret;
				}
				if (pMode != ERASE_MODE)
				{
					m_refNetwork->AddPackage(strFullPath, strUrlFullPath);
				}

				snprintf(strDubugMsg, sizeof(strDubugMsg), "Add install : %s", strFullPath.c_str());
				m_refLogger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_TUI, (const char *)strDubugMsg, NULL);
	
	                }
		}
	}
#endif	
		

	// Get *.rpm files
	nDownload=m_refNetwork->GetPackages();
        if(nDownload != NETWORK_RETOK){
		show_NetworkErrorDialog(nDownload);
                bAct=false;
		pExitCode = PAGE_BACK;
                goto ret;
        }

	// Check the Dependency inter the RPM files
	nRet=m_refRpmEngine->Check();

#ifdef NDEBUG
	if(nRet < 0){
		show_ErrorMsg("RPM Check Error Code",nRet);
		pExitCode = PAGE_BACK;
		goto ret;
	}
#endif /* NDEBUG */

	nDownload=m_refNetwork->GetPackages();

	if(nDownload != NETWORK_RETOK){
		show_NetworkErrorDialog(nDownload);
                bAct=false;
                pExitCode = PAGE_EXIT;
                goto ret;
        }

	if(nRet > -1){
		if (nRet > 0){
			m_string_vectorDepList=GetDepList(m_string_vectorDepList);
			
			if(m_bShowDependencyMsg){
				bAct=popup_DepListWithSelList(m_string_vectorDepList);
			}else{
				bAct = true;
			}

			if(!bAct){
 				pExitCode=show_DonePage(USER_CANCEL);
				m_struct_vectorDoneList=vectorTemp;
 				goto ret;
			}

			bAct=false;
			m_refRpmEngine->ChangeGrubToDefaultKernel();
		}	
		 
		
		
		m_refRpmEngine->SetRunCallBack(RunCallBack);
				
		newtPopHelpLine();
		newtPopWindow();
		
		// Destroy download progress page 
		newtFormDestroy(g_form_Progress);
		
		
		// Create update or install progress page
		newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
		newtPushHelpLine(HELP_LINE);
		set_WinTitle();
		
		label_ProgressMsg=newtLabel(1,1,PROGRESS_PAGE_LABEL);
		
	  	g_label_TotalProgressMsg=newtLabel(1,4,TOTAL_PROGRESS_MSG);
	  	g_scale_TotalProgress=newtScale(5,5,60,100);
	  
	  	g_label_CurrentProgressMsg=newtLabel(1,8,CURRENT_PROGRESS_MSG);
	  	g_scale_CurrentProgress=newtScale(5,9,60,100);
		form_Progress = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
	  	newtFormAddComponents(form_Progress,	\
	   			g_label_TotalProgressMsg,g_scale_TotalProgress,	\
	  			g_label_CurrentProgressMsg,g_scale_CurrentProgress,	\
	  			NULL);	  	

	  	g_form_Progress=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
	  	
	  	// There are no "Cancel" button because while install or update,  AXTU can not stop for rpmdb safe.
		newtFormAddComponents(g_form_Progress,label_ProgressMsg,     \
				form_Progress,NULL);	  
	  	newtRefresh();
	  	newtFormSetTimer(g_form_Progress,50);	  
	  	newtFormRun(g_form_Progress,&g_es_Progress);	
	  					
		nRet=m_refRpmEngine->Run();		
		
	
               	if(nRet == -6)
                {
			nRet=popup_ConflictDlg();
                        if(nRet){                        		                        		                     		
                                nRet=m_refRpmEngine->Run(true);                                
                        }else{
 				pExitCode=show_DonePage(USER_CANCEL);
				m_struct_vectorDoneList=vectorTemp;
				bAct = false;
                                goto ret;
                        }
                }

#ifdef NDEBUG
		if(nRet < 0){
			show_ErrorMsg("RPM Run Error Code",nRet);
			pExitCode = PAGE_BACK;
			goto ret;
		}
#endif /* NDEBUG */

	}else if(nRet == -9 || nRet == -6 || nRet == -10){  // -9: looped   -6: not found
                string strErr;
                if (nRet == -9)
                {
                        strErr = (string)NOT_FIND_REQUIRE_MSG + 	\
				(string)"(Looped) " + (string)SEE_LOG_MSG +  \
				(string)" : " + m_refLogger->GetErrorFilePath();
			newtWinMessage(ERROR_MSG,BTN_OK_MSG,(char*)strErr.c_str());
                        m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI, \
                        NOT_FIND_REQUIRE_MSG, NULL);
                }
                else if(nRet == -6)
                {
                        strErr = (string)NOT_FIND_REQUIRE_MSG + 	\
				(string)SEE_LOG_MSG + (string)" : " +   \
				m_refLogger->GetErrorFilePath();
			newtWinMessage(ERROR_MSG,BTN_OK_MSG,	\
				(char*)strErr.c_str());
                        m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,	\
                        NOT_FIND_REQUIRE_MSG, NULL);
                }else if(nRet == -10){
                       strErr = (string)"Cannot download package" +         \
				(string)SEE_LOG_MSG + (string)" : " +  \
                                m_refLogger->GetErrorFilePath();
                        newtWinMessage(ERROR_MSG,BTN_OK_MSG,    \
                                (char*)strErr.c_str());
		}

		pExitCode = DONE_FAIL_PAGE;
		m_struct_vectorDoneList=vectorTemp;
		bAct = false;
		goto ret;
        }else if(nRet == -8){
		popup_DepAndBlackList();

		if(m_refRpmEngine->GetIncompatibleCount() > 0)
		{
			pExitCode=show_DonePage(INCOMPATIBLE_PKG);
		}
		else
		{
			pExitCode=show_DonePage(BLACKLISTED_PKG);
		}

		bAct = true;
		goto ret;
	}else if(nRet == -11){
		pExitCode=show_DonePage(NONE_ENOUGH_SPACE);

		bAct = true;

		goto ret;
	}


	do{
		newtFormRun(g_form_Progress,&g_es_Progress);
		if(g_es_Progress.reason == g_es_Progress.NEWT_EXIT_COMPONENT){
			pExitCode=PAGE_BACK;
		}

		m_struct_vectorDoneList.clear();
		
		if(m_nUpdateMethod == SELF_UPDATE){
			m_struct_vectorDoneList=m_struct_vectorUpdateList;
               		for(it=m_struct_vectorInstallList.begin();	\
					it != m_struct_vectorInstallList.end();it++){
				m_struct_vectorDoneList.push_back(*it);
			}
		}else{
			m_struct_vectorDoneList=vectorTemp;
		}

		if(nRet==0){
			pExitCode=DONE_OK_PAGE;
		}else{
			pExitCode=DONE_FAIL_PAGE;
		}
	}while(pExitCode < 0);
ret:	
	newtPopHelpLine();
        newtPopWindow();

  	newtFormDestroy(g_form_Progress);
/* 	Use of bAct
	if(!bAct){
		delete m_refRpmEngine;
		m_refRpmEngine = new classRpmEngine(); 
		set_RpmCommand(pMode);
		m_refRpmEngine->ReadCacheDirInfo();
	}
*/
	m_bIsHeaderWork = false;

  	return pExitCode;
}

/*!
@brief show Progress-page for Erase the packages

@return int - next page number
*/
PAGE classTui::show_EraseProgressPage(void)
{
	newtComponent form_Progress;
	newtComponent label_ProgressMsg;
	
	struct structSessionBuffer sBuffer;
	PAGE pExitCode=INIT_VALUE;
	int nRet;
	bool bAct=true;

	set <structHeaderInfo, DereferenceLess>::iterator erase_it;
	PROGRAM_MODE pMode=get_ProgramMode();
	

        newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
	newtPushHelpLine(HELP_LINE);
        set_WinTitle();
	
  	label_ProgressMsg=newtLabel(1,1,PROGRESS_PAGE_LABEL);
	
  	g_label_TotalProgressMsg=newtLabel(1,4,TOTAL_PROGRESS_MSG);
  	g_scale_TotalProgress=newtScale(5,5,60,100);
  
  	g_label_CurrentProgressMsg=newtLabel(1,8,CURRENT_PROGRESS_MSG);
  	g_scale_CurrentProgress=newtScale(5,9,60,100);
  
  	form_Progress = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
  	newtFormAddComponents(form_Progress,	\
   				g_label_TotalProgressMsg,g_scale_TotalProgress,	\
  				g_label_CurrentProgressMsg,g_scale_CurrentProgress,	\
  				NULL);
  	

  	g_form_Progress=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

  	newtFormAddComponents(g_form_Progress,label_ProgressMsg,     \
		form_Progress,NULL);
  
  	newtRefresh();
  	newtFormSetTimer(g_form_Progress,50);
  
  	newtFormRun(g_form_Progress,&g_es_Progress);
  
	for(erase_it=m_struct_setEraseSelList.begin();erase_it != m_struct_setEraseSelList.end();erase_it++){
		m_refRpmEngine->AddFile((erase_it->strNVRA).c_str(), REMOVE, 0);
	}

	// Check the Dependency inter the RPM files
	nRet=m_refRpmEngine->Check();

	if(nRet > -1)
	{
		if (nRet > 0)
		{
			m_string_vectorDepList=GetDepList(m_string_vectorDepList);

			if(m_bShowDependencyMsg){
				bAct=popup_DepListWithSelList(m_string_vectorDepList);
			}else{
				bAct = true;
			}

			if(!bAct){
				pExitCode=show_DonePage(USER_CANCEL);
				m_struct_setEraseDoneList=m_struct_setEraseSelList;
				bAct=false;
 				goto ret;
			}
			m_refRpmEngine->ChangeGrubToDefaultKernel();
		}
		m_refRpmEngine->SetRunCallBack(RunCallBack);		
					
		nRet=m_refRpmEngine->Run();
		
	}else if(nRet == -9 || nRet == -6 || nRet == -10){  // -9: looped   -6: not found
                string strErr;
                if (nRet == -9)
                {
                        strErr = (string)NOT_FIND_REQUIRE_MSG +	\
				(string)"(Looped) " + (string)SEE_LOG_MSG + \
				(string)" : " + m_refLogger->GetErrorFilePath();
			newtWinMessage(ERROR_MSG,BTN_OK_MSG,	\
				(char*)strErr.c_str());
                        m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,	\
                        NOT_FIND_REQUIRE_MSG, NULL);
                }
                else if(nRet == -6)
                {
                        strErr = (string)NOT_FIND_REQUIRE_MSG + 	\
				(string)SEE_LOG_MSG + (string)" : " +   \
				m_refLogger->GetErrorFilePath();
			newtWinMessage(ERROR_MSG,BTN_OK_MSG,	\
				(char*)strErr.c_str());
                        m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,	\
                        NOT_FIND_REQUIRE_MSG, NULL);
                }else if(nRet == -10){
                       strErr = (string)"Cannot download package" +         \
				(string)SEE_LOG_MSG +  (string)" : " +         \
                                m_refLogger->GetErrorFilePath();
                        newtWinMessage(ERROR_MSG,BTN_OK_MSG,    \
                                (char*)strErr.c_str());
		}

		m_struct_setEraseDoneList=m_struct_setEraseSelList;
		pExitCode = DONE_FAIL_PAGE;

		goto ret;
	}

	do{
		newtFormRun(g_form_Progress,&g_es_Progress);
		if(g_es_Progress.reason == g_es_Progress.NEWT_EXIT_COMPONENT){
			pExitCode=PAGE_BACK;
		}

		m_struct_setEraseDoneList.clear();
		m_struct_setEraseDoneList=m_struct_setEraseSelList;

		if(nRet==0){
			pExitCode=DONE_OK_PAGE;
		}else{
			pExitCode=DONE_FAIL_PAGE;
		}
	}while(pExitCode < 0);
	
ret:	
/* 	Use of bAct
	if(!bAct){
	    delete m_refRpmEngine;
		m_refRpmEngine = new classRpmEngine();
		set_RpmCommand(pMode);
	}
*/
	newtPopHelpLine();
        newtPopWindow();

	newtFormDestroy(g_form_Progress);
    
	return pExitCode;
}

//! make the message of a progress result Page.
void classTui::make_DoneMsg(bool bOkFlag,char *szMsgBuffer)
{
	PROGRAM_MODE pMode=get_ProgramMode();

	if(bOkFlag){
                if(m_nUpdateMethod != SELF_UPDATE){
			switch(pMode){
                        	case UPDATE_MODE:
                        		strncpy(szMsgBuffer,UPDATE_OK_MSG,MAX_STRING);
					break;
				case INSTALL_MODE:
                        		strncpy(szMsgBuffer,INSTALL_OK_MSG,MAX_STRING);
					break;
				case ERASE_MODE:
                        		strncpy(szMsgBuffer,ERASE_OK_MSG,MAX_STRING);
					break;
				default:
					break;
			}
                }else{
			strncpy(szMsgBuffer,SELFUPDATE_OK_MSG,MAX_STRING);
                }
        }else{
                if(m_nUpdateMethod != SELF_UPDATE){
			switch(pMode){
                        	case UPDATE_MODE:
					snprintf(szMsgBuffer,MAX_STRING,	\
						"%s See %s",UPDATE_FAIL_MSG,	\
						(m_refLogger->GetErrorFilePath()).c_str());
					break;
				case INSTALL_MODE:
					snprintf(szMsgBuffer,MAX_STRING,"%s See %s",INSTALL_FAIL_MSG,(m_refLogger->GetErrorFilePath()).c_str());
					break;
				case ERASE_MODE:
					snprintf(szMsgBuffer,MAX_STRING,"%s See %s",ERASE_FAIL_MSG,(m_refLogger->GetErrorFilePath()).c_str());
					break;
				default:
					break;
			}
                }else{
			snprintf(szMsgBuffer,MAX_STRING,"%s See %s",SELFUPDATE_FAIL_MSG,(m_refLogger->GetErrorFilePath()).c_str());
                }
        }
}

/*! 
@brief show progress result Page.

@param bOkFlag - rpm's install or erase process ok? or not?
@return int - next page number
*/
PAGE classTui::show_DonePage(bool bOkFlag)
{
	vector<string>::iterator it_1;
	vector<string>::iterator it_2;
	
	vector <structAddedFile> vectorTemp;
	newtComponent listbox_CompleteList;
	newtComponent label_CompleteMsg1;
	newtComponent btn_Done,btn_Quit;
	newtComponent form;
	struct newtExitStruct es;	
	PAGE pExitCode=INIT_VALUE;
	char *szText = NULL;
	string strPkgMsg = "";
	string strTemp = "";

	newtComponent textbox_pkg;
        int textWidth,textHeight;
        char *flowedTextPkg = NULL;

	PROGRAM_MODE pMode=get_ProgramMode();
	
	newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
        set_WinTitle();
	
	szText=(char*)calloc(sizeof(char),MAX_STRING);
	if(szText == NULL){
		m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                pExitCode = INIT_VALUE;
                return pExitCode;
	}

	make_DoneMsg(bOkFlag,szText);
	label_CompleteMsg1=newtLabel(1,1,szText);

	listbox_CompleteList=newtListbox(1,3,5,NEWT_FLAG_SCROLL);
	newtListboxSetWidth(listbox_CompleteList,60);
			 
	vectorTemp = m_refRpmEngine->GetAddedFile( UPDATE | REMOVE, vectorTemp);
	
	vector <structAddedFile>::iterator it;
	int nIndex = 1;
	
	if(vectorTemp.size() > 0)
	{
		strPkgMsg += (string)_("[ Selected packages ]\n");
	}
	for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
	{
		int nAction;		 
		string strRpmName;			
		strRpmName = m_refRpmEngine->GetFullFileNameFromPath(it->strFile, strRpmName);	
		strTemp = (string)" " + strRpmName + (string)"\n";
		strPkgMsg += strTemp;
		strTemp = (string)"";
			    	
	   if (bOkFlag == true) {	
		if(pMode ==INSTALL_MODE || pMode == UPDATE_MODE)
		{
			if(it->nUpgrade)
			{
				nAction = UPDATE_ACTION;
			}
			else
			{
				nAction = INSTALL_ACTION;
			}
		}	
		else
		{					
			nAction = ERASE_ACTION;	
		}
		
		m_refLogger->RpmLogging(nAction,strRpmName.c_str());
	   }
	}
	
	vectorTemp.clear();
	vectorTemp = m_refRpmEngine->GetAddedFile(REQDEP |OTHERDEP, vectorTemp);	
	if(vectorTemp.size() > 0)
	{	
		strPkgMsg += (string)"\n";
		strPkgMsg += (string)_("[ Dependent packages ]\n");
	}
	for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
	{		
		int nAction;		 
		string strRpmName;			
		strRpmName = m_refRpmEngine->GetFullFileNameFromPath(it->strFile, strRpmName);	
		strTemp = (string)" " + strRpmName + (string)"\n";
		strPkgMsg += strTemp;
		strTemp = (string)"";
	   if (bOkFlag == true) {	
		if(pMode ==INSTALL_MODE || pMode == UPDATE_MODE)
		{	
			if(it->nUpgrade)
			{
				nAction = DEP_UPDATE_ACTION;
			}
			else
			{
				nAction = DEP_INSTALL_ACTION;
			}
		}
		else
		{	
			nAction = DEP_ERASE_ACTION;
		}							
			
		m_refLogger->RpmLogging(nAction,strRpmName.c_str());
	   }
	}	

	flowedTextPkg = newtReflowText((char*)(strPkgMsg.c_str()),60,4,4,&textWidth,&textHeight);
        textbox_pkg= newtTextbox(2,3,60,7,NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);
        newtTextboxSetText(textbox_pkg,flowedTextPkg);
        if(flowedTextPkg != NULL){
                free(flowedTextPkg);
                flowedTextPkg = NULL;
        }

	if(bOkFlag){
		if(m_nUpdateMethod == SELF_UPDATE){
			btn_Done = newtButton(30,11,BTN_RESTART_MSG);
		}else{
			btn_Done = newtButton(30,11,BTN_DONE_MSG);
		}
	}else{
		btn_Done = newtButton(30,11,BTN_RETRY_MSG);
	}
		
	btn_Quit = newtButton(50,11,BTN_QUIT_MSG);
  
  	form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

	if(m_nUpdateMethod == SELF_UPDATE){
  		newtFormAddComponents(form,label_CompleteMsg1,	\
  			textbox_pkg,btn_Done,btn_Quit,NULL);
	}else{
  		newtFormAddComponents(form,label_CompleteMsg1,	\
  			textbox_pkg,btn_Quit,NULL);
	}
  
  	do{
  		newtFormRun(form,&es);
		
		if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Done){

			if(m_nUpdateMethod == SELF_UPDATE){
				m_refRpmEngine->ReadCacheDirInfo();
				pExitCode = PROGRAM_RESTART;
			}
    		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Quit){
			pExitCode=PAGE_EXIT;
        	}
  	}while(pExitCode < 0);

        newtPopWindow();

  	newtFormDestroy(form);

	if(szText != NULL){
		free(szText);
		szText = NULL;
	}

  	return pExitCode;
}

PAGE classTui::show_DonePage(DONE_PAGE_CLASS pageClass)
{
	vector <string> vectorTemp;
	newtComponent btn_SetBlackList,btn_Retry,btn_Quit;
	newtComponent form;
	struct newtExitStruct es;
	PAGE pExitCode=INIT_VALUE;
	char *flowedTextDescript = NULL;
	newtComponent textbox_Descript;
	int textWidth,textHeight;

	PROGRAM_MODE pMode=get_ProgramMode();

	newtInit();
	newtInit();
        newtCls();
        newtDrawRootText(0,0,TITLE);
        set_WinTitle();
	
        newtCls();
        newtDrawRootText(0,0,TITLE);
        set_WinTitle();
	
	switch(pageClass){
		case BLACKLISTED_PKG:
			flowedTextDescript = newtReflowText(	\
				BLACKLISTED_PKG_ERR_MSG,60,5,5,	\
				&textWidth,&textHeight);
			break;
		case INCOMPATIBLE_PKG:
			flowedTextDescript = newtReflowText(	\
					DEP_INCOMPATIBLE_MSG/*INCOMPATIBLE_PKG_ERR_MSG*/,60,5,5,	\
				&textWidth,&textHeight);
			break;
		case NONE_AVAILABLE_PKG:
			flowedTextDescript = newtReflowText(	\
				UPDATE_PKG_NOT_EXIST_MSG,60,5,5,&textWidth,	\
				&textHeight);
			break;
		case NONE_ENOUGH_SPACE:
			flowedTextDescript = newtReflowText(	\
				NOSPACE_INSTALL_ERR_MSG,60,5,5,
				&textWidth,&textHeight);
			break;
		case USER_CANCEL:
			if(pMode == UPDATE_MODE){
				flowedTextDescript = newtReflowText(	\
					UPDATE_CANCEL_MSG,60,5,5,
					&textWidth,&textHeight);
			}else if(pMode == INSTALL_MODE){
				flowedTextDescript = newtReflowText(	\
					INSTALL_CANCEL_MSG,60,5,5,
					&textWidth,&textHeight);
			}else if(pMode == ERASE_MODE){
				flowedTextDescript = newtReflowText(	\
					UNINSTALL_CANCEL_MSG,60,5,5,
					&textWidth,&textHeight);
			}
			break;
		default:
			flowedTextDescript = newtReflowText(	\
				UNKNOWN_ERR_MSG,60,5,5,
				&textWidth,&textHeight);
			break;
	}

        textbox_Descript = newtTextbox(2,3,60,5,NEWT_FLAG_WRAP);
        newtTextboxSetText(textbox_Descript,flowedTextDescript);
	if(flowedTextDescript != NULL){
        	free(flowedTextDescript);
		flowedTextDescript = NULL;
	}

	btn_SetBlackList = newtButton(5,9,BTN_BLACKLIST_MSG);
	btn_Retry = newtButton(40,9,BTN_RETRY_MSG);
	btn_Quit = newtButton(55,9,BTN_QUIT_MSG);
		
  	form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);

	if(pageClass == BLACKLISTED_PKG){
  		newtFormAddComponents(form,textbox_Descript,	\
				btn_SetBlackList,btn_Retry,btn_Quit,NULL);
	}else{
  		newtFormAddComponents(form,textbox_Descript,btn_Quit,NULL);
	}
  
  	do{
  		newtFormRun(form,&es);
		
		if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Retry){
			if(m_nUpdateMethod == SELF_UPDATE){
				m_refRpmEngine->ReadCacheDirInfo();
				pExitCode = PROGRAM_RESTART;
			}else if(pageClass == BLACKLISTED_PKG){
				pExitCode = PROGRAM_RESTART;
			}
    		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Quit){
			pExitCode=PAGE_EXIT;
        	}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_SetBlackList){
			newtSuspend();
                        int nSystemResult;
                        nSystemResult = system(TUI_SETUP_PATH);
                        newtResume();

                        if(nSystemResult != 0){
				// Fail case
                                m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,strerror(errno),NULL);
                                newtWinMessage(ERROR_MSG,BTN_OK_MSG,_("Cannot execute the setup program for blacklist."));
                        }
		}
  	}while(pExitCode < 0);

        newtPopWindow();

  	newtFormDestroy(form);

  	return pExitCode;
}

/*!
@brief pop-up alert dialog for SelfUpdate

@return int - next page number
*/
bool classTui::popup_SelfUpdateMsg(void)
{
	int nRet;

	nRet=newtWinChoice(MYPKG_DETECT_TITLE,BTN_OK_MSG,BTN_CANCEL_MSG,MYPKG_DETECT_MSG);

	if(nRet == 1){
		return true;
	}else{
		return false;
	}	
}

/*! 
@brief pop-up a dialog to inform the UpdateList

@param vectorArg - Update List
*/
void classTui::popup_UpdateList(vector <string> vectorArg, vector <string> vectorIncompatibleArg)
{
        int rc;
        int textWidth;
        int i=0;

        vector<string>::iterator it;
        char *pkgsContents[vectorArg.size()+1];

        i=0;
        for(it=vectorArg.begin();it != vectorArg.end();it++)
	{
        	pkgsContents[i]=(char*)alloca(sizeof(*pkgsContents)*(it->length()+1));
                snprintf(pkgsContents[i],it->length()+1,"%s",it->c_str());
                i++;
	}
        for(it=vectorIncompatibleArg.begin();it != vectorIncompatibleArg.end();it++)
        	{
                	pkgsContents[i]=(char*)alloca(sizeof(*pkgsContents)*(it->length()+1));
                        snprintf(pkgsContents[i],it->length()+1,"%s",it->c_str());
                        i++;
        	}
        

        pkgsContents[i]=NULL;

        textWidth = 0;

	if( int(vectorArg.size() + vectorIncompatibleArg.size()) != 0){
        	rc = newtWinMenu(POPUP_WIN_TITLE,UPDATE_PKG_EXIST_MSG, \
                         50,5,5,8,pkgsContents,&textWidth,BTN_OK_MSG,NULL);
	}else{
        	rc = newtWinMenu(POPUP_WIN_TITLE,UPDATE_PKG_NOT_EXIST_MSG, \
                         50,5,5,8,pkgsContents,&textWidth,BTN_OK_MSG,NULL);
	}
}

//! pop-up a dialog to inform the DependencyList And BlackList
void classTui::popup_DepAndBlackList(void)
{
		int rc;
		int textWidth;
		int i;
		vector<string>::iterator it;
		vector <string> vectorTemp;
		if(m_refRpmEngine->GetIncompatibleCount() > 0)
		{
			vectorTemp = m_refRpmEngine->GetIncompatiblePackages();
			m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,DEP_INCOMPATIBLE_MSG, NULL);
		}
		else
		{
			vectorTemp = m_refRpmEngine->GetBlockedPackages();
			m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,ERROR_MSG, NULL);
		}        		
		
		string strErr;
		string strMsg;
		
		char *pkgsContents[vectorTemp.size()+1];

#ifdef NDEBUG
        m_refLogger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_TUI, DEP_BLK_MSG, NULL);
#endif /* NDEBUG */

        i=0;
        for(it=vectorTemp.begin();it != vectorTemp.end();it++)
        {
		strErr = m_refRpmEngine->GetFullFileNameFromPath(*it,strErr);
                pkgsContents[i]=(char*)alloca(sizeof(*pkgsContents)*(strErr.length()+1));
                snprintf(pkgsContents[i],strErr.length()+1,"%s",strErr.c_str());
                m_refLogger->WriteLog_char(ERROR_LOG, MYSELF_NAME_TUI,pkgsContents[i], NULL);
#ifdef NDEBUG
        	m_refLogger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_TUI,(const char*)strErr.c_str(), NULL);
#endif /* NDEBUG */
                i++;
        }

        pkgsContents[i]=NULL;

        textWidth = 0;

        if(m_refRpmEngine->GetIncompatibleCount() > 0)
        	{
        		rc = newtWinMenu(ERROR_MSG,DEP_INCOMPATIBLE_MSG, \
        				50,5,5,8,pkgsContents,&textWidth,BTN_OK_MSG,NULL);        		
        	}
        else
        	{
        	rc = newtWinMenu(ERROR_MSG,DEP_BLK_MSG, \
        			50,5,5,8,pkgsContents,&textWidth,BTN_OK_MSG,NULL);        	
        	}
}

/*! 
@brief pop-up a dialog to inform the DependencyList 

@param str_vectorArg - Dependency List
@return bool - Continue? or not?
*/
bool classTui::popup_DepList(vector <string> str_vectorArg)
{
	vector<string>::iterator it;
	
	newtComponent listbox_DepList;
	newtComponent label_DepMsg1,label_DepMsg2;
	newtComponent btn_Yes,btn_No;
	newtComponent form;
	struct newtExitStruct es;	
	PAGE pExitCode=INIT_VALUE;
	bool bRetVal;
	
	newtCenteredWindow(60,13,DEPENDENCY_WIN_TITLE);
	
	label_DepMsg1=newtLabel(1,1,DEPENDENCY_MSG);
	label_DepMsg2=newtLabel(1,2,CONTINUE_MSG);
	listbox_DepList=newtListbox(1,3,5,NEWT_FLAG_SCROLL);
	newtListboxSetWidth(listbox_DepList,56);

	for(it=str_vectorArg.begin();it != str_vectorArg.end();it++){
		newtListboxAppendEntry(listbox_DepList,(*it).c_str(),NULL);
	}
	
	btn_Yes = newtButton(15,9,BTN_YES_MSG);
  	btn_No = newtButton(35,9,BTN_NO_MSG);
  
  	form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
  	newtFormAddComponents(form,label_DepMsg1,label_DepMsg2,listbox_DepList,	\
                btn_Yes,btn_No,NULL);
  
  	do{
  		newtFormRun(form,&es);
		
		if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Yes){
			bRetVal=true;	pExitCode=PAGE_EXIT;
    		}else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_No){
			bRetVal=false;	pExitCode=PAGE_EXIT;
        	}
	}while(pExitCode < PAGE_EXIT);

	newtPopWindow();
  	newtFormDestroy(form);
	
	return bRetVal;
}

bool classTui::popup_DepListWithSelList(vector <string> str_vectorArg)
{
        vector<string>::iterator it;
        vector <string> string_vectorSelList;

	newtComponent textbox_Conflict,textbox_Descript;
	int textWidth,textHeight;
	char *flowedTextDescript = NULL;
        char *flowedTextConflict = NULL;

        newtComponent btn_Yes,btn_No;
	newtGrid grid,subgrid;
	
        newtComponent form;
        struct newtExitStruct es;
        PAGE pExitCode=INIT_VALUE;
        bool bRetVal;
	string strDepMsg = "";
	string strTemp = "";

	PROGRAM_MODE pMode=get_ProgramMode();

	if(pMode != ERASE_MODE){
		vector <structAddedFile> vectorAddedFile;
                vectorAddedFile = m_refRpmEngine->GetAddedFile( UPDATE , vectorAddedFile);
                struct2str(vectorAddedFile, string_vectorSelList);

	}else{
        	struct2str(m_struct_setEraseSelList,string_vectorSelList);
	}

	strDepMsg += (string)_("[ Selected packages ]\n");
	
	for(it=string_vectorSelList.begin();it!=string_vectorSelList.end();it++){
		strTemp = (string)" " + *it + (string)"\n";
		strDepMsg += strTemp;
		strTemp = "";
	}
	
	strDepMsg += (string)"\n";
	strDepMsg += (string)_("[ Dependent packages ]\n");

	strTemp = (string)"";
	for(it=str_vectorArg.begin();it!=str_vectorArg.end();it++){
		strTemp = (string)" " + *it + (string)"\n";
		strDepMsg += strTemp;
		strTemp = (string)"";
	}
	
	flowedTextDescript=newtReflowText(DEPENDENCY_MSG,60,5,5,&textWidth,&textHeight);
        textbox_Descript= newtTextbox(-1,-1,60,textHeight,NEWT_FLAG_WRAP );
        newtTextboxSetText(textbox_Descript,flowedTextDescript);
	if(flowedTextDescript != NULL){
        	free(flowedTextDescript);
		flowedTextDescript = NULL;
	}

	flowedTextConflict=newtReflowText((char*)(strDepMsg.c_str()),60,4,4,&textWidth,&textHeight);
        textbox_Conflict= newtTextbox(-1,-1,60,4,NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);
        newtTextboxSetText(textbox_Conflict,flowedTextConflict);
	if(flowedTextConflict != NULL){
        	free(flowedTextConflict);
		flowedTextConflict = NULL;
	}

        btn_Yes = newtButton(-1,-1,BTN_YES_MSG);
        btn_No = newtButton(-1,-1,BTN_NO_MSG);

	grid = newtCreateGrid(1,3);
	subgrid = newtCreateGrid(2,1);

        newtGridSetField(subgrid,0,0,NEWT_GRID_COMPONENT,btn_Yes,0,0,0,0,0,0);
        newtGridSetField(subgrid,1,0,NEWT_GRID_COMPONENT,btn_No,0,0,0,0,0,0);

	newtGridSetField(grid,0,0,NEWT_GRID_COMPONENT,textbox_Descript,0,0,0,1,0,0);
        newtGridSetField(grid,0,1,NEWT_GRID_COMPONENT,textbox_Conflict,0,0,0,1,0,0);
        newtGridSetField(grid,0,2,NEWT_GRID_SUBGRID,subgrid,0,0,0,0,0,NEWT_GRID_FLAG_GROWX);

        newtGridWrappedWindow(grid,DEPENDENCY_WIN_TITLE);

        form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);

        newtFormAddComponents(form,textbox_Descript,textbox_Conflict,btn_Yes,btn_No,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Yes){
                        bRetVal=true;   pExitCode=PAGE_EXIT;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_No){
                        bRetVal=false;  pExitCode=PAGE_EXIT;
                }
        }while(pExitCode < PAGE_EXIT);

        newtPopWindow();
        newtFormDestroy(form);

        return bRetVal;
}
/*!
@brief pop-up a dialog to show description or summary of packages

@param szName - a Package Name
@param szSummary - Summary of a package 
@param szDescript - Description of a package
*/
void classTui::popup_PkgSummary(const char* szName,const char* szSummary,const char* szDescript)
{
	newtComponent btn_Ok;
	newtComponent textbox_Summary,textbox_Descript;
	newtComponent form;
	newtGrid grid,subgrid;
	int textWidth,textHeight;
	char *flowedTextSummary = NULL;
	char *flowedTextDescript = NULL;
	
	flowedTextSummary=newtReflowText((char*)szSummary,50,5,5,&textWidth,&textHeight);
	textbox_Summary = newtTextbox(-1,-1,50,textHeight,NEWT_FLAG_WRAP );
	newtTextboxSetText(textbox_Summary,flowedTextSummary);
	if(flowedTextSummary != NULL){
		free(flowedTextSummary);
		flowedTextSummary = NULL;
	}
	

	flowedTextDescript=newtReflowText((char*)szDescript,50,5,5,&textWidth,&textHeight);
	textbox_Descript = newtTextbox(-1,-1,50,4,NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);
	newtTextboxSetText(textbox_Descript,flowedTextDescript);
	if(flowedTextDescript != NULL){
		free(flowedTextDescript);
		flowedTextDescript = NULL;
	}
	
	btn_Ok=newtButton(-1,-1,BTN_OK_MSG);

	grid = newtCreateGrid(1,3);
	subgrid = newtCreateGrid(1,1);

	newtGridSetField(subgrid,0,0,NEWT_GRID_COMPONENT,btn_Ok,0,0,0,0,0,0);

	newtGridSetField(grid,0,0,NEWT_GRID_COMPONENT,textbox_Summary,0,0,0,1,0,0);
	newtGridSetField(grid,0,1,NEWT_GRID_COMPONENT,textbox_Descript,0,0,0,1,0,0);
	newtGridSetField(grid,0,2,NEWT_GRID_SUBGRID,subgrid,0,0,0,0,0,NEWT_GRID_FLAG_GROWX);

	newtGridWrappedWindow(grid,(char*)szName);

	form = newtForm(NULL,NULL,0);
	newtFormAddComponents(form,textbox_Summary,textbox_Descript,btn_Ok,NULL);
	
	newtRunForm(form);
	newtPopWindow();
	newtFormDestroy(form);
}

/* This is only for debug....
void classTui::debug_echo(char* szStatement,const char *szArg,int nArg)
{
	return ;
	char buffer[1000];
	
	memset(buffer,0,1000);
	snprintf(buffer,sizeof(buffer),"echo %s-%s , %d >> ./debug_msg",szStatement,szArg,nArg);
	system(buffer);
}
*/

/*!
@brief popup a debug dialog 

@param szMsg - Error messages
@param nErrNum - Error number
*/
void classTui::show_ErrorMsg(const char* szMsg,const int nErrNum)
{
	char szErrMsg[BUFF_SZ]={0};

	if((szMsg == NULL) && (nErrNum > 0)){
		return;
	}

	if( szMsg != NULL ){
		if( nErrNum > 0){
			snprintf(szErrMsg,sizeof(szErrMsg),"%s",szMsg);
		}else{
			snprintf(szErrMsg,sizeof(szErrMsg),"%s - \"%d\"",szMsg,nErrNum);
		}
	}else{
		snprintf(szErrMsg,sizeof(szErrMsg),"\"%d\"",nErrNum);
	}
			
	newtWinMessage(ERROR_MSG,BTN_OK_MSG,(char*)szErrMsg);
}

/*!
@brief the subprocess of the show() MessageBox

@param msg
@return PAGE - select(0:0k,  1:Cancel)
*/
int show_warn_msg_on_tui(char * msg)
{
        newtComponent btn_Ok;
        newtComponent textbox_msg;
        newtComponent form;
        newtGrid grid,subgrid;
	bool bResult=false;
        int textWidth,textHeight;
        char *flowedTextSummary = NULL;
        struct newtExitStruct es;

        PAGE pExitCode = INIT_VALUE;
        char result[2];
        char *szTitle = NULL;

        szTitle=(char*)calloc(sizeof(char),20);
        if(szTitle == NULL){
                
                pExitCode = INIT_VALUE;
                goto ret;
        }
        newtInit();
        newtCls();

        newtDrawRootText(0,0,TITLE);
        newtPushHelpLine(HELP_LINE);

        flowedTextSummary=newtReflowText(msg,70,5,5,&textWidth,&textHeight);
        textbox_msg = newtTextbox(-1,-1,70,textHeight,NEWT_FLAG_WRAP );
        newtTextboxSetText(textbox_msg ,flowedTextSummary);

        if(flowedTextSummary != NULL){
                free(flowedTextSummary);
        }

        btn_Ok=newtButton(-1,-1,BTN_OK_MSG);
        btn_Cancel=newtButton(-1,-1,BTN_CANCEL_MSG);

        grid = newtCreateGrid(1,7);
        subgrid = newtCreateGrid(2,1);
        newtGridSetField(subgrid,0,0,NEWT_GRID_COMPONENT,btn_Ok,0,0,0,0,0,0);
        newtGridSetField(subgrid,1,0,NEWT_GRID_COMPONENT,btn_Cancel,0,0,0,0,0,0);

        newtGridSetField(grid,0,0,NEWT_GRID_COMPONENT,textbox_msg ,0,0,0,1,0,0);
        newtGridSetField(grid,0,1,NEWT_GRID_SUBGRID,subgrid,0,0,0,0,0,NEWT_GRID_FLAG_GROWX);

        newtGridWrappedWindow(grid,TITLE);

        form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
	newtFormAddComponents(form,textbox_msg,btn_Ok,btn_Cancel,NULL);
	do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Ok){
			bResult = true;
                	pExitCode = PAGE_EXIT;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
			bResult = false;
                        pExitCode = PAGE_EXIT;
                }
        }while(pExitCode < 0);

        newtPopHelpLine();
        newtPopWindow();
        newtFormDestroy(form);

        if(szTitle != NULL){
                free(szTitle);
        }
ret:
        return bResult;
}


bool classTui::popup_ConflictDlg(void)
{
        vector<string>::iterator it;

	newtComponent textbox_Conflict,textbox_Descript;
	int textWidth,textHeight;
	char *flowedTextDescript = NULL;
        char *flowedTextConflict = NULL;

        newtComponent btn_Yes,btn_No;
	newtGrid grid,subgrid;
	
        newtComponent form;
        struct newtExitStruct es;
        PAGE pExitCode=INIT_VALUE;
        bool bRetVal;
	string strConflictMsg;

	strConflictMsg=read_LastErrorMsg();
	if(strConflictMsg.length() == 0){
        	m_refLogger->WriteLog_char(ERROR_LOG,MYSELF_NAME_TUI,"Cannot open last log file", NULL);
	}

	flowedTextDescript=newtReflowText(ERR_FOR_CONFLICT,60,5,5,&textWidth,&textHeight);
        textbox_Descript= newtTextbox(-1,-1,60,textHeight,NEWT_FLAG_WRAP );
        newtTextboxSetText(textbox_Descript,flowedTextDescript);
	if(flowedTextDescript != NULL){
        	free(flowedTextDescript);
		flowedTextDescript = NULL;
	}

	flowedTextConflict=newtReflowText((char*)(strConflictMsg.c_str()),60,4,4,&textWidth,&textHeight);
        textbox_Conflict= newtTextbox(-1,-1,60,4,NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);
        newtTextboxSetText(textbox_Conflict,flowedTextConflict);
	if(flowedTextConflict != NULL){
        	free(flowedTextConflict);
		flowedTextConflict = NULL;
	}

        btn_Yes = newtButton(-1,-1,BTN_YES_MSG);
        btn_No = newtButton(-1,-1,BTN_NO_MSG);

	grid = newtCreateGrid(1,3);
	subgrid = newtCreateGrid(2,1);

        newtGridSetField(subgrid,0,0,NEWT_GRID_COMPONENT,btn_Yes,0,0,0,0,0,0);
        newtGridSetField(subgrid,1,0,NEWT_GRID_COMPONENT,btn_No,0,0,0,0,0,0);

	newtGridSetField(grid,0,0,NEWT_GRID_COMPONENT,textbox_Descript,0,0,0,1,0,0);
        newtGridSetField(grid,0,1,NEWT_GRID_COMPONENT,textbox_Conflict,0,0,0,1,0,0);
        newtGridSetField(grid,0,2,NEWT_GRID_SUBGRID,subgrid,0,0,0,0,0,NEWT_GRID_FLAG_GROWX);

        newtGridWrappedWindow(grid,"Conflict Alert");

        form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);

        newtFormAddComponents(form,textbox_Descript,textbox_Conflict,btn_Yes,btn_No,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Yes){
                        bRetVal=true;   pExitCode=PAGE_EXIT;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_No){
                        bRetVal=false;  pExitCode=PAGE_EXIT;
                }
        }while(pExitCode < PAGE_EXIT);

        newtPopWindow();
        newtFormDestroy(form);

        return bRetVal;
}

string classTui::read_LastErrorMsg(void)
{
	string strErrorMsg;
	char strBuf[MAX_STRING];
        char strErrorPath[MAX_STRING];
	
	string strLogPath=m_refLogger->GetLogPath();

	if(strLogPath.rfind("/") != (strLogPath.length()-1)){
		strLogPath = strLogPath + "/";
	}
        snprintf(strErrorPath,sizeof(strErrorPath),"%s%s",(strLogPath).c_str(),LAST_ERROR_LOG_FILE);
	
        ifstream fin;

        fin.open(strErrorPath);
        while (fin.getline(strBuf, sizeof(strBuf)) > 0 ){
                strErrorMsg = strErrorMsg + (string)strBuf + (string)"\n";
        }
        fin.close();

	return strErrorMsg;
}
void classTui::setRedcastleStatus(int nStatus)
{
	m_nRedcastleStatus = nStatus;
}

void classTui::Exit(int err)
{
	int nResult;
	nResult = newtWinChoice(CONFIRM_WIN_TITLE, BTN_OK_MSG, BTN_CANCEL_MSG, EXIT_CONFIRM_MSG);
	if(nResult == 1)
	{	
		DeleteRpmFile();
		exit(err);
	}
}

//! @brief Delete downloaded rpm files.
bool classTui::DeleteRpmFile(void)
{
   vector<string> vectorSections;
   vector<string>::iterator it;
   classConfigParser *cp = NULL;

   string strCacheDir;
   string strPath;
   string strTemp;

   cp = new classConfigParser();

   if (!cp->Read(CONFIG_FILE)) {
       return false;
   }

   vectorSections = cp->GetSections();
   strCacheDir = cp->GetOption("main", "cachedir");
   strCacheDir = cp->StripRString(strCacheDir, '/');
   if (strCacheDir.empty() == false) {
       strPath = strCacheDir;
   }
   else {
       return false;
   }

   for (it = vectorSections.begin(); it != vectorSections.end(); it++) {
       strTemp = strPath + "/" + *it + "/" + "packages";
       struct dirent *item;
       DIR *dp;
       string strFileName;
       string strFilePath;
       dp = opendir(strTemp.c_str());
       if (dp != NULL) {
           for (;;) {
               item = readdir(dp);
               if (item == NULL)
                   break;
               strFileName = item->d_name;
               if (strFileName.find(".rpm") == string::npos)
                   continue;
               strFilePath = strTemp + "/" + strFileName;
               if (access(strFilePath.c_str(), F_OK) == 0) {
                   remove(strFilePath.c_str());
               }
           }
           closedir(dp);
       }
   }

   if (cp != NULL) {
       delete cp;
       cp = NULL;
   }

   return true;
}
