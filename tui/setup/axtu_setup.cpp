/*!
@file axtu_setup.cpp
@brief Class source file for set on TUI 
*/
#include <rpm/stringbuf.h>
#include <rpm/rpmlib.h>
#include <rpm/header.h>
#include <rpm/rpmts.h>
#include <rpm/rpmds.h>
#include <rpm/rpmcli.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmspec.h>
#include <rpm/rpmfi.h>
#include <rpm/db.h>

#include "axtu_setup.h"
#include "commondef.h"

//! A Constructor
classBlacklist::classBlacklist(void)
{
	m_configBlacklistUpdate = new classConfigParser();
	ReadLocalHeaderInfo();
}

//! A Destructor
classBlacklist::~classBlacklist(void)
{
	delete m_configBlacklistUpdate;
	if(unlink(AXTU_SETUP_TUI_PID_FILE) != 0)
	{
		exit(1);
	}
}

void classBlacklist::shm_write(char *szArg)
{
	void *shared_memory = (void *)0;
        struct shared_use_st *shared_stuff;
        int shmid;

        shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

        if (shmid == -1) {
                fprintf(stderr, "shmget failed\n");
                exit(EXIT_FAILURE);
        }

        shared_memory = shmat(shmid, (void *)0, 0);
        if (shared_memory == (void *)-1) {
                fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
        }

        shared_stuff = (struct shared_use_st *)shared_memory;
        strncpy(shared_stuff->some_text, szArg, TEXT_SZ);

        if (shmdt(shared_memory) == -1){
                fprintf(stderr, "shmdt failed\n");
                exit(EXIT_FAILURE);
        }
}

void classBlacklist::newt_unloading(newtComponent argForm)
{
	newtPopHelpLine();
  	newtPopWindow();
  	newtFormDestroy(argForm);
	newtFinished();
}


/*!
@brief popup a dialog that can edit a blacklist

A user is able to prevent updating some packages by adding blacklist
*/
void classBlacklist::popup_BListSetup(void)
{
        newtComponent listbox_BList;
        newtComponent label_BListMsg1,label_BListMsg2;
        newtComponent btn_Save,btn_Cancel,btn_Delete;
        newtComponent btn_SelectList,btn_Input;
        newtComponent entry_TextInput;
        newtComponent form,listbox_form;
        newtGrid grid,subgrid,buttons;
        vector <string> vectorBlackUpdate;
        vector <string> str_vectorTemp;
        void **selectedList;
        int nSel=0;
        int i=0;
	int nArray[1000]={0};

        struct newtExitStruct es;
        int nExitCode=-1;
        int nCount=1;

	newtInit();
	newtCls();

  	newtDrawRootText(0,0,TITLE);

  	newtPushHelpLine(HELP_LINE);

        newtCenteredWindow(60,16,BLACKLIST_WIN_TITLE);

        label_BListMsg1=newtLabel(3,1,BLACKLIST_MAIN_MSG_HEAD);
        label_BListMsg2=newtLabel(3,2,BLACKLIST_MAIN_MSG_TAIL);
        listbox_BList=newtListbox(3,4,5,NEWT_FLAG_SCROLL | NEWT_FLAG_MULTIPLE);
        newtListboxSetWidth(listbox_BList,56);

///////////////////////////////////////////////////////////////////////////
        m_configBlacklistUpdate->Read(BLACKLIST_FILE);
		
        vectorBlackUpdate = m_configBlacklistUpdate->GetOptions("blacklist-update");

        vector <string>::iterator it;
        for(it=vectorBlackUpdate.begin();it != vectorBlackUpdate.end();it++)
        {
                newtListboxAppendEntry(listbox_BList,it->c_str(),(void*)nCount++);
        }

///////////////////////////////////////////////////////////////////////////
        btn_SelectList = newtCompactButton(4,10,BTN_ADD_FROM_LIST_MSG);
        btn_Input = newtCompactButton(40,10,BTN_ADD_BY_HAND_MSG);

        btn_Save=newtButton(8,12,BTN_SAVE_MSG);
        btn_Delete=newtButton(23,12,BTN_DEL_MSG);
        btn_Cancel=newtButton(40,12,BTN_CANCEL_MSG);

        listbox_form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(listbox_form,listbox_BList,NULL);

        form = newtForm(NULL,NULL,NEWT_FLAG_NOF12);

        newtFormAddComponents(form,label_BListMsg1,label_BListMsg2,listbox_form,        \
                btn_SelectList,btn_Input,btn_Save,btn_Delete,btn_Cancel,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Save){
                        m_configBlacklistUpdate->Write();
//                        bIsAction=false;
			shm_write("saved");
                        nExitCode=0;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
			shm_write("canceled");
                        nExitCode=0;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_SelectList){
                        str_vectorTemp.clear();
                        str_vectorTemp=popup_InstalledPkgs(str_vectorTemp);
                        for(it=str_vectorTemp.begin();it!=str_vectorTemp.end();it++){
                                if(m_configBlacklistUpdate->GetOption("blacklist-update",*it)==""){
                                        m_configBlacklistUpdate->SetOption("blacklist-update",*it,"0");
                                        vectorBlackUpdate.push_back(*it);
                                        newtListboxAppendEntry(listbox_BList,it->c_str(),(void*)nCount++);
                                }
                        }
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Input){
                        str_vectorTemp.clear();
                        str_vectorTemp=popup_InputBList(str_vectorTemp);
                        for(it=str_vectorTemp.begin();it!=str_vectorTemp.end();it++){
                                if(m_configBlacklistUpdate->GetOption("blacklist-update",*it)==""){
                                        m_configBlacklistUpdate->SetOption("blacklist-update",*it,"0");
                                        vectorBlackUpdate.push_back(*it);
                                        newtListboxAppendEntry(listbox_BList,it->c_str(),(void*)nCount++);
                                }
                        }
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Delete){
                        int nSelPosition=0;
                        selectedList=newtListboxGetSelection(listbox_BList,&nSel);
                        if(selectedList){
                                for(i=0;i < nSel;i++){
                                        nSelPosition = (long)selectedList[i];
                                        m_configBlacklistUpdate->RemoveOption("blacklist-update",vectorBlackUpdate[nSelPosition-1]);
                                        int k=0;
                                        for(it=vectorBlackUpdate.begin();it!=vectorBlackUpdate.end();it++){
                                                if(nArray[k]!=1){
                                                        if(k == nSelPosition-1){
								nArray[k]=1;
                                                        }else{
                                                                k++;
                                                        }
                                                }
                                        }
                                        newtListboxDeleteEntry(listbox_BList,(void*)nSelPosition);
                                }
                                newtListboxClearSelection(listbox_BList);
                        }else{
				newtWinMessage(WARNING_MSG,BTN_OK_MSG,ERR_NO_SELECT_MSG);
			}
                }
        }while(nExitCode < 0);

	newt_unloading(form);
}

int classBlacklist::ReadLocalHeaderInfo(void)
{
        int nCount=0;

        rpmdbMatchIterator mi;
        const char * np=NULL;
        const char * ep=NULL;
        const char * ap=NULL;
        const char * vp=NULL;
        const char * rp=NULL;
        Header hdr;
        int rc;
	char strTemp[MAX_STRING];

	if (rpmReadConfigFiles(NULL, NULL) != 0) {  // This function makes memory leak (396 byte)
		fprintf(stdout, "%s: failed to open RPM configuration file\n", AXTU_SETUP_TUI_EXE_FILE);
		exit(1);
	}

        rpmts m_rpmTs = rpmtsCreate();

        rpmtsSetRootDir(m_rpmTs, NULL);

	rc = rpmtsOpenDB(m_rpmTs, O_RDONLY);
	if (rc != 0) {
		rpmtsFree(m_rpmTs);
		fprintf(stdout, "%s: failed to open RPM database\n", AXTU_SETUP_TUI_EXE_FILE);
		exit(1);
	}

        mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_PACKAGES, NULL, 0);
	if (mi == NULL) {
		rpmtsCloseDB(m_rpmTs);
		rpmtsFree(m_rpmTs);
		fprintf(stdout, "%s: failed to initialize RPM iterator\n", AXTU_SETUP_TUI_EXE_FILE);
		exit(1);
	}

	string_setEraseList.clear();

        nCount = 0;
        while ((hdr = rpmdbNextIterator(mi)) != NULL)
        {
                int recOffset = rpmdbGetIteratorOffset(mi);
                if (recOffset)
                {
                        rc = headerNEVRA(hdr,&np,&ep, &vp, &rp, &ap); /* always return 0 */
                        if (rc)
                        {
                                rpmdbFreeIterator(mi);
				rpmtsCloseDB(m_rpmTs);
				rpmtsFree(m_rpmTs);
                                return -1;

                        }

                }
                snprintf(strTemp, sizeof(strTemp), "%s", np);

                string_setEraseList.insert(strTemp);
                nCount++;
        }

        rpmdbFreeIterator(mi);
	rpmtsCloseDB(m_rpmTs);
	rpmtsFree(m_rpmTs);

        return 0;
}

/*!
@brief pop-up installed-list dialog for Blacklist

A user can add a blacklist by select on list
@param str_vectorArg - already installed package list on system
@return vector<string> - selected list
*/
vector<string> classBlacklist::popup_InstalledPkgs(vector <string> str_vectorArg)
{
        newtComponent listbox_InstalledPkgs;
        newtComponent label_InstalledMsg;
        newtComponent btn_Ok,btn_Cancel;
        newtComponent form;
        struct newtExitStruct es;
        int nExitCode=-1;
        string strPkgName;
        int nCount=1;
        void **selectedList;
        int nSel;
        set <string>::iterator it;

        newtCenteredWindow(50,13,BLACKLIST_SEL_WIN_TITLE);

        label_InstalledMsg=newtLabel(1,1,BLACKLIST_SEL_MSG);
        listbox_InstalledPkgs=newtListbox(1,3,5,NEWT_FLAG_SCROLL | NEWT_FLAG_MULTIPLE);
        newtListboxSetWidth(listbox_InstalledPkgs,48);

        for(it=string_setEraseList.begin();it != string_setEraseList.end();it++){
                newtListboxAppendEntry(listbox_InstalledPkgs,it->c_str(),(void*)nCount++);
        }

        btn_Ok = newtButton(10,9,BTN_OK_MSG);
        btn_Cancel = newtButton(27,9,BTN_CANCEL_MSG);

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_InstalledMsg,  \
                listbox_InstalledPkgs,btn_Ok,btn_Cancel,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Ok){
                        nExitCode=0;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
                        nExitCode=0;
                }
        }while(nExitCode < 0);

        selectedList=newtListboxGetSelection(listbox_InstalledPkgs,&nSel);

        int i=0;
        nCount=1;
        if(selectedList){
                for(i=0;i < nSel;i++){
                        nCount=1;
                        for(it = string_setEraseList.begin();it!=string_setEraseList.end();it++){
                                if(nCount == (long)selectedList[i]){
                                        str_vectorArg.push_back(*it);
                                        break;
                                }else{
                                        nCount++;
                                }
                        }
                }
        }

        newtPopWindow();
        newtFormDestroy(form);

        return str_vectorArg;
}

/*!
@brief pop-up text-input dialog for Blacklist

A user can add a blacklist by typewrite directly
@param vectorArg - vector to save a blacklist string
@return vector<string> - inputed blacklist
*/
vector<string> classBlacklist::popup_InputBList(vector <string> vectorArg)
{
        newtComponent label_InputMsg1,label_InputMsg2,label_ExampleMsg;
        newtComponent entry_TextInput;
        newtComponent btn_Ok,btn_Cancel;
        newtComponent form;
        struct newtExitStruct es;
        int nExitCode=-1;
        bool bRetVal;
        const char* szValue;
        char *szTemp;
        char seps[]=" ,";
        char *szToken;

        newtCenteredWindow(40,11,BLACKLIST_INPUT_WIN_TITLE);

        label_InputMsg1=newtLabel(1,1,BLACKLIST_INPUT_MSG_HEAD);
        label_InputMsg2=newtLabel(1,2,BLACKLIST_INPUT_MSG_TAIL);
        label_ExampleMsg=newtLabel(1,3,BLACKLIST_INPUT_EX_MSG);

        entry_TextInput=newtEntry(3,5,NULL,35,&szValue,NEWT_FLAG_SCROLL);

        btn_Ok = newtButton(10,7,BTN_OK_MSG);
        btn_Cancel = newtButton(20,7,BTN_CANCEL_MSG);

        form=newtForm(NULL,NULL,NEWT_FLAG_NOF12);
        newtFormAddComponents(form,label_InputMsg1,label_InputMsg2, \
                        label_ExampleMsg,entry_TextInput,btn_Ok,btn_Cancel,NULL);

        do{
                newtFormRun(form,&es);

                if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Ok){

                        szTemp=(char*)alloca(sizeof(*szTemp)*(strlen(szValue)+1));
                        memset(szTemp,0,sizeof(*szTemp)*(strlen(szValue)+1));
                        strncpy(szTemp,szValue,strlen(szValue));

                        szToken=(char*)strtok(szTemp,seps);

                        if(szToken == NULL){
                                goto ret;
                        }

                        vectorArg.push_back((string)szToken);

                        while(szToken != NULL){
                                szToken=(char*)strtok(NULL,seps);
                                if(szToken != NULL){
                                        vectorArg.push_back((string)szToken);
                                }
                        }
ret:
                        nExitCode=0;
                }else if(es.reason == es.NEWT_EXIT_COMPONENT && es.u.co == btn_Cancel){
                        nExitCode=0;
                }
        }while(nExitCode < 0);

        newtPopWindow();
        newtFormDestroy(form);

        return vectorArg;
}
