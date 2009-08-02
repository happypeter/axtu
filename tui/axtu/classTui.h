/*!
@file classTui.h
@brief Class header file for Text User Interface of Updater
*/
#ifndef CLASSTUI_H_
#define CLASSTUI_H_

#include <newt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <alloca.h>
#include <libintl.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stack>

#include "classConfCtl.h"
#include "classRpmEngine.h"
#include "classNetwork.h"
#include "classLogger.h"
#include "hsCommon.h"

using namespace std;
using std::ostringstream;

#define SUCCESS_FLAG 		0
#define FAIL_FLAG 		1
#define INTENDED_FAIL_FLAG 	2

#define MYSELF_NAME_TUI		"TUI"
// option string define
#define OPTION_SELFUPDATE	"selfupdate_list"
#define OPTION_UPDATE		"--update"
#define OPTION_INSTALL		"--install"
#define OPTION_ERASE		"--erase"
#define OPTION_FORCE		"--force"

// ShareMemory Tag
#define SHM_TAG_UPDATE		"update_restart"
#define SHM_TAG_INSTALL		"install_restart"
#define SHM_TAG_ERASE		"erase_restart"

#define SHM_TAG_WRONG_URL	"network_err_wrong_url"
#define SHM_TAG_NOT_CONNECT	"network_err_not_connect"
#define SHM_TAG_AUTH_FAIL	"network_err_auth_fail"
#define SHM_TAG_FWRITE_FAIL	"network_err_fwrite_fail"
#define SHM_TAG_UNKNOWN		"network_err_unknown"

#define SHM_TAG_ERR_CONFIG	"config_file_not_exist"
#define SHM_TAG_ERR_NOSPACE	"no_disk_space"
#define SHM_TAG_ERR_CANNOT_EXE_AUTH "cannot_exe_auth"
#define SHM_TAG_ERR_DIR_LENGTH_OVER "dir_length_over"
#define SHM_TAG_ERR_CONF_LENGTH_OVER "conf_length_over"

#define FIFO_PATH		"/tmp/hsupdater_tui_fifo"
#define FIFO_UNSET		"1"
#define FIFO_SET		"2"

#define HELP_LINE 		_("<Tab>/<Alt-Tab> between elements | <space> selects")
#define F1_HELP_LINE		_("Press <F1> for more information on a Package")
#define UPDATE_WIN_TITLE        _("Update Wizard")
#define INSTALL_WIN_TITLE       _("Install Wizard")
#define ERASE_WIN_TITLE        	_("Erase Wizard")
#define MYPKG_DETECT_TITLE	_("New AXTU packages have been detected.")
#define EXIST_UPDATE_LIST       _("Updated packages were found.")
#define NOT_EXIST_UPDATE_LIST   _("No updates were found.")
#define EXIST_INSTALL_LIST      _("Available packages were found.")
#define NOT_EXIST_INSTALL_LIST  _("No target package were found.")
#define EXIST_ERASE_LIST       	_("Packages to be removed.")
#define NOT_EXIST_ERASE_LIST   	_("No packages found to remove.")

#define MYPKG_DETECT_MSG 	_("New AXTU packages have been detected. You must update AXTU packages first.")
#define BTN_VIEW_LIST_MSG       _("List of packages available for update")
#define BTN_NEXT_MSG            _("Next >")
#define BTN_PREV_MSG		_("< Prev")
#define BTN_BLACKLIST_MSG       _("Set Black List")
#define BTN_DONE_MSG		_("Done")
#define BTN_RESTART_MSG		_("Restart")
#define BTN_RETRY_MSG		_("Retry")
#define BTN_CANCEL_MSG          _("Cancel")
#define BTN_QUIT_MSG          	_("Quit")
#define BTN_RELOAD_MSG		_("Reload")
#define BTN_OK_MSG        	_("Ok")
#define BTN_YES_MSG             _("Yes")
#define BTN_NO_MSG     	        _("No")
#define POPUP_WIN_TITLE         _("Available package list")
#define UPDATE_PKG_EXIST_MSG    _("There are packages available to update.")
#define UPDATE_PKG_NOT_EXIST_MSG _("There are no packages available to update.")
#define TOTAL_COUNT_MSG         _("Total Count : ")
#define BLACKLISTED_COUNT_MSG   _("Blacklisted Count : ")
#define DEP_BLK_MSG		_("Some blacklisted packages have been added to the target package list as dependent packages. \nPlease remove the following packages from the blacklist.")
#define DEP_INCOMPATIBLE_MSG		_("Some incompatible packages have been added to the target package list as dependent packages. \nPlease update the following packages manually.\nIf you want to update them, please visit the TSN web page.")
#define UPDATE_METHOD_SEL_MSG	_("Select a update method")
#define ERROR_MSG		_("Error")
#define WARNING_MSG 		_("Warning")
#define CREATELIST_FAIL_MSG	_("Failed to create Package List") 

// update method page
#define TOTAL_UPDATE_MENU	_(" Full Update ")
#define CUSTOM_UPDATE_MENU	_(" Custom Update ")

#define PROGRESS_PAGE_LABEL	_("Processing...")
#define TOTAL_PROGRESS_MSG	_("Total Progress")
#define CURRENT_PROGRESS_MSG	_("Current Progress")

#define UPDATE_OK_MSG		_("The update has been completed!")
#define INSTALL_OK_MSG		_("The installation has been completed!")
#define ERASE_OK_MSG		_("The package removal has been completed!")

#define UPDATE_FAIL_MSG		_("The update has failed!")
#define INSTALL_FAIL_MSG	_("The install has failed!")
#define ERASE_FAIL_MSG		_("The erase has failed!")

#define SELFUPDATE_OK_MSG	_("The AXTU update has been completed!  AXTU will restart now.") 
#define SELFUPDATE_FAIL_MSG	_("The AXTU update has failed!") 

#define DEPENDENCY_WIN_TITLE	_("Dependency Alert")
#define DEPENDENCY_MSG		_("Some selected packages have dependent packages.")
#define CONFIRM_MSG		_("You can see items that you selected.")
#define COMFIRM_FOR_INCOMPATIBLE_MSG1 _("Following incompatible package(s) are removed")
#define COMFIRM_FOR_INCOMPATIBLE_MSG2 _("on your select.")
#define COMFIRM_FOR_INCOMPATIBLE_MSG3 _("If you want to update these packages.")
#define COMFIRM_FOR_INCOMPATIBLE_MSG4 _("Please visit TSN web page.")
#define NEED_OR_NOT_DEP_MSG	_("I don't want to see a message for dependency.")
#define CONTINUE_MSG		_("Would you like to continue?")
#define UPDATE_EXIST_MSG	_("Please select packages you want to update.")
#define UPDATE_NO_EXIST_MSG	_("No packages are available.")
#define INSTALL_EXIST_MSG	_("Please select packages you want to install.")
#define INSTALL_NO_EXIST_MSG	_("No package are available.")
#define ERASE_EXIST_MSG		_("Please select packages you want to erase.")
#define ERASE_NO_EXIST_MSG	_("No package are available.")

#define CONFIRM_WIN_TITLE	_("Confirmation")
#define UPDATE_CONFIRM_MSG	_("Do you want to Update?")
#define INSTALL_CONFIRM_MSG	_("Do you want to Install?")
#define ERASE_CONFIRM_MSG	_("Do you want really to erase?")
#define EXIT_CONFIRM_MSG	_("Do you want to exit?")

#define URL_CONFIRM_MSG		_("Check update-server address in the configuration file")
#define CONNECT_ERR_MSG		_("Cannot connect with the TSN server.")
#define URL_ERR_MSG		CONNECT_ERR_MSG
#define CONNECT_CONFIRM_MSG	_("Please check the network connection!")
#define AUTH_ERR_MSG		_("Unable to authenticate from the update server.")
#define FWRITE_ERR_MSG		_("Can NOT write the file. Please, Check your disk space")
#define UNKNOWN_ERR_MSG		_("A unknown error occured.  This cannot happen.")
#define NOT_FIND_CONF_FILE_MSG	_("Can NOT find configuration file")
#define NOT_FIND_REQUIRE_MSG	_("Required packages have NOT been found!")

#define SEE_LOG_MSG		_("See the log file")
#define SEL_ALL_ITEM_MSG	_("Select All packages")
#define ERR_FOR_NOSPACE   	_("You don't have enough space for log. You have to enlarge log directory")

#define ERR_FOR_CONFLICT	_("Packages to be updated contain some files which also belong to different packages.\nDo you want to proceed?")
#define AUTH_FAIL_MSG		_("Authentication failed.")
#define CANNOT_EXECUTE_AUTH_MSG _("Cannot execute the authentication program.")
#define ERR_DIR_LENGTH_OVER_MSG _("The maximum length of the combined path and file name is 255 characters. \nPlease modify the entry.")
#define ERR_CANNOT_READ_CONF_FILE _("File is not correct. Please modify this file.")
#define ERR_BLANK_LOG_DIR _("Configured Log Directory is blank.  Please specify a valid directory name.")
#define ERR_NO_SELECT_MSG      	_("No packages selected.")

#define BLACKLISTED_PKG_ERR_MSG	_("You can delete blacklisted packages packages by setup blacklist.")
#define INCOMPATIBLE_PKG_ERR_MSG _("You have to update incompatible packages by hand.")
#define NOSPACE_INSTALL_ERR_MSG _("You don't have enough space for install. You have to enlarge your disk space.")
#define UPDATE_CANCEL_MSG	_("The task of update is canceled.")
#define INSTALL_CANCEL_MSG	_("The task of install is canceled.")
#define UNINSTALL_CANCEL_MSG	_("The task of uninstall is canceled.")


#define TUI_UPDATER_PATH 	"/usr/share/axtu/tui/axtu-tui"
#define TUI_LOADER_PATH 	"/usr/share/axtu/tui/axtu-launcher-tui"
#define TUI_SETUP_PATH 	"/usr/share/axtu/tui/axtu-setup-tui"
#define TUI_UPDATER_NAME	"axtu-tui"

#define TEXT_SZ 2048
#define BUFF_SZ 200

extern int errno;

//! structure for the share memory 
struct shared_use_st {
//    int written_by_you;
    char some_text[TEXT_SZ];
};


//! Callback structure for All package check on TUI
struct callbackInfo{
	newtComponent *en;
	int figure;
	char *state;
};

typedef enum {
	INIT_VALUE = -1,
	PAGE_EXIT,
	PAGE_BACK,
	
	UPDATE_METHOD_PAGE,
	
	INSTALL_PAGE,
	ERASE_PAGE,

	HEADER_WORK_PROGRESS_PAGE,
	UPDATE_INSTALL_PROGRESS_PAGE,
	ERASE_PROGRESS_PAGE,
	
	UPDATE_INSTALL_LIST_PAGE,
	ERASE_LIST_PAGE,
	
	DONE_OK_PAGE,
	DONE_FAIL_PAGE,
	DONE_SOME_PAGE,

	PROGRAM_RESTART
}PAGE;

typedef enum {
	BLACKLISTED_PKG,
	INCOMPATIBLE_PKG,
	NONE_AVAILABLE_PKG,
	NONE_ENOUGH_SPACE,
	USER_CANCEL,
}DONE_PAGE_CLASS;

typedef enum {
	UPDATE_MODE=1,
	INSTALL_MODE,
	ERASE_MODE,
}PROGRAM_MODE;

typedef enum {
	TOTAL_UPDATE=1,
	CUSTOM_UPDATE,
	SELF_UPDATE
}UPDATE_METHOD;

typedef enum {
	SUCCESS_LOAD,
	FAIL_LOAD,
	CANCEL_SELF_UPDATE
}VECTOR_LOAD_STATE;

/*!
@brief SessionBuffer (other words ScreenBuffer)

Session Buffer have the previous screen information.
*/
struct structSessionBuffer{
        PAGE SessionId;
        int nData;
        string strData;
};

void show_err_msg_on_tui(const char *,const char *);
int show_warn_msg_on_tui(char *);
//void show_warn_msg_on_tui(const char *,const char *);
//PAGE show_message(char * msg, int nType);

/*!
@brief

This Method return a file name
from the path string
*/
void pathFilter(const char *,char *);

/*!
@brief check all package list

This method present '*' into Pkglist-Checkbox
when user check a AllCheck-Checkbox
*/
static void AllCheckCallback(newtComponent *co,void *data);

/*!
@brief Class for the Text User Interface

The TUI environment is a console mode.
This class was implemented using the newt library.
*/
class classTui
{
public:
	classTui(int nMode,
		classConfCtl *refConfCtl,
		classConfigParser *refConfigParser,
		classLogger *refLogger,
		classNetwork *refNetwork,
		classRpmEngine *refRpmEngine);
	~classTui(void);

	int InitTui(void);
	void UninitTui(void);
	int show_ui(void);
	PAGE show_UpdateMethodPage(struct structSessionBuffer *structBuffer);
	bool show_HeaderWorkProgressPage(void);
	PAGE show_UpdateInstallProgressPage(void);
	PAGE show_EraseProgressPage(void);
	PAGE show_UpdateInstallListPage(void);
	PAGE show_EraseListPage(void);
	PAGE show_DonePage(bool bOkFlag);
	PAGE show_DonePage(DONE_PAGE_CLASS pageClass);
	VECTOR_LOAD_STATE show_LoadPage(void);
	void show_ErrorMsg(const char* szMsg=NULL,const int nErrNum=1);
	void show_NetworkErrorDialog(int nErr);

	bool popup_SelfUpdateMsg(void);
	bool popup_DepList(vector <string> str_vectorArg);
	bool popup_DepListWithSelList(vector <string> str_vectorArg);
	void popup_PkgSummary(const char* szName,const char* szSummary,const char* szDescript);	
	void popup_UpdateList(vector <string> vectorArg, vector <string> vectorIncompatibleArg);
	void popup_BListSetup(void);
	void popup_DepAndBlackList(void);
	bool popup_ConfirmWin(vector <structFileInfo> struct_vectorArg);
	bool popup_ConfirmWinRemoveKernel(vector <string> struct_vectorArg);
	bool popup_ConfirmWin(set <structHeaderInfo, DereferenceLess> struct_setArg);
	void popup_ConfirmWin_For_Incompatible(vector <structFileInfo> struct_vectorArg);
	bool popup_ConflictDlg(void);
	vector<string> popup_InstalledPkgs(vector <string> str_vectorArg);
	vector<string> popup_InputBList(vector <string> vectorArg);
	  
	static void ReadHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void GetHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void GetPackagesCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void RunCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void CommonCallBack(int p1, int p2, const char * msg1, const char * msg2 );	

	PROGRAM_MODE get_ProgramMode(void);
	void set_ProgramMode(PROGRAM_MODE mode=UPDATE_MODE);
	void setRedcastleStatus(int nStatus);
	static void Exit(int err);	
	static bool DeleteRpmFile(void);

private:
	int GetHeader(void);
	int updater_restart(void);
	bool restarter_core(char *szArg);
	bool shm_check(void);
	string make_CountMsg(int nWhatKind,int nCount);
	void set_WinTitle(void);
	PAGE set_RpmCommand(PROGRAM_MODE mode);
	void struct2str(vector <structAddedFile> vectorOrig,vector <string> &vectorTarget);
	void struct2str(vector <structFileInfo> vectorOrig,vector <string> &vectorTarget);
	void struct2str(set <structHeaderInfo, DereferenceLess> setOrig,vector <string> &vectorTarget);
	vector <string> GetDepList(vector <string> &str_vectorArg);
	VECTOR_LOAD_STATE vectorLoad(void);
	void debug_echo(char* szStatement,const char *szArg,int nArg);
	int ReadBlacklistConfig(void);
	void FilterBlacklist(vector <structFileInfo> &vectorFiltered, vector <structFileInfo> &vectorBlacklist, vector <structFileInfo> &vectorIncompatible);
	void make_DoneMsg(bool bOkFlag,char *szMsgBuffer);
	string read_LastErrorMsg(void);

        PROGRAM_MODE m_ProgramMode;
        int m_nUpdateMethod;
	int m_nRedcastleStatus;

	/*! 
	Initialize Actions(ex.Update or Install or Erase Actions) Flag
	If this flag is unset, it needs to call "show_LoadPage()" >> "vectorLoad()"
	*/
        bool m_bIsAction;

	/*!
	Header related Actions(ex.Update or Install or Erase Actions) Flag
	If this flag is unset, it needs to call "show_HeaderWorkProgressPage()"
	*/
	bool m_bIsHeaderWork;

	vector <structFileInfo> m_struct_vectorUpdateList;
	vector <structFileInfo> m_struct_vectorInstallList;
	set <structHeaderInfo, DereferenceLess> m_struct_setEraseList;
	
	vector <string> m_string_vectorUpdateList;
	vector <string> m_string_vectorInstallList;
  
	vector <structFileInfo> m_struct_vectorSelList;
	set <structHeaderInfo, DereferenceLess> m_struct_setEraseSelList;
	
	vector <string> m_string_vectorDepList;
	vector <structFileInfo> m_struct_vectorDoneList;
	set <structHeaderInfo, DereferenceLess> m_struct_setEraseDoneList;
	
	stack <structSessionBuffer> m_stackBuffer;

	// This vector is filtered by Blacklist-Filter
	vector <structFileInfo> m_struct_vectorFilteredList;
	vector <structFileInfo> m_struct_vectorBlackList;
	vector <structFileInfo> m_struct_vectorIncompatibleList;

	classRpmEngine * m_refRpmEngine;
	classNetwork * m_refNetwork;
	classLogger * m_refLogger;
	classConfigParser * m_refConfigParser;
	classConfCtl *m_refConfCtl;

	bool m_bShowDependencyMsg;
};
#endif /*CLASSTUI_H_*/
