/*!
@file classGui.cpp
@brief Class source file for Graphic User Interface
*/
#include "classGui.h"

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qprogressbar.h> 
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcombobox.h> 
#include <qapplication.h> 
#include <qsizepolicy.h> 
#include <qlineedit.h>
#include <qcheckbox.h> 
#include <qtextedit.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qlayout.h>
#include <qstyle.h>
#include "classShowHideBox.h"
#include <unistd.h>
#include "commondef.h"
#include "classRedcastle.h"

/*!
 * Global value. 
*/

QString  staticTitle;
QProgressBar * g_progressTotal;
QProgressBar * g_progressCurrent;
QLabel * g_labelTotalStatus; 
QLabel * g_labelCurrentStatus;

char strDubugMsg[MAX_STRING];

/*!
@brief Constructor.

classGui Constructor. 
*/
classGui::classGui(int nMode) : MainDialog(0,0,FALSE,Qt::WStyle_Customize | Qt::WStyle_NormalBorder | Qt::WStyle_Title)
{  		
	m_processSetupBlack =NULL;
	m_rpmEngine = NULL;  
	m_Network = NULL;
	m_Logger = NULL;
	m_img_contain = NULL;
	
    listPackages->addColumn( tr( "Name" ) );
    listPackages->addColumn( tr( "Version" ) );
    listPackages->addColumn( tr( "Release" ) );
    listPackages->addColumn( tr( "Arch" ) );    
	comboCommand->setShown(false);
	if (nMode == INSTALL_MODE)
	{	
		SetSelectCommand(1);
		staticTitle = tr("%1 - %2").arg( tr("Install Wizard"), (AXTU_TITLE));
	}
	else if (nMode == ERASE_MODE)
	{	
		SetSelectCommand(2);		
		staticTitle = tr("%1 - %2").arg( tr("Erase Wizard"), (AXTU_TITLE));
	}
	else
	{
		SetSelectCommand(0);		
		staticTitle = tr("%1 - %2").arg( tr("Update Wizard"), (AXTU_TITLE));
		listPackages->addColumn( tr( "Blacklist" ) );
		listPackages->addColumn( tr( "Compatible" ) );
		
	}


        classRedcastle * redcastle = new classRedcastle();
	int nStatus = m_nRedcastleStatus = redcastle->GetRCStatus();
	delete redcastle;

	if(nStatus == RC_STATUS_ENABLE)
	{
		QMessageBox::critical(this, staticTitle, REDCASTLE_ENABLE_MSG_TR);
		classLogger * Logger = new classLogger();
		Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, REDCASTLE_ENABLE_MSG , NULL);
		delete Logger;
		Exit(2);
	}
	else if(nStatus == RC_STATUS_WARNING)
	{		
		int nButton = QMessageBox::warning(this, staticTitle, REDCASTLE_WARNING_CONTINUE_MSG_TR, TR_OK, TR_CANCEL, QString::null, 0,1);
		if (nButton == 1)
		{
			Exit(2);
		}
	}

	m_Logger = new classLogger();
	// Log dir have not enough space(Under 100k available space) 
	if( m_Logger->CheckLogDirSpace() == false)
	{		
		QMessageBox::critical(this, staticTitle, tr("You don't have enough space for log directory! (%1)").arg(m_Logger->GetLogPath().c_str()));		
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("You don't have enough space for log directory! (%1)").arg(m_Logger->GetLogPath().c_str()), NULL);
		Exit(2);
	}
	
	
	
	m_nFinishErrNum = -999;
	m_bDownload = false;
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setPaletteBackgroundColor(QColor(BG_COLOR));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnNext->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnNext->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnBack->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnBack->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnCancel->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnCancel->setPaletteForegroundColor(QColor(FG_COLOR));


	labelCmdExplain->setPaletteBackgroundColor(QColor(BG_COLOR));
	labelCmdExplain->setPaletteForegroundColor(QColor(FG_COLOR));	
	
	
	progressTotal->setPaletteBackgroundColor(QColor(BG_COLOR));
	progressTotal->setPaletteForegroundColor(QColor(FG_COLOR));
	
	progressCurrent->setPaletteBackgroundColor(QColor(BG_COLOR));
	progressCurrent->setPaletteForegroundColor(QColor(FG_COLOR));
	
	
	btnSetupBlacklist->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnSetupBlacklist->setPaletteForegroundColor(QColor(FG_COLOR));
	
	comboCommand->setPaletteBackgroundColor(QColor(BG_COLOR));
	comboCommand->setPaletteForegroundColor(QColor(FG_COLOR));
	
	labelTopTitle->setPaletteBackgroundPixmap(QPixmap(TOP_BANNER_IMG));
	labelTopTitle->setPaletteForegroundColor(QColor(FG_TOP_B_COLOR));
	
	chkSelectAll->setPaletteBackgroundColor(QColor(BG_COLOR));
	chkSelectAll->setPaletteForegroundColor(QColor(FG_COLOR));
		
	QPixmap pix_trans;
	
	//Top icon.
	if (nMode == INSTALL_MODE) //install
	{	
		if(pix_trans.load(INSTALL_ICON_IMG) == false)
		{			
			QMessageBox::critical(this, staticTitle, tr("Can not load file : %1").arg(INSTALL_ICON_IMG));
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Can not load file : %1").arg(INSTALL_ICON_IMG), NULL);			
			Exit(1);
		}
	}
	else if(nMode == ERASE_MODE) //erase
	{		
		if(pix_trans.load(ERASE_ICON_IMG) == false)
		{			
			QMessageBox::critical(this, staticTitle, tr("Can not load file : %1").arg(ERASE_ICON_IMG));
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Can not load file : %1").arg(ERASE_ICON_IMG), NULL);			
			Exit(1);
		}
	}
	else//update
	{
		if(pix_trans.load(UPDATE_ICON_IMG) == false)
		{	
			QMessageBox::critical(this, staticTitle, tr("Can not load file : %1").arg(UPDATE_ICON_IMG));
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Can not load file : %1").arg(UPDATE_ICON_IMG), NULL);			
			Exit(1);
		}
	}
	
	m_img_contain = new QLabel( this, "No Image" );	
	m_img_contain->setBackgroundOrigin( QLabel::ParentOrigin );
	m_img_contain->setPixmap( pix_trans );		
	m_img_contain->setMask(m_img_contain->pixmap()->createHeuristicMask());
	m_img_contain->setGeometry( 12,12, 115, 90);
	
	radioFullUpdate->setChecked(true);
	
	
	/*!
	 * Verification of configuration files
	 */	
	classConfCtl  m_ConfCtl;
	m_ConfCtl.ConfigCheck();
	
	
	if (access(CONFIG_FILE, F_OK) != 0)
	{	
		QMessageBox::critical(this, staticTitle, tr("Can NOT find the configuration file : %1").arg(CONFIG_FILE));
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)tr("Can NOT find the configuration file : %1.  TSN Updater cannot continue.").arg(CONFIG_FILE), NULL);		
		Exit(1);
	}	
		
	if (m_configEnv.Read(CONFIG_FILE) == false)
	{		
		QMessageBox::critical(this, staticTitle, tr("%1 file is not correct. Please modify this file.").arg(CONFIG_FILE));			
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("%1 file is not correct. Please modify this file.").arg(CONFIG_FILE), NULL);
		Exit(1);
	}
	m_strLogPath = m_configEnv.GetOption("main", "logdir");
	if(m_strLogPath == "")
	{	
		QMessageBox::critical(this, staticTitle, tr("The configured Log Directory is blank.  Please specify a valid directory name."));			
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("The configured Log Directory is blank.  Please specify a valid directory name."), NULL);		
		Exit(1);
	}
	m_strLogPath = m_configEnv.StripRString(m_strLogPath, '/');
	m_strLogPath = m_strLogPath + "/" ;
	
	/*!
	 * Check the length of directory name
	 */
	if(!CheckConfigFileLength())
	{		
		QMessageBox::critical(this, staticTitle, tr("The maximum length of the combined path and file name is 255 characters. \nPlease modify the entry."));		
		Exit(2);
	}
		
	
	m_nMode = nMode;
	
	g_progressTotal = progressTotal;
	g_progressCurrent = progressCurrent;  
	g_labelTotalStatus = labelTotalStatus;
	g_labelCurrentStatus= labelCurrentStatus; 
  		
			
		
	this->setCaption(staticTitle);
	ShowWelcomePage();
	
	m_processSetupBlack = new QProcess();
	
	if(connect(m_processSetupBlack, SIGNAL(processExited()), this, SLOT(slotSetupBlackExit())) ==false)
	{	
		QMessageBox::critical(this, staticTitle, tr("Failed to connect to network."));			
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to connect to network."), NULL);		
		Exit(1);
	}
	
	
	if(m_nMode != ERASE_MODE)
	{		
		edtSelectedExplain->clear();
		edtSelectedExplain->append(tr("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"font-size:9pt;font-family:Helvetica\"><p>To update ALL the installed packages of the latest version available from Asianux, please select <span style=\"font-weight:600\">'Full Update'</span>. Otherwise select <span style=\"font-weight:600\">'Custom Update'</span>.<br /><br />If you have manually installed any kind of software or packages, please choose 'Custom Update' and do NOT select those software or packages in order to keep them from being removed or modified. <br /><br />If you want some packages to NEVER be updated, please click on the 'Blacklist Button' below and add them to the blacklist. The blacklisted packages will NOT appear on the available package list unless you modify the configuration.<br /><br /> 'Incompatible Packages' are packages incompatible with the previous version installed on your system. If you want to update those packages, please visit our TSN website to download them and update them manually.</p></body></html>"));		
		m_Network = new classNetwork(GRAPHIC); 	
		
		int nResult = m_Network->CheckAuthen();
		if (nResult == FAIL_AUTH)
		{	
			QMessageBox::critical(this, staticTitle, tr("Request for authentication failed."));
			m_Logger->WriteLog_char(ERROR_LOG, (const char *)QString("Request for authentication failed."), NULL);			
			Exit(2);
		}else if(nResult == CANNOT_EXE_AUTH_PROG){			
			QMessageBox::critical(this, staticTitle, tr("Cannot execute the authentication program."));
			m_Logger->WriteLog_char(ERROR_LOG, (const char *)QString("Cannot execute the authentication program."), NULL);			
			Exit(2);
		}else if(nResult == CANCELED_FROM_AUTH_PROG){
			Exit(2);
		}

		m_ConfCtl.ConfigCheck();
		if(m_Network->SetDownloadConfig() == false)
		{
			QMessageBox::critical(this, staticTitle, tr("Failed to set the download option configuration."));
			m_Logger->WriteLog_char(ERROR_LOG, (const char *)QString("Failed to set the download option configuration."), NULL);			
			Exit(2);		
		}
		
		
	}	
}

void classGui::Exit(int nRet)
{	
	m_rpmEngine->ReadCacheDirInfo(CONFIG_FILE);
	m_rpmEngine->DeleteDownPacks();
	UnInitVariable();			
	exit(nRet);
}

void classGui::UnInitVariable()
{		
	if(m_processSetupBlack)
	{
		delete m_processSetupBlack;
		m_processSetupBlack = NULL;		
	}
	if(m_rpmEngine)
	{
		delete m_rpmEngine;
		m_rpmEngine = NULL;
	}
	if(m_Network)
	{
		delete m_Network;
		m_Network = NULL;
	}
	if(m_Logger)
	{
		delete m_Logger;
		m_Logger = NULL;
	}
	if(m_img_contain)
	{
		delete m_img_contain;
		m_img_contain = NULL;
	}
}

/*!
@brief Check the path length in cofiguration file.

classGui Check the length of  cachedir, logdir.
*/
bool classGui::CheckConfigFileLength()
{
        string strCacheDir = m_configEnv.GetOption("main", "cachedir");
        string strLogDir = m_configEnv.GetOption("main", "logdir");
        if( strCacheDir.length() >= MAX_PATH || strLogDir.length() >= MAX_PATH)
        {
                return false;
        }
        return true;
}

/*!
@brief Destructor.

classGui Destructor. 
*/
classGui::~classGui()
{	
	UnInitVariable();
	if(access(AXTU_GUI_PID_FILE, F_OK) == 0 && unlink(AXTU_GUI_PID_FILE) != 0)
	{
		exit(1);
	}	
}

/*!
@brief Init classRpmEngine object.

Init classRpmEngine object.

Main flows go here. 
*/
bool classGui::InitRpm()
{		 
	int nRet;
	m_nSelfUpdateMode = false;
	if(m_rpmEngine)
	{
		delete m_rpmEngine;
		m_rpmEngine = NULL;
	}
	m_rpmEngine = new classRpmEngine();

	if(comboCommand->currentItem() == 0 || comboCommand->currentItem() == 1) // install, or update
	{
		m_rpmEngine->SetCommand(1);	
	}
	else
	{
		m_rpmEngine->SetCommand(0);			
	}
	/*!
	 * Read Cache directory info in configuration file
	 */
	if(m_rpmEngine->ReadCacheDirInfo() == false)
	{
		QMessageBox::critical(this, staticTitle, tr("Failed to load the ReadCacheDirInfo() function."));		
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to load the ReadCacheDirInfo() function.", NULL);		
		m_nFinishErrNum =  FC_INIT_ERROR;
		return false;
	}

	
	
	ShowProgressPage(true);
	m_rpmEngine->ClearAddedFiles(); 		
	
	if (m_nMode != ERASE_MODE)
	{
		m_Network->ClearPackages();	
		nRet = m_Network->GetHeader();
		if (show_NetworkErrorDialog(nRet) == false)
		{
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Network error(%d) is occurred", nRet);
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, strDubugMsg, NULL);
			m_nFinishErrNum =  FC_NETWORK_ERROR;
			return false;
		}
	}
	
	m_rpmEngine->SetReadLocalHeaderInfoCallBack(ReadHeadersCallBack);	
	if(m_rpmEngine->ReadLocalHeaderInfo() != 0)
	{					
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to load ReadLocalHeaderInfo() function.", NULL);
		QMessageBox::critical(this, staticTitle, tr( "Failed to load the ReadLocalHeaderInfo() function."));
		m_nFinishErrNum =  FC_INIT_ERROR;
		return false;
	}
	
	if (m_nMode != ERASE_MODE)	
	{			
		m_rpmEngine->SetReadRemoteHeaderInfoCallBack(ReadHeadersCallBack);	
		if(m_rpmEngine->ReadRemoteHeaderInfo() != 0)
		{				
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to load the ReadRemoteHeaderInfo() function.", NULL);
			QMessageBox::critical(this, staticTitle, tr( "Failed to load the ReadRemoteHeaderInfo() function."));
			m_nFinishErrNum =  FC_INIT_ERROR;
			return false;
		}
		
		// We don't care  incompatible packages when INCMP_CONFIG_FILE file is not exist.
		if(access(INCMP_CONFIG_FILE, F_OK) == 0)
		{
			if(m_rpmEngine->ReadIncmplistInfo(INCMP_CONFIG_FILE) == false)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to read : ",INCMP_CONFIG_FILE, NULL);
				QMessageBox::critical(this, staticTitle, tr("Failed to read : %1").arg(INCMP_CONFIG_FILE));
				m_nFinishErrNum =  FC_INIT_ERROR;
				return false;
			}
		}
		
		m_rpmEngine->SetCreateUpdateInstallListCallBack(CommonCallBack);
		if(!m_rpmEngine->CreateUpdateInstallList())
		{	
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to create package lists", NULL);
			QMessageBox::critical(this, staticTitle, tr("Failed to create package lists"));
			m_nFinishErrNum =  FC_INIT_ERROR;
			return false;
		}		
		
		// Check self update.
		if (m_rpmEngine->CheckSelfUpdate() == true)
		{
			m_nSelfUpdateMode = true;			
		}
		else
		{
			m_nSelfUpdateMode = false;
		}
		
		
		vector <structFileInfo> vectorUpdate = m_rpmEngine->GetUpdateList();
		m_nUpdateCount = 0;
		if (m_configEnv.GetOption("main", "apply_blacklist") == "false")
		{	
			m_nUpdateCount = vectorUpdate.size();
		}
		else 
    		{
			vector<structFileInfo>::iterator it;
			for(it=vectorUpdate.begin();it!=vectorUpdate.end();it++)
        		{
				if (it->bBlacklisted == false)
        			{
					m_nUpdateCount++;
        			}
        		}
    		}
		
		if (m_rpmEngine->WriteNotifierInfo(m_nUpdateCount) == false)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Faild to load the WriteNotifierInfo() function." , NULL);
			QMessageBox::critical(this, staticTitle, tr("Faild to load the WriteNotifierInfo() function."));
			m_nFinishErrNum =  FC_INIT_ERROR;
			return false;
		}
		
		vector <structFileInfo> vectorInstall = m_rpmEngine->GetInstallList();
		
		
		if(!m_nSelfUpdateMode)
		{			
			if(comboCommand->currentItem() == 0 && vectorUpdate.size() == 0 ) //update
			{	
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "There are no packages available to update!" , NULL);
				QMessageBox::information(this, staticTitle,  tr("There are no packages available to update!"));
				m_nFinishErrNum = FC_THERE_ARE_NO_COUNT;
				return false;		
			}
			else if(comboCommand->currentItem() == 1 && vectorInstall.size() ==0 ) //install
			{	
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "There are no packages available for installation!" , NULL);
				QMessageBox::information(this, staticTitle, tr("There are no packages available for installation!"));
				m_nFinishErrNum = FC_THERE_ARE_NO_COUNT;
				return false;			
			}
		}
		 
						
		
		
		labelTotalStatus->setText(tr("Downloading header files...."));
		labelCurrentStatus->setText(tr(""));
		m_Network->SetGetHeadersCallBack(GetHeadersCallBack);
				
		m_bDownload = true;
		btnCancel->setEnabled(true);	
			
		
		//This is homework.  I have to improvement for good performance. 
		//int nRet = m_Network->GetHeaders(vectorUpdate, vectorInstall, m_rpmEngine->CheckSelfUpdate());		
		nRet = m_Network->GetHeaders(vectorUpdate, vectorInstall);
		m_bDownload = false;
		btnCancel->setEnabled(false);
	    if(m_Network->IsStopDownload() == true)
	    {	
	    	m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Downloading header has been stopped", NULL);
	    	m_nFinishErrNum =  FC_STOP_DOWNLOAD;
	    	return false;
	    }
	    else if (show_NetworkErrorDialog(nRet) == false)
		{
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Network error(%d) is occurred", nRet);
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, strDubugMsg, NULL);
			m_nFinishErrNum =  FC_NETWORK_ERROR;
			return false;
		}
						
		m_rpmEngine->SetNetwork(m_Network);
		
	
		labelTotalStatus->setText(tr("Header info download has finished."));
		labelCurrentStatus->setText(tr(""));
		if(ReadHeaders() == false)
		{	
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to load the ReadHeaders() function.", NULL);
			QMessageBox::critical(this, staticTitle, tr("Failed to load the ReadHeaders() function."));			
			m_nFinishErrNum =  FC_INIT_ERROR;
			return false;
		}		
	}	
	return true;
}

/*!
@brief Error handler for getting packages

@param nErr - network error number
*/
bool classGui::show_NetworkErrorDialog(int nErr)
{	
	bool bRet=false;
	switch(nErr)
	{
		case NETWORK_ERR_WRONG_URL:
			QMessageBox::critical(this, staticTitle, (URL_ERR_MSG));
			bRet = false;  
			break;
		case NETWORK_ERR_CONNECT:
			QMessageBox::critical(this, staticTitle, (CONNECT_ERR_MSG));
			bRet = false;             
			break;
		case NETWORK_ERR_AUTH_FAIL:
			QMessageBox::critical(this, staticTitle, (AUTH_ERR_MSG));
			bRet = false;                
			break;
		case NETWORK_ERR_UNKNOWN:
			QMessageBox::critical(this, staticTitle, (UNKNOWN_ERR_MSG));
			bRet = false;	                
			break;	
		case NETWORK_ERR_USERCANCEL:  
			QMessageBox::information(this, staticTitle, (USERCANCEL_ERR_MSG));
			bRet = false;	                
			break;
		case NETWORK_ERR_FWRITE:  
			QMessageBox::critical(this, staticTitle, (USERFWRITE_ERR_MSG));
			bRet = false;	                
			break;	 	              
		default:
			bRet = true;
			break;
	}	
	return bRet;
}


/*!
@brief Show package list.

Show update or install or erase package lists. 
*/
void classGui::ShowList()
{		
	m_nUpdateCount = 0;	
	chkSelectAll->setChecked(false);	
	ClearPackageList();
	
	if(comboCommand->currentItem() == 2) //Erase 
	{
		set <structHeaderInfo, DereferenceLess> vectorHeaderInfo;
	       
	  	vectorHeaderInfo = m_rpmEngine->GetInstalledList();
	  	set <structHeaderInfo, DereferenceLess>::iterator it;
		for(it=vectorHeaderInfo.begin();it!=vectorHeaderInfo.end();it++) 
		{		
			string  strName1, strVer1, strRel1, strArch1;
			m_rpmEngine->stripNVRA(it->strNVRA, &strName1, &strVer1, &strRel1, &strArch1);
			QCheckListItem *item = new QCheckListItem(listPackages, strName1.c_str(), QCheckListItem::CheckBox);
			item->setText(1, strVer1.c_str());
			item->setText(2, strRel1.c_str());
			item->setText(3, strArch1.c_str());
			listPackages->insertItem(item);
		}
	}
	else // install, update
    { 
		vector <structFileInfo> vectorFileInfo;
		if(m_nSelfUpdateMode == false) 
		{
			if(comboCommand->currentItem() == 0) 
		    {
				vectorFileInfo = m_rpmEngine->GetUpdateList();
		    }
			else if(comboCommand->currentItem() == 1) 
		    {
				vectorFileInfo = m_rpmEngine->GetInstallList();
		    }
			AddPackageList(vectorFileInfo);		  
		} 
		else
		{			
			vectorFileInfo = m_rpmEngine->GetUpdateList();
			AddPackageList(vectorFileInfo);		  	
			vectorFileInfo = m_rpmEngine->GetInstallList();
			AddPackageList(vectorFileInfo);
		}
    }    
    
    //printf("Count : %d\n", listPackages->childCount());
    labelTotalCount->setText("");
    qApp->processEvents();
    labelTotalCount->setText(tr("Total Count :  %1").arg(listPackages->childCount()));
    qApp->processEvents();	
    radioFullUpdate->setText(tr( "Full Update" ) 
    		+ tr("(Available packages : %1, Blacklisted packages : %2)").arg(QString("%1").arg(m_nUpdateCount), 
    		                                                                                                    QString("%1").arg(listPackages->childCount() - m_nUpdateCount)));
}

/*!
@brief Show progress.

When CreateUpdateInstallListCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::CommonCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{	
	g_progressTotal->setProgress(p1, 100);
	g_progressCurrent->setProgress(p2, 100);
	
	g_labelTotalStatus->setText(tr(msg1));
	g_labelCurrentStatus->setText(tr(msg2));
	qApp->processEvents();
}

/*!
@brief Show progress.

When ReadHeadersCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::ReadHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	
	g_progressTotal->setProgress(p1, 100);
	g_progressCurrent->setProgress(p2, 100);

	g_labelTotalStatus->setText(tr(msg1));
	g_labelCurrentStatus->setText(tr(msg2));
	qApp->processEvents();
}

/*!
@brief Show progress.

When CheckCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::CheckCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	p1=0;
	p2=0;
	msg1=NULL;
	msg2=NULL;

	qApp->processEvents();
}

/*!
@brief Show progress.

When RunCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::RunCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	
	g_progressTotal->setProgress(p1, 100);
	g_progressCurrent->setProgress(p2, 100);
	g_labelTotalStatus->setText(tr(msg1));
	g_labelCurrentStatus->setText(tr(msg2));
	
	qApp->processEvents();

}

/*!
@brief Show progress.

When GetHeadersCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::GetHeadersCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	
	g_progressTotal->setProgress(p1, 100);
	g_progressCurrent->setProgress(p2, 100);
	g_labelTotalStatus->setText(tr(msg1));
	g_labelCurrentStatus->setText(tr(msg2));
	
	qApp->processEvents();	
}

/*!
@brief Show progress.

When GetPackagesCallBack() function operate, Show progress.
 
@param p1  The percent of first progress bar.
@param p2  The second of first progress bar.
@param msg1 The percent of first messages of progress bar.
@param msg2 The percent of second messages of progress bar.
*/
void classGui::GetPackagesCallBack(int p1, int p2, const char * msg1, const char * msg2 )
{
	
	g_progressTotal->setProgress(p1, 100);
	g_progressCurrent->setProgress(p2, 100);
	
	g_labelTotalStatus->setText(tr(msg1));
	g_labelCurrentStatus->setText(tr(msg2));
	qApp->processEvents();	
}

/*!
@brief Read headers.

Read header infos from all header files(*.hdr). 
 
*/
bool classGui::ReadHeaders()
{		
	labelTotalStatus->setText(tr("Reading header files"));
	labelCurrentStatus->setText("");
		
	m_rpmEngine->SetReadHeadersCallBack(ReadHeadersCallBack);	
	if(m_rpmEngine->ReadHeaders() != 0)
	{
		return false;
	}
	labelTotalStatus->setText(tr("Gathering of package information is complete."));
	labelCurrentStatus->setText("");
	return true;
}

/*!
@brief Do Update or Install or Erase rpm packages.

Do Update or Install or Erase rpm packages.

@return 0 is ok ,  -9: looped (we can not find reqiured packages)  
*/

int classGui::ProceedUpdate()
{
	int result;
	int nRet;
	int index = 0;	
	int nCanSlectPackages=0;
	m_nIncompatibleCount = 0;
	m_vectorRemovedIncompatiblePackages.clear();	
	classShowHideBox showHideBox;
	QCheckListItem *item;
	item = (QCheckListItem *)listPackages->firstChild();
	while ( item )
	{
		
		if( m_nMode == UPDATE_MODE)
		{
			//Check incompatible packages.
			if(item->text(5) == (QString)COMPATIBLE_NO)
			{		
				m_nIncompatibleCount++;
				m_vectorRemovedIncompatiblePackages.push_back((const char *)(item->text(0) + "-" + item->text(1) + "-" + item->text(2) + "." + item->text(3)));
			}
			//Check select available packages.
			if(item->isEnabled() && item->text(5) == (QString)COMPATIBLE_YES )
			{
				nCanSlectPackages++;
			}
		}		
		
		//Check checked item.
		if (item->isOn() == true)
		{
			index++;
			showHideBox.AppendList( QString("(%1)").arg(index) + item->text(0) + "-" + item->text(1) + "-"  + item->text(2) + "."  + item->text(3));
		}
		
		item = (QCheckListItem *)item->nextSibling();
	}
	
	// Condition : Update mode && Incompatible packate counter > 0 && Full update mode.
	// When install mode,  Doesn't need to show warning message about incompatible packages 
	// because there are no imcompatible package list. 
	if(m_nMode == UPDATE_MODE && m_nIncompatibleCount > 0 && radioFullUpdate->isOn())
	{
		ShowRemovedIncompatiblePackages();
	}
	
	if ((index) == 0)
	{
		if(m_nMode == UPDATE_MODE)
		{
			if( nCanSlectPackages == 0 && radioFullUpdate->isOn())
			{				
				m_nFinishErrNum =  FC_THERE_ARE_NO_COUNT;
				return 1;
			}
			else
			{
				QMessageBox::information(this, staticTitle, tr("No packages are selected!  Please select packages to update!"));
			}
		}
		else if(m_nMode == INSTALL_MODE)
		{
			QMessageBox::information(this, staticTitle, tr("No packages are selected!  Please select packages to install!"));			
		}
		else
		{
			QMessageBox::information(this, staticTitle, tr("No packages are selected!  Please select packages to erase!"));
		}
		return 2;
	}

	
	
	
	QString strMessage;	
	
	
	showHideBox.SetCaption(staticTitle);
	showHideBox.SetOkButtonText(TR_YES);				
	showHideBox.SetCancelButtonText(TR_NO);	
	showHideBox.SetCheckOptionText(tr("Continue without any file dependency warning messages?"));
			
	if (comboCommand->currentItem() == 0)
	{		
		showHideBox.SetMessage(tr("Do you want to Update?"));
	}
	else if (comboCommand->currentItem() == 1)
	{	
		showHideBox.SetMessage(tr("Do you want to Install?"));
	}
	else
	{	
		showHideBox.SetMessage(tr("Do you want to Erase?"));
	}	
	result = showHideBox.Domodal();	
	
	if (result == 0)
	{			
		return 2;
	}	

	m_bCanNotAskAboutDependency = showHideBox.GetCheckOption();	
	
	string strFullPath;
	string strUrlFullPath;
	string strFilePath;	
	
	if(m_nSelfUpdateMode == false)
	{
		item = (QCheckListItem *)listPackages->firstChild();
		while ( item )
		{

			if (item->isOn() == true)
			{					
				
				if(comboCommand->currentItem() ==2) //Erase
				{
					strFilePath = (const char *)(item->text(0) + "-" + item->text(1) + "-"  + item->text(2) + "."  + item->text(3));
					snprintf(strDubugMsg, sizeof(strDubugMsg), "Add Erase : %s", strFilePath.c_str());
					m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);										
					if( m_rpmEngine->AddFile(strFilePath.c_str(), REMOVE, 0) != 0)
					{	
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFilePath.c_str()), NULL);
						QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFilePath.c_str()));						
						return 1;
					}
				}
				else if(comboCommand->currentItem() ==1) //install
				{
					strUrlFullPath = m_rpmEngine->GetFullPathFile(1,  false, (const char *)item->text(0), (const char *)item->text(1) , (const char *)item->text(2), (const char *)item->text(3), "", true);
					strFullPath = m_rpmEngine->GetFullPathFile(1,  false, (const char *)item->text(0), (const char *)item->text(1) , (const char *)item->text(2), (const char *)item->text(3), "");
					snprintf(strDubugMsg, sizeof(strDubugMsg), "Add install : %s", strFullPath.c_str());
					m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);
					if(m_rpmEngine->AddFile(strFullPath.c_str(), UPDATE, 0) != 0)
					{	
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()), NULL);
						QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()));						
						return 1;
					}
					if (m_nMode != ERASE_MODE)
					{
						m_Network->AddPackage(strFullPath, strUrlFullPath);
					}
				}
				else  //update
				{
					strUrlFullPath = m_rpmEngine->GetFullPathFile(0,  false, (const char *)item->text(0), (const char *)item->text(1) , (const char *)item->text(2), (const char *)item->text(3), "", true);
					strFullPath = m_rpmEngine->GetFullPathFile(0,  false, (const char *)item->text(0), (const char *)item->text(1) , (const char *)item->text(2), (const char *)item->text(3), "");
					snprintf(strDubugMsg, sizeof(strDubugMsg), "Add update : %s", strFullPath.c_str());
					m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);					
					if(m_rpmEngine->AddFile(strFullPath.c_str(), UPDATE, 1) != 0)
					{	
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()), NULL);
						QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()));						
						return 1;					
					}
					if (m_nMode != ERASE_MODE)
					{
						m_Network->AddPackage(strFullPath, strUrlFullPath);
					}
				}
			}

			item = (QCheckListItem *)item->nextSibling();

		}
	}
	else //self udpate
	{
		//update
		vector <structFileInfo>::iterator it;
		vector <structFileInfo> vectorUpdateList;
		vectorUpdateList = m_rpmEngine->GetUpdateList();
		for(it=vectorUpdateList.begin();it!=vectorUpdateList.end();it++)
		{
			strUrlFullPath = m_rpmEngine->GetFullPathFile(0,  false, it->strName.c_str(), it->strVersion.c_str() , it->strRelease.c_str(), it->strArch.c_str(), "", true);
			strFullPath = m_rpmEngine->GetFullPathFile(0,  false, it->strName.c_str(), it->strVersion.c_str() , it->strRelease.c_str(), it->strArch.c_str(), "");
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Add update : %s", strFullPath.c_str());
			m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);			
			if (m_rpmEngine->AddFile(strFullPath.c_str(), UPDATE, 1) != 0)
			{	
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()), NULL);
				QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()));						
				return 1;					
			}
			m_Network->AddPackage(strFullPath, strUrlFullPath);

		}
		//install
		
		vector <structFileInfo> vectorInstallList; 
		vectorInstallList = m_rpmEngine->GetInstallList();
		for(it=vectorInstallList.begin();it!=vectorInstallList.end();it++)
		{
			strUrlFullPath = m_rpmEngine->GetFullPathFile(1,  false, it->strName.c_str(), it->strVersion.c_str() , it->strRelease.c_str(), it->strArch.c_str(), "", true);
			strFullPath = m_rpmEngine->GetFullPathFile(1,  false, it->strName.c_str(), it->strVersion.c_str() , it->strRelease.c_str(), it->strArch.c_str(), "");
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Add install : %s", strFullPath.c_str());
			m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);			
			if (m_rpmEngine->AddFile(strFullPath.c_str(), UPDATE, 0) != 0)
			{	
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()), NULL);
				QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()));						
				return 1;					
			}
			m_Network->AddPackage(strFullPath, strUrlFullPath);
		}

	}
	if (m_nMode != ERASE_MODE)
	{
		/*m_Network->SetGetPackagesCallBack(GetPackagesCallBack);
		m_bDownload = true;
		btnCancel->setEnabled(true);
		nRet = m_Network->GetPackages();
		m_bDownload = false;
		btnCancel->setEnabled(false);
		if(m_Network->IsStopDownload() == true)
		{
			m_nFinishErrNum =  FC_STOP_DOWNLOAD;			
			return 1;
		}
		else if (show_NetworkErrorDialog(nRet) == false)
		{			
			m_nFinishErrNum =  FC_NETWORK_ERROR;
			return 1;
		}
		*/
// Redcastle does'nt support PPC.
#if defined(__i386__) || defined(__x86_64__) || defined(__ia64__)	

		if(m_nRedcastleStatus == RC_STATUS_WARNING)
		{
			if(m_rpmEngine->CheckKmodRedcastle() == false)
			{
				QString strMessage;
				classShowHideBox showHideBox;
				showHideBox.SetCaption(staticTitle);
				showHideBox.SetOkButtonText( TR_OK);
				showHideBox.SetMessage(REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG_TR);
				vector <string> vectorTemp = m_rpmEngine->GetAddedKmodRedcastleFile();
				vector <string>::iterator it;
				int nIndex = 1;
				string strKmodName;
				for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
				{
					string  strName, strVer, strRel, strArch;
					m_rpmEngine->stripNVRA(*it, &strName, &strVer, &strRel, &strArch);
					strKmodName = strName + "-" + strVer + "-" + strRel + "." + strArch;
					strMessage.sprintf("(%d) %s ", nIndex, strKmodName.c_str());
					showHideBox.AppendList(strKmodName);	
				}
		
				showHideBox.Domodal();
		
				if(m_rpmEngine->RemoveKernelAndKmodRedcastle(comboCommand->currentItem()) == false)
				{
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to remove kernel and redcastle.", NULL);
					return 1;
				}
				vector <structAddedFile> vectorForCount;
				vectorForCount = m_rpmEngine->GetAddedFile( UPDATE, vectorForCount);
				if((int)vectorForCount.size() < 1)
				{
					if(m_nMode == UPDATE_MODE)
			                {
		        	                QMessageBox::information(this, staticTitle, tr("There are no packages available to update!"));
		                	}
			                else
		        	        {
		                	        QMessageBox::information(this, staticTitle, tr("There are no packages available to install!"));
		        	        }
					return 2;
				}
			}
			else
			{
				vector <string> vectorTemp = m_rpmEngine->GetAddedKmodRedcastleFile();
		                vector <string>::iterator it;
		                int nIndex = 1;
		                for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
		                {
					string  strName1, strVer1, strRel1, strArch1;			
					m_rpmEngine->stripNVRA(*it, &strName1, &strVer1, &strRel1, &strArch1);
					int nIsUpdate;
					if(m_nMode == UPDATE_MODE)
					{
						nIsUpdate = 0;					
					}
					else				
					{
						nIsUpdate = 1;
					}
					strUrlFullPath = m_rpmEngine->GetFullPathFile(nIsUpdate,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "", true);
					strFullPath = m_rpmEngine->GetFullPathFile(nIsUpdate,  false, strName1.c_str(), strVer1.c_str() , strRel1.c_str(), strArch1.c_str(), "");
	
					if (m_nMode != ERASE_MODE)
					{
						m_Network->AddPackage(strFullPath, strUrlFullPath);
					}
					if(m_rpmEngine->AddFile(strFullPath.c_str(), REQDEP, 0) != 0)
					{	
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()), NULL);
						QMessageBox::critical(this, staticTitle, tr("Failed to load m_rpmEngine->AddFile(%1) function.").arg(strFullPath.c_str()));
						return 1;
					}

					snprintf(strDubugMsg, sizeof(strDubugMsg), "Add install : %s", strFullPath.c_str());
					m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);
					
		
		                }
			}
		}
#endif	
	
	}	
	// Download rpm packages from update server.
	if (m_nMode != ERASE_MODE)
	{
		m_Network->SetGetPackagesCallBack(GetPackagesCallBack);

		m_bDownload = true;

		btnCancel->setEnabled(true);
		nRet = m_Network->GetPackages();
		m_bDownload = false;
		btnCancel->setEnabled(false);
		if(m_Network->IsStopDownload() == true)
		{			
			m_nFinishErrNum =  FC_STOP_DOWNLOAD;			
			return 1;
		}
		else if (show_NetworkErrorDialog(nRet) == false)
		{
			m_nFinishErrNum =  FC_NETWORK_ERROR;			
			return 1;
		}
	}


	g_labelTotalStatus->setText(tr("Checking dependencies..."));
	g_labelCurrentStatus->setText(tr("Please wait a few minutes."));


	m_rpmEngine->SetCheckCallBack(CheckCallBack);

	

	result = m_rpmEngine->Check();

	if(result > -1)
	{
		if (result > 0)
		{

			//test
			//m_rpmEngine->ChangeGrubToDefaultKernel();
			
			if (!m_bCanNotAskAboutDependency)
			{
				classShowHideBox showHideBox;
				showHideBox.SetCaption(staticTitle);
				showHideBox.SetOkButtonText( TR_YES);
				showHideBox.SetCancelButtonText( TR_NO);						
				showHideBox.SetMessage(tr("Dependency files exist!  Do you want continue?"));
				vector <structAddedFile> vectorTemp;
				vectorTemp = m_rpmEngine->GetAddedFile( UPDATE | REMOVE |REQDEP | OTHERDEP, vectorTemp);
	
				vector <structAddedFile>::iterator it;
				int nIndex = 1;
				for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
				{	
					string strRpmName;			
					strRpmName = m_rpmEngine->GetFullFileNameFromPath(it->strFile, strRpmName);
					if (it->nType == REQDEP)
					{
						strMessage.sprintf("(%d) %s ", nIndex, strRpmName.c_str());
						strMessage = strMessage + "[" + tr("Dependency package") + "]";
					}
					else if(it->nType == OTHERDEP)
					{
						strMessage.sprintf("(%d) %s ", nIndex, strRpmName.c_str());
						strMessage = strMessage  + "[" + tr("Other arch package")  + "]";
					}
					else
					{
						strMessage.sprintf("(%d) %s ", nIndex, strRpmName.c_str());
						strMessage = strMessage  + "[" + tr("Selected package")  + "]";
						
					}
					showHideBox.AppendList(strMessage);
				}
				if(showHideBox.Domodal() == 0)
				{
					m_nFinishErrNum =  FC_USER_CANCEL;
					return 1;
				}			
			}
		}
		labelTotalStatus->setText(tr("Installing ... "));
		labelCurrentStatus->setText(tr(""));
		m_rpmEngine->SetRunCallBack(RunCallBack);
		nRet = m_rpmEngine->Run();

		if(nRet == -6) // conflict
		{
			QString strTemp;
			strTemp = tr("Packages to be updated contain some files which also belong to different packages.") + "\n" + tr("Do you want to continue?") ;
			
			classShowHideBox showHideBox;
			showHideBox.SetCaption(staticTitle);
			showHideBox.SetOkButtonText( TR_YES);				
			showHideBox.SetCancelButtonText( TR_NO);
			showHideBox.SetMessage(strTemp);
			
			char strBuf[MAX_STRING];
			char strErrorPath[MAX_STRING];
			snprintf(strErrorPath, sizeof(strErrorPath), "%s%s",  m_strLogPath.c_str(),  LAST_ERROR_LOG_FILE);
			ifstream fin;
			fin.open(strErrorPath);
			if (fin.is_open() == true) 
			{
				while (fin.getline(strBuf, sizeof(strBuf)) > 0 )
				{					
					showHideBox.AppendList(strBuf);					
				}
				
				if(showHideBox.Domodal() == 1)
				{
					m_rpmEngine->SetRunCallBack(RunCallBack);
					nRet = m_rpmEngine->Run(true);					
				}
				else
				{
					m_nFinishErrNum =  FC_USER_CANCEL;
					return 1;
				}				
				fin.close();
			}
			else 
			{						
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)QString("%1 not found").arg(strErrorPath), NULL);
				QMessageBox::critical(this, staticTitle, tr("%1 not found").arg(strErrorPath));
				return 1;
			}			
		}
		labelTotalStatus->setText(tr("Done"));
		labelCurrentStatus->setText(tr(""));

		ShowFinishPage(nRet);

	}
	else if(result == -9 || result == -6 ||  result == -10)  // -9: looped   -6: not found -10:network error 
	{
		
		QString strErr;
		if (result == -9)
		{
			strErr = tr("Reqiured Packages were not found. (Package Requirement Loop) See %1%2").arg(m_strLogPath, ERROR_LOG_FILE);
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Reqiured Packages were not found. (Package Requirement Loop) See %s%s", m_strLogPath.c_str(), ERROR_LOG_FILE);
		}	
		else if(result == -10)
		{			
			strErr = tr("Required data were not found on the server. See %1%2").arg(m_strLogPath, ERROR_LOG_FILE);
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Required data were not found on the server. See %s%s", m_strLogPath.c_str(), ERROR_LOG_FILE);
		}
		else
		{
			strErr = tr("Reqiured Packages were not found. See %1%2").arg(m_strLogPath, ERROR_LOG_FILE);
			snprintf(strDubugMsg, sizeof(strDubugMsg), "Reqiured Packages were not found. See %s%s", m_strLogPath.c_str(), ERROR_LOG_FILE);
		}
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)strDubugMsg, NULL);
		QMessageBox::critical(this, staticTitle, (strErr));			 
		return 1;	
	}
	
	// Blocked by blacklist.
	else if(result == -8)
	{
		
		QString strMsgDebug;
		classShowHideBox showHideBox;
		showHideBox.SetCaption(staticTitle);
		showHideBox.SetOkButtonText( TR_OK);
		
		vector <string>::iterator it;
		vector <string> vectorTemp;		
		// There are incompatible packages.
		if(m_rpmEngine->GetIncompatibleCount() > 0)
		{
			vectorTemp = m_rpmEngine->GetIncompatiblePackages();
			showHideBox.SetMessage(tr("Some incompatible packages have been added to the target package list as dependent packages. \nPlease update the following packages manually. \nIf you want to update them, \nplease visit the TSN web page."));
			strMsgDebug = QString("Some incompatible packages have been added to the target package list as dependent packages. \nPlease update the following packages manually. \nIf you want to update them, \nplease visit the TSN web page.");			
			m_nFinishErrNum =  FC_INCOMPATIBLE;	
		}
		else
		{
			vectorTemp = m_rpmEngine->GetBlockedPackages();
			showHideBox.SetMessage(tr("Some blacklisted packages have been added to the target package list as dependent packages. \nPlease remove the following packages from the blacklist."));
			strMsgDebug = QString("Some blacklisted packages have been added to the target package list as dependent packages. \nPlease remove the following packages from the blacklist.");
			m_nFinishErrNum =  FC_BLACKLIST;	
		}		
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, (const char *)strMsgDebug , NULL);
		string strRpmName;
		for(it=vectorTemp.begin();it!=vectorTemp.end();it++)
		{
			strRpmName = m_rpmEngine->GetFullFileNameFromPath(it->c_str(), strRpmName);
			showHideBox.AppendList(strRpmName);
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, strRpmName.c_str() , NULL);					
		}
		showHideBox.Domodal();	
			
		return 1;	
	}	
	else
	{		
		return 1;
	}
	
	return 0;
}

//*! @brief Push Cancel button
void classGui::slotCancel()
{	
	reject();
}

/*!
@brief Set Operation mode.

axtsn-updater includes 3 mode.(update, install, erase)
This function can select one mode. 

@param nIndex 0: Update, 1:Install, 2:Erase  
*/

void classGui::slotSelectCommand(int nIndex)
{		
	chkSelectAll->setChecked(false);
	if(nIndex == 0)
	{
		this->setCaption(tr("Update Wizard - %1").arg((AXTU_TITLE)));
		labelTitle->setText(tr("<b>Online Package Update Wizard</b>"));
		labelCmdExplain->setText(tr("<br>The Update Wizard will update installed packages on your system to the latest version available from Asianux.<br>\nPlease click the \"Next\" button."));
	}
	else if(nIndex == 1)
	{
		this->setCaption(tr("Install Wizard -  %1").arg((AXTU_TITLE)));
		labelTitle->setText(tr("<b>Online Package Install Wizard</b>"));
		labelCmdExplain->setText(tr("<br>The Install Wizard can install new packages available from the update server.<br>\nPlease click the \"Next\" button."));		
	}
	else if(nIndex == 2)
	{
		this->setCaption(tr("Erase Wizard - %1").arg((AXTU_TITLE)));
		labelTitle->setText(tr("<b>Package Erase Wizard</b>"));		
		labelCmdExplain->setText(tr("<br>You can remove installed packages on your system with it. Please be aware that removing<br>\nsome packages might result in a serious system crash.\nPlease click the \"Next\" button."));		
	}
	
	int nCommand; 
	if(comboCommand->currentItem() == 2) 
	{
		nCommand = 0; //Erase 
		chkSelectAll->setShown(false);
	}
	else 
	{
		nCommand = 1; // Update, Install
		chkSelectAll->setShown(true);
	}
}

//*! @brief Push Back button

void classGui::slotBack()
{
	MoveBackPage();
}

//*! @brief Push Next button
void classGui::slotNext()
{	
	MoveNextPage();
}

//*! @brief Select full update mode.
void classGui::slotSelectFull()
{
	radioFullUpdate->setChecked(true);
	radioCustomUpdate->setChecked(false);
	
	if(m_nUpdateCount > 0 || m_nMode == ERASE_MODE)
	{
		btnNext->setEnabled(true);
	}
	else
	{
		btnNext->setEnabled(false);
	}

}

//*! @brief Select custom update mode.
void classGui::slotSelectCustom()
{
	radioCustomUpdate->setChecked(true);
	radioFullUpdate->setChecked(false);
	btnNext->setEnabled(true);
}

//*! @brief Select all packpages on showed list.
void classGui::slotSelectAll(int nSelect) 
{	
	bool bSelect;
	if (QCheckListItem::Off == nSelect)
	{
		bSelect = false;		
	}
	else
	{
		bSelect = true;
	}		
	CheckChildItem((QCheckListItem*)listPackages->firstChild(), bSelect);	
}
 
//*! @brief Push Blacklist button.
void classGui::slotSetupBlacklist() 
{	 
	
	m_processSetupBlack->clearArguments();	
	m_processSetupBlack->addArgument(SETUP_PATH);
	m_processSetupBlack->addArgument("-b");
	
	
	if (!m_processSetupBlack->start())
	{		
		QMessageBox::critical(this, staticTitle, tr("Setup program is not installed."));
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU,  "Setup program is not installed.", NULL);
		
	}		
}

void classGui::slotChangeListPackage(QListViewItem * item)
{
	char strTemp[MAX_STRING];
	snprintf(strTemp, sizeof(strTemp), "%s-%s-%s.%s"
	  ,(const char *)item->text(0), (const char *)item->text(1) , (const char *)item->text(2), (const char *)item->text(3));
	SetPackageSummary(strTemp); 
}

void classGui::slotClickPackage(QListViewItem * item)
{
	if(!item)
	{
		return;
	}
	if( (m_nMode == UPDATE_MODE) && 
			(((QCheckListItem *)item)->state() != QCheckListItem::Off )&& 
			(((QCheckListItem *)item)->text(5) == (QString)COMPATIBLE_NO))
	{	
		QMessageBox::warning(this, staticTitle, tr("You selected some incompatible packages. If you want to update these packages, please visit the TSN web page."), TR_OK);
		((QCheckListItem*)item)->setState(QCheckListItem::Off);		
	}
}

//*! @brief Check showed item recusively.
int classGui::CheckChildItem(QCheckListItem* item, bool b)
{
	if(item ==NULL)
	{
		return 0;
	}
   

	QCheckListItem *item2;
	item2 = (QCheckListItem *)item;    
	if ( item2 )
	{     	
		do
		{   
			if (item2->isEnabled()) 
			{					
				if((m_nMode == UPDATE_MODE) && 
						((QCheckListItem *)item2)->text(5) == (QString)COMPATIBLE_NO)
				{
					item2->setOn(false);					
				}
				else
				{
					item2->setOn(b);
				}
			}
			if(item2->firstChild() != 0)
			{
				CheckChildItem(item2, b);        
			}
			item2 = (QCheckListItem *)item2->nextSibling();            
		}
		while (item2);
	}
  return 0;
}

//*! @brief Showe page.
int  classGui::ShowPage(int nPage)
{         
	if(nPage < 0)
	{
		nPage = WELCOME_PAGE;
	}
	int nRet = -1;
	if (nPage == WELCOME_PAGE) 
	{
		nRet = WELCOME_PAGE;
		ShowWelcomePage();
	}
	else if (nPage == SELECT_COMMAND_PAGE)
	{		
		nRet = SELECT_COMMAND_PAGE;		
		ShowSelectCommandPage();
	}
	else if (nPage == SELECT_PACKAGE_PAGE)
	{		
		nRet = SELECT_PACKAGE_PAGE;		
		ShowSelectPackagePage();		
	}
	else if (nPage == PROGRESS_PAGE)
	{
		nRet = PROGRESS_PAGE;
		ShowProgressPage();		
	}
	else //if (nPage == FINISH_PAGE)
	{
		nRet = FINISH_PAGE;
		ShowFinishPage();
	}
	
	return nRet;
	
}

//! @brief Move next page
int classGui::MoveNextPage(int nStep)
{	
	LockWigets();
	int nNextPage = GetCurPage() + nStep;
	if(nNextPage == SELECT_PACKAGE_PAGE)
	{
		// Selected update and Full update is check then go progress page.
		if(radioFullUpdate->isOn() == true && comboCommand->currentItem() == 0)
		{
			slotSelectAll(QCheckListItem::On);			  
			nStep ++;
		}
		
	}
	else if(nNextPage == SELECT_COMMAND_PAGE)
	{		
		if (InitRpm() == false)
		{
			return ShowFinishPage();			  	
		}
		ShowList();
		
		// Found newer updater element.
		if( m_nSelfUpdateMode == true)
		{	
			nStep++;
			int result=-1;	
			result = QMessageBox::question(this, staticTitle, tr("You have to install a new version of the TSN Updater program before installing other packages.\nDo you want to install the new updater?"), TR_YES,TR_NO,0,1);
			if (result != 0)
			{				
				return ShowFinishPage(FC_USER_CANCEL);				
			}		
			slotSelectAll(QCheckListItem::On);			  
			nStep ++;			
		}
		else
		{
			if( comboCommand->currentItem() != 0 ) // not update
			{		
				nStep++;			
			}
		}
	}
	
	// Done
	else if(nNextPage-1 == FINISH_PAGE)
	{	
		return ShowWelcomePage(); 
	}		
	return ShowPage(GetCurPage() + nStep);		
}

//! @brief Move back page
int  classGui::MoveBackPage(int nStep)
{
	
	LockWigets();
	int nPage = GetCurPage() - nStep;	
	if(nPage == SELECT_PACKAGE_PAGE)
	{
		// Selected update and Full update is check then go progress page.		
		if(radioFullUpdate->isOn() == true && comboCommand->currentItem() == 0)
		{
			slotSelectAll(QCheckListItem::On);			
			nStep ++;
		}
	}
	else if(nPage == SELECT_COMMAND_PAGE)
	{
		if( comboCommand->currentItem() != 0) // not update
		{		
			nStep++;			
		}
	}			
	
	return ShowPage(GetCurPage() - nStep);	
}

//! @brief Get current page index.
int classGui::GetCurPage()
{
	return m_nPage;	
}

//! @brief Set current page index.
void classGui::SetCurPage(int nPage)
{
	m_nPage = nPage;
}

//! @brief Set package summary and show.
void classGui::SetPackageSummary(const char * strNVRA)
{
	const char * description = NULL;
	const char * summary = NULL;
	
	Header header=NULL;
	
	if(m_rpmEngine->GetCommand() == 1)  //update, install
	{
		structRPMInfo *result=NULL;
		string  strName1, strVer1, strRel1, strArch1;
		m_rpmEngine->stripNVRA(strNVRA, &strName1, &strVer1, &strRel1, &strArch1);
		result = m_rpmEngine->FindHeaderFromRemote((char *)strName1.c_str(), (char *)strVer1.c_str(),(char *) strRel1.c_str(),result);
		edtPackageSummary->clear();
		if(result)
		{
			edtPackageSummary->append(result->summary);
			edtPackageSummary->append(" \n");
			edtPackageSummary->append(result->shortDesp);
			edtPackageSummary->append("obsoleteName:\t");//the obsolete info added here is now for debug
			edtPackageSummary->append(*(result->obsoleteName));//but maybe it is a nice feather to keep
			edtPackageSummary->append("obsoleteVersion:\t");
			edtPackageSummary->append(*(result->obsoleteVersion));
			edtPackageSummary->moveCursor(QTextEdit::MoveHome, false);
		}
	}
	else // erase
	{
		header = m_rpmEngine->FindHeaderFromLocal(strNVRA, header);
		headerGetEntry(header, RPMTAG_DESCRIPTION, NULL, (void **)&description, NULL);
		headerGetEntry(header, RPMTAG_SUMMARY, NULL, (void **)&summary, NULL);
		edtPackageSummary->clear();
		edtPackageSummary->append(summary);	
		edtPackageSummary->append("");
		edtPackageSummary->append(description);
		edtPackageSummary->moveCursor(QTextEdit::MoveHome, false);
	}
	
}

//! @brief Show welcome page
int classGui::ShowWelcomePage()
{	
	
	SetCurPage(WELCOME_PAGE);
	
	labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Welcome</b></font></p>"));
	
	btnBack->setEnabled(false);
	btnNext->setEnabled(true);
	btnCancel->setEnabled(true);
	btnNext->setText(TR_NEXT);
	btnCancel->setText(TR_CANCEL);
	
	btnSetupBlacklist->setShown(false);
	
	comboCommand->setEnabled(true);
	
	frameWelcome->setShown(true);	
	frameWelcome->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frameSelectCommand->setShown(false);
	frameSelectPackage->setShown(false);	
	frameProgress->setShown(false);
	frameFinish->setShown(false);
	
	frameWelcome->setEnabled(true);	
	frameSelectCommand->setEnabled(false);
	frameSelectPackage->setEnabled(false);	
	frameProgress->setEnabled(false);
	frameFinish->setEnabled(false);
	
	qApp->processEvents();
	return 	WELCOME_PAGE;

}

//! @brief Show select command page
int classGui::ShowSelectCommandPage()
{	
	SetCurPage(SELECT_COMMAND_PAGE);
	slotSelectAll(false);
	labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Select Update Method</b></font></p>"));
	btnBack->setEnabled(true);
	
	if(m_nUpdateCount > 0 || radioCustomUpdate->isChecked() == true || m_nMode == ERASE_MODE)
	{
		btnNext->setEnabled(true);
	}
	else
	{
		btnNext->setEnabled(false);
	}
	btnCancel->setEnabled(true);
	btnNext->setText(TR_NEXT);
	btnCancel->setText(TR_CANCEL);
	
	if (m_nMode == UPDATE_MODE )
	{
		btnSetupBlacklist->setShown(true);
	}	
	else
	{
		btnSetupBlacklist->setShown(false);
	}
	
	comboCommand->setEnabled(false);
	
	frameWelcome->setShown(false);		
	frameSelectCommand->setShown(true);
	frameSelectCommand->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frameSelectPackage->setShown(false);	
	frameProgress->setShown(false);
	frameFinish->setShown(false);
	
	frameWelcome->setEnabled(false);	
	frameSelectCommand->setEnabled(true);
	frameSelectPackage->setEnabled(false);	
	frameProgress->setEnabled(false);
	frameFinish->setEnabled(false);
	qApp->processEvents();
	return SELECT_COMMAND_PAGE;
	
}

//! @brief Show select package page
int  classGui::ShowSelectPackagePage()
{	
	SetCurPage(SELECT_PACKAGE_PAGE);
	labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Select Package</b></font></p>"));
	btnBack->setEnabled(true);	
	if(m_nUpdateCount > 0 || m_nMode == ERASE_MODE)
	{
		btnNext->setEnabled(true);
	}
	else
	{
		btnNext->setEnabled(false);
	}
	btnCancel->setEnabled(true);
	btnNext->setText(TR_NEXT);
	btnCancel->setText(TR_CANCEL);
	edtPackageSummary->clear();  
  
	if (m_nMode == UPDATE_MODE)
	{
		btnSetupBlacklist->setShown(true);		
	}
	else
	{
		btnSetupBlacklist->setShown(false);
		listPackages->hideColumn(4);
	}
	
	comboCommand->setEnabled(false);
	
	frameWelcome->setShown(false);	
	frameSelectCommand->setShown(false);
	frameSelectPackage->setShown(true);	
	frameSelectPackage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frameProgress->setShown(false);
	frameFinish->setShown(false);
	
	frameWelcome->setEnabled(false);	
	frameSelectCommand->setEnabled(false);
	frameSelectPackage->setEnabled(true);	
	frameProgress->setEnabled(false);
	frameFinish->setEnabled(false);
	
	qApp->processEvents();
	return SELECT_PACKAGE_PAGE;	
	
}

//! @brief Show progress page
int  classGui::ShowProgressPage(bool bJustShow)
{	
	if (bJustShow == false)
	{
		SetCurPage(PROGRESS_PAGE);
	}
	
	btnSetupBlacklist->setShown(false);
	
	labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Progress</b></font></p>"));
	comboCommand->setEnabled(false);
	btnBack->setEnabled(false);	
	btnNext->setEnabled(false);
	btnCancel->setEnabled(false);
	btnNext->setText(TR_NEXT);
	btnCancel->setText(TR_CANCEL);
	frameWelcome->setShown(false);	
	frameSelectCommand->setShown(false);
	frameSelectPackage->setShown(false);	
	frameProgress->setShown(true);
	frameProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	frameFinish->setShown(false);
	
	frameWelcome->setEnabled(false);	
	frameSelectCommand->setEnabled(false);
	frameSelectPackage->setEnabled(false);	
	frameProgress->setEnabled(true);
	frameFinish->setEnabled(false);
	
	if (bJustShow == false)
	{	
		int nRet = ProceedUpdate();
		if (nRet != 0)
		{
			if(nRet == 1)
			{
				ShowFinishPage();
			}
			else if(nRet == 2)
			{
				MoveBackPage();
			}				
		}
	}
	qApp->processEvents();	
	return PROGRESS_PAGE;
}

//! @brief Show finish page
int classGui::ShowFinishPage(int nErr)
{		
	if(system(RESTART_NOTIFIER_DAEMON) == -1)
	{
		QMessageBox::warning(this, staticTitle, tr("Failed to restart the notifer daemon."));
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to restart the notifer daemon." , NULL);
	}
	btnSetupBlacklist->setShown(false);
	edtFinish_result->setShown(false);
	btnNext->setEnabled(false);
	if (nErr == 0)
	{
		btnNext->setEnabled(true);
		edtFinish_result->setShown(true);
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);	
		
		if (m_nMode == INSTALL_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Install has been completed!</b></font></p>"));
			btnNext->setText(TR_INSTALL_MORE_PACKAGES);
		}
		else if (m_nMode == ERASE_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Erase has been completed!</b></font></p>"));
			btnNext->setText(TR_ERASE_MORE_PACKAGES);
		}
		else
		{
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Update has been completed!</b></font></p>"));
			btnNext->setText(TR_UPDATE_MORE_PACKAGES);
		}		
		lableContent_Finish->setText(tr("Please see the success log if you want to review the updating work in detail : %1%2").arg(m_strLogPath, SUCCESS_LOG_FILE));
		ShowCompleteMessage();
	}
	else if(nErr == FC_NOT_ENOUGH_DISK_SPACE)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);
		if (m_nMode == INSTALL_MODE || m_nMode == UPDATE_MODE) 
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Not enough disk space for installation.</b></font></p>"));			
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Not enough disk space for installation.", NULL);
		}
		
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	else if(nErr == FC_THERE_ARE_NO_COUNT)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);
		if (m_nMode == INSTALL_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>There are no packages available for installation!</b></font></p>"));			
		}
		else if (m_nMode == ERASE_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>There are no packages available to erase!</b></font></p>"));			
		}
		else
		{
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>There are no packages available to update!</b></font></p>"));			
		}
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	
	else if(nErr == FC_NETWORK_ERROR)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);		
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>A network error has occurred.</b></font></p>"));
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	else if(nErr == FC_INIT_ERROR)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);			
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Initialization has failed.</b></font></p>"));
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}	
	else if(nErr == FC_STOP_DOWNLOAD)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);	
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>File downloading has been stopped.</b></font></p>"));
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	else if(nErr == FC_USER_CANCEL)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);			
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>User has stopped installaton.</b></font></p>"));			
		lableContent_Finish->setShown(false);		
	}	
	else if(nErr == FC_BLACKLIST)
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Some blacklisted packages have been added to the target package list as dependent packages. <br>You can remove the blacklisted packages with the \"Setup blacklist\" configuration option.</b></font></p>"));
		btnSetupBlacklist->setShown(true);		
		btnNext->setText(TR_RETRY);
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	
	else if(nErr == FC_INCOMPATIBLE)
	{
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);
		labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Some incompatible packages have been added <br>to the target package list as dependent packages. <br>You have to update incompatible packages manually.<br>If you want to update these packages,<br>please visit the TSN web page.</b></font></p>"));		
		btnNext->setText(TR_RETRY);
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
	}
	else 
	{		
		labelTopTitle->setText(tr("<p align=\"center\"><font size=\"+3\"><b>Finish</b></font></p>"));
		SetCurPage(FINISH_PAGE);
		if (m_nMode == INSTALL_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Installation has failed!</b></font></p>"));			
		}
		else if (m_nMode == ERASE_MODE)
		{	
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Erase has been failed!</b></font></p>"));			
		}
		else
		{
			labelTitle_Finish->setText(tr("<p align=\"center\"><font size=\"+1\"><b>Update has failed!</b></font></p>"));			
		}	
		
		lableContent_Finish->setText(tr("See error log : %1%2").arg(m_strLogPath, ERROR_LOG_FILE));
		
	}
	
			
	btnBack->setEnabled(false);
	
	
	btnCancel->setEnabled(true);
	
	
	btnCancel->setText(TR_DONE);
	
	comboCommand->setEnabled(false);
	
	frameWelcome->setShown(false);	
	frameSelectCommand->setShown(false);
	frameSelectPackage->setShown(false);	
	frameProgress->setShown(false);
	frameFinish->setShown(true);
	frameFinish->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	frameWelcome->setEnabled(false);	
	frameSelectCommand->setEnabled(false);
	frameSelectPackage->setEnabled(false);	
	frameProgress->setEnabled(false);
	frameFinish->setEnabled(true);
	
	qApp->processEvents();
	
	if (m_rpmEngine->IsKernelInstalled() && nErr == 0)
	{
		QMessageBox::information(this, staticTitle, tr("Kernel has been updated. If you want to use new kernel, you have to restart your system."));
	}
	
	if (m_nSelfUpdateMode && nErr == 0)
	{
		QMessageBox::information(this, staticTitle, tr("The TSN Updater has been updated.  It must restart."));
		delete m_rpmEngine;
		m_rpmEngine = NULL;
		
		if (m_nMode == INSTALL_MODE)
		{	
			execl(UPDATER_PATH, "axtu-gui", "-i", "-f", NULL);
		}
		else if (m_nMode == ERASE_MODE)
		{	
			execl(UPDATER_PATH, "axtu-gui", "-e", "-f", NULL);
		}
		else
		{
			execl(UPDATER_PATH, "axtu-gui", "-u", "-f", NULL);
		}
		
		// If execl functions was worked normally then bellow command will not be executed. 
		QMessageBox::critical(this, staticTitle, tr("Failed to restart"));
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME_AXTU, "Failed to restart", NULL);
				
	}
	return FINISH_PAGE; 
}


//! @brief Show complete message
void classGui::ShowCompleteMessage()
{
	edtFinish_result->clear();	
	vector <structAddedFile> vectorTemp; 
	vectorTemp = m_rpmEngine->GetAddedFile( UPDATE | REMOVE | REQDEP |OTHERDEP , vectorTemp);
	
	vector <structAddedFile>::iterator it;
	int nIndex = 1;
	for(it=vectorTemp.begin();it!=vectorTemp.end();it++,nIndex++)
	{
		int nAction;
		QString strMessage; 
		string strRpmName;			
		strRpmName = m_rpmEngine->GetFullFileNameFromPath(it->strFile, strRpmName);
		strMessage.sprintf("(%d) %s ", nIndex, strRpmName.c_str());
		if (it->nType == UPDATE || it->nType == REMOVE)
		{
			strMessage = strMessage + "[" + tr("Selected package") + "]";
			if(m_nMode == INSTALL_MODE || m_nMode == UPDATE_MODE)
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
		}
		else
		{
			strMessage = strMessage + "[" + tr("Dependency package") + "]";
			if(m_nMode == INSTALL_MODE || m_nMode == UPDATE_MODE)
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
		}	
		m_Logger->RpmLogging(nAction,strRpmName.c_str());
		edtFinish_result->append(strMessage);
	}
	
	
}

//! @brief Clear  listPackages items.
void classGui::ClearPackageList()
{
	listPackages->clear();
}

//! @brief Add vectorFileInfo to package list.
void classGui::AddPackageList(vector <structFileInfo> vectorFileInfo)
{
	vector <structFileInfo>::iterator it;
	for(it=vectorFileInfo.begin();it!=vectorFileInfo.end();it++) 
	{	
		//printf("%s\n", it->strName.c_str());
		QCheckListItem *item = new QCheckListItem(listPackages, it->strName, QCheckListItem::CheckBox);
		item->setText(1, it->strVersion);
		item->setText(2, it->strRelease);
		item->setText(3, it->strArch);
		if(it->bBlacklisted == true)
		{				
			item->setText(4, "Blacklisted");
			item->setEnabled(false);
		}	
		else
		{
			item->setText(4, "");
			item->setEnabled(true);
			m_nUpdateCount++;
		}
		
		if(it->bIncompatible == true)
		{
			item->setText(5, COMPATIBLE_NO);
		}
		else
		{
			item->setText(5, COMPATIBLE_YES);
		}
		
		listPackages->insertItem(item);						
	}
}

//! @brief When setup is fished then apply new blacklist to vector, and refresh showd package list. 
void classGui::slotSetupBlackExit()
{
	m_rpmEngine->ApplyBlacklistToUpdate();
	ShowList();
	if(m_nUpdateCount > 0 || m_nMode == ERASE_MODE)
	{
		btnNext->setEnabled(true);
	}
	else
	{
		btnNext->setEnabled(false);
	}
		
}

//! @brief Lock wigets on window.
void classGui::LockWigets()
{
	frameWelcome->setEnabled(false);	
	frameSelectCommand->setEnabled(false);
	frameSelectPackage->setEnabled(false);	
	frameProgress->setEnabled(false);
	frameFinish->setEnabled(false);
	comboCommand->setEnabled(false);
	btnBack->setEnabled(false);
	btnNext->setEnabled(false);
	btnCancel->setEnabled(false);
}

//! @brief Unlock wigets on window.
void classGui::UnlockWigets()
{
	frameWelcome->setEnabled(true);	
	frameSelectCommand->setEnabled(true);
	frameSelectPackage->setEnabled(true);	
	frameProgress->setEnabled(true);
	frameFinish->setEnabled(true);
	btnBack->setEnabled(true);
	btnNext->setEnabled(true);
	btnCancel->setEnabled(true);
}

//! @brief Set select command. (update, install, erase)
void classGui::SetSelectCommand(int nIndex)
{
	comboCommand->setCurrentItem(nIndex);
	slotSelectCommand(nIndex);
}

//! @brief When user want to exit, This function is called.
void classGui::reject()
{	
	if (!btnCancel->isEnabled())
	{
		QMessageBox::information(this, staticTitle, tr("Updater is working, please wait a minute."));
		return;				
	}
	
	if (m_bDownload)
	{
		int result = QMessageBox::question(this, staticTitle, tr("Do you want to cancel the download and exit?"), TR_YES,TR_NO,0,1);
		if (result == 0)
		{
			Exit(2);
		}
		else
		{
			return;
		}
	}	
	QDialog::reject();
}

void classGui::ShowRemovedIncompatiblePackages()
{	
	classShowHideBox showHideBox;
	showHideBox.SetCaption(staticTitle);
	showHideBox.SetOkButtonText( TR_OK);
	showHideBox.SetMessage(tr("The following incompatible package(s) have been removed from your selection. \nIf you want to update these packages,  \nplease visit the TSN web page."));
	
	vector <string>::iterator it;		
	for(it=m_vectorRemovedIncompatiblePackages.begin();it!=m_vectorRemovedIncompatiblePackages.end();it++)
	{			
		showHideBox.AppendList(*it);								
	}
	showHideBox.Domodal();
}



///////////////////////////////////////////////////////////////////////////////////////////
// Dependant package list Dialog
classDepListDlg::classDepListDlg(QWidget *parent) : frmDepList(parent,0,FALSE,Qt::WStyle_Customize | Qt::WStyle_NormalBorder)
{
	this->setIcon(QPixmap(DEFAULT_ICON));
	this->setPaletteBackgroundColor(QColor(BG_COLOR));
	this->setPaletteForegroundColor(QColor(FG_COLOR));
	
	btnClose->setPaletteBackgroundColor(QColor(BG_COLOR));
	btnClose->setPaletteForegroundColor(QColor(FG_COLOR));
	
    connect(btnClose, SIGNAL(clicked()), this, SLOT(ClickedClose()));    
    edtDepList->clear();        
    // Font
    //QFont oldFont = font();    
    //oldFont.setFamily(g_font);
    //oldFont.setPointSize(g_font_size);
    //setFont(oldFont, true);
    this->setCaption(staticTitle);    
}
classDepListDlg::~classDepListDlg()
{
	
}


///////////////////////////////////////////////////////////////////////////////////////////
// 
void classDepListDlg::AddMessage(QString strMessage)
{      
   edtDepList->append(strMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 
void classDepListDlg::ClearMessage()
{      
   edtDepList->clear();
} 

///////////////////////////////////////////////////////////////////////////////////////////
// Click close button 
void classDepListDlg::ClickedClose()
{	
	close();       
}

 
///////////////////////////////////////////////////////////////////////////////////////////
// Show depend dialog
int classDepListDlg::Domodal()
{
    return exec();
}


