/*!
@file axtu_setup.h
@brief Class header file for set on TUI 
*/
#ifndef __CLASSBLACKLIST_SETUP_H__
#define __CLASSBLACKLIST_SETUP_H__

#include <libintl.h>
#include <locale.h>
#include <newt.h>
//#include <popt.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../libs/classConfigParser.h"
#include "../libs/hsCommon.h"

using namespace std;


#define PAGE_EXIT 	0

//#define HAANSOFT_COPYRIGHT      "AXTU 1.0 - (c) 2006 HAANSOFT, Inc."
#define HELP_LINE               gettext("<Tab>/<Alt-Tab> between elements | <space> selects")
#define BTN_CANCEL_MSG          gettext("Cancel")
#define BTN_OK_MSG          	gettext("Ok")
#define BTN_ADD_FROM_LIST_MSG	gettext("Add from Package List")
#define BTN_ADD_BY_HAND_MSG	gettext("Add Manually")
#define BTN_SAVE_MSG		gettext("Save")
#define BTN_DEL_MSG		gettext("Delete")



#define BLACKLIST_WIN_TITLE	  gettext("BlackList Setup")
#define BLACKLIST_SEL_WIN_TITLE   gettext("Select Packages")
#define BLACKLIST_SEL_MSG 	  gettext("Select Packages in order to list on the blacklist")
#define BLACKLIST_MAIN_MSG_HEAD	  gettext("Blacklisted packages will NOT be updated.")
#define BLACKLIST_MAIN_MSG_TAIL	  gettext(" ")
#define BLACKLIST_INPUT_WIN_TITLE gettext("Input Target Packages")
#define BLACKLIST_INPUT_MSG_HEAD  gettext("Input packages you wish to list")
#define BLACKLIST_INPUT_MSG_TAIL  gettext("that can includes wildcard(*)")
#define BLACKLIST_INPUT_EX_MSG    gettext("Ex) kde*, *paint*, *-devel")

#define ERR_NO_SELECT_MSG         gettext("No items selected.")
#define ERROR_MSG                 gettext("Error")
#define WARNING_MSG		  gettext("Warning")


#define TEXT_SZ 2048
//#define EXIT_FAILURE -1

#ifndef AXTU_SETUP_TUI_EXE_FILE
#define AXTU_SETUP_TUI_EXE_FILE "axtu-setup-tui"
#endif

struct shared_use_st {
//    int written_by_you;
    char some_text[TEXT_SZ];
};

/*!
@brief Class for editing a blacklist on TUI

A User is able to edit blacklist on the Console.
This class is used newt library
*/
class classBlacklist
{
public:
	classBlacklist(void);	
	~classBlacklist(void);
	void popup_BListSetup(void);
	vector<string> popup_InputBList(vector <string> vectorArg);
	vector<string> popup_InstalledPkgs(vector <string> str_vectorArg);
private:
	int ReadLocalHeaderInfo(void);
	void newt_unloading(newtComponent argForm);
	void shm_write(char *szArg);

	classConfigParser *m_configBlacklistUpdate;		
	set <string> string_setEraseList;
};

#endif
