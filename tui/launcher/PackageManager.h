/*!
@file PackageManager.h
@brief Launcher on TUI header file
*/
#ifndef __PACKAGEMANAGER_H__
#define __PACKAGEMANAGER_H__

#include <newt.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define _ gettext
#define PAGE_EXIT 		0

#define TITLE 			"[Asianux TSN Updater]"

#define HELP_LINE 		gettext("<Tab>/<Alt-Tab> between elements | <space> selects")
// button message
#define BTN_UPDATE_MSG          gettext("Online Package Update - To update installed packages")
#define BTN_INSTALL_MSG         gettext("Online Package Install - To install additional packages")
#define BTN_ERASE_MSG           gettext("Package Erase - To remove installed packages")
#define BTN_SETUP_MSG           gettext("Setup - To configure the environment")

#define BTN_NEXT_MSG            gettext("Next")
#define BTN_CANCEL_MSG          gettext("Cancel")
#define BTN_OK_MSG          	gettext("Ok")

#define MAIN_WIN_TITLE       	gettext("Asianux TSN Updater Launcher")
#define ERROR_WIN_TITLE       	gettext("Error")

// Error message
#define ERROR_MSG_WRONG_URL 	gettext("The URL of the update server is NOT correct!")
#define ERROR_MSG_NOT_CONNECT  	gettext("Can NOT connect to the updater server")
#define ERROR_MSG_AUTH_FAIL   	gettext("Can NOT authenticate from the update server")
#define ERROR_MSG_UNKNOWN   	gettext("Unknown error occured.")
#define ERROR_MSG_CONFIG_FILE  	gettext("Can NOT find the configuration file")
#define ERROR_MSG_MANY_ARG	gettext("There are too many arguments.")
#define ERROR_MSG_NOSPACE   	gettext("You don't have enough space for log. You have to enlarge log directory.")
#define ERROR_MSG_CANNOT_EXE_AUTH gettext("Cannot execute the authentication program.")
#define ERROR_MSG_DIR_LENGTH_OVER gettext("Sorry. Cache or Log directory is 255 over.")
#define ERROR_MSG_CONF_LENGTH_OVER gettext("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.")
#define ERROR_MSG_CANNOT_EXE	gettext("Fail to execute program")

// axtu commands
#define UPDATE_CMD "/usr/share/axtu/tui/axtu-tui -u"
#define INSTALL_CMD "/usr/share/axtu/tui/axtu-tui -i"
#define ERASE_CMD "/usr/share/axtu/tui/axtu-tui -e"
#define SETUP_CMD "/usr/share/axtu/tui/axtu-setup-tui"

#define MY_PATH 		"/usr/share/axtu/tui/axtu-launcher-tui"
#define MY_NAME 		"axtu-launcher-tui"

// ShareMemory Tag
#define SHM_TAG_UPDATE          "update_restart"
#define SHM_TAG_INSTALL         "install_restart"
#define SHM_TAG_ERASE           "erase_restart"

#define SHM_TAG_WRONG_URL       "network_err_wrong_url"
#define SHM_TAG_NOT_CONNECT     "network_err_not_connect"
#define SHM_TAG_AUTH_FAIL       "network_err_auth_fail"
#define SHM_TAG_FWRITE_FAIL     "network_err_fwrite_fail"
#define SHM_TAG_UNKNOWN         "network_err_unknown"

#define SHM_TAG_ERR_CONFIG      "config_file_not_exist"
#define SHM_TAG_ERR_NOSPACE     "no_disk_space"
#define SHM_TAG_ERR_CANNOT_EXE_AUTH "cannot_exe_auth"
#define SHM_TAG_ERR_DIR_LENGTH_OVER "dir_length_over"
#define SHM_TAG_ERR_CONF_LENGTH_OVER "conf_length_over"

#define UPDATE_OPTION		"--update"
#define INSTALL_OPTION		"--install"
#define ERASE_OPTION		"--erase"

#define TEXT_SZ 2048
//#define EXIT_FAILURE -1


struct shared_use_st {
//    int written_by_you;
    char some_text[TEXT_SZ];
};

static int nArgUpdate;
static int nArgInstall;
static int nArgErase;
static int nArgSetup;

static int nError;

int nSelect = -1;

static struct poptOption optionsTable[] = {


{ "update", (char) 'u', POPT_ARG_NONE,&nArgUpdate,0,

 "You can bring up to date pre-installed packages", NULL },

{ "install", (char) 'i', POPT_ARG_NONE,&nArgInstall,0,

 "You can setup the Not installed packages", NULL },

{ "erase", (char) 'e', POPT_ARG_NONE,&nArgErase,0,

 "You can erase already pre-installed packages", NULL },

{ "setup", (char) 's', POPT_ARG_NONE,&nArgSetup,0,

 "You can configure this program's environment", NULL },

POPT_TABLEEND

};

// Using popt library Get Option
void get_option(int argc,char **argv);

// ShareMemory Check
int shm_check(void);

void newt_unloading(newtComponent argForm);


#endif /* __PACKAGEMANAGER_H__ */
