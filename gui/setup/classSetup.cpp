/*!
@file classSetup.cpp
@brief Class source file for setup on GUI
*/

#include "classSetup.h"
#include <qlistbox.h>

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
#include <fcntl.h>
#include <set.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qfile.h>
#include <stdio.h>
#include <exception>
#include <commondef.h>
#include <qevent.h>

char strDubugMsg[MAX_STRING];

classEditServer::classEditServer(QWidget *parent, WFlags f ):frmEditServer(parent,0,FALSE,f)
{
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setPaletteBackgroundColor(QColor(BG_COLOR));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnOk->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnOk->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnCancel->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnCancel->setPaletteForegroundColor(QColor(FG_COLOR));
	
	m_bResult = false;
	this->setCaption(AXTU_SETUP_TITLE);	
}

classEditServer::~classEditServer()
{
}

//! Edit server url.
int classEditServer::Domodal(bool bEditSection)
{
	if (bEditSection == false)
	{
		editSectionName->setEnabled(false);
	}
	
	editSectionName->setText(m_strSection);
	editServerName->setText(m_strServer);
	editURL->setText(m_strURL);
		
	return exec();
}
	
//! Click Ok button.
void classEditServer::slotClickOk()
{
	m_strSection = editSectionName->text().ascii();
	m_strServer = editServerName->text().ascii();
	m_strURL = editURL->text().ascii();	
	m_bResult = true;
	done(0);	
	
}

//! Click Cancel button.
void classEditServer::slotClickCancel()
{
	m_strSection = "";
	m_strServer = "";
	m_strURL = "";
	m_bResult = false;
	done(2);
}

//! You can get result that user has clicked Ok(true) or Cancel(false).
bool classEditServer::GetResult()
{
	return m_bResult;
}

//! Set section on .conf file. 
void classEditServer::SetSection(QString strSection)
{
	m_strSection = strSection;	
}

//! Set server name on .conf file.
void classEditServer::SetServer(QString strServer)
{	
	m_strServer = strServer;
}

//! Set server URL on .conf file.
void classEditServer::SetURL(QString strURL)
{	
	m_strURL = strURL;
}

//! Get section
QString classEditServer::GetSection()
{
	return m_strSection;
}

//! Get Server name
QString classEditServer::GetServer()
{
	return m_strServer;
}

//! Get Server URL.
QString classEditServer::GetURL()
{
	return m_strURL;
}

/*!
@brief Add Additioanl Blacklist Dialog Class .
this class is created when you clicked the "Add package names" button
*/
classAddAdditionalBlacklist::classAddAdditionalBlacklist(QWidget *parent, WFlags f ):frmAddAdditionalBlacklist(parent,0,FALSE,f)
{
	
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setCaption(AXTU_SETUP_TITLE);
	this->setPaletteBackgroundColor(QColor(BG_COLOR));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	btnAdditionalOk->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnAdditionalOk->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnAdditionalCancel->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnAdditionalCancel->setPaletteForegroundColor(QColor(FG_COLOR));
}

classAddAdditionalBlacklist::~classAddAdditionalBlacklist()
{
}

int classAddAdditionalBlacklist::Domodal()
{
	m_strText = "";
	edtBlacklist->setFocus();
	return exec();
}

string classAddAdditionalBlacklist::text()
{
	return m_strText;
}

void classAddAdditionalBlacklist::slotAdditionalOk()
{
	m_strText = (const char *)edtBlacklist->text();
	m_bResult = true;
	if (m_strText.length() <= 0)
	{	
		QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Value is not correct."));
		return;
	}
	done(0);	
}

void classAddAdditionalBlacklist::slotAdditionalCancel()
{
	m_bResult = false;
	done(2);; 
}

classSetup::classSetup(int nMode, QWidget *parent, WFlags f ):frmSetup(parent,0,FALSE,f)
{		
	m_nMode = nMode;
	
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setCaption(AXTU_SETUP_TITLE);
	this->setPaletteBackgroundColor(QColor(BG_COLOR));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	
	tabMain->setPaletteBackgroundColor(QColor(BG_COLOR));
	tabMain->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnAddServer->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnAddServer->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnEditServer->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnEditServer->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnRemoveServer->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnRemoveServer->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnApply->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnApply->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnOk->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnOk->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnCancel->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnCancel->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnDownpath->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnDownpath->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnLogpath->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnLogpath->setPaletteForegroundColor(QColor(FG_COLOR));
	
	comboPeriod->setPaletteBackgroundColor(QColor(BG_COLOR));
	comboPeriod->setPaletteForegroundColor(QColor(FG_COLOR));
		
	btnAddBlacklist->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnAddBlacklist->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnRemoveBlacklist->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnRemoveBlacklist->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnAddAdditionalBlacklist->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnAddAdditionalBlacklist->setPaletteForegroundColor(QColor(FG_COLOR));	
	labelDate->setPaletteBackgroundColor(QColor(BG_COLOR));
	labelDate->setPaletteForegroundColor(QColor(FG_COLOR));
	
	labelFromTo->setPaletteBackgroundColor(QColor(BG_COLOR));
	labelFromTo->setPaletteForegroundColor(QColor(FG_COLOR));
	
	labelCommand->setPaletteBackgroundColor(QColor(BG_COLOR));
	labelCommand->setPaletteForegroundColor(QColor(FG_COLOR));
	
	dateFrom->setPaletteBackgroundColor(QColor(BG_COLOR));
	dateFrom->setPaletteForegroundColor(QColor(FG_COLOR));
	
	dateTo->setPaletteBackgroundColor(QColor(BG_COLOR));
	dateTo->setPaletteForegroundColor(QColor(FG_COLOR));
	
	
	comboCmd->setPaletteBackgroundColor(QColor(BG_COLOR));
	comboCmd->setPaletteForegroundColor(QColor(FG_COLOR));
	
	chkCaseSensitive->setPaletteBackgroundColor(QColor(BG_COLOR));
	chkCaseSensitive->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnLogSearch->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnLogSearch->setPaletteForegroundColor(QColor(FG_COLOR));
	
	labelPackageName->setPaletteBackgroundColor(QColor(BG_COLOR));
	labelPackageName->setPaletteForegroundColor(QColor(FG_COLOR));

	 
	
	m_bInit = false;	
	if (Init() != 0)
	{
		done(1);
	}	
	m_bInit = true;	
}

classSetup::~classSetup()
{	
	Uninit();	
	if(unlink(AXTU_SETUP_PID_FILE) != 0)
	{
		exit(1);
	}
}


QString classSetup::ReadObinfo(QString strObsoletee)
{
        QStringList ::Iterator it;
        QStringList strObList;
        QStringList strObList2;
        QString strOb;
	QString obsoleter;
        QFile file("/var/tmp/obinfo.tmp");
        file.open(IO_ReadOnly);
        QTextStream in(&file);
        strOb = in.readLine();
        strObList2=strObList.split(" ",strOb);
        it = strObList2.begin();
        while(1)
        {
                if((*it).find(strObsoletee)!=-1)//-1 means not found
                {
                        while((*it).find("+")==-1)
                        {
                                it++;
                        }
			obsoleter=(*it).remove("+");
                        QMessageBox::information(this, AXTU_SETUP_TITLE,obsoleter+" will be added as well, since it obsoletes "+strObsoletee);
                }
                it++;
                if((*it)==NULL) break;
        }

        return obsoleter;
}

bool classSetup::LoadConfig()
{	
	ReadBlacklistConfig();
	if (ReadLocalHeaderInfo() != 0)
	{
		return false;
	}
	
	// Read Server Info
	GetServerList();
	
	// Read Path Info
	GetPathInfo();
	
	// Read Alarm Info
	GetAlarmInfo();
	
	// Read proxy Info
	GetProxyInfo();
	
	return true;	
}

int classSetup::Init()
{	
	m_strOldHttpPort = edtHttpPort->text();
	m_strOldHttpsPort = edtHttpsPort->text();
	
	m_configBlacklistUpdate = NULL;
	m_configEnv = NULL;
	m_configProxy = NULL;
	m_ConfCtl = NULL;
	m_Logger = NULL;
	
	m_Logger = new classLogger();
	
	dateTo->setDate(QDate::currentDate());
	
	m_ConfCtl=new classConfCtl;

	m_ConfCtl->ConfigCheck();
	
	m_configBlacklistUpdate = new classConfigParser();
	if(m_configBlacklistUpdate->Read(BLACKLIST_FILE) == false)
	{
		// It's Ok.
	}
	
	m_configProxy = new classConfigParser();
	if(m_configProxy->Read(PROXY_CONFIG_FILE) == false)
	{
		// It's Ok.  
	}
		
	//Set default page
	int nPage;
	if(m_nMode == PATH_SETUP)
	{
		nPage=1;    
	}
	else if(m_nMode == ALARM_SETUP)
	{
		nPage=2;
    }
	else if(m_nMode == BLACKLIST_SETUP)
	{
		nPage=3;
	}
	else if(m_nMode == LOG_SETUP)
	{
		nPage=4;
	}
	else
	{    
		nPage=0;
	}	
	tabMain->setCurrentPage(nPage);	
	
	m_configEnv = new classConfigParser();
	if (m_configEnv->Read(CONFIG_FILE) == false)
	{		
		return 1;
	}
	
	if(LoadConfig() == false)
	{
		return 1;
	}
	
	m_bClickNotifierButton = false;
	if(m_configEnv->GetOption("main", "authen") != "false")
	{
		btnAddServer->setShown(false);
		btnEditServer->setShown(false);
		btnRemoveServer->setShown(false);
	}	
	
	btnApply->setEnabled(false); 
	m_bConfigEdited = false;
		
	return 0;
        
}

void classSetup::Uninit()
{
	if(m_ConfCtl) delete m_ConfCtl;
	if(m_Logger) delete m_Logger;
	if(m_configBlacklistUpdate) delete m_configBlacklistUpdate;
	if(m_configEnv) delete m_configEnv;
	if(m_configProxy) delete m_configProxy;
}
 
 
//! Read Blacklist config file.
int classSetup::ReadBlacklistConfig()
{
	vector<string> vectorBlackUpdate;
 	
	lstBlacklist->clear();
	
	vectorBlackUpdate = m_configBlacklistUpdate->GetOptions("blacklist-update");
	
	vector <string>::iterator it;      
	for(it=vectorBlackUpdate.begin();it!=vectorBlackUpdate.end();it++)
	{
		lstBlacklist->insertItem((QString)*it);							
	} 	
	return 0;
}


//! Get server list ex(BASE, UPDATE, SELF...)
void classSetup::GetServerList()
{
	lstServer->clear();
	vector<string> vectorSections;
	vector<string>::iterator it; 
	
	vector<string> vectorOptions;
	vector<string>::iterator it2;
	
	vectorSections = m_configEnv->GetSections();
	
	for (it=vectorSections.begin();it!=vectorSections.end();it++)
	{
		if (*it != "main")
		{
			vectorOptions = m_configEnv->GetOptions(*it);	    
			string strURL = m_configEnv->GetOption(*it, "baseurl");
			string strServer = m_configEnv->GetOption(*it, "name");
			QListViewItem *item = new QListViewItem(lstServer , *it ,strServer, strURL,"");	     		
			lstServer->insertItem(item);
		}
	}
}

//! Get config information of Path.
void classSetup::GetPathInfo()
{
	edtDownpath->setText(m_configEnv->GetOption("main", "cachedir"));
	edtLogpath->setText(m_configEnv->GetOption("main", "logdir"));
	if (m_configEnv->GetOption("main", "removepackages") == "true")
	{
		chkRemovePackages->setChecked(true);
	}
	else
	{
		chkRemovePackages->setChecked(false);
	}
}

//! Get config information of Alarm(notify).
void classSetup::GetAlarmInfo()
{
	string alarm = m_configEnv->GetOption("main", "alarm");
	string alarm_period = m_configEnv->GetOption("main", "alarm_period");	
	string apply_blacklist = m_configEnv->GetOption("main", "apply_blacklist");
	
	if (alarm == "true")
	{
		radioAlarmEnable->setChecked(true);
		radioAlarmDisable->setChecked(false);
	}
	else
	{
		radioAlarmEnable->setChecked(false);
		radioAlarmDisable->setChecked(true);		
	}
	
	comboPeriod->setCurrentText(alarm_period); 
	
	if( apply_blacklist == "false") 
	{
		chkApplyBlacklist->setChecked(false);
	}
	else
	{
		chkApplyBlacklist->setChecked(true);		
	}
}

//! Get config information of Alarm(notify).
void classSetup::GetProxyInfo()
{		
	string strUseProxy = m_configProxy->StripString(m_configProxy->GetOption(__DEFAULT__, "use_proxy"));
	string strHttpProxy = m_configProxy->StripString(m_configProxy->GetOption(__DEFAULT__, "http_proxy"));	
	string strHttpsProxy = m_configProxy->StripString(m_configProxy->GetOption(__DEFAULT__, "https_proxy"));	
	string strProxyId = m_configProxy->StripString(m_configProxy->GetOption(__DEFAULT__, "proxy-user"));	
	string strProxyPasswd = m_configProxy->StripString(m_configProxy->GetOption(__DEFAULT__, "proxy-passwd"));
		
	
	QStringList listHttpProxy;
	QString strHttpURL="";
	QString strHttpPort="";
	
	QStringList listHttpsProxy;
	QString strHttpsURL="";
	QString strHttpsPort="";
	int nCount=0;
	if(strHttpProxy != "")
	{		
		listHttpProxy = QStringList::split(":", strHttpProxy);
		
		nCount = listHttpProxy.size();
		if(nCount == 3)
		{	
			if(listHttpProxy[1] && listHttpProxy[1] != "")
			{			
				strHttpURL = m_configProxy->StripString((const char *)listHttpProxy[1], '/');
			}
			if(listHttpProxy[2] && listHttpProxy[2] != "")
			{
				strHttpPort = m_configProxy->StripString((const char *)listHttpProxy[2], '/');
			}
		}
		else if(nCount == 2)
		{
			if(listHttpProxy[1] && listHttpProxy[1] != "")
			{			
				strHttpURL = m_configProxy->StripString((const char *)listHttpProxy[1], '/');
			}
		}
	}
	
	if(strHttpsProxy != "")
	{		 
		listHttpsProxy = QStringList::split(":", strHttpsProxy);
		nCount = listHttpsProxy.size();
		if(nCount == 3)
		{	
			if(listHttpsProxy[1] && listHttpsProxy[1] != "")
			{
				strHttpsURL = m_configProxy->StripString((const char *)listHttpsProxy[1], '/');
			}
			if(listHttpsProxy[2] && listHttpsProxy[2] != "")
			{
				strHttpsPort = m_configProxy->StripString((const char *)listHttpsProxy[2], '/');
			}
		}		
		else if(nCount == 2)
		{
			if(listHttpsProxy[1] && listHttpsProxy[1] != "")
			{
				strHttpsURL = m_configProxy->StripString((const char *)listHttpsProxy[1], '/');
			}
		}
	}		
		
	edtHttpUrl->setText(strHttpURL);
	edtHttpPort->setText(strHttpPort);
	
	edtHttpsUrl->setText(strHttpsURL);
	edtHttpsPort->setText(strHttpsPort);
	
	edtProxyID->setText(strProxyId.c_str());
	edtProxyPass->setText(strProxyPasswd.c_str());
	
	if(strUseProxy == "on")
	{
		chkConnectionProxy->setChecked(true);
		edtHttpUrl->setEnabled(true);
		edtHttpPort->setEnabled(true);
		edtHttpsUrl->setEnabled(true);
		edtHttpsPort->setEnabled(true);
		edtProxyID->setEnabled(true);
		edtProxyPass->setEnabled(true);
	}
	else
	{
		chkConnectionProxy->setChecked(false);
		edtHttpUrl->setEnabled(false);
		edtHttpPort->setEnabled(false);
		edtHttpsUrl->setEnabled(false);
		edtHttpsPort->setEnabled(false);
		edtProxyID->setEnabled(false);
		edtProxyPass->setEnabled(false);
	}
}
		
//! Get user input condition for seaching log.
bool classSetup::GetLogCondition(QString strDate, QString strCmd, QString strName)
{	
	QStringList listDate = QStringList::split("/", strDate);		
	if(listDate.size() != 3)
	{
		return false;
	}
	
	bool bDate=false, bCmd=false, bName=false;

	// Compare to date
	QDate from = dateFrom->date();
	QDate to = dateTo->date();
	QDate target(listDate[0].toInt(),listDate[1].toInt(),listDate[2].toInt());
	if (from <= target && target <= to)	
	{
		bDate= true;
	}
	
	// Compare to command
	int nSel = comboCmd->currentItem();
	if (nSel == 0) // All
	{
		bCmd = true;
	}
	else if(nSel == 1) // Installed
	{
		if(strCmd.find("Install") != -1)
		{
			bCmd = true;
		}		
	}
	else if(nSel == 2) // Updated
	{
		if(strCmd.find("Update") != -1)
		{
			bCmd = true;
		}		
	}	
	else if(nSel == 3) // Dep
	{
		if(strCmd.find("Dep") != -1)
		{
			bCmd = true;
		}		
	}
	/*
	else if(nSel == 3) // Erased
	{
		if(strCmd.find("Erase") != -1)
		{
			bCmd = true;
		}		
	}
	*/
	
	// Compare to package name
	QString strSource;
	QString strTarget;
	
	if (chkCaseSensitive->isChecked() == true) // case sensitive
	{
		strSource = strName;
		strTarget = editPackageName->text().stripWhiteSpace();
	}
	else
	{
		strSource = strName.lower();
		strTarget = editPackageName->text().lower().stripWhiteSpace();
	}
	
	if(strTarget == "")
	{
		bName = true;
	}
	else if (strSource.find(strTarget) != -1)
	{
		bName = true;
	}
	
	return (bDate && bCmd && bName);
}


//! Read Local rpm db header info.
int classSetup::ReadLocalHeaderInfo()
{
	set <string> setInstalledList;
	setInstalledList.clear();

	lstInstalledList->clear();
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
		QMessageBox::critical(this, tr("Error"), tr("Failed to open RPM configuration file"));
		done(1);
	}
	
	rpmts m_rpmTs = rpmtsCreate();

	rpmtsSetRootDir(m_rpmTs, NULL);

	rc = rpmtsOpenDB(m_rpmTs, O_RDONLY);
	if (rc != 0) {
		QMessageBox::critical(this, tr("Error"), tr("Failed to open RPM database"));
		done(1);
	}

	mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_PACKAGES, NULL, 0);
	if (mi == NULL) {
		rpmtsCloseDB(m_rpmTs);
		rpmtsFree(m_rpmTs);
		QMessageBox::critical(this, tr("Error"), tr("Internal Error #1, please contact your support provider, or try again later."));
		done(1);
	}

	nCount = 0;
	while ((hdr = rpmdbNextIterator(mi)) != NULL)
	{
		int recOffset = rpmdbGetIteratorOffset(mi);
		if (recOffset)
		{
			rc = headerNEVRA(hdr,&np,&ep, &vp, &rp, &ap);
			if (rc)
			{
				rpmdbFreeIterator(mi);
				rpmtsCloseDB(m_rpmTs);
				rpmtsFree(m_rpmTs);
				return 1;
			}
		}
		snprintf(strTemp, sizeof(strTemp), "%s", np);
		//printf("test : %s\n", strTemp );	

		setInstalledList.insert(strTemp);
		//headerFree(hdr);
		nCount++;
	}

	rpmdbFreeIterator(mi);
	rpmtsCloseDB(m_rpmTs);
	rpmtsFree(m_rpmTs);

	set <string>::iterator it;
	for(it=setInstalledList.begin();it!=setInstalledList.end();it++)
	{
		lstInstalledList->insertItem(*it);
	}
	return 0;
}

//! Add black list.
//when you click the >>> button
void classSetup::slotAddBlacklist()
{	
	string strItem;
	
	if (lstInstalledList->currentItem() != -1)
	{ 
		for(unsigned int i=0;i<lstInstalledList->count();i++)
		{
			if (lstInstalledList->isSelected(i) == true)
			{
				strItem = (const char *)lstInstalledList->item(i)->text();	
				if (m_configBlacklistUpdate->GetOption("blacklist-update", strItem) == "")
				{	
					string strObItem=ReadObinfo(strItem);
					lstBlacklist->insertItem(strItem);
					lstBlacklist->insertItem(strObItem);
					m_configBlacklistUpdate->SetOption("blacklist-update", strItem, "0");
					m_configBlacklistUpdate->SetOption("blacklist-update", strObItem, "0");
					m_bConfigEdited = true;
					btnApply->setEnabled(true);
				}
				else
				{
					QString strMsg = "[" + strItem + "]" + tr("is already blacklisted.");
					QMessageBox::information(this, AXTU_SETUP_TITLE, strMsg);		   	  
				}
			} 
		}
	}
	else
	{
	    QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Do you want to remove these selected items?"));
	}
}

//! Remove black list.
void classSetup::slotRemoveBlacklist()
{
	string strItem;
	int nResult;
	if (lstBlacklist->currentItem() != -1) 
	{
		nResult = QMessageBox::question(this, AXTU_SETUP_TITLE,tr("Do you want to remove these selected items?"),  tr("&Yes"), tr("&No"), QString::null, 0, 1 );
		
		if (nResult == 1)
			return ;
		for (int i=(int)lstBlacklist->count()-1; i>=0 ;i--)
		{
			if (lstBlacklist->isSelected(i) == true) 
			{
				strItem = (const char *)lstBlacklist->item(i)->text();
				if(m_configBlacklistUpdate->RemoveOption("blacklist-update", strItem) == true)
				{
					lstBlacklist->removeItem(i);
				}
			}
		}
		m_bConfigEdited = true;
		btnApply->setEnabled(true);
	}	
}

//! Add Additional black list.
void classSetup::slotAddAdditionalBlacklist()
{	
	classAddAdditionalBlacklist dlgBlacklist(this, Qt::WStyle_Customize | Qt::WStyle_NormalBorder);
	dlgBlacklist.setCaption(AXTU_SETUP_TITLE);
	dlgBlacklist.Domodal();
	if (dlgBlacklist.m_bResult == true)
	{
		if (m_configBlacklistUpdate->HasOption("blacklist-update", dlgBlacklist.text()) == 0 )
		{
			lstBlacklist->insertItem(dlgBlacklist.text());
			m_configBlacklistUpdate->SetOption("blacklist-update", dlgBlacklist.text(), "0");
			m_bConfigEdited = true;
			btnApply->setEnabled(true);
		}
		else
		{
			QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Item already exists in the list!"));
		}
	}	
}

//! Add Server name and URL.
void classSetup::slotAddServer()
{	
	if(m_configEnv->GetOption("main", "authen") != "false")
	{
		return;
	}
	classEditServer dlg(this, Qt::WStyle_Customize | Qt::WStyle_NormalBorder);
	dlg.setCaption(AXTU_SETUP_TITLE);
		
	QString strSection;
	QString strServer;
	QString strURL;
	
	strSection = "";
	strServer = "";
	strURL = "";
	
	dlg.SetSection("");
	dlg.SetServer("");
	dlg.SetURL("");
		
goto1 :
	dlg.Domodal();
	
	strSection = dlg.GetSection().stripWhiteSpace();
	strServer = dlg.GetServer().stripWhiteSpace();
	strURL = dlg.GetURL().stripWhiteSpace();
	
	// Ok
	if(dlg.GetResult())
	{
		if(strSection!="")
		{	
			if(m_configEnv->HasSection(strSection.ascii()))
			{
				QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Seclected section already exists!  Please try again."));
				goto goto1;				
			}
			else
			{
				m_configEnv->SetOption(strSection.ascii(), "name", strServer.ascii());
				m_configEnv->SetOption(strSection.ascii(), "baseurl", strURL.ascii());
				QListViewItem *item = new QListViewItem(lstServer , strSection ,strServer, strURL,"");	     		
				lstServer->insertItem(item);
				m_bConfigEdited = true;
				btnApply->setEnabled(true);
			}
		}
		else
		{
			QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Input section name."));
			goto goto1;			
		}
	}
}

//! Edit URL of Server.
void classSetup::slotEditServer(QListViewItem * item)
{
	if(m_configEnv->GetOption("main", "authen") != "false")
	{
		return;
	}
	if(!item)
	{
		QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Item is not selected!"));
		return;
	} 
	classEditServer dlg(this, Qt::WStyle_Customize | Qt::WStyle_NormalBorder);
	dlg.setCaption(AXTU_SETUP_TITLE);
	
	QString strSection;
	QString strServer;
	QString strURL;
	
	
	strSection = item->text(0);
	strServer = item->text(1);
	strURL = item->text(2);	
	
	dlg.SetSection(strSection);
	dlg.SetServer(strServer);
	dlg.SetURL(strURL); 

goto2 :	
	dlg.Domodal(false);
	
	strSection = dlg.GetSection().stripWhiteSpace();
	strServer = dlg.GetServer().stripWhiteSpace();
	strURL = dlg.GetURL().stripWhiteSpace();
	
	// Ok
	if(dlg.GetResult())
	{
		if(strSection!="")
		{	
			m_configEnv->SetOption(strSection.ascii(), "name", strServer.ascii());
			m_configEnv->SetOption(strSection.ascii(), "baseurl", strURL.ascii());
			
			item->setText(0, strSection);
			item->setText(1, strServer);
			item->setText(2, strURL);
			
			m_bConfigEdited = true;
			btnApply->setEnabled(true);			
		}
		else
		{
			QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Input section name."));
			goto goto2;			
		}
	}	
}

//! Edit URL of Server.
void classSetup::slotEditServer()
{
	if(m_configEnv->GetOption("main", "authen") != "false")
	{
		return;
	}
	QListViewItem *item = lstServer->selectedItem();	
	slotEditServer(item);
	
}

//! Remove Server list.
void classSetup::slotRemoveServer()
{
	if(m_configEnv->GetOption("main", "authen") != "false")
	{
		return;
	}
	QListViewItem *item = lstServer->selectedItem();
	if(!item)
	{
		QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Item is not selected!"));
		return;
	} 
	
	int result = QMessageBox::question(this, AXTU_SETUP_TITLE, tr("Do you want to remove the item?"), tr("&Yes"),tr("&No"),0,1);
	if (result == 0)
	{
		m_configEnv->RemoveSection(item->text(0).ascii());		
		lstServer->removeItem(item);
		m_bConfigEdited = true;
		btnApply->setEnabled(true);
	}		
}

//! Change Download path.
void classSetup::slotChangeDownPath()
{
	QFileDialog fd;        
	fd.setMode(QFileDialog::DirectoryOnly);        
	fd.setDir(edtDownpath->text());
	fd.setModal(true);
	fd.exec();
	int  nResult = fd.result();
	if (nResult == true)
	{
		m_configEnv->SetOption("main","cachedir",fd.selectedFile().ascii());
		edtDownpath->setText(fd.dirPath());
		edtDownpath->update();		
		m_bConfigEdited = true;
		btnApply->setEnabled(true);
	}
}

//! Change log path.
void classSetup::slotChangeLogPath()
{
	QFileDialog fd;    
	fd.setMode(QFileDialog::DirectoryOnly);        
	fd.setDir(edtDownpath->text());
	fd.setModal(true);
	fd.exec();
	int  nResult = fd.result();
	if (nResult == true)
	{
		m_configEnv->SetOption("main","logdir",fd.selectedFile().ascii());
		edtLogpath->setText(fd.dirPath());
		edtLogpath->update();
		
		m_bConfigEdited = true;
		btnApply->setEnabled(true);
	}
}

//! You can choose remove or not download rpm packages after installation has finished.
void classSetup::slotChangeChkRemove()
{
	if (chkRemovePackages->state() == 0)
	{
		m_configEnv->SetOption("main","removepackages","false");				
	}
	else
	{
		m_configEnv->SetOption("main","removepackages","true");
	}
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}

//! Select to use notification.
void classSetup::slotUpdateAlarmEnable()
{
	radioAlarmDisable->setChecked(false);
	radioAlarmEnable->setChecked(true);
	m_configEnv->SetOption("main", "alarm", "true" );
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
	m_bClickNotifierButton = true;
}

//! Select to not use notification.
void classSetup::slotUpdateAlarmDisable() 
{
	radioAlarmEnable->setChecked(false);
	radioAlarmDisable->setChecked(true);
	m_configEnv->SetOption("main", "alarm", "false" );
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
	m_bClickNotifierButton = true;
}

//! Select to not use notification.
void classSetup::slotAlarmPeriod(const QString &strPeriod) 
{	
	m_configEnv->SetOption("main", "alarm_period", strPeriod.ascii() );	
	
	if(m_bInit)
	{
		m_bConfigEdited = true;
		btnApply->setEnabled(true);
	}
}

//! Apply check blacklist or not when check notification.
void classSetup::slotCheckApplyBlacklist()
{
	if(chkApplyBlacklist->state() == 0) 
	{
		m_configEnv->SetOption("main", "apply_blacklist", "false");
	}	
	else
	{
		m_configEnv->SetOption("main", "apply_blacklist", "true");
		
	}
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}


//! Search log by user input condition.
void classSetup::slotLogSearch()
{
	btnLogSearch->setFocus();
	lstLog->clear();
	
	char strBuf[MAX_STRING];
	ifstream fin;
	string strLogFile;
	strLogFile = m_configEnv->GetOption("main","logdir");
	strLogFile = strLogFile + "/";
	strLogFile = strLogFile + SUCCESS_LOG_FILE;
	
	//printf("strLogFile = %s\n", strLogFile.c_str());
	
	fin.open(strLogFile.c_str());		
	
	QStringList listTemp;	
	QStringList listTemp2;
	
	QString strDate;
	QString strCmd;
	
	
	while (fin.getline(strBuf, sizeof(strBuf)) > 0 )
	{	
		listTemp = QStringList::split(" ", strBuf);
		listTemp2 = QStringList::split("/", listTemp[0]);
		strDate.sprintf("20%s/%s/%s", listTemp2[2].ascii(), listTemp2[0].ascii(), listTemp2[1].ascii());		
		
				
		strCmd = listTemp[2];
			
	
		if (GetLogCondition(strDate, strCmd, listTemp[3]))
		{ 
			lstLog->insertItem(new QListViewItem( lstLog , strDate, listTemp[1], strCmd, listTemp[3]));
		}		
		
	}
	fin.close();
	
}

//! Chagne Proxy Value
int classSetup::ChangeProxyValue()
{
	// Check proxy value.  
	if(chkConnectionProxy->state() != 0) 
	{
		if (edtHttpUrl->text().stripWhiteSpace().isEmpty() == TRUE 
			|| edtHttpPort->text().stripWhiteSpace().isEmpty() == TRUE
			|| edtHttpsUrl->text().stripWhiteSpace().isEmpty() == TRUE
			|| edtHttpsPort->text().stripWhiteSpace().isEmpty() == TRUE )
		{
			QMessageBox::warning(this, AXTU_SETUP_TITLE, tr("Selecting \"Connection via proxy server\" requires entries for BOTH http: and https: connections.\nPlease enter all required proxy server information."));
			return 2;
		}				
	}
	
	try
	{
		//Check proxy Enable
		if(chkConnectionProxy->state() != 0) 
		{		
			m_configProxy->SetOption(__DEFAULT__, "use_proxy", "on");
		}	
		else
		{	
			m_configProxy->SetOption(__DEFAULT__, "use_proxy", "off");
		}
	
		string strTemp;
		// http url , port
		strTemp="";
		strTemp += "http://";
		strTemp += (string)(const char *)(edtHttpUrl->text() + ":" + edtHttpPort->text());
		m_configProxy->SetOption(__DEFAULT__, "http_proxy", strTemp);
	
		// https url , port
		strTemp="";
		strTemp += "https://";
		strTemp += (string)(const char *)(edtHttpsUrl->text() + ":" + edtHttpsPort->text());
		m_configProxy->SetOption(__DEFAULT__, "https_proxy", strTemp);
	
		// proxy user
		QString strId = edtProxyID->text();
		m_configProxy->SetOption(__DEFAULT__, "proxy-user", (const char *)strId);
		
		// proxy passwd
		QString strPass = edtProxyPass->text();
		m_configProxy->SetOption(__DEFAULT__, "proxy-passwd", (const char *)strPass);
	}
	catch(std::exception &e)
	{
		return 1;
	}


	return 0;	

}

void classSetup::slotCheckProxyEnable()
{
	if(chkConnectionProxy->state() != 0) 
	{		
		edtHttpUrl->setEnabled(true);
		edtHttpPort->setEnabled(true);
		edtHttpsUrl->setEnabled(true);
		edtHttpsPort->setEnabled(true);
		edtProxyID->setEnabled(true);
		edtProxyPass->setEnabled(true);
	}	
	else
	{	
		edtHttpUrl->setEnabled(false);
		edtHttpPort->setEnabled(false);
		edtHttpsUrl->setEnabled(false);
		edtHttpsPort->setEnabled(false);
		edtProxyID->setEnabled(false);
		edtProxyPass->setEnabled(false);
	}
	m_bConfigEdited = true;
	btnApply->setEnabled(true);		
}


void classSetup::slotHttpUrl(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}

void classSetup::slotHttpPort(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}
void classSetup::slotHttpsUrl(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}
void classSetup::slotHttpsPort(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}
void classSetup::slotProxyId(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
	
}
void classSetup::slotProxyPasswd(const QString &str)
{
	QString strTem=str;
	m_bConfigEdited = true;
	btnApply->setEnabled(true);
}


void classSetup::slotHttpPortEdit(const QString& strPort)
{
	unsigned int  index;
	for(index=0;index<strPort.length();index++)
	{
		if( !('0' <= strPort.at(index) && strPort.at(index) <= '9') )
		{
			QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Please intput an arabic numeral."));
			edtHttpPort->setText(m_strOldHttpPort);
			return ;
		}		
	}	
	m_strOldHttpPort = edtHttpPort->text();		
}
void classSetup::slotHttpsPortEdit(const QString& strPort)
{
	unsigned int index;
	for(index=0;index<strPort.length();index++)
	{
		if( !('0' <= strPort.at(index) && strPort.at(index) <= '9') )
		{
			QMessageBox::information(this, AXTU_SETUP_TITLE, tr("Please intput an arabic numeral."));
			edtHttpsPort->setText(m_strOldHttpsPort);
			return;
		}
		
	}
	m_strOldHttpsPort = edtHttpsPort->text();
}
	
void classSetup::done(int nRet)
{	
	QDialog::done(nRet);
}

//! Click Ok
void classSetup::slotOK()
{ 
	if(DoSave() == 0)
	{
		done(0);;
	}
}

//! Click Apply
void classSetup::slotApply()
{	
	DoSave();
}

//! Save configuration infomation to conf file.
int classSetup::DoSave() 
{ 
	if (m_bConfigEdited == true)
	{
		int result = QMessageBox::question(this, AXTU_SETUP_TITLE, tr("Do you want to save the changed values?"), tr("&Yes"),tr("&No"),0,1);
		if (result == 0)
		{
			int nRet = ChangeProxyValue();
			if(nRet == 1)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_SETUP, "Failed ChangeProxyValue()" , NULL);
				return 1;					
			}
			else if(nRet == 2)
			{
				return 2;
			}
	  		
	  		m_configBlacklistUpdate->Write();
	  		m_configEnv->Write();
	  		m_configProxy->Write();
	  		if(chmod(m_configProxy->GetConfigFilePath().c_str(), S_IRUSR | S_IWUSR) != 0)
	  		{
				m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_SETUP, "Failed : Cannot change mode for proxy configuration file." , NULL);
	  		}
			if(m_bClickNotifierButton == true)
			{
	  			if(system(RESTART_NOTIFIER_DAEMON) == -1)
				{
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_SETUP, "Failed : RESTART_NOTIFIER_DAEMON" , NULL);
					return 1;
				}
				m_bClickNotifierButton = false;
			}
			m_bConfigEdited = false;
	  		btnApply->setEnabled(false);	  		
	  	}	  
	}
	return 0;
}

