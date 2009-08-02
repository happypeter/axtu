///////////////////////////////////////////////////////////////////////////
// Copyright 2006 HAANSOFT, INC
// Url      : www.haansoft.com
// Author : Yusop Kim
// E-mail : yusop@haansoft.com
// Description : This file is class header of Downloader
///////////////////////////////////////////////////////////////////////////
/*!
@file classDownloader.h
@brief header file of download class(wget library's Upper Layer class)
*/
#ifndef __CLASSDOWNLOADER_H__
#define __CLASSDOWNLOADER_H__

#include <wget/config.h>

#include <ctype.h>

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/types.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_NLS
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif /* HAVE_LOCALE_H */
#endif /* HAVE_NLS */
#include <assert.h>

#include <errno.h>

#ifndef errno
//! Global Error Number variable 
extern int errno;
#endif

#include <hswgetlib.h>

using namespace std;

/*! 
@brief For CallBackFunction

The Callback function passed progress values
@param p - download progress percentage 
@param ds - State of downloaded files 
@param strFileName - current download file name
*/
typedef void (*DownloaderCallBackFunc)(int p,DOWNLOAD_STATE ds,string strFileName);

/*!
@brief Register the callback function
*/
void SetDownloaderCallBack(DownloaderCallBackFunc callBackFunc);

static string strCurrentDownloadFileName;

//! Repository information
struct structRepoInfo
{
	string strName;
	string strUrl;
	string strLocalHeaderDir;
	string strLocalHeadersDir;
	string strLocalpkgsDir;
};

/*!
@brief Class using wget library 

Wget is a network utility to retrieve files from the Web using http and ftp, etc.
We made a wget library Using wget sources.classDownloader is middle class between classNetwork and wget
@see classNetwork
@see wget
*/
class classDownloader
{
public:
	classDownloader(void);
	~classDownloader(void);

	void setUser(string user);
	void setPass(string pass);
	void setUrl(string url);
	void setTargetDir(string dirpath);
	uerr_t getFile(bool bUseProgress=true);
	void setTimestamping(void);
	void unsetTimestamping(void);
	void setSilence(void);
	void unsetSilence(void);
	void setCheckCertificate(void);
        void unsetCheckCertificate(void);
        void setPostData(const char *szPostData);
	void setMaxRequest(int nMaxReq);
	void setConnectTimeOutValue(double dValue);
	void setReadTimeOutValue(double dValue);
	void setDnsTimeOutValue(double dValue);
	double getConnectTimeOutValue();
	double getReadTimeOutValue();
	double getDnsTimeOutValue();



	
private:
/* The function extract Download File Name from "m_strUrl"
 *  and push the value into "strCurrentDownloadFileName"
 */
	void pushFileName();
		
	//! A url to download
 	string m_strUrl;
};

#endif
