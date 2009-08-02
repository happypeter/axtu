/*!
@file classGui.h
@brief Class header file for Graphic User Interface
*/
#ifndef CLASSGUI_H_
#define CLASSGUI_H_

#include "ui/frmMainDialog.h" 
#include "ui/frmDepList.h"
 
#include "classRpmEngine.h"
#include "classNetwork.h"
#include "classAuthen.h"

#include <qlistview.h>
#include <qprocess.h>
#include "classConfigParser.h"
#include "classConfCtl.h"

#include <hsCommon.h>
#define MYSELF_NAME_AXTU "GUI"

#define AXTU_TITLE tr("Asianux TSN Updater")

/// Network error messages
#define URL_CONFIRM_MSG_C   "Check the TSN Server address in /etc/axtu/axtu.conf."
#define CONNECT_ERR_MSG_C   "Required data were not found on the server."
#define URL_ERR_MSG_C     CONNECT_ERR_MSG_C
#define CONNECT_CONFIRM_MSG_C "Network error."
#define AUTH_ERR_MSG_C        "Cannot connect to the authentication server."
#define UNKNOWN_ERR_MSG_C     "An Unknown Error has occurred."
#define USERCANCEL_ERR_MSG_C  "Download is canceled."
#define USERFWRITE_ERR_MSG_C  "Can NOT write the file. Please, Check your disk space."

#define URL_CONFIRM_MSG   tr("Check the TSN Server address in /etc/axtu/axtu.conf.")
#define CONNECT_ERR_MSG   tr("Required data were not found on the server.")
#define URL_ERR_MSG CONNECT_ERR_MSG
#define CONNECT_CONFIRM_MSG tr("Network error.")
#define AUTH_ERR_MSG        tr("Cannot connect to the authentication server.")
#define UNKNOWN_ERR_MSG     tr("An Unknown Error has occurred.")
#define USERCANCEL_ERR_MSG  tr("Download is canceled.")
#define USERFWRITE_ERR_MSG  tr("Can NOT write the file. Please, Check your disk space.")

/// Progress messages
#define PROGRESS_1     tr("Reading header information...")
#define PROGRESS_2     tr("Finish")
#define PROGRESS_3     tr("Remove finished....")
#define PROGRESS_4     tr("Start RPM callback install...")
#define PROGRESS_5     tr("Progress....")
#define PROGRESS_6     tr("Installation progress....")
#define PROGRESS_7     tr("Preparing...")
#define PROGRESS_8     tr("Preparing packages for installation...")
#define PROGRESS_9     tr("Stop transaction...")
#define PROGRESS_10    tr("Repackaging erased files...")
#define PROGRESS_11    tr("Progress repackage...")
#define PROGRESS_12    tr("Stop Repackage...")
#define PROGRESS_13    tr("Upgrading packages...")
#define PROGRESS_14    tr("Removing packages...")
#define PROGRESS_15    tr("Removing old version of packages....")
#define PROGRESS_16    tr("Reading remote header info....")
#define PROGRESS_17    tr("Finished reading remote header info....")
#define PROGRESS_18    tr("Start read local header info...")
#define PROGRESS_19    tr("Reading local header info....")
#define PROGRESS_20    tr("Gathering package information in finished.")
#define PROGRESS_21    tr("Create update and install list...")
#define PROGRESS_22    tr("Gathering package information from the Asianux TSN Server....Please wait several minutes.")
#define PROGRESS_23    tr("Download has finished.")
#define PROGRESS_24    tr("Downloading packages....")
#define PROGRESS_25    tr("Downloading Header files....")
#define PROGRESS_26    tr("Verifying package integrity....")
#define PROGRESS_27    tr("Verifying Header files integrity....")

/// Images
#define TOP_BANNER_IMG "/usr/share/axtu/gui/images/Update_top_bg.png"
#define BOTTOM_BANNER_IMG "/usr/share/axtu/gui/images/Update_bottom_bg.png"
#define UPDATE_ICON_IMG "/usr/share/axtu/gui/images/Update_TT.png"
#define INSTALL_ICON_IMG "/usr/share/axtu/gui/images/Install_TT.png"
#define ERASE_ICON_IMG "/usr/share/axtu/gui/images/Erase_TT.png"

/// Colors
#define BG_COLOR "#C7EEFF"
#define FG_COLOR "#0268A8"
#define FG_TOP_B_COLOR "#007DCC"

/// Finish errors
#define FC_THERE_ARE_NO_COUNT 1
#define FC_OUT_OUT_MAX_LENGTH 2
#define FC_INIT_ERROR 3
#define FC_STOP_DOWNLOAD 4
#define FC_NETWORK_ERROR 5
#define FC_USER_CANCEL 6
#define FC_BLACKLIST 7
#define FC_INCOMPATIBLE 8

#define FC_NOT_ENOUGH_DISK_SPACE -11

#define COMPATIBLE_YES "yes"
#define COMPATIBLE_NO "no"


// Translation message define
#define TR_OK tr("&Ok")
#define TR_YES tr("&Yes")
#define TR_NO tr("&No")
#define TR_CANCEL tr("&Cancel")
#define TR_NEXT tr("&Next>>")
#define TR_RETRY tr("&Retry")
#define TR_DONE tr("&Done")
#define TR_INSTALL_MORE_PACKAGES tr("&Install more packages")
#define TR_ERASE_MORE_PACKAGES tr("&Erase more packages")
#define TR_UPDATE_MORE_PACKAGES tr("&Update more packages")

/// Ther variable of finish error. 
static int m_nFinishErrNum;

/// Operation mode
enum { 
	UPDATE_MODE=1,
	INSTALL_MODE,
	ERASE_MODE,
};



/*!
@brief Class for dependency dialog. 

Class for dependency dialog. 
*/
class classDepListDlg : public frmDepList
{
	Q_OBJECT
public:
	classDepListDlg(QWidget *parent);
	~classDepListDlg();
	void AddMessage(QString szFile);
	void ClearMessage();	
public slots:
	void ClickedClose();        
	int Domodal();
};


/*!
@brief Class for GUI-Updater. 

Class for GUI-Updater
*/

class classGui : public MainDialog  
{
	Q_OBJECT
public: 
	classGui(int nMode);
	virtual ~classGui();
	virtual void reject();	
	
	bool show_NetworkErrorDialog(int nErr);
	void ShowList();
	
	static void CommonCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void ReadHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void GetHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void GetPackagesCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void RunCallBack(int p1, int p2, const char * msg1, const char * msg2 );
	static void CheckCallBack(int p1, int p2, const char * msg1, const char * msg2 );

	
	
	bool ReadHeaders();
	int ProceedUpdate();	
	
	
	virtual void slotCancel();
	virtual void slotSelectCommand(int nIndex);
	virtual void slotBack();
	virtual void slotNext(); 
	virtual void slotSelectCustom();
	virtual void slotSelectFull();
	virtual void slotSelectAll(int nSelect);
	virtual void slotChangeListPackage(QListViewItem * item);
	virtual void slotSetupBlacklist();
	virtual void slotClickPackage(QListViewItem * item);
	void Exit(int nRet);	
	
	int ShowWelcomePage();
	int ShowSelectCommandPage();
	int ShowSelectPackagePage();
	int ShowProgressPage(bool bJustShow=false);
	int ShowFinishPage(int nErr=m_nFinishErrNum);
	
	int  ShowPage(int nPage);
	void ShowCurPage();
	int MoveNextPage(int nStep=1);
	int MoveBackPage(int nStep=1);
	
	int CheckChildItem(QCheckListItem* item, bool b);
	
	int GetCurPage();
	void SetCurPage(int nPage);
	
	void SetPackageSummary(const char * strNVRA);
	
	void LockWigets();
	void UnlockWigets();
	
	void SetSelectCommand(int nIndex);
	
	bool InitRpm();
	void ReInitRpm();
	void ShowCompleteMessage();
	
	void ClearPackageList(); 
	void AddPackageList(vector <structFileInfo> vectorFileInfo);
	bool CheckConfigFileLength();
	void UnInitVariable();
	void ShowRemovedIncompatiblePackages();
	
public slots:
	void slotSetupBlackExit();
  
private:	
	// Need new operation
	QProcess * m_processSetupBlack;
	classRpmEngine * m_rpmEngine;  
	classNetwork * m_Network;
	classLogger * m_Logger;	
	QLabel *m_img_contain;
	
	int m_nPage;	
	int m_nMode;
	int m_nSelfUpdateMode;
	classConfigParser m_configEnv;	
	string m_strLogPath; 
	bool m_bAuthen;
	bool m_bDownload;	
	bool m_bCanNotAskAboutDependency;	
	int  m_nUpdateCount;
	int m_nRedcastleStatus;
	int m_nIncompatibleCount;
	vector <string> m_vectorRemovedIncompatiblePackages;

};
#endif /*CLASSGUI_H_*/
