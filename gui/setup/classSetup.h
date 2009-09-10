/*!
@file classSetup.h
@brief Class header file for setup on GUI
*/
#ifndef CLASSSETUP_H_
#define CLASSSETUP_H_

#include "ui/frmEditServer.h"
#include "ui/frmSetup.h"
#include "ui/frmAddAdditionalBlacklist.h" 
#include <hsCommon.h> 
#include "classConfigParser.h"
#include "classConfCtl.h"
#include <fstream>
#include <iostream>
#include "classLogger.h"
#include <qlineedit.h>
#define AXTU_SETUP_TITLE tr("Setup - Asianux TSN Updater")

#define BG_COLOR "#C7EEFF"
#define FG_COLOR "#0268A8"

#define MYSELF_NAME_SETUP "classSetup"

using namespace std;
enum {
	SERVER_SETUP=1, 
	PATH_SETUP,
	ALARM_SETUP,
	BLACKLIST_SETUP,
	LOG_SETUP
};


/*!
 * @brief Edit Server Dialog
 * 
 * Class for Edit Server Dialog.
 */
class classEditServer : public frmEditServer
{
	Q_OBJECT
public:
	classEditServer( QWidget *parent, WFlags f );
	virtual ~classEditServer();
	int Domodal(bool bEditSection=true);
	
	virtual void slotClickOk();
	virtual void slotClickCancel();
	
	void SetSection(QString strSection);
	void SetServer(QString strServer);
	void SetURL(QString strURL);
	
	QString GetSection();
	QString GetServer();
	QString GetURL();
	bool GetResult();
	
private:
	bool m_bResult;  
	QString m_strSection;
	QString m_strServer;
	QString m_strURL;	
};

/*!
 *@brief Additional Blacklist Dialog.
 * 
 * Class for Add Additional Blacklist Dialog.
 */
class classAddAdditionalBlacklist : public frmAddAdditionalBlacklist
{
	Q_OBJECT
public:
	classAddAdditionalBlacklist( QWidget *parent, WFlags f );
	virtual ~classAddAdditionalBlacklist();	
	int Domodal();
	string text();
	virtual void slotAdditionalOk(); 
	virtual void slotAdditionalCancel();	
	string m_strText;
	bool m_bResult;
private:
	
};
    
/*!
@brief Class for setup program. 

This program can setup axtsn-updater.
*/
class classSetup : public frmSetup
{
	Q_OBJECT
public:
	classSetup(int nMode, QWidget *parent, WFlags f );
	virtual ~classSetup(); 
	int Init(); 
	void Uninit();
	int ReadBlacklistConfig(); 
	int ReadLocalHeaderInfo();
	void GetServerList();
	void GetPathInfo();
	void GetAlarmInfo();
	void GetProxyInfo();
	bool GetLogCondition(QString strDate, QString strCmd, QString strName);
	int DoSave();	
	bool LoadConfig();
	virtual void slotAddBlacklist();
	virtual void slotRemoveBlacklist();
	virtual void slotAddAdditionalBlacklist(); 
	virtual void slotOK(); 
	virtual void slotApply();	
	virtual void done(int nRet);	
	virtual void slotAddServer();
	virtual void slotEditServer();
	virtual void slotEditServer(QListViewItem * item);
	virtual void slotRemoveServer();
	virtual void slotChangeDownPath();
	virtual void slotChangeLogPath();	
	virtual void slotChangeChkRemove();
	virtual void slotUpdateAlarmEnable();
	virtual void slotUpdateAlarmDisable();
	virtual void slotAlarmPeriod(const QString& );
	virtual void slotCheckApplyBlacklist();
	virtual void slotLogSearch();	
	virtual void slotCheckProxyEnable();
	virtual void slotHttpUrl(const QString&);
	virtual void slotHttpPort(const QString&);
	virtual void slotHttpsUrl(const QString&);
	virtual void slotHttpsPort(const QString&);	
	virtual void slotProxyId(const QString&);
	virtual void slotProxyPasswd(const QString&);
	virtual void slotHttpPortEdit(const QString&);
	virtual void slotHttpsPortEdit(const QString&);
	int ChangeProxyValue();
	QString ReadObinfo(QString strObsoletee);

private: 
	int m_nMode;
	QString m_strOldHttpPort;
	QString m_strOldHttpsPort;
	// new
	classConfigParser * m_configBlacklistUpdate;
	classConfigParser * m_configEnv;
	classConfigParser * m_configProxy;
	classConfCtl *m_ConfCtl;
	classLogger * m_Logger;
	
	bool m_bConfigEdited;  
	bool m_bInit;
	bool m_bClickNotifierButton;
	
};

#endif /*CLASSSETUP_H_*/
