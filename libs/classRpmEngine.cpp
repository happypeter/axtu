/*!
@file classRpmEngine.cpp
@brief Class source file for rpm related api
*/
#include <sstream> //for int to string conversion
#include "classRpmEngine.h"
#include <fstream>
#include "trace.h"
#include "classLogger.h"
/// Exterm vaiable for checking smp kernel
#ifdef __cplusplus
	extern "C" {
#endif

#include "smp.h"

#ifdef __cplusplus
	}
#endif

#define MAX_LOOP 20
#define NULL_FILE "/dev/null"
commonCallBackFunc g_callBackReadHeaders;
commonCallBackFunc g_callBackRun;
commonCallBackFunc g_callBackCheck;
commonCallBackFunc g_callBackReadRemoteHeaderInfo;
commonCallBackFunc g_callBackReadLocalHeaderInfo;
commonCallBackFunc g_callBackCreateUpdateInstallList;

int g_nRemoveCount;
int g_nRemoveSelectedPackages;
int oldRpmcliProgressCurrent;
int g_nTotalRemoveCountByUpdate;
bool g_bNoSpace;
static classLogger * m_Logger;

string g_strLoopBuffer[MAX_LOOP];
int g_nLoop;


/*!
@brief Constructor.

classRpmEngine Constructor.
 
*/
classRpmEngine::classRpmEngine()
{
	InitTs();
}

/*!
@brief Destructor.

classRpmEngine Destructor.
 
*/
classRpmEngine::~classRpmEngine()
{
	UnInitTs();
} 


/*!
 * @brief You can ignore self update function
 * 
 * This function is usefull when user doesn't want to update by self.
 * ex) cl updater
 */ 
void classRpmEngine::SetIgnoreSelfUpdate(bool bFlag)
{
	m_bIgnoreSelfUpdate = bFlag;
}

/*
@brief  make the name available for gui/axtu/classGui

FIXME: only one obsoletee name is returned, while there maybe more

*/

char * classRpmEngine::getOB()
{
	return OBname[0];
}


/*!
@brief Initialize classRpmEngine class.

Initialize classRpmEngine class.

*/
int classRpmEngine::InitTs()
{	
	m_nIncompatible = 0;
	m_vectorIncompatibleUpdatePackages.clear();
	m_bKernelInstall = false;
	m_vectorDepAddedFiles.clear();
	m_bIgnoreSelfUpdate = false;
	// Get smp available.
	int nSmp = detectSMP();
	if(nSmp)
	{
		m_bSmp = true;
	}
	else
	{
		m_bSmp = false;
	}

	m_Logger = new classLogger();

	int rc;
	m_bSelfUpdate = false;
	m_nCommand = 1;
	ReadCacheDirInfo();

	if (rpmReadConfigFiles(NULL, NULL) != 0) {  // This function makes memory leak (396 byte)
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open RPM configuration file", NULL);
		return -1;
	}

	m_rpmTs = rpmtsCreate();

	const char *rootDir = "/";
	rpmtsSetRootDir(m_rpmTs, rootDir);

	rc = rpmtsOpenDB(m_rpmTs, O_RDONLY);	
	
	if (rc)
	{
		rpmtsFree(m_rpmTs);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open RPM database", NULL);
		return -1;
	}
	
	signal(SIGHUP, sig_hup);
		
	m_vectorAddedFile.reserve(1000);

	m_vectorInstallList.reserve(1000);
	m_vectorUpdateList.reserve(1000);	


	vector<string> vectorBlackUpdate;	
	m_configBlacklistUpdate.Read(BLACKLIST_FILE);
	m_vectorBlackUpdate.clear();
	m_vectorBlackUpdate = m_configBlacklistUpdate.GetOptions("blacklist-update");

	return 0;
}


/*!
@brief Uninitialize classRpmEngine class.

Uninitialize classRpmEngine class.
 
*/
int classRpmEngine::UnInitTs()
{	
	rpmtsFree(m_rpmTs);	
	rpmFreeMacros(NULL);
	rpmFreeMacros(rpmCLIMacroContext);
	rpmFreeRpmrc();

	freeFilesystems();
	urlFreeCache();
	rpmlogClose();

	// Free memory for header info of headers.
	vector<structRPMInfo*>::iterator it;
	for(it=m_vectorRPMInfo.begin();it!=m_vectorRPMInfo.end();it++)
	{
		free((*it)->name);
		free((*it)->version);
		free((*it)->release);
		free((*it)->arch);
		free((*it)->group);

		free((*it)->summary);
		free((*it)->shortDesp);
		for(int i=0;(*it)->requireName[i] !=NULL;i++)
			free((*it)->requireName[i]);
		for(int i=0;(*it)->requireVersion[i] !=NULL;i++)
			free((*it)->requireVersion[i]);
		for(int i=0;(*it)->provideName[i] !=NULL;i++)
			free((*it)->provideName[i]);
		for(int i=0;(*it)->provideVersion[i] !=NULL;i++)
			free((*it)->provideVersion[i]);
		for(int i=0;(*it)->obsoleteName[i] !=NULL;i++)
			free((*it)->obsoleteName[i]);
		for(int i=0;(*it)->obsoleteVersion[i] !=NULL;i++)
			free((*it)->obsoleteVersion[i]);

		free((*it)->requireName);
		free((*it)->requireVersion);
		free((*it)->provideName);
		free((*it)->provideVersion);
		free((*it)->obsoleteName);
		free((*it)->obsoleteVersion);

		headerFree((*it)->h);
		free((*it)->containFiles);
		free(*it);

	}
	// Free memory

	ClearAddedFiles();

	delete m_Logger;

  for (vector<vector<string>*>::iterator p = m_vectorCsvData.begin( ); p != m_vectorCsvData.end( ); ++p) {
    delete *p;
  }

	return 0;

}

/*!
@brief Set command install(update) or remove

Set command install(update) or remove

@param nType   0 : remove  1: update or install
 
*/
void classRpmEngine::SetCommand(int nType)
{
	m_nCommand = nType;
}

/*!
@brief Get command

Get command type , install(update) or remove

@return The command type (0 : remove  1: update or install)
 
*/
int  classRpmEngine::GetCommand()
{
	return m_nCommand;
}


/*!
@brief Read cache directory info.

Read cache directory info from config file(CONFIG_FILE).
 
*/
bool classRpmEngine::ReadCacheDirInfo(string strConfigFilePath)
{
	vector<string> vectorSections;
	vector<string>::iterator it;
	string strTemp;
	string strCacheDir;
	string strPath;

	m_vectorCacheDirInfo.clear();

	if (!m_configEnv.Read(strConfigFilePath))
	{
		return false;
	}
	vectorSections = m_configEnv.GetSections();
	strCacheDir = m_configEnv.GetOption("main", "cachedir");
	strCacheDir = m_configEnv.StripRString(strCacheDir, '/');
	if (strCacheDir.empty() == false)
	{
		strPath = strCacheDir;
	}
	for(it=vectorSections.begin();it!=vectorSections.end();it++)
	{
		if ( *it == "main")
		{
			continue;
		}
		strTemp = strPath + "/" + *it;
		structCacheDirInfo cacheDirInfo;
		cacheDirInfo.strHeaderInfoFile = strTemp + "/header.info";
		cacheDirInfo.strCacheDir = strTemp;
		cacheDirInfo.strURL = m_configEnv.GetOption(*it,"baseurl");
		cacheDirInfo.strName = *it;

		m_vectorCacheDirInfo.push_back(cacheDirInfo);
	}
	return true;
}


/*!
@brief Read remote header info file. (xxx.header.info)

Read remote header info file(xxx.header.info), 
and write data to m_setRemoteHeaderInfo.
 
*/
int classRpmEngine::ReadRemoteHeaderInfo()
{

	if(g_callBackReadRemoteHeaderInfo)
		g_callBackReadRemoteHeaderInfo(1,1, "Reading remote header info....", "");
	int nCount=0;
	int nCount2=0;
	int nTotal=0;
	int nPercent=0;
	m_setRemoteHeaderInfo.clear();	
	vector<structCacheDirInfo>::iterator it;
	char strBuf[MAX_STRING];
	string strTemp;

	string::size_type index1;
	string::size_type index2;

	int nType=0;

	nTotal = 2000;
	for(it=m_vectorCacheDirInfo.begin();it!=m_vectorCacheDirInfo.end();it++, nCount++)
	{
		if(g_callBackReadRemoteHeaderInfo)
			g_callBackReadRemoteHeaderInfo(1,1, "Reading remote header info....", it->strHeaderInfoFile.c_str());

		ifstream fin;
		//fin.exceptions(ifstream::eofbit |  ifstream::failbit | ifstream::badbit);		
		fin.exceptions(ifstream::badbit);
		
		if( access(it->strHeaderInfoFile.c_str(), F_OK) == 0)
		{
			fin.open(it->strHeaderInfoFile.c_str());
			if(!fin.good())
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "File open failed : ", it->strHeaderInfoFile.c_str() ,NULL);
				return 1;
			}
		}
		else
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "File access failed : ", it->strHeaderInfoFile.c_str() ,NULL);
			return 1;
		}
		
		try
		{
			while (fin.getline(strBuf, sizeof(strBuf)) > 0 )
			{				
				strTemp = strBuf;
	
				index1 = strTemp.find(":");
				index2 = strTemp.find("=");
	
				structHeaderInfo headerInfo;
	
				headerInfo.nType = nType;
				headerInfo.strEpoch.assign(strTemp, 0, index1);
				headerInfo.strNVRA.assign(strTemp, index1 + 1 , index2-(index1 + 1));
				if (it->strName == "self")
				{
					headerInfo.bSelfUpdate = true;
				}
				else
				{
					headerInfo.bSelfUpdate = false;
				}
	
	
				if(!(nCount2 % 100))
				{
					nPercent = (int)(nCount2 * 100./nTotal);
					if(g_callBackReadRemoteHeaderInfo)
						g_callBackReadRemoteHeaderInfo(nPercent,nPercent, "Reading remote header info....", it->strHeaderInfoFile.c_str());
				}
	
				if (m_setRemoteHeaderInfo.insert(headerInfo).second)
				{
					/*
					//  Check to insert item.
					if(headerInfo.strNVRA.find("mysql") != string::npos)
					{
						printf("test : %s\n", headerInfo.strNVRA.c_str());
					}
					*/
					
				}
				nCount2++;
			}
		}
		catch (ifstream::failure e)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "File read failed : ", it->strHeaderInfoFile.c_str() ,NULL);
			return 1;
		}
		fin.close();

		nType++;

	}

	if(g_callBackReadRemoteHeaderInfo)
		g_callBackReadRemoteHeaderInfo(1,1, "Finished reading remote header info....", "");
	
	return 0;
}


/*!
@brief Read local rpm db.

Read local rpm db, write to m_setLocalHeaderInfo,
and write data to m_setLocalHeaderInfo. 
*/
////////////////////////////////////////////////////////////////
// Interface
// Read local rpm db, write to m_setLocalHeaderInfo
int classRpmEngine::ReadLocalHeaderInfo()
{
	if(g_callBackReadLocalHeaderInfo)
		g_callBackReadLocalHeaderInfo(1,1, "Start read local header info...", "");
	int nTotal=0;
	int nCount=0;
	int nPercent=0;
	m_setLocalHeaderInfo.clear();

	rpmdbMatchIterator mi;
	const char * np=NULL;
	const char * ep=NULL;
	const char * ap=NULL;
	const char * vp=NULL;
	const char * rp=NULL;
	Header hdr;
	int rc;
	classBlockSignal blocksignal(SIGHUP); //Util this class is destroyed, SIGHUP will be destroy. 
	mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_PACKAGES, NULL, 0);



	//nTotal = rpmdbGetIteratorCount(mi);
	nTotal = 1000;
	nCount = 0;
	int_32 * ep_int;
	int_32 i_1=0;
	while ((hdr = rpmdbNextIterator(mi)) != NULL)
	{
		int recOffset = rpmdbGetIteratorOffset(mi);
		if (recOffset)
		{
			rc = headerNEVRA(hdr,&np,&ep, &vp, &rp, &ap);
			if (rc)
			{
				rpmdbFreeIterator(mi);
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to query RPM database", NULL);
				return -1;

			}

		}
		char strTemp[MAX_STRING];
		structHeaderInfo headerInfo;
		string strNVRA, strName;	
		headerInfo.nType = -1;  
		if (!(headerGetEntry(hdr, RPMTAG_EPOCH, NULL, (void **) &ep_int, NULL)))
		{
 			ep_int = NULL; //if read failed
		}
		if(ep_int == NULL)
                {
			ep_int=&i_1;
                }
	        stringstream out;
		out<<*ep_int;
		headerInfo.strEpoch =out.str();
		snprintf(strTemp, sizeof(strTemp), "%s-%s-%s.%s", np, vp, rp, ap);
		headerInfo.strNVRA = strTemp;
		m_setLocalHeaderInfo.insert(headerInfo);

		if(!(nCount % 100))
		{
			nPercent = (int)( nCount* 100./nTotal );			
			if(g_callBackReadLocalHeaderInfo)
				g_callBackReadLocalHeaderInfo(nPercent,nPercent, "Reading local header info....", strTemp);

		}
		//headerFree(hdr);
		nCount++;
	}

  //
  // if not found "axtu-incmp-list.noarch.rpm" package, add the dummy entry to the list.
  //
  bool isFound = false;
	string  strNameL, strVerL, strRelL, strArchL;
	set <structHeaderInfo, DereferenceLess>::iterator it;
	for(it = m_setLocalHeaderInfo.begin(); it != m_setLocalHeaderInfo.end(); it++){
    stripNVRA(it->strNVRA, &strNameL, &strVerL, &strRelL, &strArchL);
    if (strNameL == INCMP_RPM_NAME) {
      isFound = true;
      break;
    }
	}

  if (isFound == false) {
    structHeaderInfo headerInfo;
		headerInfo.nType = -1;
		headerInfo.strEpoch = "";
		headerInfo.strNVRA = INCMP_NVRA_DUMMY;
		m_setLocalHeaderInfo.insert(headerInfo);
    nPercent = (int)( nCount* 100./nTotal );
    nCount++;
#ifdef DEBUG
    printf("axtu-incmp-list is not installed yet\n");
#endif
  }

	rpmdbFreeIterator(mi);

	if(g_callBackReadLocalHeaderInfo)
		g_callBackReadLocalHeaderInfo(100,100, "Finish read local header info...", "");

	if(nCount == 0)
	{
		return 2;
	}
	else	
	{
		return 0;
	}
}


bool classRpmEngine::AddUpdateInstallList(structFileInfo *fileInfo,string strName,string strVer,string strRel,string strEpoch,string strArch,int nType,bool bSelfUpdate,bool bUpdate)
{	
	if(bUpdate)
		m_nUpdateAvailableCount++;
	fileInfo->strName = strName;
	fileInfo->strVersion = strVer;
	fileInfo->strRelease = strRel;
	fileInfo->strEpoch = strEpoch;
	fileInfo->strArch = strArch;
	fileInfo->strCachePath = m_vectorCacheDirInfo.at(nType).strCacheDir;
	fileInfo->strURLFullPath = m_vectorCacheDirInfo.at(nType).strURL;
	fileInfo->nType = nType;
	fileInfo->bBlacklisted = false;
	fileInfo->bSelfUpdate = bSelfUpdate;

	if(m_bSelfUpdate == false)
	{
		if(bSelfUpdate == true && m_bIgnoreSelfUpdate == false)
		{
			m_bSelfUpdate = true;
			m_vectorUpdateList.clear();
			m_vectorInstallList.clear();
		}

		// Self update case
		if(m_bSelfUpdate)
		{
			AddUpdateList(fileInfo);
		}
		else
		{
			if(bUpdate){
				AddUpdateList(fileInfo);
			}
			else
			{
				AddInstallList(fileInfo);
			}
		}
	}
	else
	{
		if(fileInfo->bSelfUpdate == true)
		{
			AddUpdateList(fileInfo);
		}
	}
}

bool classRpmEngine::CheckLoop(string strArg)
{
	int nLoop=0;
	bool bLoop;

	// Fill the strLoopBuffer Array
	if(g_nLoop >= MAX_LOOP){
		g_nLoop = 0;
	}

	if(strArg.length() != 0){
                g_strLoopBuffer[g_nLoop++].assign(strArg);
        }

	// Loop Check
	for(nLoop=0;nLoop < MAX_LOOP-1;nLoop++){
		if(g_strLoopBuffer[nLoop] == g_strLoopBuffer[nLoop+1]){
			if((g_strLoopBuffer[nLoop].length() != 0) && \
				(g_strLoopBuffer[nLoop+1].length() != 0)){
				continue;
			}else{
				bLoop=false;
				break;
			}
		}else{
			bLoop=false;
			break;
		}
	}

	if(nLoop > MAX_LOOP-3){
		bLoop=true;
	}

	return bLoop;
}


/*!
@brief Create update(install) package list. 

Creaet update list and install list by header.info and local rpm db.
 
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// Create update package list, install package list
bool classRpmEngine::CreateUpdateInstallList()
{

	m_nIncompatible = 0;
	m_vectorIncompatibleUpdatePackages.clear();
  bool rtv = false;
#ifdef DEBUG
	/////////////////
	// test code start

	set <structHeaderInfo, DereferenceLess>::iterator itTest1;
	set <structHeaderInfo, DereferenceLess>::iterator itTest2;
	 
	char szMessage[MAX_STRING];				
	for(itTest1 = m_setRemoteHeaderInfo.begin();itTest1 != m_setRemoteHeaderInfo.end();itTest1++){
		//printf("Remote : %s\n", itTest1->strNVRA.c_str());
		snprintf(szMessage, sizeof(szMessage), "Remote : %s\n", itTest1->strNVRA.c_str());
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME, szMessage,NULL);
	}
	 
	for(itTest2 = m_setLocalHeaderInfo.begin();itTest2 != m_setLocalHeaderInfo.end();itTest2++){
		//printf("Local : %s\n", itTest2->strNVRA.c_str());
		snprintf(szMessage, sizeof(szMessage), "Local : %s\n", itTest2->strNVRA.c_str());
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME, szMessage,NULL);
	}
	// test code end
	/////////////////////
#endif

	m_nUpdateAvailableCount = 0;
	// CreateUpdateInstallList() method start
	m_bSelfUpdate = false;

	m_vectorInstallList.clear();
	m_vectorUpdateList.clear();

	// Iterator of Pkg-Set from Local System
	set <structHeaderInfo, DereferenceLess>::iterator itLocal;

	// Iterator of Pkg-Set from Remote Server
	set <structHeaderInfo, DereferenceLess>::iterator itRemote;

	// Detail variable for a Pkg from Local System
	string  strNameL, strVerL, strRelL, strArchL;

	// Detail variable for a Pkg from Remote Server
	string  strNameR, strVerR, strRelR, strArchR;

	// for LocalSystemPkgSet
	string  strOldName="";	
	string  strOldVer="";
	string  strOldRel="";
	string  strOldArch="";

	// for RemoteServerPkgSet
	string  strOldName2="";	
	string  strOldVer2="";
	string  strOldRel2="";
	string  strOldArch2="";


	int nCompare=0;
	int nPercent = 0;
	int nCount = 0;
	bool bFind;
	bool bIsLoop=false;

	structFileInfo fileInfo;

	int nTotal = m_setRemoteHeaderInfo.size();
	int nEscapeCount = 0;

	itRemote = m_setRemoteHeaderInfo.begin();
	itLocal  = m_setLocalHeaderInfo.begin();

  string msg;
  classLogger log;

	// Main Loop
	while(true)
	{
		if(CheckLoop(strOldName) || CheckLoop(strOldName2)){
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME,"Looping in CreateUpdateInstallList()", NULL);
			bIsLoop=true;
			break;
		}

		// Checking the empty RemotePkgSet
		if (m_setRemoteHeaderInfo.begin() == m_setRemoteHeaderInfo.end())
		{
			break;
		}

		// Checking the empty LocalPkgSet
		if (m_setLocalHeaderInfo.begin() == m_setLocalHeaderInfo.end())
		{
			break;
		}

		///////////////
		// End condition
		// Checking the terminating condition of LocalPkgSet and RemotePkgSet 
		if (itLocal == m_setLocalHeaderInfo.end() && itRemote == m_setRemoteHeaderInfo.end())
		{
			break;
		}
		
		if (itLocal != m_setLocalHeaderInfo.end())
		{
			//itLocal--;
			stripNVRA(itLocal->strNVRA, &strNameL, &strVerL, &strRelL, &strArchL);
			strOldName = strNameL;
			strOldVer = strVerL;
			strOldRel = strRelL;
			strOldArch = strArchL;

#ifdef DEBUG
      if (itLocal->strNVRA == "axtu-incmp-list-0.1-1AX.noarch=axtu-incmp-list-0.1-1AX.noarch.rpm") {
        printf("axtu-incmp-list is found in m_setLocalHeaderInfo\n");
      }

      if (itLocal->strNVRA.rfind("axtu-incmp-list") != string::npos ) {
        cout << "axtu-incmp-list found in m_setLocal strNVRA =" << itLocal->strNVRA << endl;
      }
#endif
		}
		else
		{
			//printf("itLocal is on end point.");			
		}

		/////////////
		// Same name pass and find last version in LocalSystem's PkgSet.
		while(strOldName == strNameL)
		{
			if(itLocal != m_setLocalHeaderInfo.end()) itLocal++;
			//Check end point
			if (itLocal == m_setLocalHeaderInfo.end())
			{
				itLocal--;
				break;
			}

			stripNVRA(itLocal->strNVRA, &strNameL, &strVerL, &strRelL, &strArchL);
			if(strOldName != strNameL || (strOldName == strNameL && !CheckArch(strOldArch, strArchL)))
			{
				itLocal--;
				stripNVRA(itLocal->strNVRA, &strNameL, &strVerL, &strRelL, &strArchL);
#ifdef DEBUG				
				if(strNameL.find("zsh") != string::npos)
			    	{					
					printf("xxxxx : %s %s %s %s\n", strNameL.c_str(), strVerL.c_str(), strRelL.c_str(), strArchL.c_str() );
			    	}
#endif
				break;
			}

		}

		if (itRemote != m_setRemoteHeaderInfo.end())
		{
			stripNVRA(itRemote->strNVRA, &strNameR, &strVerR, &strRelR, &strArchR);
			strOldName2 = strNameR;
			strOldVer2 = strVerR;
			strOldRel2 = strRelR;
			strOldArch2 = strArchR;
		}
		else
		{
			//printf("itRemote is on end point.");
			break;
		}

		/////////////
		// Same name pass and find last version in RemoteServer's PkgSet.
		while(strOldName2 == strNameR)
		{
			if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
			//Check end point
			if (itRemote == m_setRemoteHeaderInfo.end())
			{
				itRemote--;
				break;
				
			}
			nCount++;
			stripNVRA(itRemote->strNVRA, &strNameR, &strVerR, &strRelR, &strArchR);

			if(strOldName2 != strNameR || (strOldName2 == strNameR && !CheckArch(strOldArch2 , strArchR)))
			{   
				itRemote--;
				nCount--;
				stripNVRA(itRemote->strNVRA, &strNameR, &strVerR, &strRelR, &strArchR);
				break;
			}


		}
		// arithmetic for Progress Value
		if( !(nCount % 50) )
		{
			nPercent = (int)( nCount * 100./nTotal);
			if(g_callBackCreateUpdateInstallList)
			{
				g_callBackCreateUpdateInstallList(nPercent,nPercent,"Create update and install list...",itRemote->strNVRA.c_str());
			}
		}
		if(strNameR == strNameL)
		{
			bFind = false;

			if(strArchR == strArchL){
				bFind = true;
			}else if( strArchR == "noarch" || strArchL == "noarch"){
				bFind = true;
			}
			// nCompare > 0         ==>     RemotePkg is higher than LocalPkg
			// nCompare = 0         ==>     RemotePkg is same with LocalPkg
			// nCompare < 0         ==>     RemotePkg is lower than LocalPkg

			if((itLocal->strEpoch)<(itRemote->strEpoch))
			{
				nCompare=1;
			}
			else if((itLocal->strEpoch)>(itRemote->strEpoch))
			{
				nCompare=-1;
			}
			else
			{
				nCompare = CompareVerRel(strVerR, strRelR, strVerL, strRelL);
			}
// Debug for kyagi
#ifdef DEBUG
			if(strNameL.find("axtu-incmp-list") != string::npos)
			{
				printf("nCompare = %d,  Local : %s %s %s %s  Remote : %s %s %s %s\n", \
				nCompare, strNameL.c_str(), strVerL.c_str(), strRelL.c_str(), strArchL.c_str(), \
				strNameR.c_str(), strVerR.c_str(), strRelR.c_str(), strArchR.c_str() );
			}
#endif

			if(nCompare > 0)
			{
				// UPDATE CASE
				if(bFind == true)
				{				
					rtv = ApplyIncmplistToUpdate(strNameL, strVerL, strRelL, strVerR, strRelR);
					if (rtv == true) {
						fileInfo.bIncompatible = true;						
					} else {
						fileInfo.bIncompatible = false;
					}


					AddUpdateInstallList(&fileInfo,strNameR, \
						strVerR,strRelR,itRemote->strEpoch, \
						strArchR,itRemote->nType,       \
						itRemote->bSelfUpdate, CheckSmp(strNameR));
					if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
					if(itLocal != m_setLocalHeaderInfo.end()) itLocal++;
				}
				// INSTALL CASE
				else
				{
					AddUpdateInstallList(&fileInfo, \
						strNameR,strVerR,strRelR, \
						itRemote->strEpoch,strArchR, \
						itRemote->nType,        \
						false,false);
					if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;					
				}				
				nCount++;
			}
			else if(nCompare == 0)
			{	
				if(strArchR != strArchL)
				{
					//	INSTALL CASE	BY 	ARCHITECTURE
					AddUpdateInstallList(&fileInfo,	\
						strNameR,strVerR,strRelR, \
						itRemote->strEpoch,strArchR, \
						itRemote->nType,	\
						false, false);

					if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;					
					nCount++;
				}
				else // strArchR == strArchL
				{
					if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
					if(itLocal != m_setLocalHeaderInfo.end()) itLocal++;
					nCount++;
				}				
			}
			else
			{
				if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
				nCount++;
			}
		}
		else if(strNameL > strNameR)
		{
			// INSTALL CASE	BY 	NAME
			AddUpdateInstallList(&fileInfo,strNameR,strVerR, \
				strRelR,itRemote->strEpoch,strArchR, \
				itRemote->nType,false /*itRemote->bSelfUpdate*/,false);

			if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
			nCount++;
		}
		else  //strNameL < strNameR
		{
			if(itLocal != m_setLocalHeaderInfo.end()) itLocal++;

			if (itLocal == m_setLocalHeaderInfo.end())
			{
				// INSTALL CASE	BY 	NAME 	At the LocalPkgSetTermination time
				AddUpdateInstallList(&fileInfo,strNameR, \
					strVerR,strRelR,itRemote->strEpoch, \
					strArchR,itRemote->nType,	\
					false /*itRemote->bSelfUpdate*/,false);

				if(itRemote != m_setRemoteHeaderInfo.end()) itRemote++;
				nCount++;
			}
		}
	}

  // if (m_bSelfUpdate == true) {
  // rtv = ApplyIncmplistToUpdate(strNameL, strVerL, strRelL, strVerR, strRelR);
  // return rtv;
  // }

	if(!bIsLoop){
		ApplyBlacklistToUpdate();

    vector <structFileInfo>::iterator it;
    for(it=m_vectorUpdateList.begin();it!=m_vectorUpdateList.end();it++) {
      if (it->bBlacklisted == false && it->bIncompatible == true) {
        msg = it->strName + "-" + it->strVersion + "-" + it->strRelease + " is not updated because it is incompatible";
        log.WriteLog_syslog(msg);
        //cout << msg << endl;
      }
    }

		return true;
	}else{
		return false;
	}

}


/*!
@brief Check arch. 

Check arch that is available or not. 
 
@return true is ok.
*/
///////////////////////////////////////////////////////////////
// Check arch that is same or not. 
bool classRpmEngine::CheckArch(string strArch1, string strArch2)
{
	bool bRet=false;
	if(strArch1 == "noarch" || strArch2 == "noarch")
	{
		bRet = true;
	}

	else if(strArch1 == "i386" && strArch2 == "i486")
	{
		bRet = true;
	}

	else if(strArch1 == "i386" && strArch2 == "i586")
	{
		bRet = true;
	}

	else if(strArch1 == "i386" && strArch2 == "i686")
	{
		bRet = true;
	}
	else if(strArch1 == "i386" && strArch2 == "athlon")
	{
		bRet = true;
	}

	else if(strArch1 == "i486" && strArch2 == "i386")
	{
		bRet = true;
	}

	else if(strArch1 == "i486" && strArch2 == "i586")
	{
		bRet = true;
	}

	else if(strArch1 == "i486" && strArch2 == "i686")
	{
		bRet = true;
	}
	else if(strArch1 == "i486" && strArch2 == "athlon")
	{
		bRet = true;
	}


	else if(strArch1 == "i586" && strArch2 == "i386")
	{
		bRet = true;
	}

	else if(strArch1 == "i586" && strArch2 == "i486")
	{
		bRet = true;
	}

	else if(strArch1 == "i586" && strArch2 == "i686")
	{
		bRet = true;
	}
	else if(strArch1 == "i586" && strArch2 == "athlon")
	{
		bRet = true;
	}


	else if(strArch1 == "i686" && strArch2 == "i386")
	{
		bRet = true;
	}

	else if(strArch1 == "i686" && strArch2 == "i486")
	{
		bRet = true;
	}

	else if(strArch1 == "i686" && strArch2 == "i586")
	{
		bRet = true;
	}
	else if(strArch1 == "i686" && strArch2 == "athlon")
	{
		bRet = true;
	}


	else if(strArch1 == "athlon" && strArch2 == "i386")
	{
		bRet = true;
	}

	else if(strArch1 == "athlon" && strArch2 == "i486")
	{
		bRet = true;
	}

	else if(strArch1 == "athlon" && strArch2 == "i586")
	{
		bRet = true;
	}
	else if(strArch1 == "athlon" && strArch2 == "i686")
	{
		bRet = true;
	}

	else if(strArch1 == strArch2)
	{
		bRet = true;
	}

	return bRet;
}

/*!
@brief Remove package from Install List.
Remove Item from Install List.
@param strName : obsoleted package name.
*/
bool classRpmEngine::RemoveUpdateInstallList(string strName)
{
	vector <structFileInfo>::iterator it;
	for (it=m_vectorInstallList.begin(); it!=m_vectorInstallList.end(); it++) 
	{
		if (strName == it->strName) 
		{
			m_vectorInstallList.erase(it--);
		}
	}
	for (it=m_vectorUpdateList.begin(); it!=m_vectorUpdateList.end(); it++) 
	{//not tested
		if (strName == it->strName) 
		{
			m_vectorUpdateList.erase(it--);
			m_nUpdateAvailableCount--; //many other places need to concern about this
		}
	}
}

/*!
@brief copy obsoleter from installlist to update list
@return true is "done".
*/
bool classRpmEngine::CopyObsoleterFromInstallToUpdate(string strName, string strArch)
{
//refer to JeongHong's FindInsertPosFromUpdateList() 
//and the last part of CopyObsoletePackagesFromInstallToUpdate()
	string strResult;
        int nCount=0;
        vector <structFileInfo>::iterator itUpdate;
        for(itUpdate = m_vectorUpdateList.begin();itUpdate!=m_vectorUpdateList.end();itUpdate++)
        {
		if(itUpdate->strName > strName)
		{
			break;
		}
		nCount++;
	//not very confident with the nCount here, need to test when I have time
        }
	vector <structFileInfo>::iterator itInstall;
        for (itInstall=m_vectorInstallList.begin(); itInstall!=m_vectorInstallList.end(); itInstall++)
        {
                if ((strName == itInstall->strName)&&(strArch==itInstall->strArch))
                {
                        break;
                }
        }
	m_vectorUpdateList.insert(m_vectorUpdateList.begin()+nCount, *(itInstall));
	//it is the fomer itInstall-- here that break my m_vectorObsoleteToUpdate, 
	//thus sos can not be updated, now it is OK
	m_nUpdateAvailableCount++; //for notifier
	m_vectorObsoleteToUpdate.push_back(*itInstall);
	return true;
}

/*!
@brief Is Package Installed?
Is Package Installed on local system?.
@param strName : package name.
@return true is "installed".
*/
bool classRpmEngine::IsPackageInstalled(string strName)
{
	set <structHeaderInfo, DereferenceLess>::iterator it;
	string strNameL, strVerL, strRelL, strArchL;
	for (it=m_setLocalHeaderInfo.begin(); it!=m_setLocalHeaderInfo.end();it++) 
	{
		stripNVRA(it->strNVRA, &strNameL, &strVerL, &strRelL, &strArchL);
		if (strNameL == strName) 
		{
			return true;
		}
	}
	return false;
}
 
/*!
@brief used by classGui::ApplyObsoletes().
only modify update install lists, nothing else
*/
int classRpmEngine::ApplyObsoletes()
{	
	//first we remove the obsoleted from both lists
	int i;
	m_vectorObsoleteToUpdate.clear();
	vector<structRPMInfo*>::iterator it;
	for(it=m_vectorRPMInfo.begin();it!=m_vectorRPMInfo.end();it++)
	{
		for(i=0;(*it)->obsoleteName[i];i++) //refer to how they use provideName
		{
			RemoveUpdateInstallList((*it)->obsoleteName[i]);
		}
	}

	//below we add obsoleter to update list
        for(it=m_vectorRPMInfo.begin();it!=m_vectorRPMInfo.end();it++)
        {
                for(i=0;(*it)->obsoleteName[i];i++) //refer to how they use provideName
                {
                        if(IsPackageInstalled((*it)->obsoleteName[i])==true)
                        {	//here I've tested if A obsoletes more than 1 tags, axtu can handle it properly
				CopyObsoleterFromInstallToUpdate((*it)->name, (*it)->arch);
				//if A obsoletes B C, and B,C are both installed already
				// then A will be added twice, note|
				
                        }

                }
        }
	return 0;
}

/*!
@brief Read headers.

Read header infos from all header files(*.hdr). 
 
*/
int classRpmEngine::ReadHeaders()
{

	vector <structFileInfo> vectorUpdateList;
	vectorUpdateList = GetUpdateList();
	vector <structFileInfo>::iterator it;

	vector <structFileInfo> vectorInstallList;
	vectorInstallList = GetInstallList();
	vector <structFileInfo>::iterator it2;

	string strFullPath;
	int nIndex = 0;
	int nPercent=0;
	int nTotal = vectorUpdateList.size() + vectorInstallList.size();
	for(it = vectorUpdateList.begin();it!=vectorUpdateList.end();it++)
	{
		strFullPath = it->strCachePath + "/headers/" + it->strName + "-" + it->strEpoch + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".hdr";
		if(OpenHeader(strFullPath, it->nType, true) < 0)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "OpenHeader error",NULL);
			return -1;
		}
		if(!(nIndex % 10))
		{
			nPercent = int(nIndex*100./nTotal);
			if(g_callBackReadHeaders)
				g_callBackReadHeaders(nPercent, nPercent, "Reading header information...", strFullPath.c_str());
		}
		nIndex++;
	}


	for(it2 = vectorInstallList.begin();it2!=vectorInstallList.end();it2++)
	{
		strFullPath = it2->strCachePath + "/headers/" + it2->strName + "-" + it2->strEpoch + "-" + it2->strVersion + "-" + it2->strRelease + "." + it2->strArch + ".hdr";
		if(OpenHeader(strFullPath, it2->nType, false) < 0)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "OpenHeader error",NULL);
			return -1;
		}
		if( !(nIndex % 10))
		{
			nPercent = int(nIndex*100./nTotal);
			if(g_callBackReadHeaders)
				g_callBackReadHeaders(nPercent, nPercent, "Reading header information...", strFullPath.c_str());
		}
		nIndex++;
	}
	if(g_callBackReadHeaders)
		g_callBackReadHeaders(100, 100 ,"Finish","Finish");
	return 0;

}

/*!
@brief Open rpm file. 

1. Open rpm file, 
2. Get header info, 
3. Set header info to m_vectorRPMInfo.
 
@param strFilePath : Header.
@param index : server index(the value of m_vectorCacheDirInfo->nType). 

@return 0 is ok.
*/
int classRpmEngine::OpenHeader(string strFilePath, int nIndex, bool bUpgrade)
{
	Header h = NULL;

	//ungzip
	gzFile zfp;
	char * buf;
	zfp = gzopen(strFilePath.c_str(), "rb");
	if (zfp == NULL)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "can not open gzip file: ",strFilePath.c_str() , NULL);
		return -1;
	}

	int total=0;

	// get size of gzip file(hdr)
	int size = 0;
	while(gzgetc(zfp) != -1)
	{
		total ++;
	}

	//load header
	gzrewind(zfp);
	buf = new char [total];
	size = gzread(zfp, buf, total);
	h = headerLoad(buf);
	if(!h)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "can not read header file: ",strFilePath.c_str() ,NULL);
		delete [] buf;
		gzclose(zfp);
		return -2;
	}

	structRPMInfo * rpmInfo = GetHeaderInfo(h, 9, nIndex);
	if(rpmInfo == NULL)
	{
		headerFree(h);
		delete [] buf;
		gzclose(zfp);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "GetHeaderInfo() error", NULL);
		return -3;
	}
	if(bUpgrade)
	{
		rpmInfo->upgradeFlag = 1;
	}
	else
	{
		rpmInfo->upgradeFlag = 0;
	}
	m_vectorRPMInfo.push_back(rpmInfo);

	headerFree(h);
	delete [] buf;
	gzclose(zfp);
	return 0;
}


/*!
@brief Get header info from sturuct Header.

Get header info from sturuct Header to  struct structRPMInfo.
 
@param h : Header.
@param k : count tag1(You can just fix 9).
@param index : server index(the value of m_vectorCacheDirInfo->nType). 

@return struct structRPMInfo.
*/
struct structRPMInfo *classRpmEngine::GetHeaderInfo(Header h, int k, int index /*Server index*/)
{
	char * data;
	const char **arrayData, *ptr;
	char **tmp2[5];
	hCNT_t c;
	int i, j;
	struct structRPMInfo *rpmInfo;
	int tag1[9] = { RPMTAG_NAME, RPMTAG_VERSION, RPMTAG_RELEASE,
			RPMTAG_SIZE, RPMTAG_GROUP, RPMTAG_SUMMARY,
			RPMTAG_DESCRIPTION, RPMTAG_ARCH, 1000004 };
	int tag2[6] = { RPMTAG_REQUIRENAME, RPMTAG_REQUIREVERSION,
			RPMTAG_PROVIDENAME, RPMTAG_PROVIDEVERSION,
			RPMTAG_OBSOLETENAME, RPMTAG_OBSOLETEVERSION };

	rpmInfo = (struct structRPMInfo *)malloc(sizeof(struct structRPMInfo));
	if(rpmInfo == NULL)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "malloc error in GetHeaderInfo() function",NULL);
		return NULL;
	}
	memset(rpmInfo, 0x00, sizeof(struct structRPMInfo));
	rpmInfo->matchNumber = -1;

	for(j = 0; j < k; j++) {
		int_32 pt2;
		if (!headerGetEntry(h, tag1[j], &pt2, (void **)&data, NULL)) {
			if (j == 7) {
				sprintf(rpmInfo->arch, "%s", "");
				continue;
			}
			else
			{
				continue;
				//return NULL;   ??????????????????
			}
		}

		switch(tag1[j]) {
		    case RPMTAG_NAME:
			rpmInfo->name = strdup((char *)data);
			if(rpmInfo->name == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case RPMTAG_VERSION:
			rpmInfo->version = strdup((char *)data);
			if(rpmInfo->version == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case RPMTAG_RELEASE:
			rpmInfo->release = strdup((char *)data);
			if(rpmInfo->release == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case RPMTAG_ARCH:
			rpmInfo->arch = strdup((char *)data);
			if(rpmInfo->arch == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case RPMTAG_GROUP:
			rpmInfo->group = strdup((char *)data);
			if(rpmInfo->group == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case 1000004:
			rpmInfo->matchNumber = (int)*((int_32 *)data);
			break;
		    case RPMTAG_SIZE:
			rpmInfo->size = (int)*((int_32 *)data); 
			break;
		    case RPMTAG_SUMMARY:
			rpmInfo->summary = strdup((char *)data);
			if(rpmInfo->summary == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    case RPMTAG_DESCRIPTION:
			rpmInfo->shortDesp = strdup((char *)data);
			if(rpmInfo->shortDesp == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(data, (rpmTagType)pt2);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			break;
		    default:
			break;
		}
		headerFreeData(data, (rpmTagType)pt2);
	}

	c = (int_32 *)malloc(sizeof(int));
	if(c == 0)
	{
		FreeRequiredRPM(rpmInfo);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "malloc error in GetHeaderInfo() function",NULL);
		return NULL;
	}
	for (j = 0; j < 6; j++)
	{
		*c = 0;
		int_32 pt;
		if(!headerGetEntry(h, tag2[j], &pt, (void **)&arrayData	, c))
		{
			tmp2[j] = (char **)calloc((1), sizeof(char *));
			if (tmp2[j] == NULL) {
				FreeRequiredRPM(rpmInfo);
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "calloc error in GetHeaderInfo() function", NULL);
				return NULL;
			}
			tmp2[j][0] = NULL;
			if(j == 0)
				rpmInfo->requireFileNumber = 0;
			continue;
		}

		if (( tmp2[j] = (char **)calloc((*c+1), sizeof(char *))) == NULL) {
			FreeRequiredRPM(rpmInfo);
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "calloc error in GetHeaderInfo() function", NULL);
			return NULL;
		}

		ptr = *arrayData;
		for (i = 1; i <= (*c); i++) {
			tmp2[j][i-1] = strdup((char *)ptr);
			if(tmp2[j][i-1] == NULL)
			{
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetHeaderInfo() function",NULL);
				headerFreeData(arrayData, (rpmTagType)pt);
				FreeRequiredRPM(rpmInfo);
				return NULL;
			}
			ptr = strchr(ptr, 0);
			ptr++;
		}
		tmp2[j][i-1] = NULL;

		if(j == 0)
		{
			rpmInfo->requireFileNumber = i - 1;
		}
		headerFreeData(arrayData, (rpmTagType)pt);
	}

	rpmInfo->requireName = tmp2[0];
	rpmInfo->requireVersion = tmp2[1];
	rpmInfo->provideName = tmp2[2];
	rpmInfo->provideVersion = tmp2[3];
	rpmInfo->obsoleteName = tmp2[4];
	rpmInfo->obsoleteVersion = tmp2[5];
	rpmInfo->h = headerCopy(h);
	//rpmInfo->upgradeFlag = 0; //default set to install
	rpmInfo->disknum = index;  //default set to disk(updater use server num)
	rpmInfo->containFiles = NULL;
	free(c);
	c=0;
	return rpmInfo;
}

/*!
@brief Add package to vector m_vectorAddedFile.

Add package to vector m_vectorAddedFile.

@param strPackName package name.  
Install or update includes full path,  Remove include NVRA name.
@param nType  UPDATE 1, REMOVE 2, REQDEP 4, OTHERDEP 8.
@return 0 is ok.
*/
int classRpmEngine::AddFile(const char *strPackName, int nType, int nUpgrade)
{
	if (CheckSameFile(m_vectorAddedFile, strPackName) < 0)
	{
		return 0;
	}

	structAddedFile tmpStruct;
	tmpStruct.strFile = strdup(strPackName);
	if(tmpStruct.strFile == NULL)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in AddFile() function",NULL);
		return -1;
	}
	
	string  strName1, strVer1, strRel1, strArch1;
	stripNVRA(strPackName, &strName1, &strVer1, &strRel1, &strArch1);
		
	tmpStruct.nType = nType;
	
	int nRet = CheckKernel(strName1);
	if (nRet == 0)
	{
		nUpgrade = 0;
		m_bKernelInstall = true;
	}
	else if(nRet == 2)  // Only install.
	{
		nUpgrade = 0;
	}
	tmpStruct.nUpgrade = nUpgrade;
	tmpStruct.bIncompatible = isIncompatiblePack(strName1, strVer1, strRel1);
	if(tmpStruct.bIncompatible == true)
	{
		m_nIncompatible += 1;
		m_vectorIncompatibleUpdatePackages.push_back(strPackName);
	}
	//printf("%s : %s, %s, %s == %d\n" , strPackName, strName1.c_str(), strVer1.c_str(), strRel1.c_str(), tmpStruct.bIncompatible);
	m_vectorAddedFile.push_back(tmpStruct);	

	return 0;
}



/*!
@brief Clear vector m_vectorAddedFile.

Clear vector m_vectorAddedFile.
*/
void classRpmEngine::ClearAddedFiles()
{
	// Added files
	vector<structAddedFile>::iterator it;
	for(it=m_vectorAddedFile.begin();it!=m_vectorAddedFile.end();it++)
	{
		if(it->strFile)
		{
			free((void*)it->strFile);
			it->strFile = NULL;
		}
	}	
	m_vectorAddedFile.clear();
	
	
	// Added Dependency files.
	vector<char *>::iterator it2;
	for(it2=m_vectorDepAddedFiles.begin();it2!=m_vectorDepAddedFiles.end();it2++)
	{
		if(*it2)
		{
			free((void*)*it2);
			*it2 = NULL;
		}
	}
	m_vectorDepAddedFiles.clear();
}

/*!
@brief Get data from vector m_vectorAddedFile.

Get Update, Install, Remove, Other Dep, Dep from vector m_vectorAddedFile.

@param nType  UPDATE 1, REMOVE 2, REQDEP 4, OTHERDEP 8.  
@param vectorTemp input vector.

@return result output vector.
*/
vector <structAddedFile> classRpmEngine::GetAddedFile(int nType, vector <structAddedFile> vectorTemp)
{
	int nCount = 1;
	vector<structAddedFile>::iterator it;
	for(it=m_vectorAddedFile.begin();it!=m_vectorAddedFile.end();it++)
	{
		for(nCount=1;nCount<=nType;nCount=nCount*2)
		{
			if(int(nType & nCount) == nCount)
			{
				if( int(it->nType & nCount) == nCount)
				{
					structAddedFile structTemp;
					structTemp.strFile = it->strFile;
					structTemp.nType = nCount;
					structTemp.nUpgrade = it->nUpgrade;

					vectorTemp.push_back(structTemp);
				}
			}
		}
	}
	return vectorTemp;
}



/*!
@brief Get Update list vector.

Get Update list vector from m_vectorUpdateList.

@return result Update list vector.
*/

vector <structFileInfo> classRpmEngine::GetUpdateList()
{
	return m_vectorUpdateList;
}

/*!
@brief Get Install list vector.

Get Install list vector from m_vectorInstallList.

@return result Install list vector.
*/
vector <structFileInfo> classRpmEngine::GetInstallList()
{
	return m_vectorInstallList;
}

/*!
@brief Get Installed(will be erase) list vector.

Get Installed list vector from m_setLocalHeaderInfo.

@return result Installed list vector.
*/
set <structHeaderInfo , DereferenceLess> classRpmEngine::GetInstalledList()
{
	return m_setLocalHeaderInfo;
}

/*!
@brief Check added rpm packages.

1. Add selected package files to ts.
2. Check dependency.
3. Add Dependency file to ts.

@return  negative num : error,	0 : ok
*/
int classRpmEngine::Check(bool bForceBlackist)
{
	m_vectorBlockedUpdatePackages.clear();
	m_nDepCount = 0;
	m_nDepTotalCount = 0;
	m_bKernelInstall = false;
	int rc;
	
	rpmps probs=NULL;

	vector<structAddedFile>::iterator it;
	vector<structAddedFile> vectorInstall;
	vector<structAddedFile> vectorRemove;
	vectorInstall = GetAddedFile(UPDATE | REQDEP | OTHERDEP , vectorInstall);
	vectorRemove = GetAddedFile(REMOVE, vectorRemove);
	for(it=vectorInstall.begin();it!=vectorInstall.end();it++)
	{
		rc=AddInstallPackage(m_rpmTs, it->strFile, it->nUpgrade);
		if(rc != 0){
			char strtmp[MAX_STRING];
			snprintf(strtmp, sizeof(strtmp), "AddInstallPackage() failed(return = %d)", rc);
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, strtmp, NULL);
			return rc;
		}
	}
	if(g_callBackCheck)
		g_callBackCheck(0, 0, "", "");
	for(it=vectorRemove.begin();it!=vectorRemove.end();it++)
	{
		string  strName1, strVer1, strRel1, strArch1;
		stripNVRA(it->strFile, &strName1, &strVer1, &strRel1, &strArch1);

		if (AddRemovePackage(m_rpmTs, it->strFile) != 0)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "AddRemovePackage() failed", NULL);
			return -1;
		}
	}
	if(g_callBackCheck)
		g_callBackCheck(0, 0, "", "");
	
	if (m_bSelfUpdate == true) // nodeps force option.
	{
		return 0;
	}
	rc = rpmtsCheck(m_rpmTs);
	if(g_callBackCheck)
		g_callBackCheck(0, 0, "", "");
	probs = rpmtsProblems(m_rpmTs);
	int result=0;
	if (rc || probs->numProblems > 0) {
		result = DealwithDependence(m_rpmTs, probs);
	}
	
	if(result > -1)
	{	
		vector <structAddedFile> vectorTemp;
		vectorTemp = GetAddedFile(REQDEP | OTHERDEP, vectorTemp);
		result = vectorTemp.size();
	}	

	if(g_callBackCheck)
		g_callBackCheck(0, 0, "", "");


	if (bForceBlackist == false)
	{
		// Add Blacklist return value
		if (m_vectorBlockedUpdatePackages.size() > 0)
		{
			result = -8;
		}
	}
	
	if(result < 0 && (probs->numProblems) && (probs != NULL))
	{
		rpmpsPrint(NULL, probs);
		WriteDepLog(probs);
	}
	return result;   //0 Ok,   -9 no find dep,  -8 blcked by black,  -6 not found, -10 network error 
}

/*!
@brief Check added rpm packages for kmod redcastle.

1. Check integrity between kernel and kmod redcastle.
2. Add required package from update or install available.

This function must call before Check() function of classRpmEngine

@return true or false
*/
bool classRpmEngine::CheckKmodRedcastle()
{
	bool bResult = true;
	string strFind;
	m_vectorAddedKmodRedcastleFile.clear();
        vector<structAddedFile>::iterator it;
        for(it=m_vectorAddedFile.begin();it!=m_vectorAddedFile.end();it++)
        {
		if(it->nType == UPDATE)
		{
			string  strName1, strVer1, strRel1, strArch1;
			stripNVRA(it->strFile, &strName1, &strVer1, &strRel1, &strArch1);
			if(CheckKernel(strName1) == 0)
			{
				if( strName1.find("xen") != string::npos)
					continue;
				m_vectorAddedKmodRedcastleFile.push_back(it->strFile);
				// Not find
				if(GetKmodRedcastleName(strName1, strVer1, strRel1, strFind) == false)
				{
					bResult = false;
					continue;
				}	
				else
				{
					m_vectorAddedKmodRedcastleFile.push_back(strFind);
				}
			}
		}	
	}
	
	

	// There are no kernel.	
	return bResult;
}

bool classRpmEngine::GetKmodRedcastleName(string strName, string strVersion, string strRelease, string &strFind)
{
	vector <structFileInfo>::iterator it3;
	string strKmodRelease = strVersion + "_" + strRelease;
	string strKmodName;

	if(strName.find("kernel-") == 0)
	{
		string strKmodType = strName.substr(strlen("kernel-"), strName.length() - strlen("kernel-"));
		strKmodName = string(RC_KMOD_NAME) + "-" + strKmodType;
	}
	else
	{
		strKmodName = RC_KMOD_NAME;
	}


        for(it3=m_vectorUpdateList.begin();it3!=m_vectorUpdateList.end();it3++)
        {
		if(it3->strName == strKmodName && it3->strRelease.find(strKmodRelease) != string::npos)
		{
			strFind = GetFullPathFile(it3->nType, false, it3->strName, it3->strVersion, it3->strRelease, it3->strArch, it3->strEpoch, false);
			return true;
		}		
	}

	vector <structFileInfo>::iterator it4;
        for(it4=m_vectorInstallList.begin();it4!=m_vectorInstallList.end();it4++)
        {
		if(it4->strName == strKmodName && it4->strRelease.find(strKmodRelease) != string::npos)
		{
			strFind = GetFullPathFile(it4->nType, false, it4->strName, it4->strVersion, it4->strRelease, it4->strArch, it4->strEpoch, false);
			return true;

		}		
	}
	return false;
}

vector <string> classRpmEngine::GetAddedKmodRedcastleFile()
{	
	return m_vectorAddedKmodRedcastleFile;
}

/*!
@brief Remove kerenel and kmod redcastle packages from added rpm packages.

This function must call before Check() function of classRpmEngine

@return true or false
*/
bool classRpmEngine::RemoveKernelAndKmodRedcastle(int nType)
{
	if (nType != 0 && nType != 1)
		return false;
	vector <structAddedFile> vectorTemp;
	vectorTemp = m_vectorAddedFile;
	m_vectorAddedFile.clear();
	m_Network->ClearPackages();

	string  strName1, strVer1, strRel1, strArch1;
	string  strName2, strVer2, strRel2, strArch2;
	vector<structAddedFile>::iterator it;
	vector<string>::iterator it2;
	bool bFind;
	for(it=vectorTemp.begin();it!=vectorTemp.end();it++)
	{
		bFind = false;	
		stripNVRA(it->strFile, &strName1, &strVer1, &strRel1, &strArch1);
		for(it2=m_vectorAddedKmodRedcastleFile.begin();it2!=m_vectorAddedKmodRedcastleFile.end();it2++)
	        {
			stripNVRA(*it2, &strName2, &strVer2, &strRel2, &strArch2);
			if( strName1 == strName2 && strVer1 == strVer2 && strRel1 == strRel2 && strArch1 == strArch2 )
			{
				bFind = true;
				break;
			}
		}		
		
		if(bFind == false)
		{
			m_vectorAddedFile.push_back(*it);
			m_Network->AddPackage(it->strFile, GetFullPathFile(nType,  false, strName1, strVer1, strRel1, strArch1, "", true) );
		}

	}
	
	return true;
}


/*!
@brief Write Dependency Log.

Write Dependency Log.

@param ps : rpmps
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// Write require element to '/var/log/SP2dep.log' file.
// nOperation : 0:Remove  1.Overwrite 2.Append
bool classRpmEngine::WriteDepLog(rpmps ps)
{
	char strLastErrPath[MAX_STRING];
	snprintf(strLastErrPath, sizeof(strLastErrPath), "%s/%s", m_Logger->GetLogPath().c_str(), LAST_ERROR_LOG_FILE);
	ofstream fout;
	remove(strLastErrPath);
	fout.open(strLastErrPath, ofstream::out);

	const char * msg=NULL;
	int i;

	if (ps == NULL || ps->probs == NULL || ps->numProblems <= 0)
		return false;

	for (i = 0; i < ps->numProblems; i++) {
	    rpmProblem p;
	    int j;
	
	    p = ps->probs + i;
	
	    if (p->ignoreProblem)
	        continue;
	
	    /* Filter already displayed problems. */
	    for (j = 0; j < i; j++) {
	        if (!sameProblem(p, ps->probs + j))
	            /*@innerbreak@*/ break;
	    }
	    if (j < i)
	        continue;
	    
	    char buf [MAX_STRING];
	    memset(buf, 0, sizeof(buf));
	    
	    
	    msg = rpmProblemString(p);
	    // requir file
	    snprintf(buf, sizeof(buf) , "\t%s\n", msg);
	    
	    
	    //file.writeBlock( buf, strlen(buf));
	    m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, (const char *)buf, NULL);
	    try
	    {
	        fout.write( buf , strlen(buf));
	    	msg = (const char *)_free(msg);
			msg = NULL;
	    }
	    catch(ios_base::failure failer)
	    {
	    	fout.close();
	    	return false;
	    }
	}
	
	try
	{
	    fout.write( "\n" , strlen("\n"));
	}	
	catch(ios_base::failure failer)
	{
		fout.close();
		return false;
	}

	fout.close();
	
	return true;
	
}

/*!
@brief Check same problem.

Write Check same problem.

@param ap : rpmProblem
@param bp : rpmProblem

*/
int classRpmEngine::sameProblem(const rpmProblem ap, const rpmProblem bp)
    /*@*/
{
    if (ap->type != bp->type)
    	return 1;
    
    if (ap->pkgNEVR) {
    	if (bp->pkgNEVR && strcmp(ap->pkgNEVR, bp->pkgNEVR))
    		return 1;
    }
    
    if (ap->altNEVR) {
	    if (bp->altNEVR && strcmp(ap->altNEVR, bp->altNEVR))
	        return 1;
    }
    
    if (ap->str1) {
    	if (bp->str1 && strcmp(ap->str1, bp->str1))
    		return 1;
    }
    
    if (ap->ulong1 != bp->ulong1)
    	return 1;

    return 0;
}


/*!
@brief Add package to install or update.

Add Install or Update package to ts.
If there are same package diffrent arch will be install or update.

@param upgrade  0:install 1:upgrade(old version package will be delete).
@return 0 is ok.
*/
int classRpmEngine::AddInstallPackage(rpmts ts, const char * file, int upgrade)
{
	FD_t fd;
	Header hdr;
	int rc = 0;
	int nRet=0;

	fd = Fopen(file, "r.ufdio");
	if (fd == NULL) {
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open ", file, NULL);
		return -1;
	}

	rc = rpmReadPackageFile(ts, fd, file, &hdr);

	if (rc == RPMRC_NOTFOUND || rc == RPMRC_FAIL) {
		Fclose(fd);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to read package ", file, NULL);
		return rc;
	}

	rc = AddInstallElement(ts, hdr, file, 0, upgrade);
	if (rc > 0) {
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to add to transaction ", file, NULL);
		goto out;
	}
	else if(rc < 0)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, file, " not found", NULL);
		goto out;
	}

#if defined(__x86_64__) || defined(__PPC__) || defined(__ia64__)
	rc=GetOtherArchPackages(ts, hdr, upgrade);
	if(rc > 0){
		nRet=HAVE_OTHERARCH;
	}else if(rc ==0){
		nRet=0;
	}
	else
	{
		return -10;
	}


#endif

out:
	headerFree(hdr);
	Fclose(fd);
	return nRet;	
}


/*!
@brief Add package to erase.

Add Erase package to ts.

@param ts rpmts
@param strName package name.
@return 0 is ok.
*/
int classRpmEngine::AddRemovePackage(rpmts ts, const char * strName)
{
	rpmdbMatchIterator mi;
	const char * np=NULL;
	const char * ep=NULL;
	const char * ap=NULL;
	const char * vp=NULL;
	const char * rp=NULL;
	Header hdr;
	int rc;


	classBlockSignal blocksignal(SIGHUP); //Util this class is destroyed, SIGHUP will be destroy.
	mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_LABEL, strName, strlen(strName));

	while ((hdr = rpmdbNextIterator(mi)) != NULL)
	{
		int recOffset = rpmdbGetIteratorOffset(mi);
		if (recOffset)
		{
			rc = headerNEVRA(hdr,&np,&ep, &vp, &rp, &ap);
			if (rc)
			{
				rpmdbFreeIterator(mi);
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to query RPM database", NULL);
				return -1;
			}
			int rc = 0;

			rc = rpmtsAddEraseElement(ts, hdr, recOffset);
			if (rc != 0) {
				rpmdbFreeIterator(mi);	
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to add to transaction ", strName, NULL);
				return -1;
			}		  
		}
	}
	rpmdbFreeIterator(mi);
	return 0;	
}

/*!
@brief Find Header.

Find Header From Local rpm DB.

@param strNVRA Name-Version-Release.Arch.
@param strName input header.
@return header Founded header.
*/

Header classRpmEngine::FindHeaderFromLocal(const char * strNVRA, Header header)
{
	rpmdbMatchIterator mi;
	const char * np=NULL;
	const char * ep=NULL;
	const char * ap=NULL;
	const char * vp=NULL;
	const char * rp=NULL;
	Header hdr;
	int rc;


	string  strName1, strVer1, strRel1, strArch1;
	stripNVRA(strNVRA, &strName1, &strVer1, &strRel1, &strArch1);

	classBlockSignal blocksignal(SIGHUP); //Util this class is destroyed, SIGHUP will be destroy.
	mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_LABEL, strName1.c_str(), strName1.length());

	while ((hdr = rpmdbNextIterator(mi)) != NULL)
	{
		int recOffset = rpmdbGetIteratorOffset(mi);
		if (recOffset)
		{
			rc = headerNEVRA(hdr,&np,&ep, &vp, &rp, &ap);
			if (rc)
			{
				rpmdbFreeIterator(mi);
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to query RPM database", NULL);
				return NULL;
			}


			header = headerCopy(hdr);


		}
	}
	rpmdbFreeIterator(mi);
	return header;
}

/*!
@brief Find Header.

Find Header From remote header.info.

@param name Package name.
@param version Package version. 
@param rel Package release.
@param result input structRPMInfo.
@return result output structRPMInfo.
*/

struct structRPMInfo * classRpmEngine::FindHeaderFromRemote(char *name, char *version, char *rel, struct structRPMInfo *result)
{
	vector<structRPMInfo*>::iterator it;
	for(it=m_vectorRPMInfo.begin();it!=m_vectorRPMInfo.end();it++)
	{
		if( (strcmp(name, (*it)->name) == 0) && (strcmp(version, (*it)->version) == 0) && (strcmp(rel, (*it)->release) == 0))
		{
			result = *it;
			break;
		}
	}
	return result;
}


/*!
@brief Add other arch.

If other arch is  exist on update or install list vector,
then add other arch also add to ts.

@param ts rpmts.
@param h source header. 
@param upgrade 0:install 1:upgrade(old version package will be delete).

@return 0  : not find other arch,  positive : Find and add package,  negative : error 
*/
int  classRpmEngine::GetOtherArchPackages(rpmts ts, Header h, int upgrade)
{
	if(upgrade)
		return 0;
	int rc;
	int nRet=0;
	const char * strNameL;
	const char * strVerL;
	const char * strRelL;
	const char * strArchL;
	headerGetEntry(h, RPMTAG_NAME, NULL, (void **)&strNameL, NULL);
	headerGetEntry(h, RPMTAG_VERSION, NULL, (void **)&strVerL, NULL);
	headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **)&strRelL, NULL);
	headerGetEntry(h, RPMTAG_ARCH, NULL, (void **)&strArchL, NULL);

	vector <structFileInfo>::iterator it;
	string strFullPath, strUrlFullPath;
	char * strTemp;
	//char strtmp[128];
	for(it=m_vectorUpdateList.begin();it!=m_vectorUpdateList.end();it++)
	{
		if(it->strName == strNameL && it->strVersion == strVerL && it->strRelease == strRelL && it->strArch != strArchL)
		{
			strFullPath = it->strCachePath + "/packages/" +  it->strName + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";
			strUrlFullPath = it->strURLFullPath + "/" +  it->strName + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";

			if (AddFile(strFullPath.c_str(), OTHERDEP, 1) == 0)
			{
				m_Network->AddPackage(strFullPath, strUrlFullPath);
				nRet = m_Network->GetPackages();
				if (nRet != NETWORK_RETOK)
				{
					return -10;
				}

				FD_t fd;
				Header hdr;
				int rc = 0;

				fd = Fopen(strFullPath.c_str(), "r.ufdio");

				if (fd == NULL) {
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open ", strFullPath.c_str(), NULL);
					return -1;
				}

				rc = rpmReadPackageFile(ts, fd, strFullPath.c_str(), &hdr);

				if (rc == RPMRC_NOTFOUND || rc == RPMRC_FAIL) {
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to read package ", strFullPath.c_str(), NULL);
					return -1;
				}
				strTemp = strdup(strFullPath.c_str());
				if(strTemp == NULL)
				{
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetOtherArchPackages() function",NULL);
					return -1;
				}
				m_vectorDepAddedFiles.push_back(strTemp);
				rc = AddInstallElement(ts, hdr, strTemp, 0, 1);
				if (rc > 0)
				{
					headerFree(hdr);
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to add to transaction ", strFullPath.c_str(), NULL);
					return -1;
				}
				else if(rc < 0)
				{
					headerFree(hdr);
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, strFullPath.c_str(), " not found", NULL);
					return -1;
				}
				else
				{
					nRet=1;
				}
				headerFree(hdr);
				Fclose(fd);
			}
		}
	}
	/*
	vector <structFileInfo>::iterator it2;
	for(it2=m_vectorInstallList.begin();it2!=m_vectorInstallList.end();it2++)
	{
		if(it2->strName == strNameL && it2->strVersion == strVerL && it2->strRelease == strRelL && it2->strArch != strArchL)
		{
			strFullPath = it2->strCachePath + "/packages/" +  it2->strName + "-" + it2->strVersion + "-" + it2->strRelease + "." + it2->strArch + ".rpm";
			strUrlFullPath = it2->strURLFullPath + "/" +  it2->strName + "-" + it2->strVersion + "-" + it2->strRelease + "." + it2->strArch + ".rpm";

			if(AddFile(strFullPath.c_str(), OTHERDEP, 0) == 0)
			{
				m_Network->AddPackage(strFullPath, strUrlFullPath);
				nRet = m_Network->GetPackages();
				if (nRet != NETWORK_RETOK)
				{
					return -10;
				}

				FD_t fd;
				Header hdr;
				int rc = 0;

				fd = Fopen(strFullPath.c_str(), "r.ufdio");
			
				if (fd == NULL) {
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open ", strFullPath.c_str(), NULL);
					return -1;
				}

				rc = rpmReadPackageFile(ts, fd, strFullPath.c_str(), &hdr);

				if (rc == RPMRC_NOTFOUND || rc == RPMRC_FAIL) {
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to read package ", strFullPath.c_str(), NULL);
					return -1;
				}
				strTemp = strdup(strFullPath.c_str());
				if(strTemp == NULL)
				{
					headerFree(hdr);
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetOtherArchPackages() function",NULL);
					return -1;			
				}
				m_vectorDepAddedFiles.push_back(strTemp);
				rc = AddInstallElement(ts, hdr, strTemp, 0, 0);
				if (rc > 0)
				{
					headerFree(hdr);
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to add to transaction ", strFullPath.c_str(), NULL);
					return -1;
				}
				else if(rc < 0)
				{
					headerFree(hdr);
					Fclose(fd);
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, strFullPath.c_str(), " not found", NULL);
					return -1;
				}
				else
				{
					nRet=1;
				}
				headerFree(hdr);
				Fclose(fd);
			}
		}
	}
	*/
	return nRet;
}


/*!
@brief Check Same File.

Check Same File

@param vectorFiles const char*.
@param strPackName package name.

@return 	 0 : There is no same file,  -2 : There is same file 
*/
int classRpmEngine::CheckSameFile(vector <const char *> vectorFiles,   const char* strPackName)
{
	bool bFind = false;
	int nRet = 0;
	// Ready add file is ignored by this routine.
	vector<const char *>::iterator it;
	for(it=vectorFiles.begin();it!=vectorFiles.end();it++)
	{
		if (strcmp( strPackName , (*it)) == 0 )
		{
			bFind = true;
		}
	}
	if (bFind == true)
	{
		nRet = -2;
	}

	return nRet;
}


/*!
@brief Check Same File.

Check Same File

@param vectorFiles const char*.
@param strPackName package name.

@return 	 0 : There is no same file,  -2 : There is same file 
*/
int classRpmEngine::CheckSameFile(vector <structAddedFile> vectorFiles,   const char* strPackName)
{
	bool bFind = false;
	int nRet = 0;
	// Ready add file is ignored by this routine.
	vector<structAddedFile>::iterator it;
	for(it=vectorFiles.begin();it!=vectorFiles.end();it++)
	{
		if (strcmp( strPackName , it->strFile) == 0 )
		{
			bFind = true;
		}
	}
	if (bFind == true)
	{
		nRet = -2;
	}

	return nRet;
}


bool classRpmEngine::IsKernelModuleType(string strName)
{
   bool bKernel=false;
   if(strName == "kernel-i386")
   {
       bKernel = true;
   }
    else if(strName == "kernel-i486")
   {
       bKernel = true;
   }
    else if(strName == "kernel-i586")
   {
       bKernel = true;
   }
    else if(strName == "kernel-i686")
   {
       bKernel = true;
   }
    else if(strName == "kernel-athlon")
   {
       bKernel = true;
   }
   else if(strName == "kernel-x86_64")
   {
       bKernel = true;
   }
   else if(strName == "kernel-ppc")
   {
       bKernel = true;
   }
   else if(strName == "kernel-ia64")
   {
       bKernel = true;
   }

    return bKernel;
}



/*!
@brief Check kernel.

Check kernel.
Wheater strName is kernel or not.

@param strName package name.

@return 	 kernel : 0, 
*/
int classRpmEngine::CheckKernel(string strName)
{
	//printf("Check kernel %s\n", strName.c_str());
	bool bKernel=false;
	bool bInstall = false;
	if(strName == "kernel")
	{
		bKernel = true;		
	}	
	else if(strName == "kernel-BOOT")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-smp")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-largesmp")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-hugemem")
	{
		bKernel = true;		
	}	
	else if(strName == "kernel-bigmem")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-enterprise")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-xen")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-PAE")
	{
		bKernel = true;		
	}
	else if(strName == "kernel-kdump")
	{
		bKernel = true;		
	}

	//////////////////////////////////////////
	//redcastle kernel module have to be installed by i option(install).
	else if(strName == "kmod-redcastle")
    {
    	bInstall = true;
    }
	else if(strName == "kmod-redcastle-PAE")
    {
    	bInstall = true;
    }
	//
	//////////////////////////////////////////

	// The pakage that is relative with kernel.
	// These pakcage have to install (or update) to system.
	else if(strName == "kernel-devel")
	{
		bInstall = true;
	}	
	else if(strName == "kernel-kdump-devel")
	{
		bInstall = true;
	}	
	else if(strName == "kernel-xen-devel")
	{
		bInstall = true;
	}
	else if(strName == "kernel-PAE-devel")
	{
		bInstall = true;
	}	
	
	if (bKernel == true) // Install and notify that kernel is installed.
	{		
		return 0; //success		
	}
	else if(bInstall == true) // Only install
	{
		return 2;
	}
	else
	{
		return 1;
	}
}
/*!
@brief Check if file is a obsoleter
inorder to decide whether to update it or not
@return          true : given file is a oblsoleter;  false: not one
*/
bool classRpmEngine::CheckObsoleteToUpdate(string strName, string strVersion, string strRelease, string strArch)
{
//I think this funtion won't work well, since it does not do obsoleteversion check here, am I right?
//and also I think only check name is sufficient, maybe arch is necessay in some cases.
        vector <structFileInfo>::iterator it;
        bool bRet = false;
        for(it=m_vectorObsoleteToUpdate.begin();it!=m_vectorObsoleteToUpdate.end();it++)
        {
                if(strName == it->strName && strVersion == it->strVersion && strRelease == it->strRelease && strArch == it->strArch)
                {
                        bRet = true;
                        break;
                }
        }
        return bRet;
}
/*!
@brief Add Header to ts.

Add Header to ts.

@param ts rpmts.
@param h Header.
@param ts fnpyKey is file path.
@param relocs rpmRelocation.

@return 	 -1 : file is not exist,  return value of rpmtsAddInstallElementkernel()
*/
///////////////////////////////////////////////////////////////
// Add package to ts 
int classRpmEngine::AddInstallElement(rpmts ts, Header h, const fnpyKey key, rpmRelocation *relocs, int nUpgrade)
{
	if (access((const char *)key, F_OK) != 0)
	{
		return -1;
	}
	int nRet;
	const char * strName;
        const char * strVersion;
        const char * strRelease;
        const char * strArch;
        headerGetEntry(h, RPMTAG_NAME, NULL, (void **)&strName, NULL);
        headerGetEntry(h, RPMTAG_VERSION, NULL, (void **)&strVersion, NULL);
        headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **)&strRelease, NULL);
        headerGetEntry(h, RPMTAG_ARCH, NULL, (void **)&strArch, NULL);
	//do I need to free the pointers here?
	OBname[0]=(char *)strName; 
	nRet = CheckKernel(strName);
        if ((nRet == 0)||(nRet == 2))
        {
		nUpgrade = 0;
        }
        else
        {
                if(CheckObsoleteToUpdate(strName, strVersion, strRelease, strArch) == true)
                {	
			nUpgrade = 1;
                }
        }

	if (nUpgrade)
	{
		if (CheckBlacklist((const char *)key) == true)
		{
			m_vectorBlockedUpdatePackages.push_back((const char *)key);
		}

		if (CheckIncmplist((const char *)key) == true)
		{
			m_vectorBlockedUpdatePackages.push_back((const char *)key);
		}

	}

	int rc=0;
		
	rc = rpmtsAddInstallElement(ts, h, key, nUpgrade ,relocs);

	return rc;
}


/*!
@brief change grub.

This isn't used now.
After kernel package install, Change default of /etc/gruf.conf.  

@return 	 0 is ok.
*/
int classRpmEngine::ChangeGrubToDefaultKernel()
{
	char strCommand[1024];
	string strDefault = m_configEnv.GetOption("main", "bootloader");
	if (strDefault == "")
	{
		strDefault = "1";
	}

	//example grubby --set-default=/boot/vmlinuz-2.6.9-34.21AXsmp

	if (m_bKernelInstall == true && strDefault == "0")
	{
		FILE *fp;
		int state;

		char buff[512];
		fp = popen("grubby --default-kernel", "r");
		if (fp == NULL)
		{
			return -1;
		}

		fgets(buff, 512, fp);

		state = pclose(fp);


		snprintf(strCommand, sizeof(strCommand)
			,	"grubby --set-default=/boot/vmlinuz-%s"
			, buff) ;

		popen(strCommand, "r");

		printf("Test grub : %s\n", strCommand);

	}
	return 0;
}

/*!
@brief Read Headers CallBack.

Set callBackFunc for ReadHeaders();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetReadHeadersCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackReadHeaders = callBackFunc;
}

/*!
@brief Run CallBack.

Set callBackFunc for Run();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetRunCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackRun = callBackFunc;
}


/*!
@brief Check CallBack.

Set callBackFunc for Check();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetCheckCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackCheck = callBackFunc;
}

/*!
@brief Check ReadRemoteHeaderInfo.

Set callBackFunc for ReadRemoteHeaderInfo();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetReadRemoteHeaderInfoCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackReadRemoteHeaderInfo = callBackFunc;
}

/*!
@brief Check ReadLocalHeaderInfo.

Set callBackFunc for ReadLocalHeaderInfo();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetReadLocalHeaderInfoCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackReadLocalHeaderInfo = callBackFunc;
}

/*!
@brief Check CreateUpdateInstallList.

Set callBackFunc for CreateUpdateInstallList();  

@param callBackFunc Callback func.
*/
void classRpmEngine::SetCreateUpdateInstallListCallBack(commonCallBackFunc callBackFunc)
{
	g_callBackCreateUpdateInstallList = callBackFunc;
}


/*!
@brief Get file path.

Get full file path from  m_vectorInstallList or m_vectorInstallList.

@param nUpdate 1 is install, 0 is install. 
@param bHeader bHeader.
@param strName Package name.
@param strVersion Package version.
@param strRelease Package release.
@param strArch Package arch.
@param strEpoch Package Epoch.
@param bURL true : url path(http://xxxx.xxxx... ), false : file path on local.  

@return file path.

*/
string classRpmEngine::GetFullPathFile(int nUpdate, bool bHeader, string strName, string strVersion, string strRelease, string strArch, string strEpoch, bool bURL)
{
	string strResult = "";
	bool bType;
	if(nUpdate)
		bType = true;
	else
		bType = false;
	for(int i=0;i<2;i++)
	{
		vector <structFileInfo> vectorFileInfo;
		if (bType == true)
		{
			vectorFileInfo = m_vectorUpdateList;
			bType = !bType;
			
		}
		else 
		{
			vectorFileInfo = m_vectorInstallList;
			bType = !bType;
		}
		vector <structFileInfo>::iterator it;
		for(it=vectorFileInfo.begin();it!=vectorFileInfo.end();it++)
		{
			if(bHeader == false)
			{
				if(it->strName == strName && it->strVersion == strVersion && it->strRelease == strRelease && it->strArch == strArch)
				{
					if(bURL == true) // URL
					{
						strResult = it->strURLFullPath  + "/" +  it->strName + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";
						return strResult;
					}
					else
					{
						strResult = it->strCachePath  + "/packages/" +  it->strName + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";
						return strResult;
					}
				}
			}
			else
			{
				if(it->strName == strName && it->strVersion == strVersion && it->strRelease == strRelease && it->strArch == strArch && it->strEpoch == strEpoch)
				{
					if(bURL == true) // URL
					{
						strResult = it->strCachePath  + "/" + it->strName + "-" + it->strEpoch + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";
						return strResult;
					}
					else
					{
						strResult = it->strCachePath  + "/packages/" + it->strName + "-" + it->strEpoch + "-" + it->strVersion + "-" + it->strRelease + "." + it->strArch + ".rpm";
						return strResult;
					}
				}
			}
		}
	}
	return strResult;
}

/*!
@brief Add dependency package files to ts

Add dependency package files to ts

@param ts rpmts. 
@param ps rpmps.
 

@return 0 or positive is ok.

*/
int classRpmEngine::DealwithDependence(rpmts ts,  rpmps ps)
{
	if(g_callBackCheck)
	g_callBackCheck(0, 0, "", "");

	if(m_nDepCount > 50 ||  m_nDepTotalCount > 100)
	{
		char szMessage[MAX_STRING] ;
		snprintf(szMessage, sizeof(szMessage),"Can not find the required package. DepCount=%d, DepTotalCount=%d",m_nDepCount,m_nDepTotalCount);
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, szMessage,NULL);
		return -9;
	}
	m_nDepTotalCount++;

	int i=0; 
	int result=0; 
	int upgrade=0; 
	int rc=0;
	int ignoreNumber=0;
	char tmp[MAX_STRING],tmp2[MAX_STRING] ;
	char strName[MAX_STRING]; 
	memset(tmp, 0, sizeof(tmp)); 
	memset(tmp2, 0, sizeof(tmp2));
	memset(strName, 0, sizeof(strName));


	struct structRPMInfo *requiredRPM=NULL;

	if (ps == NULL || ps->probs == NULL || ps->numProblems <= 0)
	{

		return 0;
	}

	ignoreNumber = 0;
	for (i = 0; i < ps->numProblems; i++) {
		requiredRPM = (struct structRPMInfo *)malloc(sizeof(struct structRPMInfo)); 
		if(requiredRPM == NULL)
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "malloc error in DealwithDependence() function",NULL);
			return -1;
		}
		memset(requiredRPM, 0, sizeof(structRPMInfo));
		requiredRPM->name = NULL;
		requiredRPM->version = NULL;
		requiredRPM->release = NULL;
		requiredRPM->arch = NULL;
		requiredRPM->group = NULL;
		requiredRPM->size = 0;
		requiredRPM->matchNumber = 0;
		requiredRPM->upgradeFlag = 0;
		requiredRPM->disknum = 0;
		requiredRPM->summary = NULL;
		requiredRPM->shortDesp = NULL;
		requiredRPM->fileList = NULL;
		requiredRPM->requireName = NULL;
		requiredRPM->requireVersion = NULL;
		requiredRPM->provideName = NULL;
		requiredRPM->provideVersion= NULL;
		requiredRPM->requireFileNumber = 0;
		requiredRPM->containFiles = NULL;
		requiredRPM->h = NULL;

		rpmProblem p;
		int j=0;

		p = ps->probs + i;

		if (p->ignoreProblem) {
			ignoreNumber++;
			if(requiredRPM)
			{
				free(requiredRPM);
				requiredRPM=NULL;
			}
			continue;
		}

        // Filter already displayed problems.
		for (j = 0; j < i; j++) {
			if (!SameProblem(p, ps->probs + j))
			//@innerbreak@
			break;
		}

		if (j < i) {//It is same problem
			ignoreNumber++;
			if(requiredRPM)
			{
				free(requiredRPM);
				requiredRPM=NULL;
			}
			continue;
		}
		// update
		if (m_nCommand == 1)
		{
			result = GetRequiredPkgFromRemote(p, requiredRPM);
		}
		else
		{
			result = GetRequiredPkgFromLocal(p, strName);
		}
		string strCurRequired="";
		switch(result) {
			case -1: //not find
				m_nDepCount++;
				break;
			case -2: //find but is in m_selectedPkgRPMInfoList, should be ignore.
			case -3: //find but have been installed, should be ignore.
				//printf("Ignored!\n");
				ignoreNumber++;
				break;
			case -5: //require more space on hard disk
				if(requiredRPM)
				{
					free(requiredRPM);
					requiredRPM=NULL;
				}
			return -7;
				break;
			case 0: //find
				if (m_nCommand == 1)
				{
	
					if(MAX_STRING > m_vectorCacheDirInfo.at(requiredRPM->disknum).strCacheDir.length() + strlen(requiredRPM->name) + strlen(requiredRPM->version) + strlen(requiredRPM->release) + strlen(requiredRPM->arch) + 17)
					{
	
						snprintf(tmp, sizeof(tmp),  "%s/packages/%s-%s-%s.%s.rpm",
							m_vectorCacheDirInfo.at(requiredRPM->disknum).strCacheDir.c_str(),
							requiredRPM->name,
							requiredRPM->version,
							requiredRPM->release,
							requiredRPM->arch);
					}
					else
					{
						FreeRequiredRPM(requiredRPM);
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "The length of file path error in DealwithDependence() function",NULL);
						return -1;
					}
					if(MAX_STRING > m_vectorCacheDirInfo.at(requiredRPM->disknum).strURL.length() + strlen(requiredRPM->name) + strlen(requiredRPM->version) + strlen(requiredRPM->release) + strlen(requiredRPM->arch) + 8)
					{
						snprintf(tmp2, sizeof(tmp2),  "%s/%s-%s-%s.%s.rpm",
							m_vectorCacheDirInfo.at(requiredRPM->disknum).strURL.c_str(),
							requiredRPM->name,
							requiredRPM->version,
							requiredRPM->release,
							requiredRPM->arch);
					}
					else
					{
						FreeRequiredRPM(requiredRPM);
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "The length of file path error in DealwithDependence() function",NULL);
						return -1;
					}
	
					if (AddFile(tmp, REQDEP, requiredRPM->upgradeFlag) < 0)
					{
						break;
					}
	
					m_Network->AddPackage(tmp, tmp2);
	
					int nRet = m_Network->GetPackages();
					if (nRet != NETWORK_RETOK)
					{
						FreeRequiredRPM(requiredRPM);	
						return -10;
					}
	
					m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME, "Find required packages : ",tmp ,NULL);
	
					upgrade = requiredRPM->upgradeFlag;
					char * strTemp = strdup(tmp);
					if(strTemp == NULL)
					{
						FreeRequiredRPM(requiredRPM);
						m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in DealwithDependence() function",NULL);
						return -1;
					}
					m_vectorDepAddedFiles.push_back(strTemp);
					rc = AddInstallElement(ts, requiredRPM->h, strTemp, 0, upgrade);
					#if defined(__x86_64__) || defined(__PPC__) || defined(__ia64__)
					if (GetOtherArchPackages(ts, requiredRPM->h, upgrade) < 0)
					{
						FreeRequiredRPM(requiredRPM);
						return -10;
					}
					#endif
	
				}
				else
				{
					bool bFindRemove=false;
					vector<structAddedFile>::iterator it;
					vector<structAddedFile> vectorRemove;
					vectorRemove = GetAddedFile(REMOVE, vectorRemove);
					for(it=vectorRemove.begin();it!=vectorRemove.end();it++)
					{
						string  strName1, strVer1, strRel1, strArch1;
						stripNVRA(it->strFile, &strName1, &strVer1, &strRel1, &strArch1);
						if ( strcmp(strName, strName1.c_str()) == 0)
						{
							bFindRemove=true;
							break;
						}
					}
	
					if (bFindRemove == false)
					{
						char * strTemp = strdup(strName);
						if(strTemp == NULL)
						{
							if(requiredRPM)
							{
								free(requiredRPM);
								requiredRPM=NULL;
							}
							m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in DealwithDependence() function",NULL);
							return -1;
						}
	
						AddFile(strTemp, REQDEP, 0);
						AddRemovePackage(ts, strTemp);
					}
				}
	
				break;
		} // end switch.
		
		if (m_nCommand == 1 && result == 0)
		{
			FreeRequiredRPM(requiredRPM);
		}
		else
		{
			if(requiredRPM)
			{
				free(requiredRPM);
				requiredRPM=NULL;
			}
		}
	}
	//rpmtsClean(ts);
	rpmpsFree(ps);
	ps = NULL;
	
	/// In DealwithDependence Function
	rc = rpmtsCheck(ts);	

	ps = rpmtsProblems(ts);
	if (rc || ps->numProblems > 0) {
		// Disk space is not enough error        
		result = DealwithDependence(ts, ps);
		return result;
	} else {
		//rpmtsClean(ts);
		rpmpsFree(ps);
		ps = NULL;  
		vector<structAddedFile> vectorInstall;	
		vector<structAddedFile> vectorRemove;
		vectorInstall = GetAddedFile(UPDATE, vectorInstall);
		vectorRemove = GetAddedFile(REMOVE, vectorRemove);

		//return vectorInstall.size() + vectorRemove.size();
		return 0;
	}

}

void classRpmEngine::FreeRequiredRPM(struct structRPMInfo * requiredRPM)
{
	if (requiredRPM == NULL) return;
	if(requiredRPM->name)
	{
		free(requiredRPM->name);
		requiredRPM->name=NULL;
	}
	if(requiredRPM->version)
	{
		free(requiredRPM->version);
		requiredRPM->version=NULL;
	}
	if(requiredRPM->release)
	{
		free(requiredRPM->release);
		requiredRPM->release=NULL;
	}
	if(requiredRPM->arch)
	{
		free(requiredRPM->arch);
		requiredRPM->arch=NULL;
	}
	if(requiredRPM->group)
	{
		free(requiredRPM->group);
		requiredRPM->group=NULL;
	}
	if(requiredRPM->summary)
	{
		free(requiredRPM->summary);
		requiredRPM->summary=NULL;
	}
	if(requiredRPM->shortDesp)
	{
		free(requiredRPM->shortDesp);
		requiredRPM->shortDesp=NULL;
	}
	if(requiredRPM->h)
	{
		headerFree(requiredRPM->h);
		requiredRPM->h=NULL;
	}
	if(requiredRPM)
	{
		free(requiredRPM);
		requiredRPM=NULL;
	}
}

/*!
 * @brief You can get the count of incompatible packages.
 *  
 * @return the count of incompatible count.
 */
int classRpmEngine::GetIncompatibleCount()
{
	return m_nIncompatible;
}


vector <string> classRpmEngine::GetIncompatiblePackages()
{
	return m_vectorIncompatibleUpdatePackages;
}

/*!
 * @brief You can get udate or install available packages count.
 *  
 * @return available packages count.
 * 
 * This function can work after you have executed CreateUpdateInstallList() fuction 
 */
int classRpmEngine::GetUpdateAvailableCount()
{
	return m_nUpdateAvailableCount;
}

/*!
@brief Check same problem.

Let me know same problem.

@param ap rpmProblem. 
@param bp rpmProblem.
 

@return 0 : no same problem.

*/
int classRpmEngine::SameProblem(const rpmProblem ap, const rpmProblem bp)
{
    if (ap->type != bp->type)
    	return 1;
	if (ap->pkgNEVR) {
        if (bp->pkgNEVR && strcmp(ap->pkgNEVR, bp->pkgNEVR))
            return 1;
	}
    if (ap->altNEVR) {
        if (bp->altNEVR && strcmp(ap->altNEVR, bp->altNEVR))
            return 1;
    }
    if (ap->str1) {
        if (bp->str1 && strcmp(ap->str1, bp->str1))
            return 1;
    }
    if (ap->ulong1 != bp->ulong1)
        return 1;

    return 0;
}


/*!
@brief Get required packages from remote header info.

Get required package rpm info from remote header.info

@param prob rpmProblem. 
@param result(output) structRPMInfo is result.
 
@return 0 is ok

*/
int classRpmEngine::GetRequiredPkgFromRemote(rpmProblem prob, struct structRPMInfo *result)
{
	char *name=NULL; char*version=NULL;
	char *compare=NULL;
    char buf[MAX_STRING]; 
    char szDepMsg[MAX_STRING];
    memset(buf, 0, sizeof(buf)); 
    char *ptr=NULL;
	char *ptr2=NULL;
    int type=0; 
    int rc=0;
    string  strName1, strVer1, strRel1, strArch1;    
    string  strNamePkg, strVerPkg, strRelPkg, strArchPkg;
    const char * pkgNEVR = (prob->pkgNEVR ? prob->pkgNEVR : "?pkgNEVR?");
    const char * altNEVR = (prob->altNEVR ? prob->altNEVR : "? ?altNEVR?");
    
    if(prob->ulong1==0)   
	{	
    	altNEVR = (prob->pkgNEVR ? prob->pkgNEVR : "?pkgNEVR?");    		    		
    	stripNVRA(altNEVR, &strName1, &strVer1, &strRel1, &strArch1);
    	altNEVR = strName1.c_str();
	}    
    else
	{
    	altNEVR = (prob->altNEVR ? prob->altNEVR : "? ?altNEVR?");    		    		
	}
    

    //printf("goto here!\n");
    switch (prob->type) {
	    case RPMPROB_DISKNODES :
	    case RPMPROB_DISKSPACE :
	        return -5;
	        break;	
	    case RPMPROB_CONFLICT:
	    case RPMPROB_REQUIRES:
	    	if(prob->ulong1==0) 
	    	{
	    		snprintf(buf, sizeof(buf),"%s", (altNEVR));
	    	}
	    	else
	    	{
	    		snprintf(buf, sizeof(buf),"%s", (altNEVR + 2));
	    	}
	        ptr = strchr(buf, ' ');
	        
	        if(buf[0] == '?')
	            break;
	
	        if(ptr == NULL) 
	        {
	            if(buf[0] == '/') {
	                type = 1; // Require a file/path name
	            } else {
	                type = 2; // Require a libname or rpmname without version
			    }
			    name = strdup(buf);
				if(name == NULL)
				{
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetRequiredPkgFromRemote() function",NULL);
					return -1;
				}
				version = NULL;
	        }
	        else 
	        {
	            *ptr = '\0';
	            name = strdup(buf);
				if(name == NULL)
	            {
	            	m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetRequiredPkgFromRemote() function",NULL);
		            return -1;
	            }
				compare = strdup(ptr + 1);
				if(compare == NULL)
	            {
					free(name);
					name=NULL;
	            	m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetRequiredPkgFromRemote() function",NULL);
		            return -1;
	            }
				ptr2 = strchr(compare + 1, ' ');
				*ptr2 = '\0';
	            ptr = strchr(ptr + 1, ' ');
	            if(ptr == NULL) // not happen!
				{
					free(name);
					name=NULL;
					free(compare);
					compare=NULL;
	                break;
				}
	
	            version = strdup(ptr + 1);
				if(version== NULL)
				{
				    free(name);
					name=NULL;
					free(compare);
					compare=NULL;
				    m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetRequiredPkgFromRemote() function",NULL);
				    return -1;
				}
				type = 3; // Require a libname or rpmname with version
	       	}

	        snprintf(szDepMsg, sizeof(szDepMsg), "PackageNEVR:%s,  AlterNEVR:%s",  pkgNEVR, altNEVR);
	    	if(prob->type == RPMPROB_CONFLICT)	    	
	    		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME,"RPMPROB_CONFLICT",szDepMsg,NULL);
	    	else
	    		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME,"RPMPROB_REQUIRES",szDepMsg,NULL);
	    	
	    	stripNVRA(pkgNEVR, &strNamePkg, &strVerPkg, &strRelPkg, &strArchPkg);     
			rc = GetFromRPMInfoListFromRemote(m_vectorRPMInfo, name, version, (char *)strArchPkg.c_str(), type, compare, result);
	        free(name);
			name=NULL;
	        free(version);
			version=NULL;
	        free(compare);
			compare=NULL;
			if(rc < 0)
			{
				return -1;
			}
	        else if (rc == 0) 
			{
	        	if(prob->type == RPMPROB_CONFLICT)
	        	{
	                return -3;  //ignore
	            }
	            else
	            {
	                return -1;  //can not find
	            }
	        }
	        else
	        {
	            /*rc = CheckInSelectedPkgList(result);
	            if (rc == 1) //get the same one
	            return -2;
	            */
	        }
	        return 0;
	        break;
	    case RPMPROB_BADARCH:
	        break;
	    case RPMPROB_BADOS:
	        break;
	    case RPMPROB_PKG_INSTALLED:
	        break;
	    case RPMPROB_BADRELOCATE:
	        break;	
	    case RPMPROB_NEW_FILE_CONFLICT:
	        break;	
	    case RPMPROB_FILE_CONFLICT:
	        break;
	    case RPMPROB_OLDPACKAGE:
	        break;
	    case RPMPROB_BADPRETRANS:
	        break;
	    default:
	        break;
    } // end of switch (prob->type)
    return -3;
}



/*!
@brief Get required packages from local rpm DB.

Get required packages from local rpm DB.

@param prob rpmProblem. 
@param result(output) result is package name.
 
@return 0 is ok

*/
int classRpmEngine::GetRequiredPkgFromLocal(rpmProblem prob, char *result)
{
	char*version=NULL;
	char buf[MAX_STRING]; 
	memset(buf, 0, sizeof(buf)); 
	char *ptr=NULL;
	int type=0;
	
	const char * pkgNEVR = (prob->pkgNEVR ? prob->pkgNEVR : "?pkgNEVR?");
	const char * altNEVR = (prob->altNEVR ? prob->altNEVR : "? ?altNEVR?");
    
    switch (prob->type) {
	    case RPMPROB_DISKNODES :
	    case RPMPROB_DISKSPACE :
	    	return -5;
	    	break;
	
	    case RPMPROB_CONFLICT:
	    case RPMPROB_REQUIRES:
	    	snprintf(buf, sizeof(buf),"%s", (pkgNEVR));
	    	ptr = strchr(buf, ' ');
	
	    	if(buf[0] == '?')
	    		break;
	
	    	if(ptr == NULL) 
	    	{
	    		if(buf[0] == '/') {
	    			type = 1; // Require a file/path name
	    		} else {
	    			type = 2; // Require a libname or rpmname without version
	    		}
				string  strName1, strVer1, strRel1, strArch1;
				stripNVRA(buf, &strName1, &strVer1, &strRel1, &strArch1);
				strcpy(result ,buf);		
				version = NULL;
	    	} 
	    	else
	    	{
				*ptr = '\0';
				strcpy(result , buf);
				ptr = strchr(ptr + 1, ' ');
				if(ptr == NULL) // not happen!
					break;		
				version = strdup(ptr + 1);
				if(version == NULL)
				{
					m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in GetRequiredPkgFromLocal() function", NULL);
					return -1;
				}
				type = 3; // Require a libname or rpmname with version
				free(version);
				version = NULL;
	        }	
	    	return 0;
	    	break;
	    case RPMPROB_BADARCH:
	    	break;
	    case RPMPROB_BADOS:
	        break;
	    case RPMPROB_PKG_INSTALLED:
	        break;
	    case RPMPROB_BADRELOCATE:
	        break;	
	    case RPMPROB_NEW_FILE_CONFLICT:
	        break;	
	    case RPMPROB_FILE_CONFLICT:
	        break;
	    case RPMPROB_OLDPACKAGE:
	        break;
	    case RPMPROB_BADPRETRANS:
	        break;
	    default:
	        break;
    } // end of switch (prob->type)
    return -3;
}

/*!
@bief Get Kernle type from version that is made by kmod script.

Get Kernle type from version that is made by kmod script.

@param strVersion
@return "" : No kernel typpe.   Not "" : smp, hugemem, bigmem, ....xen, PAE.

*/
string classRpmEngine::GetKernelType(string strVersion)
{
	string strRet="";
	if(strVersion.find("BOOT") == strVersion.length()-((string)"BOOT").length() )
	{
		strRet = "BOOT";
	}
	else if(strVersion.find("smp") == strVersion.length()-((string)"smp").length() )
	{
		strRet = "smp";
  	}
	else if(strVersion.find("largesmp") == strVersion.length()-((string)"largesmp").length() )
	{
		strRet = "largesmp";
	}
	else if(strVersion.find("hugemem") == strVersion.length()-((string)"hugemem").length() )
	{
		strRet = "hugemem";
	}
	else if(strVersion.find("bigmem") == strVersion.length()-((string)"bigmem").length() )
	{
		strRet = "bigmem";
	}
	else if(strVersion.find("enterprise") == strVersion.length()-((string)"enterprise").length() )
	{
		strRet = "enterprise";
	}
	else if(strVersion.find("xen") == strVersion.length()-((string)"xen").length() )
	{
		strRet = "xen";
	}
	else if(strVersion.find("PAE") == strVersion.length()-((string)"PAE").length() )
	{
	   strRet = "PAE";
	}
	return strRet;
}
/*!

/*!
@brief Get required package rpm info from rpm info that readed from header files *.hdr

Find required package rpm info from rpm info that readed from header files *.hdr

@param vectorRPMInfo structRPMInfo type vector. 
@param name package name.
@param version package version.
@param type 1: Require a file/path name,   2: Require a libname or rpmname without version,  3:Require a libname or rpmname with version.
@param result(output) founed structRPMInfo.
 
@return 0 is ok

*/
int classRpmEngine::GetFromRPMInfoListFromRemote(vector <structRPMInfo*> vectorRPMInfo, char *name, char *version, char *arch,int type, char *compare, struct structRPMInfo *result)
{	
	string strTemp, strName, strVersion, strKernelType;
	if(type == 3)
	{
		strVersion = version;
        if(strVersion.rfind(":") != string::npos && strVersion.length() > strVersion.rfind(":"))
        {
            strVersion = strVersion.substr(strVersion.rfind(":") + 1);
        }
 
        //The kernel is required. then we have change name and version.
        if(IsKernelModuleType(name) == true)
        {
        	strTemp = name;
        	if(strTemp.find("-") != string::npos)
            {
        		strName = strTemp.substr(0, strTemp.find("-"));
            } 
            strKernelType = GetKernelType(strVersion);
            if(strKernelType != "")
            {
                strVersion = strVersion.substr(0, strVersion.find(strKernelType));
                strName = strName + "-" + strKernelType;
            }
        }
        else
        {
            strName = name;
        }
	}
	else
    {
		strName = name;
        strVersion = "";
    }
 
	struct structFileList *fileListItem;
	int i=0; 
    int matchFlag=0;
        
    matchFlag = 0;
    vector <structRPMInfo*>::iterator it;    
    
	for(it=vectorRPMInfo.begin();it!=vectorRPMInfo.end();it++)
	{
#ifdef DEBUG				
		char szMessage[MAX_STRING] ;		
		snprintf(szMessage, sizeof(szMessage),"CheckArch([%s], [%s])",arch, (*it)->arch);
		m_Logger->WriteLog_char(DEBUG_LOG, MYSELF_NAME, szMessage,NULL);
#endif
		//Check only same arch.
#if defined(__x86_64__)
       if(!CheckArch(arch ,(*it)->arch) && !strcmp(name, (*it)->name)) {
           if (!strncmp(arch, "x86_64", 6)) continue;
       }
#else                  
       if(!CheckArch(arch ,(*it)->arch)) continue;
#endif

		switch(type) {
			// Old method.
	        /*case 1: // just need a file path name
	            fileListItem = (*it)->containFiles;
	            if (fileListItem != NULL) {
	                for(i = 0; fileListItem->fileList[i]; i++) {
	                    if(strcmp(name, fileListItem->fileList[i]) == 0) {
	                        CopyData(*it, result);
	                        matchFlag = 1;
	                        break; // break from for(i = 0; fileListItem->fileList[i]; i++)
	                    }
	                } //end of for(i = 0; fileListItem->fileList[i]; i++)
	            } //end of  if (fileListItem != NULL)
	 
	            break;
	            */
	        case 1: // just need a file path name
	        	for(i = 0; (*it)->provideName[i]; i++) {
	        		if(strName.find((*it)->provideName[i]) != string::npos) {
	        			if(CopyData(*it, result) == false)
	        			{
	        				return -1;
	        			}
	        			matchFlag = 1;
	        			break;
	                }
	            } 
	            break;
	            
	        case 2: // need a libname or rpmname without version
	        	for(i = 0; (*it)->provideName[i]; i++) {
	        		if(strName == (string)(*it)->provideName[i]) {
	        			if(CopyData(*it, result) == false)
	        			{
	        				return -1;
	        			}
	        			matchFlag = 1;
	        			break;
	        		}
	        	} 
	            break;
	        case 3: // need a libname or rpmname with version
	            for(i = 0; (*it)->provideName[i]; i++) {
	                if(strName == (string)(*it)->provideName[i]) {
	                    string strProvideVersion = (*it)->provideVersion[i];
	                    if(strProvideVersion.rfind(":") != string::npos && strProvideVersion.length() > strProvideVersion.rfind(":"))
	                    {
	                        strProvideVersion = strProvideVersion.substr(strProvideVersion.rfind(":") + 1);
	                    }
	 					
						if(string(compare).find(">") != string::npos && string(compare).find("=") != string::npos)
	                    {
	                        if(rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) < 0 || rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) == 0) {
	                            if(CopyData((structRPMInfo*)*it, result) == false)
								{
									return -1;
								}
	                            matchFlag = 1;
	                            break;
	                        }
	 
	                    } 
	                    else if(string(compare).find("<") != string::npos && string(compare).find("=") != string::npos)
	                    {
	                        if(rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) > 0 || rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) == 0) {
	                             if(CopyData((structRPMInfo*)*it, result) == false)
								 {
								 	return -1;
								 }
	                             matchFlag = 1;
	                             break;
	                         }
	                    }
	                    else if(string(compare).find(">") != string::npos)
	                    {
	                        if(rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) < 0) {
	                            if(CopyData((structRPMInfo*)*it, result) == false)
								{
									return -1;
								}
	                            matchFlag = 1;
	                            break;
	                        }
	 
	                    }
	 
	                    else if(string(compare).find("<") != string::npos)
	                    {
	                        if(rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) > 0) {
	                             if(CopyData((structRPMInfo*)*it, result) == false)
								 {
								 	return -1;
								 }
	                             matchFlag = 1;
	                             break;
	                         }
	                    }
	                    
	                    else if(string(compare).find("=") != string::npos)
	                    {
	                        if( (rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) == 0 || rpmvercmp(strVersion.c_str(), strProvideVersion.c_str()) < 0) && rpmvercmp(strVersion.c_str(), (strVersion+".1").c_str())<0) {
	                            if(CopyData((structRPMInfo*)*it, result) == false)
								{
									return -1;
								}
	                            matchFlag = 1;
	                            break;
	                        }
	                    }
	                   /* 
	                    //if(strVersion == (string)(*it)->provideVersion[i]) {
	                        CopyData((structRPMInfo*)*it, result);
	                        matchFlag = 1;
	                        break;
	                    //}
						*/
	                }
	            }
	 
	            break;
	        default:
	            break;
		} 
		if (matchFlag == 1)
			return 1; 
	}
    return 0;
}


/*!
@brief Get required package rpm info from rpm info that readed Local rpm DB

Find required package rpm info from rpm info that readed from Local rpm DB
 
@param name package name.
@param version package version.
@param type 1: Require a file/path name,   2: Require a libname or rpmname without version,  3:Require a libname or rpmname with version.
@param result(output) founed structRPMInfo.
 
@return 0 is ok

*/
int classRpmEngine::GetFromRPMInfoListFromLocal(char *name, char *version, int type, struct structRPMInfo *result)
{	
    struct structFileList *fileListItem;
    int i=0; 
    int matchFlag=0;    
    matchFlag = 0;
    Header hdr;
	//vector <structRPMInfo*>::iterator it;
	rpmdbMatchIterator mi;        
	classBlockSignal blocksignal(SIGHUP); //Util this class is destroyed, SIGHUP will be blocked.
	mi = rpmtsInitIterator(m_rpmTs, (rpmTag)RPMDBI_PACKAGES, NULL, 0);
		
	while ((hdr = rpmdbNextIterator(mi)) != NULL)
	{				
		structRPMInfo * rpmInfo = GetHeaderInfo(hdr, 9, 0);
		if(rpmInfo == NULL);
		{
			m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "GetHeaderInfo() error", NULL);
			rpmdbFreeIterator(mi);
			return -1;	
		}			
		switch(type) {
			case 1: // just need a file path name
				fileListItem = rpmInfo->containFiles;
				if (fileListItem != NULL) {
				    for(i = 0; fileListItem->fileList[i]; i++) {
				    	if(strcmp(name, fileListItem->fileList[i]) == 0) {
				    		if(CopyData(rpmInfo, result) == false)
				    		{
				    			rpmdbFreeIterator(mi);
				    			return -1;
				    		}
				    		matchFlag = 1;
				    		break; // break from for(i = 0; fileListItem->fileList[i]; i++)
				        }
				    } //end of for(i = 0; fileListItem->fileList[i]; i++)
				} //end of  if (fileListItem != NULL) 
		
				break;
			case 2: // need a libname or rpmname without version
				for(i = 0; (rpmInfo)->provideName[i]; i++) {
					if(strcmp(name, (rpmInfo)->provideName[i]) == 0) {
						if(CopyData(rpmInfo, result) == false)
						{
							rpmdbFreeIterator(mi);
							return -1;
						}
						matchFlag = 1;
						break; //break from for(i = 0; listPtr->data->provideName[i], i++)
				    }
				} // end of for(i = 0; listPtr->data->provideName[i], i++)			
			    break;
			case 3: // need a libname or rpmname with version
				for(i = 0; (rpmInfo)->provideName[i]; i++) {					
					if(strcmp(name, (rpmInfo)->provideName[i]) == 0) {
						//if(strcmp(version, listPtr->data->provideVersion[i]) == 0) {
						if(CopyData((structRPMInfo*)rpmInfo, result) == false)
						{
							rpmdbFreeIterator(mi);
							return -1;
						}
						matchFlag = 1;
						break; //break from for(i = 0; listPtr->data->provideName[i], i++)
						//}
				    }
				} // end of for(i = 0; listPtr->data->provideName[i], i++)
		
				break;
			default:
				break;
		}

		if (matchFlag == 1)
		{
			rpmdbFreeIterator(mi);
			return 1;
		}
	
	}		
	rpmdbFreeIterator(mi);
	return 0;
}

/*!
@brief Copy structRPMInfo. 

Copy structRPMInfo. 
dest must be memory free.
 
@param src source structRPMInfo.
@param dest destination structRPMInfo.


*/
bool classRpmEngine::CopyData(struct structRPMInfo *src, struct structRPMInfo *dest)
{
	if (src == NULL || dest == NULL) return false;

	if ((dest->name = strdup(src->name)) == NULL)		goto cpfail;
	if ((dest->version = strdup(src->version)) == NULL)	goto cpfail;
	if ((dest->release = strdup(src->release)) == NULL)	goto cpfail;
	if ((dest->arch = strdup(src->arch)) == NULL)		goto cpfail;
	if ((dest->group = strdup(src->group)) == NULL)		goto cpfail;
	dest->size = src->size;
	dest->matchNumber = src->matchNumber;
	if ((dest->summary = strdup(src->summary)) == NULL)	goto cpfail;
	if ((dest->shortDesp = strdup(src->shortDesp)) == NULL)	goto cpfail;
	dest->upgradeFlag = src->upgradeFlag;
	dest->disknum = src->disknum;
	if ((dest->h = headerCopy(src->h)) == NULL)		goto cpfail;
	return true;

cpfail:
	if (dest->name)		free(dest->name);
	if (dest->version)	free(dest->version);
	if (dest->release)	free(dest->release);
	if (dest->arch)		free(dest->arch);
	if (dest->group)	free(dest->group);
	if (dest->summary)	free(dest->summary);
	if (dest->shortDesp)	free(dest->shortDesp);
	if (dest->h)		headerFree(dest->h);
	memset(dest, 0x00, sizeof(struct structRPMInfo));

	m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "strdup error in CopyData() function", NULL);
	return false;
}

////////////////////////////////////////////////////////////////
// Test
int classRpmEngine::Test()
{  
	return 0;
}


/*!
@brief Install or Update or Erase packages

Really do install or Update or Erase packages(selected and dep added files) 
 
@param bForce if value is true then install by --nodeps --force.

@return 0 is ok.  -6 : confilct  

*/
int classRpmEngine::Run(bool bForce)
{		
	rpmps probs;
	int probFilter = 0;
	int notifyFlags = 0;
	int tsFlags = 0;
	int rc;

	if( m_bSelfUpdate == false && bForce == false )
	{
		rc = rpmtsCheck(m_rpmTs);
		probs = rpmtsProblems(m_rpmTs);

		if (rc || probs->numProblems > 0)
		{
			rpmpsPrint(NULL, probs);
			WriteDepLog(probs);
			rpmpsFree(probs);
			return -3;
		}
		rpmpsFree(probs);
	}

	// Create ordering for the transaction
	rc = rpmtsOrder(m_rpmTs);
	if (rc > 0)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to ordering transaction", NULL);
		rpmpsFree(probs);
		return -4;
	}

	rpmtsClean(m_rpmTs);

	// Set callback routine & flags, for example -vh
	notifyFlags |= INSTALL_LABEL | INSTALL_HASH;

	g_nRemoveCount = 0;
	vector<structAddedFile> vectorRemove;
	vectorRemove = GetAddedFile(REMOVE, vectorRemove);
	g_nRemoveSelectedPackages=vectorRemove.size();
	rpmtsSetNotifyCallback(m_rpmTs, myRpmShowProgress, (void *)((long)notifyFlags));

	g_nTotalRemoveCountByUpdate = m_vectorUpdateList.size();

	// Set transaction flags and run the actual transaction
	rpmtsSetFlags(m_rpmTs, (rpmtransFlags)(rpmtsFlags(m_rpmTs) | tsFlags));
	
	//nodeps force option. (when self update or force option)
	if(m_bSelfUpdate == true || bForce) 
	{
		probFilter = (rpmprobFilterFlags)(rpmprobFilterFlags_e)(RPMPROB_FILTER_REPLACEPKG|RPMPROB_FILTER_REPLACENEWFILES|RPMPROB_FILTER_REPLACEOLDFILES|RPMPROB_FILTER_OLDPACKAGE | RPMPROB_FILTER_FORCERELOCATE);
	}
	else
	{
		probFilter = 0;
	}

	g_bNoSpace = false;

	// Remove the message from each RPM file
	FD_t ffd = Fopen(NULL_FILE,"w.ufdio");
	if (ffd == NULL) {
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME, "failed to open ", NULL_FILE, NULL);
		return -1;
	}
	rpmtsSetScriptFd(m_rpmTs, ffd);
		
	rc = rpmtsRun(m_rpmTs, NULL, (rpmprobFilterFlags)probFilter);
	
	signal(SIGHUP, sig_hup);
			
	if(ffd != NULL){
		Fclose(ffd);
	}

	if (g_bNoSpace == true)
	{
		return -11;
	}
	oldRpmcliProgressCurrent = 0;
	if(g_callBackRun)
		g_callBackRun(100, 100, "Remove finished....", "");  
	
	// Check for results ..
	rpmcliHashesCurrent=0;
	rpmcliProgressCurrent=0;
	rpmcliPackagesTotal=0;  
	probs = rpmtsProblems(m_rpmTs);

	if (rc != 0 && probs->numProblems > 0)
	{
		rpmpsPrint(stderr, probs);
		WriteDepLog(probs);
		rpmpsFree(probs);

		// update, install
		if (m_nCommand == 1)
		{
			if(probs->probs->type == RPMPROB_CONFLICT || probs->probs->type == RPMPROB_NEW_FILE_CONFLICT
								||probs->probs->type == RPMPROB_FILE_CONFLICT  )
			{
				return -6;
			}
		}
		return -5;
	}
	//printf("xxxxxxx rc = %d, probs count = %d\n", rc, probs->numProblems);
	else if(rc == -1)
	{
		m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME,"Error is occured in rpmtsRun()", NULL);
		rpmpsFree(probs);
		return -1;
	}
	else  // success
	{
		if (m_configEnv.GetOption("main", "removepackages") == "true")
		{
			if(DeleteDownPacks() == false)
			{				
				m_Logger->WriteLog_char(ERROR_LOG, MYSELF_NAME,"Error is occured in DeleteDownPacks()", NULL);
				rpmpsFree(probs);
				return -1;
			}
		}
		rpmtsClean(m_rpmTs);
	}
	rpmpsFree(probs);
	return 0;
}


//! @brief Delete downloaded rpm files.
bool classRpmEngine::DeleteDownPacks()
{
	vector<string> vectorSections;
	vector<structCacheDirInfo>::iterator it;
	string strTemp;
	string strCacheDir;
	string strPath;
	vectorSections = m_configEnv.GetSections();	
	strCacheDir = m_configEnv.GetOption("main", "cachedir");
	strCacheDir = m_configEnv.StripRString(strCacheDir, '/');
	if (strCacheDir.empty() == false)
	{
		strPath = strCacheDir;
	}		
	else
	{
		return false;
	}
	for(it=m_vectorCacheDirInfo.begin();it!=m_vectorCacheDirInfo.end();it++)
	{			
		strTemp = strPath + "/" + it->strName + "/" + "packages" ;

		struct dirent *item;
		DIR *dp;
		string strFileName;
		string strFilePath;
		dp = opendir(strTemp.c_str());
		if (dp != NULL)
	    {
		    for(;;)
			{
				item = readdir(dp);
				if (item == NULL)
					break;
				strFileName = item->d_name;	            
				if( strFileName.find(".rpm") == string::npos)
					continue;
				strFilePath = strTemp + "/" + strFileName;			
				if (access(strFilePath.c_str(), F_OK ) == 0)
				{
					remove(strFilePath.c_str());
				}
			}
		    closedir(dp);
	    }		
	}
	return true;
}


//! @brief Callback function for rpmtsRun. Show progress.
void * classRpmEngine::myRpmShowProgress(/*@null@*/ const void * arg,
            const rpmCallbackType what,
            const unsigned long amount,
            const unsigned long total,
            /*@null@*/ fnpyKey key,
            /*@null@*/ void * data)
    /*@globals rpmcliHashesCurrent, rpmcliProgressCurrent, rpmcliProgressTotal,  fileSystem @*/
    /*@modifies rpmcliHashesCurrent, rpmcliProgressCurrent, rpmcliProgressTotal,  fileSystem @*/
{	  
	int nPercent;
	/*@-abstract -castexpose @*/
	Header h = (Header) arg;
	/*@=abstract =castexpose @*/
	char * s;
	int flags = (int) ((long)data);
	void * rc = NULL;
	/*@-abstract -assignexpose @*/
	const char * filename = (const char *)key;
    
    
	/*@=abstract =assignexpose @*/
	static FD_t fd = NULL;
	int xx;
	
	switch (what) {
		case RPMCALLBACK_INST_OPEN_FILE:
			/*@-boundsread@*/
			if (filename == NULL || filename[0] == '\0')
				return NULL;
			/*@=boundsread@*/
			fd = Fopen(filename, "r.ufdio");
			/*@-type@*/ /* FIX: still necessary? */
			if (fd == NULL || Ferror(fd)) {
				rpmError(RPMERR_OPEN, ("open of %s failed: %s\n"), filename,
						Fstrerror(fd));
					if (fd != NULL) {
						xx = Fclose(fd);
						fd = NULL;
					}
			} else
				fd = fdLink(fd, "persist (showProgress)");
			/*@=type@*/
			/*@+voidabstract@*/
			return (void *)fd;
			/*@=voidabstract@*/
			/*@notreached@*/ 
			break;
	
	    case RPMCALLBACK_INST_CLOSE_FILE:
	    	/*@-type@*/ /* FIX: still necessary? */
	    	fd = fdFree(fd, "persist (showProgress)");
	    	/*@=type@*/
	    	if (fd != NULL) {
	    		xx = Fclose(fd);
	    		fd = NULL;
	    	}
	    	break;
	
	    case RPMCALLBACK_INST_START:
		    rpmcliHashesCurrent = 0;
		    if (h == NULL || !(flags & INSTALL_LABEL)) {
		    	break;
		    }
		    /* @todo Remove headerSprintf() on a progress callback. */
		    if (flags & INSTALL_HASH) {
		        s = headerSprintf(h, "%{NAME}",
		                rpmTagTable, rpmHeaderFormats, NULL);
		        if (isatty (STDOUT_FILENO))
		        {
		        	if(g_callBackRun) {
		        		g_callBackRun(0, 0, "Start RPM callback install...", filename);
		        	}
		        	else {
		        		fprintf(stdout, "%-28.28s", s);
		        	}
		        }
		        (void) fflush(stdout);
		        s = (char *)_free(s);
		    } else {
		        s = headerSprintf(h, "%{NAME}-%{VERSION}-%{RELEASE}", rpmTagTable, rpmHeaderFormats, NULL);		        
		        if(g_callBackRun) {
		        	g_callBackRun(0, 0, "Start RPM callback install...", filename);
		        }		        
		        s = (char *)_free(s);
		    }
		    break;
	
	    case RPMCALLBACK_TRANS_PROGRESS:
	    case RPMCALLBACK_INST_PROGRESS:
	    	/*@+relaxtypes@*/			
			nPercent = (int) ((amount)* 100. / total);
			if (flags & INSTALL_PERCENT)
	    	{	                
				if (rpmcliPackagesTotal == 0) // remove
				{
					if(g_callBackRun) {
						g_callBackRun(nPercent, nPercent, "Progress....", filename);
					}
				}
				else {
					if (oldRpmcliProgressCurrent != 0 && oldRpmcliProgressCurrent != rpmcliProgressCurrent) {
						if(g_callBackRun) {
							g_callBackRun(int(((rpmcliProgressCurrent)*100. + nPercent)/rpmcliPackagesTotal), 100, "Installation progress....", filename);
	          			}
					}
					else
	          		{
						oldRpmcliProgressCurrent = rpmcliProgressCurrent;
						if(g_callBackRun) {
							g_callBackRun(int(((rpmcliProgressCurrent)*100. + nPercent)/rpmcliPackagesTotal), nPercent, "Installation progress....", filename);
						}
	          		}
	          			
				}	         
	    	}
			else if (flags & INSTALL_HASH)    	    		
	    	{             
				if (rpmcliPackagesTotal == 0) // remove
				{   
					if(g_callBackRun) {
						g_callBackRun(nPercent, nPercent, "Progress....", filename);
					}
				}
				else
				{
					if (oldRpmcliProgressCurrent != 0 && oldRpmcliProgressCurrent != rpmcliProgressCurrent)
					{
						if(g_callBackRun) {
							g_callBackRun(int(((rpmcliProgressCurrent)*100. + nPercent)/rpmcliPackagesTotal), nPercent, "Installation progress....", filename);
						}
					}
					else
					{
						oldRpmcliProgressCurrent = rpmcliProgressCurrent;
						if(g_callBackRun) {
							g_callBackRun(int(((rpmcliProgressCurrent)*100. + nPercent)/rpmcliPackagesTotal), nPercent, "Installation progress....", filename);
						}
					}  
				}
	    	}
			if (nPercent == 100)
			{
				rpmcliProgressCurrent++;
			}
			
			/*@=relaxtypes@*/
			(void) fflush(stdout);
			break;
	
	    case RPMCALLBACK_TRANS_START:
		    rpmcliHashesCurrent = 0;
		    rpmcliProgressTotal = 1;
		    rpmcliProgressCurrent = 0;
		    if (!(flags & INSTALL_LABEL)) {		    	
		    	break;
		    }
		    if (flags & INSTALL_HASH)
			{		    
		    	if(g_callBackRun) {
		    		g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Preparing... ", filename);
		    	}
			}
		    else
		    {		    	
		    	if(g_callBackRun) {
		    		g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Preparing packages for installation...", filename);
		    	}
		    }
		    (void) fflush(stdout);
		    break;
	
	    case RPMCALLBACK_TRANS_STOP:
		    if (flags & INSTALL_HASH)
			{        
		    	if(g_callBackRun) {
		    		g_callBackRun(1, 1, "Stop transaction...", filename);
		    	}
			}
			rpmcliProgressTotal = rpmcliPackagesTotal;
			rpmcliProgressCurrent = 0;
			
			break;
	    
	
	    case RPMCALLBACK_REPACKAGE_START:
	    	rpmcliHashesCurrent = 0;
		    rpmcliProgressTotal = total;
		    rpmcliProgressCurrent = 0;
		    if (!(flags & INSTALL_LABEL)) {
		    	break;
		    }
		    if (flags & INSTALL_HASH)
		    {  
		    	if(g_callBackRun) {
		    		g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Preparing...", filename);
		    	}
		    }
		    else
		    {  
		    	if(g_callBackRun) {
		        	g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Repackaging erased files...", filename);
		    	}
		    }	
		    (void) fflush(stdout);
		    break;
	
	    case RPMCALLBACK_REPACKAGE_PROGRESS:
		    if (amount && (flags & INSTALL_HASH))
		    {        
		    	if(g_callBackRun) {
		        	g_callBackRun(1, 1, "Progress repackage...", filename);
		    	}
		    }
		    break;
	
	    case RPMCALLBACK_REPACKAGE_STOP:
		    rpmcliProgressTotal = total;
		    rpmcliProgressCurrent = total;
		    if (flags & INSTALL_HASH)
			{        
		    	if(g_callBackRun) {
		    		g_callBackRun(1, 1, "Stop Repackage...", filename);
		    	}
			}
		    rpmcliProgressTotal = rpmcliPackagesTotal;
		    rpmcliProgressCurrent = 0;
		    if (!(flags & INSTALL_LABEL)) {
		        break;		        
		    }
		    if (flags & INSTALL_HASH)
		    {  
		    	if(g_callBackRun) {
		        	g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Upgrading packages...", filename);
		    	}
		    }
		    else
		    {
		        //fprintf(stdout, "%s\n", ("Upgrading packages..."));
		    	if(g_callBackRun) {
		        	g_callBackRun(rpmcliProgressTotal, rpmcliProgressCurrent, "Upgrading packages...", filename);
		    	}
		    }
		    (void) fflush(stdout);
		    break;
	
	    case RPMCALLBACK_UNINST_PROGRESS:
	    	if(g_callBackRun) {   
	    		g_callBackRun(0, 0, "Progress....", "");
	    	}
	    	(void) fflush(stdout);	
	    	break;
	    case RPMCALLBACK_UNINST_START:	    
	    	break;
	    case RPMCALLBACK_UNINST_STOP:    	
	    	g_nRemoveCount ++;    	
	    	if (g_nRemoveSelectedPackages > 0) 
	    	{
	    		nPercent = (int)( g_nRemoveCount * 100. / g_nRemoveSelectedPackages);
	    		if(g_callBackRun) {
	    			g_callBackRun(nPercent, nPercent, "Removing packages...", "");
	    		}
	    	}
	    	else
	    	{
	    		if (g_nTotalRemoveCountByUpdate > 0)
	    		{
	    			nPercent = (int) (g_nRemoveCount *100. /g_nTotalRemoveCountByUpdate);		    		
	    		}
	    		else
	    		{
	    			nPercent = 100;
	    		}
		    	if(g_callBackRun) {
		    		g_callBackRun(nPercent, nPercent, "Removing old version of packages....", "");
		    	}
	    	}	    	
	    	(void) fflush(stdout);
	    	break;
	    case RPMCALLBACK_UNPACK_ERROR:
	    	g_bNoSpace = true;
	    	break;
	    case RPMCALLBACK_CPIO_ERROR:
	    	break;
	    case RPMCALLBACK_UNKNOWN:
	    default:
	    	break;
    }

    return rc;
}

//! Compare HeaderInfo
int classRpmEngine::CompareHeaderInfo(vector <structHeaderInfo> vectorHeaderInfo, structHeaderInfo headerInfo)
{
	int nResult=0;
	string  strName1, strVer1, strRel1, strArch1;
	string  strName2, strVer2, strRel2, strArch2;
	int nCompare=0;
	stripNVRA(headerInfo.strNVRA, &strName1, &strVer1, &strRel1, &strArch1);
	
	vector <structHeaderInfo>::iterator it;
	for(it=vectorHeaderInfo.begin();it!=vectorHeaderInfo.end();it++)
	{
		stripNVRA(it->strNVRA, &strName2, &strVer2, &strRel2, &strArch2);				
		if (strName2 == strName1)
		{ 
			nCompare = CompareVerRel(strVer2, strRel2, strVer1, strRel1);
			if (nCompare > 0 )
			{
				nResult = 1;
				break;
			}
		}		
	}			
	return nResult;
}


/*!
@brief Add udpate to m_vectorUpdateList.

Add udpate to update list  
 
@fileInfo structFileInfo.
*/ 
void classRpmEngine::AddUpdateList(structFileInfo *fileInfo)
{	
	m_vectorUpdateList.push_back(*fileInfo);
}


/*!
@brief Add install to m_vectorInstallList.

Add install to m_vectorInstallList.
 
@fileInfo structFileInfo.
*/ 
void classRpmEngine::AddInstallList(structFileInfo *fileInfo)
{		
	m_vectorInstallList.push_back(*fileInfo);	
}


//! Compare Ver and Rel
 
int 	classRpmEngine::CompareVerRel(string strVer1, string strRel1, string strVer2, string strRel2)
{
	int nRet =0;
	int nVer = 0;
	int nRel = 0;
	nVer = rpmvercmp(strVer1.c_str(), strVer2.c_str());
	
	if (nVer == 0)
	{
		nRel = rpmvercmp(strRel1.c_str(), strRel2.c_str());
		nRet = nRel;
	}
	else
	{		
		return nRet = nVer;
	}
	
	return nRet;	
}

//! Split header info to name, ver, rel, arch.
void 	classRpmEngine::stripNVRA(string str, string  *strName, string  *strVer, string  *strRel, string  *strArch)
{    
    if(str.rfind("/") != string::npos)
    {
	int nFind = str.rfind("/");
	if(nFind < str.length())
	{	
		nFind = nFind + 1;
		str = str.substr(nFind, str.length() - nFind);
	}
    }
    string strTemp = str;
    int nLength = strTemp.length();		
    int lastIndex = nLength -1;
    if (nLength > 4)
	{			
		if(strTemp.compare(lastIndex - 3 , lastIndex, string(".rpm")) == 0)
		{
			strTemp.erase(lastIndex - 3 , lastIndex);
			nLength = strTemp.length();
			lastIndex = nLength -1;
		}
	}		
    string strTemp2;
    int 		archIndex, relIndex, verIndex;
    archIndex = strTemp.rfind(".");	

    strArch->assign(strTemp, archIndex + 1 , lastIndex);
		
    strTemp2.assign(strTemp,0, archIndex);
    relIndex = strTemp2.rfind("-");

    strRel->assign(strTemp2, relIndex+1, archIndex);

    strTemp2.assign(strTemp,0, relIndex);
    verIndex = strTemp2.rfind("-");

    strVer->assign(strTemp2, verIndex+1, relIndex);
    strName->assign(strTemp, 0, verIndex); 
		
}

//! Add classNetwork object to classRpmEngine.

void 	classRpmEngine::SetNetwork(classNetwork *network)
{
	m_Network = network;
}

bool classRpmEngine::SaveObInfo()
{
	ofstream obinfo;
	obinfo.open ("/var/tmp/obinfo.tmp");
	int i;
        vector<structRPMInfo*>::iterator it;
        for(it=m_vectorRPMInfo.begin();it!=m_vectorRPMInfo.end();it++)
        {	
                for(i=0;(*it)->obsoleteName[i];i++) //refer to how they use provideName
                {	
			string strName;
			strName=(string)((*it)->obsoleteName[i]) + " ";
			obinfo<<strName;
                }
                if((*it)->obsoleteName[0]) //if here is for the sake of getting rid of names with no obsoletee
                {       
			string strObname;
			strObname="+"+(string)((*it)->name)+" ";
			obinfo<<strObname;
                }
        }
	obinfo.close();
	return true;
}

//! Apply blacklist to update
int classRpmEngine::ApplyBlacklistToUpdate()
{
	m_vectorBlackUpdate.clear();
	m_configBlacklistUpdate.Read(BLACKLIST_FILE);
	m_vectorBlackUpdate = m_configBlacklistUpdate.GetOptions("blacklist-update");
	
	vector <structFileInfo>::iterator it;      
	for(it=m_vectorUpdateList.begin();it!=m_vectorUpdateList.end();it++)
	{
		if (CheckBlacklist(it->strName) == true)
		{
			it->bBlacklisted = true;
			m_nUpdateAvailableCount--;
		}	
		else
		{
			it->bBlacklisted = false;
		}		
	}
	return 0;
}



//! Check blacklist. If strName is fullpath this function make path to name.
bool 	classRpmEngine::CheckBlacklist(string strPathName)
{	
	string strTemp;
	string strName;
	// This is path
	if(strPathName.rfind("/") != string::npos)
	{
		strName = GetFileNameFromPath(strPathName, strName);
	}
	else
	{
		strName = strPathName;
	}
	string strSection;
	string strStripSection;
	int 	nCountStar=0;	

	int i = 0;
	vector <string>::iterator it;
	for (it=m_vectorBlackUpdate.begin();it!=m_vectorBlackUpdate.end();it++, i++)	
	{
		strSection = *it;
	
		nCountStar = 0;
		strStripSection = "";		 
		for(int j=0;j<(int)strSection.length();j++) 
		{
			if(strSection.at(j) == '*')
			{
				nCountStar++;				
			}
			else
			{
				strStripSection += strSection.at(j);				
			}
		}	
		
		if (strName.length() < strSection.length() - nCountStar)
		{				
			
	        continue;
		}

		if (nCountStar == 1)
		{

			// Look like format : *xxx
			if (strSection.find("*") == 0)
			{
	        	// Look like foramt : *
				if (strSection.length() == 1 )
		        {
		        	//printf("*\n");
		            return true;
		        }
	            //printf("*xxx\n");
	            else if (strName.rfind(strStripSection) == (unsigned int)strName.length()-(unsigned int)strStripSection.length())
	            {	             
	                return true;
	            }
	            else
	            {
	            	continue;
	            }
	        }
	        // Look like format : xxx*
	        else if (strSection.find("*") == (unsigned int)strSection.length()-1)
	        {
	        	//printf("xxx*\n");
	            if (strName.find(strStripSection) == 0)
	            {	             
	                return true;
	            }
	            else
	            {
	            	continue;
	            }	            
	        
	        }	        
	        // Look like format : xx*xx	        
	        else 
	        {
	        	//printf(" xx*xx\n");
	        	string str1, str2;
	            int indexStar = strSection.find("*");
	            str1.assign(strSection, 0, indexStar);
	            str2.assign(strSection, indexStar+1, strSection.length() - indexStar);
	            
	            if (strName.find(str1) == 0 && strName.rfind(str2) == (unsigned int)strName.length() - (int)str2.length())
	            {	             
	                return true;
	            }
	            else
	            {
	            	continue;
	            }
	        }
	    }
	    else if (nCountStar == 2)
	    {	        
			// Look like format : *xxx*
			//printf("*xxx*\n");
			if (strSection.find("*") == 0 && strSection.rfind("*") == (unsigned int)strSection.length()-1 )
			{
				if (strSection == "*.*")
				{	             
				    return true;
				}
				else
				{
					if (strName.find(strStripSection) != string::npos)
					{	                 
					    return true;
					}
					else
					{
						continue;
					}
				}
			}
	    }
	    else 
	    {	
	    	if (strName == strSection)
	        {	
	        	return true;
	        }
	    }
	}	
	return false;
    
}

bool 	classRpmEngine::CheckIncmplist(string pathToRPM)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  bool rtv = false;
  string strNamewithPATH, strVer, strRel, strArch;
  int verIndex = 0;

	stripNVRA2(pathToRPM, &strNamewithPATH, &strVer, &strRel, &strArch);
  verIndex = strNamewithPATH.rfind("/");
  string strName(strNamewithPATH, verIndex+1, strNamewithPATH.size());

	vector <structFileInfo>::iterator it;  
	for(it=m_vectorUpdateList.begin();it!=m_vectorUpdateList.end();it++)
	{
		if(it->strName == strName && it->bIncompatible == true) {
      rtv = true;
    }
  }

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  return rtv;
}

//! return state Self updating or not.
bool 	classRpmEngine::CheckSelfUpdate()
{
	return m_bSelfUpdate;
}

//! Get File name from Full path file path.
string classRpmEngine::GetFileNameFromPath(string strStr, string strStr2)
{
	string strTemp, strTemp2;	
	strTemp = m_configEnv.StripRString(strStr, '/');
	strTemp2.assign(strTemp, strTemp.rfind("/")+1,strTemp.length()-1);
	string v,r,a;
	stripNVRA(strTemp2, &strStr2, &v, &r, &a);
	return strStr2;
}

//! Get File name from Full path file path.  remove ".rpm".
string classRpmEngine::GetFullFileNameFromPath(string strStr, string strStr2)
{
	if(strStr.find("/") != 0)
	{
		strStr2 = strStr;
	}
	else
	{
		string strTemp;	
		strTemp = m_configEnv.StripRString(strStr, '/');
		strStr2.assign(strTemp, strTemp.rfind("/")+1,strTemp.length()-1);
	}	
	
	if (strStr2.rfind(".rpm") != string::npos)
	{
		strStr2.assign(strStr2, 0, strStr2.rfind(".rpm"));		
	}
	
	return strStr2;
}


//! Get Blocklisted Packages
vector <string> classRpmEngine::GetBlockedPackages()
{
	return m_vectorBlockedUpdatePackages;
}

/*! Check smp 
     return : true is smp available status, false is not available status.
 */
bool classRpmEngine::CheckSmp(string strName)
{		 
	if (m_bSmp)
	{
		return true;
	}
	else
	{
		if (strName.find("smp") != string::npos || strName.find("largesmp") != string::npos || strName.find("hugemem") != string::npos)
		{			
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool classRpmEngine::IsKernelInstalled()
{
	if(m_bKernelInstall)
	{
	   return true;
	}
	else
	{
	   return false;
	}
}

//! Start notifier
bool classRpmEngine::WriteNotifierInfo(int nCount)
{	
	int nResult, len;
	char buf[MAX_STRING];
	FILE *fp;
	
	fp = fopen(UPDATE_COUNT_TMP, "w");
	if (fp != NULL) 
	{
		snprintf(buf, sizeof(buf), "%d\n", nCount);
		len = strlen(buf);
		nResult = fwrite(buf, 1, len, fp);
		if (nResult < len){			
			// write failed
			fclose(fp);
			return false;
			
		}
		fclose(fp);
		
		chmod(UPDATE_COUNT_TMP, 0644);
	}
	else {
		// file open failed
		return false;
	}	
	nResult = system(CHECK_NOTIFIER);
	if(nResult == -1)
	{
		return false;  // This is not usesless.
	}
    
	return true;
}
bool classRpmEngine::CheckConfigFileLength()
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
@brief Check available dir space
@strPath : Dirtory that you want to check.
@lTargetSize : Check size Do you want to.
@return bool : strPath directory have lTargetSize or not.
*/

bool classRpmEngine::CheckDirSpace(const char * strPath, long lTargetSize)
{
	long lSize;
	struct statfs lstatfs;
	
	if(access(strPath, F_OK) == 0)
	{
		statfs(strPath, &lstatfs);
		lSize = lstatfs.f_bavail * (lstatfs.f_bsize/KILO_BYTE);
		//printf("Size = %ld\n", lSize);
	}
	else
	{
		return false;
	}
	//printf("Size ::::::::::::: %ld\n", MP->size.avail);
	if (lSize > lTargetSize)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool classRpmEngine::isIncompatiblePack(string strName, string strVer, string strRel)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif
	bool bResult = false;
	vector <structFileInfo>::iterator it;
	for(it=m_vectorUpdateList.begin();it!=m_vectorUpdateList.end();it++)
	{
		if(it->bIncompatible == true)
		{
			if (it->strName == strName && it->strVersion == strVer && it->strRelease == strRel)
			{
				bResult = true;
				break;
			}
		}
	}
	
	return bResult;
}
#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif
  
bool classRpmEngine::isIncompatible(string &borderVer, string &installedVer, string &upgradeVer)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  bool rtv = false;

  const char *bVer = borderVer.c_str();
  const char *iVer = installedVer.c_str();
  const char *uVer = upgradeVer.c_str();

  // rpmvercmp(const char * a, const char * b) return
  // +1 if a is "newer", 0 if equal, -1 if b is "newer"
  if ( ((rpmvercmp(iVer, bVer) < 0)  && (rpmvercmp(bVer, uVer) < 0)) ||
       ((rpmvercmp(iVer, bVer) == 0) && (rpmvercmp(bVer, uVer) < 0)) ||
       ((rpmvercmp(iVer, bVer) < 0) && (rpmvercmp(bVer, uVer) == 0)) ) {
    rtv = true;
  }

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  return rtv;
}

bool classRpmEngine::ReadIncmplistInfo(char *path)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  bool rtv = false;
  class classConfigParser cp;
  string msg;
  classLogger log;

  m_vectorCsvData.clear();

  rtv = cp.parseCSV(path, m_vectorCsvData, msg);
  if (rtv == false) {
    log.WriteLog_syslog(msg);
    return rtv;
  }

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  return rtv;
}

bool classRpmEngine::ApplyIncmplistToUpdate(string strNameL, string strVerL, string strRelL, string strVerR, string strRelR)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  bool rtv = false;
  string msg;
  bool isFound = false;
  string packageName;
  string packageVerRelB;
  string packageVerRelL;
  string packageVerRelR;
  classLogger log;

  packageName = strNameL;
  packageVerRelL = strVerL + "-" + strRelL;
  packageVerRelR = strVerR + "-" + strRelR;

  // *******************
  // * FILE FORMAT
  // *******************
  //
  // --- /etc/axtu/incmp.conf ---------
  // |samba, 3.0.24-1AX, 4.1.23-2AX   | 
  // |httpd, 1.2.3-1AX, 2.3.4-1AX     |
  // ----------------------------------

  // *******************
  // * DATA STRUCTURE
  // *******************
  //                    ---------------------------------------------------
  // m_vectorCsvData -> |vector * -> |samba   | 3.0.24-1AX  | 4.1.23-2AX   | < eachLine
  //                    ---------------------------------------------------
  //                    |vector * -> |httpd   |  1.2.3-1AX  | 2.3.4-1AX    | < eachLine
  //                    ---------------------------------------------------
  //                          ^           ^             ^
  //                        eachVal      eachVal      eachVal

  for (vector<vector<string>*>::iterator eachLine = m_vectorCsvData.begin( ); eachLine != m_vectorCsvData.end( ); ++eachLine) {
    for (vector<string>::iterator eachVal = (**eachLine).begin(); eachVal != (**eachLine).end(); ++eachVal) {
      // Check the first field(== eachVal[0])
      if (*eachVal == packageName) {
        // Found the package in config file AXTU_INCMP_CONFIG_FILE
        isFound = true;
        continue;
      }

      // Check the second, third, forth, ... Nth field(== eachVal[1] ... eachVal[2])
      if (isFound) {
        packageVerRelB = *eachVal;
        if(isIncompatible(packageVerRelB, packageVerRelL, packageVerRelR) == true) {

#ifdef DEBUG
          msg = "NG " + packageName + " is incompatible =" + " BorderVerRel: " + packageVerRelB;
          msg += " LocalVerRel: " + packageVerRelL + " RemoteVerRel: " + packageVerRelR + "\n";
          cerr << msg;
          log.WriteLog_syslog(msg);
#endif
          rtv = true;

        }
      }
    }

    isFound = false;
  } 

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  return rtv;
}

void classRpmEngine::sig_hup(int sig)
{	
	if(sig == SIGHUP)
	{			
		exit(1);
	}
}


classBlockSignal::classBlockSignal(int signal)
{	
	sigset_t  newmask;	
	sigemptyset(&newmask);
	sigaddset(&newmask, signal);	
	if(sigprocmask(SIG_BLOCK, &newmask, &m_oldmask) < 0)
	{
		return;
	}
}

classBlockSignal::~classBlockSignal()
{		
	if(sigprocmask(SIG_SETMASK, &m_oldmask, NULL) < 0)
	{
			return;
	}
	
}
