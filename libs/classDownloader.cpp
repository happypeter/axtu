//////////////////////////////////////////////////////////////////////////
// Copyright 2006 HAANSOFT, INC
// Url      : www.haansoft.com
// Author : Yusop Kim
// E-mail : yusop@haansoft.com
// Description : This file is class source of Downloader
//////////////////////////////////////////////////////////////////////////
/*!
@file classDownloader.cpp
@brief source file of download class(wget library's Upper Layer class)
*/
#include "classDownloader.h"

//! User Id(Repository key)'s maximum length
const int user_max_length = 32;
//! User Password(Repository key)'s maximum length
const int pass_max_length = 16;
//! DirecotryName(local download space)'s maximum length
const int dirpath_max_length = 512;

//! wget library's option structure
struct options opt;

extern SUM_SIZE_INT total_downloaded_bytes;
extern char *version_string;

extern struct cookie_jar *wget_cookie_jar;

static RETSIGTYPE redirect_output_signal PARAMS ((int));

const char *exec_name;

DownloaderCallBackFunc g_callBackDownloader;

/*!
@brief Callback Function for download a file

@param p - rate value to downloading a file
@param ds - There are status when a file is downloaded. 
	1. Real Download action.
	2. A file is checked with server's file.
*/
void GetFileCallBack(int p,DOWNLOAD_STATE ds)
{
	if(g_callBackDownloader)g_callBackDownloader((int)p,ds,strCurrentDownloadFileName);
}

//! A Constructor
classDownloader::classDownloader()
{
  	initialize();
	opt.user=(char*)calloc(user_max_length+1,sizeof(char));
	opt.passwd=(char*)calloc(pass_max_length+1,sizeof(char));
	opt.dir_prefix=(char*)calloc(dirpath_max_length+1,sizeof(char));
}

//! A Destructor
classDownloader::~classDownloader()
{
	free(opt.user);
	free(opt.passwd);
	free(opt.dir_prefix);
}

/*!
@brief register download callback function
*/
void SetDownloaderCallBack(DownloaderCallBackFunc callBackFunc)
{
	g_callBackDownloader = callBackFunc;
}

/*!
@brief A user ID is set.

To download the files from server protected by mod_auth_mysql.
@param strUser - user ID
*/
void classDownloader::setUser(string strUser)
{
	strncpy(opt.user,strUser.c_str(),user_max_length+1);
}

/*!
@brief A user Password is set.

To download the files from server protected by mod_auth_mysql.
@param strPass - user Password
*/
void classDownloader::setPass(string strPass)
{
	strncpy(opt.passwd,strPass.c_str(),pass_max_length+1);
}

/*!
@brief A Server Url to be downloaded is set.

@param strUrl - server Url
*/
void classDownloader::setUrl(string strUrl)
{
	m_strUrl=strUrl;
}

/*!
@brief A directory path that save a files is set.

@param strDirpath - A directory path to save
*/
void classDownloader::setTargetDir(string strDirpath)
{
	strncpy(opt.dir_prefix,strDirpath.c_str(),dirpath_max_length+1);
}

/*!
@brief On a timestamp option of wget library 

The timestamp option confirm that the downloaded files are proper.
If the option's stat is "On", confirm, but "Off", do not confirm.
*/
void classDownloader::setTimestamping()
{
	opt.timestamping=1;
  	opt.noclobber=0;	
}


/*!
@brief Off a timestamp option of wget library 

The timestamp option confirm that the downloaded files are proper.
If the option's stat is "On", confirm, but "Off", do not confirm.
*/
void classDownloader::unsetTimestamping()
{
	opt.timestamping=0;
	opt.noclobber=1;
}


/*!
@brief On a silence option of wget library

If silence option's stat is "On" do not print any output on console screen.
*/
void classDownloader::setSilence()
{
	opt.silence=1;
}

/*!
@brief Off a silence option of wget library

If silence option's stat is "On" do not print any output on console screen.
*/
void classDownloader::unsetSilence()
{
	opt.silence=0;
}

/*!
@brief On a Check-Certificate option of wget library

If the Check-Certificate option's stat is "On" do not Check Certificate.
*/
void classDownloader::setCheckCertificate(void)
{
        opt.check_cert=1;
}

/*!
@brief Off a Check-Certificate option of wget library

If the Check-Certificate option's stat is "On" do not Check Certificate.
*/
void classDownloader::unsetCheckCertificate(void)
{
        opt.check_cert=0;
}

/*!
@brief Set connect-timeout value
*/
void classDownloader::setConnectTimeOutValue(double dValue)
{
        opt.connect_timeout = dValue;
}

/*!
@brief Set read-timeout value
*/
void classDownloader::setReadTimeOutValue(double dValue)
{
        opt.read_timeout = dValue;
}

/*!
@brief Set dns-timeout value
*/
void classDownloader::setDnsTimeOutValue(double nValue)
{
        opt.dns_timeout = nValue;
}


/*!
@brief Get connect-timeout value
*/
double classDownloader::getConnectTimeOutValue()
{
        return opt.connect_timeout;
}

/*!
@brief Get read-timeout value
*/
double classDownloader::getReadTimeOutValue()
{
        return opt.read_timeout;
}

/*!
@brief Get dns-timeout value
*/
double classDownloader::getDnsTimeOutValue()
{
        return opt.dns_timeout;
}


/*
@brief The maximum number of requests to the server is set.

@param nMaxReq - The maximum number of requests to the server
*/
void classDownloader::setMaxRequest(int nMaxReq)
{
	opt.ntry = nMaxReq;
}

/*!
@brief Set arguments of "post" method in wget library
*/
void classDownloader::setPostData(const char *szPostData)
{
        setoptval("postdata",szPostData,"post-data");
}

/*!
@brief download a file.

@return uerr_t - the error value after downloading a file.
@see wget
*/
uerr_t classDownloader::getFile(bool bUseProgress)
{
	uerr_t err_value=RETROK;
	int index;

  	char *filename = NULL, *redirected_URL = NULL;
  	int dt;

  	if (opt.verbose == -1){
                opt.verbose = !opt.quiet;
	}

  	if (opt.verbose && opt.quiet){
  		cout<<"Can't be verbose and quiet at the same time."<<endl;
    		exit (1);
    	}

  	if (opt.timestamping && opt.noclobber){
    		cout<<"Can't timestamp and not clobber old files at the same time."<<endl;
    		exit (1);
    	}

  	if (opt.verbose){
    		set_progress_implementation (opt.progress_type);
	}

	if(bUseProgress){
		SetGetFileCallBack(GetFileCallBack);
	}else{
		SetGetFileCallBack(NULL);
	}
	
	pushFileName();
	
	err_value=retrieve_url(m_strUrl.c_str(), &filename, &redirected_URL, NULL, &dt);
  
  	return err_value;
}

/*!
@brief control the file name

The function extract Download File Name from "m_strUrl"
and push the value into "strCurrentDownloadFileName"
*/
void classDownloader::pushFileName()
{
	string::size_type index1;
		
	index1 = m_strUrl.rfind("/");
	
	strCurrentDownloadFileName=m_strUrl.substr(index1+1,m_strUrl.length());
}

