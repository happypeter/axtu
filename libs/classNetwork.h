/*!
@file classNetwork.h
@brief class header of Network
*/
#ifndef CLASSNETWORK_H_
#define CLASSNETWORK_H_

#include <iostream>
#include <string>
#include "classAuthen.h"
#include "classDownloader.h"
#include "hsCommon.h"
#include "classConfigParser.h"
#include "classLogger.h"
#include "classConfigParser.h"
#include "classConfCtl.h"

//! Network class work successfully
#define NETWORK_RETOK		0
//! Network error - wrong url
#define NETWORK_ERR_WRONG_URL	-1
//! Network error - cannot connect 
#define NETWORK_ERR_CONNECT	-2
//! Network error - authentication failed
#define NETWORK_ERR_AUTH_FAIL	-3
//! Network error - occured unknown error
#define NETWORK_ERR_UNKNOWN	-4

#define NETWORK_ERR_USERCANCEL	-5
//! Network error - cannot write to filesystem
#define NETWORK_ERR_FWRITE 	-6

#define CountOfMaxRequest	20

using namespace std;
              
#define CANNOT_EXE_AUTH_PROG	-1
#define FAIL_AUTH		0
#define SUCC_AUTH		1

/*!
@brief For CallBackFunction

The Callback function passed progress values 
@param p1 - Total progress percentage
@param p2 - Current progress percentage
@param msg1 - Total progress message
@param msg2 - Current progress message
*/
typedef void (*commonCallBackFunc)(int p1, int p2, const char * msg1, const char * msg2);


//! structure of package information
struct structPkgInfo
{
        string strTargetPath;
        string strUrlFullPath;
};

/*!
@brief Network related Class

@see classDownload
@see wget
*/
class classNetwork
{
public:
	classNetwork(EXECUTE_MODE eMode=TEXT);
	virtual ~classNetwork();
	int CheckAuthen(bool bAct=true);
	bool SetDownloadConfig(void);
	int GetHeader(void);
	int GetHeaders(void);
	int GetHeaders(vector <structFileInfo> vectorFileInfo,int nDone=0,int nToDo=0, bool bSelf=false);
	int GetHeaders(vector <structFileInfo> vectorFileInfo1,vector <structFileInfo> vectorFileInfo2, bool bSelf=false);	
	void AddPackage(string strLocalPath, string strRemoteUrl);
	int GetPackages(void);
	void ClearPackages(void);
	void StopDownload();
	bool IsStopDownload();
	void SetDontUseUI(void);
    void SetDontGetTk(void);
	bool SetDoNotGetTkFlag(bool bFlag);
	
	
// Inner Method
	void SetGetHeadersCallBack( commonCallBackFunc callBackFunc );
	void SetGetPackagesCallBack( commonCallBackFunc callBackFunc );
	
private:
  	uerr_t GetHdrFile(string strUrl);
	bool ReadRepoInfo(string strConfigFilePath);
	uerr_t GetFile(structFileInfo fileInfo);
	uerr_t GetFile(structPkgInfo pkgInfo);
	bool IsUniqueFromPkgList(string strUrl);
	int ConvertNetworkErrorNumber(int nArg);

	classAuthen *m_clsAuth;
	classDownloader *m_clsDown;
	classConfigParser *m_clsConfParser;
	vector <structRepoInfo> m_vectorRepoInfo;
	vector <string> m_vectorFailedFiles;
	string m_strUserId,m_strUserPass;
	vector <structPkgInfo> m_vectorPkgList;

	//! Instance of classLogger
	classLogger *m_Logger;
	
	classConfCtl *m_ConfCtl;
	bool m_bStop;
    bool m_bDoNotGetTk;
    int m_nDownloadedIndex;
};

#endif
