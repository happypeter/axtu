/*!
@file classNetwork.cpp
@brief class source of Network
*/
#include "classNetwork.h"
#include "classDownloader.h"

#define	USERCANCEL	100

//! total count of download files
static int nToDoCount;
//! count of downloaded files
static int nDoneCount;

commonCallBackFunc g_callBackGetHeaders;
commonCallBackFunc g_callBackGetPackages;

/*!
@brief Callback Function for download a file

@param p - rate value to downloading a file
@param ds - There are status when a file is downloaded.
        1. Real Download action.
        2. A file is checked with server's file.
@param strFileName - A downloaded Target File name
*/
void DownloaderCallback(int p,DOWNLOAD_STATE ds,string strFileName)
{
	string strDownloadLabel;
	int nTotalPercent=0;
	int nCurrentPercent=p;
	int length=strFileName.length();

	if(!strFileName.compare(length-3,3,(string)"hdr",0,3)){
		// When Hdr files are downloaded,
		// Set the label of ProgressBar according to DownloadState.
		if(ds == DS_DOWN){
		        strDownloadLabel="Downloading Header files....";
		}else if(ds == DS_CHECK){
		        strDownloadLabel="Verifying Header files integrity....";
		}else{
		        strDownloadLabel="Unknown state";
		}

		nTotalPercent=int((float)(nCurrentPercent+100*nDoneCount)/(float)nToDoCount);		
		if(g_callBackGetHeaders)g_callBackGetHeaders(nTotalPercent,nCurrentPercent,strDownloadLabel.c_str(),strFileName.c_str());
	}else if(!strFileName.compare(length-3,3,(string)"rpm",0,3)){
		// When Pkg files are downloaded,
		// Set the label of ProgressBar according to DownloadState.
		if(ds == DS_DOWN){
		        strDownloadLabel="Downloading packages....";
		}else if(ds == DS_CHECK){
		        strDownloadLabel="Verifying package integrity....";
		}else{
		        strDownloadLabel="Unknown state";
		}

		nTotalPercent=int((float)(nCurrentPercent+100*nDoneCount)/(float)nToDoCount);
		if(g_callBackGetPackages)g_callBackGetPackages(nTotalPercent,nCurrentPercent,strDownloadLabel.c_str(),strFileName.c_str());
	}
}

//! A constructor
classNetwork::classNetwork(EXECUTE_MODE eMode)
{
	m_ConfCtl=new classConfCtl();

	m_ConfCtl->ConfigCheck();

	m_clsAuth = new classAuthen(eMode);
	m_clsDown = new classDownloader();
	m_clsConfParser = new classConfigParser();
	m_Logger = new classLogger();
	
	// directory setting conf file
	ReadRepoInfo(CONFIG_FILE);
	
	// Set the callback function
	SetDownloaderCallBack(DownloaderCallback);
	
	m_nDownloadedIndex=0;
}

//! A destructor
classNetwork::~classNetwork()
{
	delete m_ConfCtl;
	delete m_clsAuth;
	delete m_clsDown;
	delete m_clsConfParser;
	delete m_Logger;
}

void classNetwork::SetDontUseUI(void)
{
	m_clsAuth->SetDontUseUI();
}

void classNetwork::SetDontGetTk(void)
{
	m_clsAuth->SetDontGetTk();
}

/*! 
@brief Check routine the authentication

This method read configure file.
and if authentication flag is "true",
it request the temporary key. and then get the temporary key.
@return bool - Authentication Success? or Not?
*/
int classNetwork::CheckAuthen(bool bAct)
{	
	int nRet=FAIL_AUTH;
	bool bRet=false;
	//Default value of authen is true. 

		
	string strAuthen = m_clsConfParser->GetOption("main", "authen");
	if(strAuthen == "true")
	{
		// Checking that AK is exist or not
		if(!bAct){
			bRet=m_clsAuth->GetAK(m_strUserId);
			if(!bRet){
				if(!access(RELAY_CONFIG_FILE,F_OK)){
					nRet=FAIL_AUTH;
				}else{
					return FAIL_AUTH;
				}				
			}else{
				nRet=FAIL_AUTH;
			}
		}
				
		nRet=m_clsAuth->CheckAuthen();
		
		if(nRet == OK_GET_TK){
                        if(m_clsAuth->GetAK(m_strUserId) && (m_clsAuth->GetDontGetTk()?true:m_clsAuth->GetTK(m_strUserPass))){
				m_ConfCtl->ConfigCheck();
                                nRet=SUCC_AUTH;
                        }else{
                                nRet=FAIL_AUTH;
                        }
		}else if(nRet < 0){
			nRet = CANNOT_EXE_AUTH_PROG;
		}
	}else{
		nRet=SUCC_AUTH;
	}

	return nRet;
}

//! configuring for Download
bool classNetwork::SetDownloadConfig(void)
{	
	if(ReadRepoInfo(CONFIG_FILE) == false)
	{
		return false;
	}
	// download setting
	m_clsDown->setMaxRequest(CountOfMaxRequest);
	m_clsDown->setSilence();
	m_clsDown->setUser(m_strUserId);
  	m_clsDown->setPass(m_strUserPass);
  	return true;
}

/*! 
@brief Read the Repository information

This method find out a cache directory path from configure file.
And it make a local repository path using cache path.
A cache directory is place to save.
@param strConfigFilePath - configure file's path
*/
bool classNetwork::ReadRepoInfo(string strConfigFilePath)
{			
	vector<string> vectorSections;
	vector<string>::iterator it;
	string strTemp;
	string strCacheDir;
	string strPath;
	
	m_vectorRepoInfo.clear();
	
	if(m_clsConfParser->Read(strConfigFilePath) == false)
	{
		return false;
	}
	vectorSections = m_clsConfParser->GetSections();
	
	strCacheDir = m_clsConfParser->GetOption("main", "cachedir");
	strCacheDir = m_clsConfParser->StripRString(strCacheDir, '/');
	if (strCacheDir.empty() == false)
	{
		strPath = strCacheDir;
	}	
	
	for(it=vectorSections.begin();it!=vectorSections.end();it++)
	{			
		if ( *it == "main" || *it == "selfupdate_list")
		{
			continue;
		}
		
		structRepoInfo repoInfo;
			
		repoInfo.strName=*it;
		
		repoInfo.strUrl = m_clsConfParser->GetOption(*it, "baseurl");
		repoInfo.strLocalHeaderDir = strPath + "/" + *it;
		repoInfo.strLocalHeadersDir = strPath + "/" + *it + "/headers";
	  	repoInfo.strLocalpkgsDir = strPath + "/" + *it + "/packages";
	
		m_vectorRepoInfo.push_back(repoInfo);
	}	
	return true;
}

/*!
@brief getting a "header.info"

@return int - Getting a "header.info" is Ok? or Not?
*/
int classNetwork::GetHeader()
{
	uerr_t err_value;
	vector<structRepoInfo>::iterator it;
	string url;
	char *szMoveCmd;
	
	for(it=m_vectorRepoInfo.begin();it!=m_vectorRepoInfo.end();it++)
	{
		m_clsDown->setTargetDir(it->strLocalHeaderDir.c_str());
		
		url = m_clsConfParser->StripRString(it->strUrl, '/');
                url=url.substr(0,url.rfind("/"));  // remove RPMS
		url.append(".header.info");
		
		m_clsDown->setTimestamping();
		m_clsDown->setUrl(url);
		err_value=m_clsDown->getFile();
		
		
		if(err_value != RETROK)
        	{
    			url=it->strUrl + "/headers/header.info";
			m_clsDown->setTimestamping();
      			m_clsDown->setUrl(url);
      			err_value=m_clsDown->getFile();

      			if(err_value != RETROK){
				m_Logger->WriteLog_char(DEBUG_LOG,"Cannot get header.info",url.c_str(),NULL);
	    			m_Logger->WriteLog_char(ERROR_LOG,"Cannot get ",url.c_str(), NULL);
      				break;
            		}
    		}else{
    			string strLocalPath=(it->strLocalHeaderDir)+(url.substr(url.rfind("/"),url.size()));
    			string strTargetPath=it->strLocalHeaderDir+((string)"/header.info");
    			if(!access(strLocalPath.c_str(),R_OK)){
				int nret;
				nret = rename(strLocalPath.c_str(), strTargetPath.c_str());
				if (nret != 0) {
					string strTmp;
					strTmp = "from ";
					strTmp += strLocalPath;
					strTmp += " to ";
					strTmp += strTargetPath;
	    				m_Logger->WriteLog_char(ERROR_LOG,"Cannot rename ", strTmp.c_str(), NULL);
				}
    			}
        	}
	}

	return ConvertNetworkErrorNumber(err_value);
}

/*!
@brief getting a "*.hdr" file

@param fileInfo - structure of the file information
@return bool - Getting a "*.hdr" is Ok? or Not?
*/
uerr_t classNetwork::GetFile(structFileInfo fileInfo)
{
	if(m_bStop)
		return  (uerr_t)USERCANCEL;
	string strUrlPath, strLocalPath;
	string strTemp;
	uerr_t err_value;
	bool ret;

	strUrlPath = fileInfo.strURLFullPath; 
	
	strUrlPath += "/headers";
	
	strUrlPath += "/";
	strUrlPath += fileInfo.strName;
	strUrlPath += "-";
		
  	strUrlPath += fileInfo.strEpoch;
	strUrlPath += "-";	
	
	strUrlPath += fileInfo.strVersion;
	strUrlPath += "-";
	strUrlPath += fileInfo.strRelease;
	strUrlPath += ".";
	strUrlPath += fileInfo.strArch;

	
	strUrlPath += ".hdr";
	m_clsDown->setTargetDir(fileInfo.strCachePath + "/headers");
	
//	m_clsDown->unsetTimestamping();
	m_clsDown->setTimestamping();
  	m_clsDown->setUrl(strUrlPath);
  
  	err_value=m_clsDown->getFile();
  	if(err_value == RETROK){
  		ret=true;
  	}else{
		strTemp=strUrlPath.substr( (strUrlPath.rfind("/")+1) , (strUrlPath.length()) );
  		m_vectorFailedFiles.push_back(strTemp);
  		ret=false;
		m_Logger->WriteLog_char(DEBUG_LOG,"Cannot get .hdr",strTemp.c_str(),NULL);
		m_Logger->WriteLog_char(ERROR_LOG,"Cannot get ",strTemp.c_str(), NULL);
   	}

	return err_value;
}

/*!
@brief getting a "*.rpm" file

@param pkgInfo - structure of the package information
@return bool - Getting a "*.rpm" is Ok? or Not?
*/
uerr_t classNetwork::GetFile(structPkgInfo pkgInfo)
{
	if(m_bStop)
		return  (uerr_t)USERCANCEL;
	uerr_t err_value;
	bool ret;
	string strTemp;

//  m_clsDown->unsetTimestamping();
	m_clsDown->setTimestamping();
	m_clsDown->setTargetDir(pkgInfo.strTargetPath);
	m_clsDown->setUrl(pkgInfo.strUrlFullPath);

	err_value=m_clsDown->getFile();

	if(err_value == RETROK){
        	ret=true;
  	}else{
		strTemp=(pkgInfo.strUrlFullPath).substr( ((pkgInfo.strUrlFullPath).rfind("/")+1) , ((pkgInfo.strUrlFullPath).length()) );
        	m_vectorFailedFiles.push_back(strTemp);
        	ret=false;
		m_Logger->WriteLog_char(DEBUG_LOG,"Cannot get .rpm",strTemp.c_str(),NULL);
		m_Logger->WriteLog_char(ERROR_LOG,"Cannot get ",strTemp.c_str(), NULL);
   	}

  	return err_value;
}

/*!
@brief getting some "*.rpm" in UpdateServer

@return bool - Getting some "*.rpm" is Ok? or Not?
*/
int classNetwork::GetPackages(void)
{
	m_bStop = false;
	vector<structPkgInfo>::iterator it;
	bool ret;
	uerr_t err_value=(uerr_t)31;

	nToDoCount=0;
	nDoneCount=0;

	m_vectorFailedFiles.clear();
	nToDoCount=m_vectorPkgList.size();
	it = m_vectorPkgList.begin();
	for(int i=0;i<m_nDownloadedIndex;i++){
		it++;
		nDoneCount++;
	}
	
	for(;it!=m_vectorPkgList.end();it++){		  
  		err_value=GetFile(*it);
    		if(err_value == RETROK){
    			nDoneCount++;
    			m_nDownloadedIndex++;
        	}else{
        		goto ret;
        	}

    		if(nDoneCount == nToDoCount)
        	{
    			if(g_callBackGetPackages)g_callBackGetPackages(100,100,"Downloading packages....","");
        	}
	}
ret:
	return ConvertNetworkErrorNumber(err_value);
}


//! Clear all saved information of packages
void classNetwork::ClearPackages(void)
{
	m_vectorPkgList.clear();
	m_nDownloadedIndex=0;
}

/*!
@brief Check download stop button is pressed or not.
*/
bool classNetwork::IsStopDownload()
{
        return m_bStop;
}


/*!
@brief Stop to down packages.
*/
void classNetwork::StopDownload()
{
	m_bStop = true;
}

/*!
@brief Add a Url to download

Add a url into inner PkgList
@param strRemoteUrl - A Url of string type
*/
void classNetwork::AddPackage(string strLocalPath, string strRemoteUrl)
{
	string strTemp;
	strTemp = strLocalPath.substr(0, strLocalPath.rfind("/"));
	structPkgInfo tempPkgInfo;
	tempPkgInfo.strTargetPath = strTemp;
	tempPkgInfo.strUrlFullPath = strRemoteUrl;	
	
	m_vectorPkgList.push_back(tempPkgInfo);
	
}

// check overlapping the inner PkgList and a argument
bool classNetwork::IsUniqueFromPkgList(string strUrl)
{
	vector<structPkgInfo>::iterator it;
	bool ret=true;

  	for(it=m_vectorPkgList.begin();it!=m_vectorPkgList.end();it++)
    	{
  		if(strUrl.compare(it->strUrlFullPath) == 0){
  			ret=false;
    			break;
		}
	}
  	return ret;
}


/*!
@brief getting some "*.hdr" in UpdateServer

@param vectorFileInfo - File information of a vector type
@param nDone - Count of files already downloaded
@param nToDo - Count of files to download
@return bool - getting some "*.hdr" is Ok? or Not?
*/
int classNetwork::GetHeaders(vector <structFileInfo> vectorFileInfo,int nDone,int nToDo, bool bSelf)
{
	m_bStop = false;
	vector<structFileInfo>::iterator it;
	bool ret;
	uerr_t err_value=(uerr_t)31;

	nDoneCount=nDone;	
	
	if(nToDo == 0){	
		nToDoCount=vectorFileInfo.size();
		m_vectorFailedFiles.clear();
	}
	
	int nPercent;	
	for(it=vectorFileInfo.begin();it!=vectorFileInfo.end();it++)
	{
		if (bSelf == true && it->bSelfUpdate == false){
                        continue;
                }

		err_value=GetFile(*it);
		if(err_value == RETROK){
			nDoneCount++;
                	nPercent = ((nDoneCount*100)/nToDoCount);
			string strTemp;
			strTemp = it->strName;
        		strTemp += "-";

        		strTemp += it->strEpoch;
        		strTemp += "-";

        		strTemp += it->strVersion;
        		strTemp += "-";
        		strTemp += it->strRelease;
        		strTemp += ".";
        		strTemp += it->strArch;
        		strTemp += ".hdr";
		}else{
			goto ret;
		}
			
		if(nDoneCount == nToDoCount)
		{
			if(g_callBackGetHeaders)g_callBackGetHeaders(100,100,"Downloading Header files....","");
		}
	}
ret:
	return ConvertNetworkErrorNumber(err_value);
}

/*!
@brief getting some "*.hdr" in UpdateServer

@param vectorFileInfo1 - File information of a vector type
@param vectorFileInfo2 - File information of a vector type
@return bool - getting some "*.hdr" is Ok? or Not?
*/
int classNetwork::GetHeaders(vector <structFileInfo> vectorFileInfo1,vector <structFileInfo> vectorFileInfo2, bool bSelf)
{
	int nRet;
	nToDoCount=0;
	nDoneCount=0;
	
	nToDoCount = vectorFileInfo1.size() + vectorFileInfo2.size();
	m_vectorFailedFiles.clear();
	
	nRet=GetHeaders(vectorFileInfo1,nDoneCount,nToDoCount, bSelf);
	if(nRet != NETWORK_RETOK){
		return nRet;
	}
	nRet=GetHeaders(vectorFileInfo2,nDoneCount,nToDoCount, bSelf);
	if(nRet != NETWORK_RETOK){
		return nRet;
	}else{
		return nRet;
	}
}

/*!
@brief the CallBack-function to get headers is set

@param callBackFunc
*/
void classNetwork::SetGetHeadersCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackGetHeaders = callBackFunc;
}

/*!
@brief the CallBack-function to get packages is set

@param callBackFunc
*/
void classNetwork::SetGetPackagesCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackGetPackages = callBackFunc;
}
/*
// getting some "*.rpm" in UpdateServer
bool classNetwork::GetPackages(vector <structFileInfo> vectorFileInfo)
{
	vector<structFileInfo>::iterator it;
	bool bIsHdrFile;
	bool ret;
	
	nToDoCount=0;
	nDoneCount=0;
	
	m_vectorFailedFiles.clear();
	nToDoCount=vectorFileInfo.size();	
	
	for(it=vectorFileInfo.begin();it!=vectorFileInfo.end();it++)
	{
	  bIsHdrFile=false;
		ret=GetFile(*it,bIsHdrFile);
		if(ret == true){
			nDoneCount++;
		}

		if(nDoneCount == nToDoCount)
		{
			g_callBackGetHeaders(100,100,"RPM File Download","Download has finished.");
		}
	}

	return ret;
}
*/



// getting a "*.hdr" file 
uerr_t classNetwork::GetHdrFile(string strUrl)
{	
	uerr_t err_value;
//	m_clsDown->unsetTimestamping();
	m_clsDown->setTimestamping();
  	m_clsDown->setUrl(strUrl);
  	err_value=m_clsDown->getFile();

	return err_value;
}

/*! 
@brief getting all "*.hdr" in UpdateServer

@return bool - getting all "*.hdr" is Ok? or Not?
*/
int classNetwork::GetHeaders()
{
	vector<structRepoInfo>::iterator it;
	char strBuf[MAX_STRING];
	string strTemp; 
	string strDownloadFileName;
	string strEpoch;
	string strUrl;
	int nResult=0;
	uerr_t err_value;
	
	
	string::size_type index1;
	string::size_type index2;
	string::size_type index3;
	
	int nType=0;
	for(it=m_vectorRepoInfo.begin();it!=m_vectorRepoInfo.end();it++)
	{
		ifstream fin;
		
		string strHeaderFileName;
		strHeaderFileName=it->strLocalHeaderDir + "/header.info";

		fin.open(strHeaderFileName.c_str());
		
		while (fin.getline(strBuf, sizeof(strBuf)) > 0 )
		{	
			strTemp = strBuf;
			
      			index1 = strTemp.find(":");
			index2 = strTemp.find("=");
			strEpoch = strTemp.substr(0, index1);
			strEpoch += "-";
			strDownloadFileName=strTemp.substr(index1+1,index2-index1-1);

			index3 = strDownloadFileName.find_last_of("-");
			strTemp = strDownloadFileName.substr(0,index3);
			index3 = strTemp.find_last_of("-");
			
			strDownloadFileName.insert(index3+1,strEpoch);
			m_clsDown->setTargetDir(it->strLocalHeadersDir.c_str());

			strDownloadFileName += ".hdr";
			strUrl=it->strUrl + "/headers/" + strDownloadFileName;
//			cout<<strDownloadFileName<<endl;
			
		  	err_value=GetHdrFile(strUrl);
			if(err_value != RETROK){
				break;
			}
		}
		fin.close();
		
		if(err_value != RETROK){
			break;
		}
		nType++;		
	}		

	return ConvertNetworkErrorNumber(err_value);
}

int classNetwork::ConvertNetworkErrorNumber(int nArg)
{
	
	int nRet=0;

	switch(nArg){
		case HOSTERR:
			nRet=NETWORK_ERR_CONNECT;
			break;
		case CONIMPOSSIBLE:
		case TRYLIMEXC:
			nRet=NETWORK_ERR_CONNECT;
			break;
		case URLERROR:
			nRet=NETWORK_ERR_WRONG_URL;
			break;
		case FWRITEERR:
			nRet=NETWORK_ERR_FWRITE;
			break;
		case RETROK:
			nRet=NETWORK_RETOK;
			break;
		case WRONGCODE:
			nRet=NETWORK_ERR_WRONG_URL;
			break;
		case AUTHFAILED:
			nRet=NETWORK_ERR_AUTH_FAIL;
			break;
		case USERCANCEL:
			nRet=NETWORK_ERR_USERCANCEL;
			break;
		default:
			nRet=NETWORK_ERR_UNKNOWN;
			break;
	}
	return nRet;
}

bool classNetwork::SetDoNotGetTkFlag(bool bFlag)
{
		m_bDoNotGetTk=bFlag;
}
