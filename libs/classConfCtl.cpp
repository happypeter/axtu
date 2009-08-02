/*!
@file classConfCtl.cpp
@brief class source file that control the configure files
*/
#include "classConfCtl.h"
#include "hsCommon.h"

//! A Constructor
classConfCtl::classConfCtl(void)
{
	m_NewConfParser=new classConfigParser();	
	m_OldConfParser=new classConfigParser();
}

//! A destructor
classConfCtl::~classConfCtl()
{
	delete m_NewConfParser;
	delete m_OldConfParser;
}

/*!
@brief check if there is a new configure file or not
The new configure file has some contents to update,

@return bool - Exist or not?
*/
bool classConfCtl::NewConfCheck(void)
{
	bool bRet;

	if(!access(NEW_CONFIG_FILE,F_OK)){
		bRet=true;
	}else{
		bRet=false;
	}
		
	return bRet;
}

/*!
@brief check if there is a configure file or not

@return bool - Exist or not?
*/
bool classConfCtl::ExConfCheck(void)
{
	bool bRet;

	if(!access(CONFIG_FILE,F_OK)){
		bRet=true;
	}else{
		bRet=false;
	}
		
	return bRet;
}

/*!
@brief check a configure file
This method do checking status of configure file.
If there is a new configure file that has some contents to update,
it integrate with real configure file and the new configure file.

@bool - Success or not
*/
bool classConfCtl::ConfigCheck(void)
{
	bool bRet;

	if (m_NewConfParser->Read(NEW_CONFIG_FILE) != true) {
		// Although reading a new-conf-file is failed, 
		// the result is true.
		// Because this is not effectable to this program. 
		bRet = true;
	}else{
		if(!ExConfCheck()){
			MakeNewConf();
		/****************************
		// We don't need this function 
		// So, I removed this routine
			MakeDefaultConf();
		****************************/
			bRet = true;
		}else{
			if(!m_OldConfParser->Read(CONFIG_FILE)){
				bRet=false;
			}else{
				ModifyExConfFile();
		/****************************
		// We don't need this function 
		// So, I removed this routine
				MakeDefaultConf();
		****************************/
				bRet=true;
			}
		}
	}

	if(bRet){
		unlink(NEW_CONFIG_FILE);
	}

	return bRet;
}

bool classConfCtl::ModifyExConfFile(void)
{
        vector<string> vectorNewSections;
        vector<string> vectorTempSections;
        vector<string>::iterator NewIt;
        vector<string>::iterator TempIt;

	bool bFindFlag;
	bool bRet;

        string strTemp;
	string strBackupFileName = (string)CONFIG_FILE + (string)".old";

	if(!access(TEMP_CONFIG_FILE,F_OK)){
		unlink(TEMP_CONFIG_FILE);
	}

	FileCopy(CONFIG_FILE,TEMP_CONFIG_FILE);

	m_TempConfParser = NULL;
	m_TempConfParser = new classConfigParser();
	if(m_TempConfParser == NULL){
		return false;
	}

	if (m_TempConfParser->Read(TEMP_CONFIG_FILE) != true){
		return false;
	}

        vectorTempSections = m_TempConfParser->GetSections();
        vectorNewSections = m_NewConfParser->GetSections();

        for(NewIt=vectorNewSections.begin();NewIt!=vectorNewSections.end();NewIt++)
        {
                if ( *NewIt == "main" || *NewIt == "authentication" || *NewIt == ""){
                        continue;
                }
	
		bFindFlag=m_TempConfParser->HasSection(*NewIt);
		if(bFindFlag){
            		strTemp = m_NewConfParser->GetOption(*NewIt, "name");
      			m_TempConfParser->SetOption(*NewIt,"name",strTemp);

              		strTemp = m_NewConfParser->GetOption(*NewIt, "baseurl");
               		m_TempConfParser->SetOption(*NewIt,"baseurl",strTemp);
		}else{
			m_TempConfParser->AddSection(*NewIt);

			strTemp = m_NewConfParser->GetOption(*NewIt, "name");
			m_TempConfParser->SetOption(*NewIt,"name",strTemp);

			strTemp = m_NewConfParser->GetOption(*NewIt, "baseurl");
			m_TempConfParser->SetOption(*NewIt,"baseurl",strTemp);
		}
        }
	bRet = m_TempConfParser->Write();

	if(m_TempConfParser != NULL){
		delete m_TempConfParser;
	}

	if(rename(CONFIG_FILE,strBackupFileName.c_str()) != 0){
		bRet = false;
	}else{
		if(chmod(strBackupFileName.c_str(),g_file_mode_for_axtu) != 0){
			bRet = false;
		}
	}

	if(bRet){
		bRet = FileCopy(TEMP_CONFIG_FILE, CONFIG_FILE);
	}

	unlink(TEMP_CONFIG_FILE);

	return bRet;
}


bool classConfCtl::MakeNewConf(void)
{
	return FileCopy(NEW_CONFIG_FILE,CONFIG_FILE);
}

/*!
@brief create default configure file like template

@return bool - Success or not
*/
bool classConfCtl::MakeDefaultConf(void)
{
	if( m_NewConfParser->HasSection("main") ){
		return FileCopy(NEW_CONFIG_FILE,DEFAULT_CONFIG_FILE);
	}else{
		if(!access(DEFAULT_CONFIG_FILE,F_OK)){
			m_OldConfParser->Read(DEFAULT_CONFIG_FILE);
			ModifyExConfFile();

			return false;
		}
		return false;
	}
}

/*!
@brief copy a file to another file.
Additionally, dest file's permission is set as the variable("g_file_mode_for_axtu")

@param szOrgFileName - source file name
@param szDestFileName - dest file name

@return bool - Success or not
@see g_file_mode_for_axtu
*/
bool classConfCtl::FileCopy(char *szOrgFileName,char *szDestFileName)
{
	int readCnt,writeCnt,orgFile,newFile;
	char buffer[BUFLEN];

	orgFile = open(szOrgFileName,O_RDONLY);
	if (orgFile < 0) {
		return false;
	}

	newFile = open(szDestFileName,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (newFile < 0) {
		close(orgFile);
		return false;
	}

	for(readCnt = 1;readCnt > 0;){
		readCnt = read(orgFile,buffer,BUFLEN);
		if (readCnt == 0) break;	// end of file
		writeCnt = write(newFile,buffer,readCnt);
		if (writeCnt != readCnt) {	// write failed
			close(orgFile);
			close(newFile);
			return false;
		}
	}

	close(orgFile);
	close(newFile);

	if(chmod(szDestFileName,g_file_mode_for_axtu) != 0){
		return false;
	}

	return true;
}
