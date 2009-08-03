/*!
@file hsCommon.h
@brief A common header file
*/
#ifndef STRUCTHSCOMMON_H_
#define STRUCTHSCOMMON_H_

#include <rpm/header.h>
#include <string>
using namespace std;

#define MAX_STRING 512
#define MAX_PATH 256

#define AUTHEN_FILE             "/var/axtu/asianux-auth"
#define NEW_CONFIG_FILE 	"/var/axtu/.axtu.new"
#define CONFIG_FILE 		"/etc/axtu/axtu.conf"
#define DEFAULT_CONFIG_FILE 	"/etc/axtu/.axtu.default"
#define BLACKLIST_FILE "/etc/axtu/blacklist.conf"
#define PROXY_CONFIG_FILE "/usr/share/axtu/.conf/.wgetrc"
#define RELAY_CONFIG_FILE "/etc/axtu-authen-client/relay.conf"
#define INCMP_CONFIG_FILE "/var/axtu/incompatible.conf"
#define INCMP_RPM_NAME    "axtu-common-info"
#define INCMP_NVRA_DUMMY  "axtu-common-info-0.0-1AX.noarch"

#define SUCCESS_LOG_FILE "success.log"
#define ERROR_LOG_FILE "error.log"
#define DEBUG_LOG_FILE "debug.log"
#define LAST_ERROR_LOG_FILE "lasterror.log"


#define SETUP_PATH "/usr/share/axtu/gui/axtu-setup-gui"
#define UPDATER_PATH "/usr/share/axtu/gui/axtu-gui"
#define NOTIFIER_PATH "/usr/share/axtu/gui/axtu-notifier-gui"
#define NOTIFIER_NAME "axtu-notifier-gui"

#define QM_FILE_PATH "/usr/share/axtu/gui/qm"

#define DEFAULT_ICON "/usr/share/axtu/gui/images/updateicon_title.png"
#define UPDATE_COUNT_TMP "/var/axtu/axtu-update-count"
#define CHECK_NOTIFIER "kill -USR1 `/sbin/pidof axtu-notifier-gui` 2> /dev/null"
#define RESTART_NOTIFIER_DAEMON "/etc/init.d/axtu-notifier restart > /dev/null"
#define CHECK_NOTIFIER_DAEMON "kill -USR1 `/sbin/pidof axtu-daemon` 2> /dev/null"
#define RC_KMOD_NAME "kmod-redcastle"

#define __DEFAULT__ "__DEFAULT__SECTION__"

#define UPDATE 1
#define REMOVE 2
#define REQDEP 4
#define OTHERDEP 8


#define WELCOME_PAGE 0
#define SELECT_COMMAND_PAGE 1 
#define SELECT_PACKAGE_PAGE 2
#define PROGRESS_PAGE 3
#define FINISH_PAGE 4

#define TITLE "[Asianux TSN Updater]"

struct structCacheDirInfo
{		
	string strCacheDir;	
	string strURL;
	string strHeaderInfoFile;
	string strName;
};

struct structHeaderInfo
{	
	string strEpoch;
	string strNVRA;
	int nType; 									// Which cache dir(If value is -1 then local rpmDB header info)
	bool bSelfUpdate;
};


struct structFileList {
    char **fileList;
    int matchNumber;
    Header h2;
    struct structFileList *next;
};

struct structRPMInfo
{
	char *name;
	char *version;
	char *release;
	char *arch;
	char *group;
	int size;
	int matchNumber;
	int upgradeFlag;
	int disknum;
	char * summary;
	char * shortDesp;
	char ** fileList;
	char ** requireName;
	char ** requireVersion;
	char ** provideName;
	char ** provideVersion;
	char ** obsoleteName;
	char ** obsoleteVersion;
	int requireFileNumber;
	struct structFileList *containFiles;
	Header h;
};


struct structFileInfo
{
	string strName;
	string strVersion;
	string strRelease;
	string strArch;
	string strEpoch;	  
	string strCachePath; 
	string strURLFullPath; 
	int nType;
	bool bBlacklisted;
	bool bSelfUpdate;
  bool bIncompatible;
};


struct structAddedFile {
	const char * strFile;
	int nType;    	
	int nUpgrade;
	bool bIncompatible;
};

#endif /*STRUCTHSCOMMON_H_*/

