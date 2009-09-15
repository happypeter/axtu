/*!
@file classRpmEngine.h
@brief Class header file for rpm related api
*/
#ifndef CLASSRPMENGINE_H_
#define CLASSRPMENGINE_H_

#include <iostream>
#include <stdio.h>
#include <zlib.h>
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
#include <string>
#include <vector>
#include <set>
#include <fcntl.h>
#include "hsCommon.h"
#include "classConfigParser.h"
#include "classNetwork.h"


#define MYSELF_NAME "classRpmEngine"
#define HAVE_OTHERARCH 5

using namespace std;

typedef void (*commonCallBackFunc)(int p1, int p2, const char * msg1, const char * msg2);


/*!
@brief Compare version and release. 

Compare version and release to sort set container with rpmvercmp.

@param strVer1 first version.
@param strRel1 first release.
@param strVer2 second version.
@param strRel2 second release.

@return nRet > 0 : first > second , nRet == 0 : first == second , nRet < 0 :  first < second
*/
static int 	CompareVerRel2(string strVer1, string strRel1, string strVer2, string strRel2)
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


/*!
@brief Split NVRA type string to name, ver, rel, arch.

Split NVRA(Name-Version-Relese.Arch) type string to name, ver, rel, arch.

@param str(output) :  string(NVRA type).
@param strName(input) : Name.
@param strVer(input) : Version.
@param strRel(input) : Release.
@param strArch(input) : Architecture.
*/
static void 	stripNVRA2(string str, string  *strName, string  *strVer, string  *strRel, string  *strArch)
{	
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
    int archIndex, relIndex, verIndex;
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

/*!
@brief Compare structHeaderInfo.

Compare between two struct structHeaderInfo.

@param headerInfo1 
@param headerInfo2  

@return nResult 
*/

static bool CompareHeaderInfo2(structHeaderInfo headerInfo1, structHeaderInfo headerInfo2)
{	
	bool nResult=false;
	string  strName1, strVer1, strRel1, strArch1;
	string  strName2, strVer2, strRel2, strArch2;
	int nCompare=0;
	stripNVRA2(headerInfo1.strNVRA, &strName1, &strVer1, &strRel1, &strArch1);
	stripNVRA2(headerInfo2.strNVRA, &strName2, &strVer2, &strRel2, &strArch2);
	
	if (strcmp(strName1.c_str() , strName2.c_str()) < 0)
	{	
		nResult = true;
	}
	else if(strcmp(strName1.c_str(), strName2.c_str()) == 0)
	{	//the below "noarch" is here to support 
		//updating pkgA-1.0.x86_86(i386) with pkgA-1.1.noarch
		//or updating pkgA-1.0.noarch with pkgA-1.1.x86_86(i386)
		if((strArch1 < strArch2)&&!(strArch1 == "noarch" || strArch2 == "noarch"))
		{							
			nResult = true;
		}		
		else if(( strArch1 == strArch2)||(strArch1 == "noarch" || strArch2 == "noarch"))
		{
			if((headerInfo1.strEpoch)<(headerInfo2.strEpoch))
			{
				nCompare=1;
			}
			else  if((headerInfo1.strEpoch)>(headerInfo2.strEpoch))
			{
				nCompare=-1;
			}
			else
			{
			nCompare = CompareVerRel2(strVer2, strRel2, strVer1, strRel1);
			}
			if (nCompare > 0 )
			{
				nResult = true;
			}
			else if(nCompare == 0)
			{		
				
				if((int)headerInfo2.bSelfUpdate > (int)headerInfo1.bSelfUpdate)
				{
					nResult = true;				
				}			
			}
		}		
	}	
				
	return nResult;
}

/*!
@brief Condition for set container.

This is sorting condition for set container. 

@return bool 
*/
struct  DereferenceLess
{
  template <typename PtrType>
  bool operator()(PtrType pT1, PtrType pT2) const
    {	  
        return CompareHeaderInfo2(pT1, pT2);
    }
};

/*!
@brief Class for the rpm operation.


The classRpmEngine can install, update, erase rpm files.
and also classRpmEngine can find other required packages.

*/

class classRpmEngine
{
private:
	vector <structCacheDirInfo> m_vectorCacheDirInfo;		
	set <structHeaderInfo, DereferenceLess> m_setRemoteHeaderInfo;
	set <structHeaderInfo, DereferenceLess> m_setLocalHeaderInfo;
	vector <structRPMInfo*>  m_vectorRPMInfo;	
	classConfigParser m_configEnv;
	classConfigParser m_configBlacklistUpdate;
	
	vector <structAddedFile> m_vectorAddedFile;
	vector <string> m_vectorAddedKmodRedcastleFile;
	vector <structFileInfo> m_vectorObsoleteToUpdate;//to mark obsoleter shall be updated
	//adding this line leads to a mem leak on my xen machine, but it is OK on my host machine.	
	vector <structFileInfo> m_vectorUpdateList;
	vector <structFileInfo> m_vectorInstallList;
	
	int  m_bKernelInstall;
	int  m_nDepCount;
	int  m_nDepTotalCount;
	rpmts m_rpmTs;
	rpmts m_testTs;

	classNetwork * m_Network;
	
	int m_nCommand;
	bool m_bSelfUpdate;
	vector<string> m_vectorBlackUpdate;
	vector<string> m_vectorBlockedUpdatePackages;
	vector<char *> m_vectorDepAddedFiles;
  vector<vector<string>*> m_vectorCsvData;
	bool m_bSmp;
	bool m_bIgnoreSelfUpdate;
	int m_nUpdateAvailableCount;
	int m_nIncompatible;
	vector<string> m_vectorIncompatibleUpdatePackages;	
public:
			 
	
	classRpmEngine();
	virtual ~classRpmEngine();
	char * OBname[5];
	char * getOB();	
	///////////////////
	// Interface 
	
	// Init fuction	
	void SetCommand(int nType); // 0 : remove  1: update or install
	int  GetCommand();
	bool ReadCacheDirInfo(string strConfigFilePath=CONFIG_FILE);
	int ReadRemoteHeaderInfo();
	int ReadLocalHeaderInfo();
	bool CopyObsoleterFromInstallToUpdate(string strName,string strArch);
	bool IsPackageInstalled(string strName);
	bool RemoveUpdateInstallList(string strName);
	int ApplyObsoletes();
	bool SaveObInfo();	//----especially for gui-setup blacklist
	int ReadHeaders();	// -------------------------------------------------- Callback
	int OpenHeader(string strFilePath, int nIndex, bool bUpgrade);
	int CompareHeaderInfo(vector <structHeaderInfo> vectorHeaderInfo, structHeaderInfo headerInfo);	
	bool CreateUpdateInstallList();
				
	void ClearAddedFiles();
	
	int AddFile(const char *strPackName, int nType, int nUpgrade);	
		
	vector <structAddedFile> GetAddedFile(int nType, vector <structAddedFile> vectorTemp);   
		
	vector <structFileInfo> GetUpdateList();
	vector <structFileInfo> GetInstallList();
	set <structHeaderInfo, DereferenceLess> GetInstalledList();
	int Check(bool bForceBlackist=false);         
	bool CheckKmodRedcastle();
	bool CheckObsoleteToUpdate(string strName, string strVersion, string strRelease, string strArch);
	vector <string> GetAddedKmodRedcastleFile();
	bool RemoveKernelAndKmodRedcastle(int nType);
	int Run(bool bForce=false);	// -------------------------------------------------- Callback
	int Test();
	void SetNetwork(classNetwork * network);
	
	// Interface
	//////////////////	
	int InitTs();
	int UnInitTs();
  
	struct structRPMInfo *GetHeaderInfo(Header h, int k, int index);
	bool CopyData(struct structRPMInfo *src, struct structRPMInfo *dest);	
	int AddInstallPackage(rpmts ts, const char * file, int upgrade);
	int AddRemovePackage(rpmts ts, const char * file); 
	int DealwithDependence(rpmts ts, rpmps ps);
	int SameProblem(const rpmProblem ap, const rpmProblem bp);
	int GetRequiredPkgFromRemote(rpmProblem prob, struct structRPMInfo *result);
	int GetRequiredPkgFromLocal(rpmProblem prob, char *result);
	int GetFromRPMInfoListFromRemote(vector <structRPMInfo*> vectorRPMInfo, char *name, char *version, char *arch, int type, char *compare, struct structRPMInfo *result);
	int GetFromRPMInfoListFromLocal(char *name, char *version, int type, struct structRPMInfo *result);	
	static void * myRpmShowProgress(/*@null@*/ const void * arg,
	const rpmCallbackType what,
	const unsigned long amount,
	const unsigned long total,
			/*@null@*/ fnpyKey key,
			/*@null@*/ void * data);	
	void AddUpdateList(structFileInfo *fileInfo);
	void AddInstallList(structFileInfo *fileInfo);
	
	bool AddUpdateInstallList(structFileInfo *,string,string,string,string,string,int,bool,bool);
	bool CheckLoop(string strArg);
	int CompareVerRel(string strVer1, string strRel1, string strVer2, string strRel2);	
	void stripNVRA(string str, string  *strName, string  *strVer, string  *strRel, string  *strArch);
	int  GetOtherArchPackages(rpmts ts, Header h, int upgrade);
	int CheckSameFile(vector <const char* > vectorFiles,  const char * strPackName);
	int CheckSameFile(vector <structAddedFile> vectorFiles,   const char* strPackName);
	int CheckKernel(string strName);	
	bool GetKmodRedcastleName(string strName, string strVersion, string strRelease, string &strFind);
	int AddInstallElement(rpmts ts, Header h, const fnpyKey key, rpmRelocation *relocs, int nUpgrade);
	int ChangeGrubToDefaultKernel();
	void SetReadHeadersCallBack( commonCallBackFunc callBackFunc);
	void SetRunCallBack( commonCallBackFunc callBackFunc);
	void SetCheckCallBack( commonCallBackFunc callBackFunc);	
	void SetReadRemoteHeaderInfoCallBack(commonCallBackFunc callBackFunc);
	void SetReadLocalHeaderInfoCallBack(commonCallBackFunc callBackFunc);
	void SetCreateUpdateInstallListCallBack(commonCallBackFunc callBackFunc);

	string GetFullPathFile(int nUpdate, bool bHeader, string strName, string strVersion, string strRelease, string strArch, string strEpoch, bool bURL=false);
	Header FindHeaderFromLocal(const char * strHeader, Header header);
	struct structRPMInfo * FindHeaderFromRemote(char *name, char *version, char *rel, struct structRPMInfo *result);
	int ApplyBlacklistToUpdate();	
	bool CheckBlacklist(string string);
	bool CheckSelfUpdate();
	vector <string> GetBlockedPackages();
	string GetFileNameFromPath(string strStr, string strStr2);
	string GetFullFileNameFromPath(string strStr, string strStr2);
	bool WriteDepLog(rpmps ps);
	int sameProblem(const rpmProblem ap, const rpmProblem bp);
	bool DeleteDownPacks();
	bool CheckArch(string strArch1, string strArch2);
	bool CheckSmp(string strName);
	bool IsKernelInstalled();
	void SetIgnoreSelfUpdate(bool bFlag);
	static bool WriteNotifierInfo(int nCountt);
	bool CheckConfigFileLength();
	bool CheckDirSpace(const char * strPath, long lTargetSize);
	string GetKernelType(string strVersion);
	bool IsKernelModuleType(string strName);
	void FreeRequiredRPM(struct structRPMInfo * );
	int GetUpdateAvailableCount();
  bool ReadIncmplistInfo(char *path);
  bool isIncompatible(string &borderVer, string &installedVer, string &upgradeVer);
  bool ApplyIncmplistToUpdate(string strNameL, string strVerL, string strRelL, string strVerR, string strRelR);

  int GetIncompatibleCount();
  vector <string> GetIncompatiblePackages();
  bool isIncompatiblePack(string strName, string strVer, string strRel);
  bool CheckIncmplist(string pathToRPM);
  static void sig_hup(int sig);  
};

class classBlockSignal
{
public:	
	classBlockSignal(int signal);
	~classBlockSignal();
private:
	sigset_t  m_oldmask;		
};

#endif /*CLASSRPMENGINE_H_*/
