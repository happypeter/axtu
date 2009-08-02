/*!
@file classConfigParser.cpp
@brief Class source file for parsing a configure file' contents
*/
// Copyright 2006 HAANSOFT, INC  

#include "classConfigParser.h"
#include "hsCommon.h"

#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "message.h"
#include "trace.h"
#include "classLogger.h"

//! Construct
classConfigParser::classConfigParser()
{
  m_vectorConfig.clear();
  m_bEditConfig = false;
  m_lReadLines = 0;
}

classConfigParser::~classConfigParser()
{		
	m_vectorConfig.clear();
	
}

//! Read file and it puts in m_vectorConfig
bool classConfigParser::Read(string strPath)
{		
	m_lReadLines = 0;
	if (strPath == "")
	{
		strPath = GetConfigFilePath();
	}
	else
	{
		SetConfigFilePath(strPath);
	}
	m_vectorConfig.clear();
		
	if (access(strPath.c_str(),F_OK) != 0)
	{
		return false;
	}
	
	string strTemp;
	string strComment;
	char * strBuf = NULL;	
	size_t len;
  	ssize_t read;
	
	bool bFrontComment;
	bool bRearComment;
	int nSecNumber = -1;
 	int nCharCount=0;
	FILE * fd=NULL;
	long fsize = 0;

	fd = fopen(strPath.c_str(), "r");	
	if (fd == NULL)
		return false;

	(void) fseek(fd, 0, SEEK_END);
	fsize = ftell(fd);

	if (fsize == 0) {
		(void) fclose(fd);
		return false;
	} 

	(void) fseek(fd, 0, SEEK_SET);

	while ( (read = getline(&strBuf, &len, fd)) !=  -1)
        {
	
        	bFrontComment = false;
        	bRearComment = false;
        	m_lReadLines ++;
        	strTemp = StripString(strBuf);		
        	string::size_type nLeft = strTemp.find('[');
        	string::size_type nRight = strTemp.find(']');
        	string::size_type nComment = strTemp.find('#');
        	string::size_type nEqual	=	strTemp.find('=');
        	
        	if(nComment != string::npos)
        	{
        		strComment.assign(strTemp, nComment, strTemp.length() - nComment);
        	}	
        	
        	// If strTemp is section
        	if (nLeft != string::npos  && nRight != string::npos && nLeft < nRight)
        	{	
        		// Find comment
        		if(nComment != string::npos )
        		{		
        			if ( nRight < nComment )
        			{
        				bRearComment = true;					
        			}
        			else
        			{
        				bFrontComment = true;
        			}
        		}
        		
        		string strSection;						
        		structSECTION newSection;
        		if(bFrontComment == true)
        		{				
        			newSection.name = strComment;
        			newSection.bCommented = true;
        		}
        		else
        		{
        	  	strSection.assign(strTemp, nLeft+1 , nRight-(nLeft+1));
        	  	newSection.name = strSection;
        	  	newSection.bCommented = false;
        		}		  
        		
        		newSection.name =StripString(newSection.name);
        					
        		if(bRearComment)
        		{
        			newSection.rearComment = strComment;				
        		}
        		newSection.bHide = false;
        		m_vectorConfig.push_back(newSection);
        		nSecNumber++;
        	}
        			
        	// This is option.
        	else if(nEqual != string::npos )
        	{	
        		// Find comment
        		if(nComment != string::npos )
        		{		
        			if ( nEqual < nComment )
        			{
        				bRearComment = true;
        			}
        			else
        			{
        				bFrontComment = true;					
        			}
        		}
        		
        		// Make space section.
        		if (nSecNumber == -1)
        		{										
        			structSECTION newSection;		  		  
        			newSection.name = __DEFAULT__;
        			newSection.bHide = true;
        			newSection.bCommented = false;
        			m_vectorConfig.push_back(newSection);			  
        		  nSecNumber++;
        		}
        		
        		structOPTION option;
        		if (nEqual > 1)
        		{
        			option.name.assign(strTemp, 0, nEqual);
        			if(bRearComment)
        			{	
        				option.rearComment = strComment;
        				option.value.assign(strTemp, nEqual + 1, nComment - (nEqual + 1));				
        			}
        			else
        			{
        				option.value.assign(strTemp, nEqual + 1, strTemp.length() - (nEqual + 1));
        			}				
        			option.name = StripString(option.name);
        			option.value = StripString(option.value);	
        			option.bCommented = bFrontComment;			
        			
        		}
        		m_vectorConfig.at(nSecNumber).item.push_back(option);			
        	}
        	else if(nComment  != string::npos)
        	{	
        		
        		// Make space section.
        		if (nSecNumber == -1)
        		{										
        			structSECTION newSection;		  		  
        			newSection.name = __DEFAULT__;
        			newSection.bHide = true;
        			newSection.bCommented = false;
        			m_vectorConfig.push_back(newSection);
        		  nSecNumber++;
        		}
        		
        		string strOption;						
        		structOPTION newOption;
        		strOption.assign(strTemp, nComment, strTemp.length() - nComment);		  
        	  newOption.name = strOption;
        	  newOption.name =StripString(newOption.name);		
        	  newOption.bCommented = true;
        		m_vectorConfig.at(nSecNumber).item.push_back(newOption);
        		
        	}
        	// Empty line
        	else if(StripString(strTemp) == "")
        	{
        		// Make space section.
        		if (nSecNumber == -1)
        		{										
        			structSECTION newSection;		  		  
        			newSection.name = "";		
        			newSection.bHide = false;
        			newSection.bCommented = true;
        			m_vectorConfig.push_back(newSection);
        		  nSecNumber++;
        		}									
        		structOPTION newOption;		  
        	  newOption.name = "";		  		
        	  newOption.bCommented = true;
        		m_vectorConfig.at(nSecNumber).item.push_back(newOption);		
        	}
        }
	
	if (strBuf)
        	free(strBuf);
	fclose(fd);
	
	return true;
}

//! Get sections
vector<string> 	classConfigParser::GetSections(bool bHide)
{		
	vector<string> sectionVector;
	vector<SECTION>::iterator i;
	for(i=m_vectorConfig.begin();i != m_vectorConfig.end();i++)
	{
		if (bHide == true)
		{
			if(!i->bCommented && i->name != "")
				sectionVector.push_back(i->name);
		}
		else
		{
			if(!i->bCommented && i->name != "" && i->bHide == false)
				sectionVector.push_back(i->name);
		}
	}
	return sectionVector;
}
	
//! Get options
vector<string>	classConfigParser::GetOptions(string	section)
{		
	vector<string> optionsVector;
	if (IsValidIndex(section) == true )
	{		
		vector<OPTION>::iterator i2;			
	  int index = IndexSection(section);
		for (i2 = m_vectorConfig.at(index).item.begin(); i2 != m_vectorConfig.at(index).item.end();i2++)
		{	
			if(!i2->bCommented)
				optionsVector.push_back( i2->name);
		}
	}
	return optionsVector;
}
	
	
//! Add section
bool	classConfigParser::AddSection(string	section)
{		
	if (! HasSection(section) )
	{			
		
		structSECTION newSection;
		newSection.name = section;
		newSection.name = StripString(newSection.name);
		newSection.rearComment= "";
		newSection.bCommented = false;
		if(section == __DEFAULT__)
		{
			newSection.bHide = true;
		}
		else
		{
			newSection.bHide = false;
		}
		m_vectorConfig.push_back(newSection);
	}
	else
	{
		return false;
	}
	
	return true;
}
	

//! Get index section
int		classConfigParser::IndexSection(string section)
{
	int nCount=-1;
		
  vector<SECTION>::iterator i;
	for(i=m_vectorConfig.begin();i != m_vectorConfig.end();i++)	
	{
		nCount ++;
		if ((*i).name == section && i->bCommented == false)
		{
			break;								
		}			
	}	
	return nCount;	
}
	
//! Get index option
int		classConfigParser::IndexOption(string section, string option)
{
	int nCount=-1;	
	int index = IndexSection(section);
	
	vector<OPTION>::iterator i2;
	
	for (i2 = m_vectorConfig.at(index).item.begin(); i2 != m_vectorConfig.at(index).item.end();i2++)
	{
		nCount ++;
		//printf("%d :::::::%s ::: %s\n", index_section(section), m_vectorConfig.at(index_section(section))->item.at(i)->name.ascii() , option.ascii());
		if ((*i2).name == option  && i2->bCommented == false)
		{		
		  //printf("%d\n",nRet);
			break;			
						
		}
	}
			
	return nCount;	
}

//! has_section
bool	classConfigParser::HasSection(string	section)
{
	bool bRet = false;
    
	vector<string> sectionVector ;
	vector<string>::iterator i;
	sectionVector = GetSections(true);	
	for(i=sectionVector.begin();i!=sectionVector.end();i++)
	{		
		if ( *i == section)
		{
			bRet = true;
			break;
		}
	}
			
    return bRet;
}
		
//! has_option
bool	classConfigParser::HasOption(string section,string option)
{
	bool bRet = false;
	
	if (HasSection(section))
	{	
		vector<string> optionList = GetOptions(section);
		vector<string>::iterator i;
		
		string strTemp;
		for(i=optionList.begin();i!=optionList.end();i++)
		{	
			if ( *i == option)
			{
				bRet = true;
				break;
			}
		}
	}		
  return bRet;	
}
	
	
	
//! Get the value
string		classConfigParser::GetOption(string section, string option)
{		
		 
	if (IsValidIndex(section, option) == false)
	{
		return "";
	}	

	if (HasOption(section, option) == true)
	{	
		return m_vectorConfig.at(IndexSection(section)).item.at(IndexOption(section,option)).value;
	}
	return "";
}
	
//! Set Option
bool	classConfigParser::SetOption(string section, string option, string value)
{	
	m_bEditConfig = true;
	
	//Edit option
	if (HasOption(section, option) == true)
	{		
		/*
		if (HasSection(section) == false)
		{				
			if(AddSection(section) == false)
			{
				return false;
			}			
		}	
		*/			
		m_vectorConfig.at(IndexSection(section)).item.at(IndexOption(section,option)).value = value;				
	}
	//Insert option
	else
	{
		if (HasSection(section) == false)
		{
			if(AddSection(section) == false)
			{	
				return false;
			}
		}				
		structOPTION newOption;
		newOption.name = option;
		newOption.value = value;		
		newOption.name = StripString(newOption.name);
		newOption.value = StripString(newOption.value);
		newOption.rearComment = "";
		newOption.bCommented = false; 
		m_vectorConfig.at(IndexSection(section)).item.push_back(newOption);		
		
	}
	return true;
}	
	
//! Remove option
bool	classConfigParser::RemoveOption(string section, string option)
{
	
	if (IsValidIndex(section, option) == false)
	{
		return false;
	}
	m_bEditConfig = true;
	
	vector<SECTION>::iterator i;
	vector<OPTION>::iterator i2;
	
	i = m_vectorConfig.begin() + IndexSection(section);	
	i2 = i->item.begin() + IndexOption(section, option);
	
	i->item.erase(i2);	
		
	return true;
}
	
	
//! Remove Section
bool		classConfigParser::RemoveSection(string section)
{
	if (IsValidIndex(section) == false)
	{
		return false;
	}	
	
	m_bEditConfig = true;
	
	vector<string> optionVector;
	optionVector = GetOptions(section);
	vector<string>::iterator i;
	// Remove sub options 
	for(i=optionVector.begin();i!=optionVector.end();i++)
	{
		RemoveOption(section, *i);		
	}		
	
	vector<SECTION>::iterator i2;
	i2 = m_vectorConfig.begin() + IndexSection(section);
	
	m_vectorConfig.erase(i2);	

	return true;
}
	
//! Write to file
bool classConfigParser::Write(string strPath)
{		
	
	if(strPath == "")
	{
		strPath = m_strConfigFilePath;
	}
	
	string strDir;
	strDir.assign(strPath, 0, strPath.rfind("/"));	
	if(!_mkdir(strDir.c_str())){
		return false;
	}
	
	ofstream fout;
	remove(strPath.c_str());
	fout.open(strPath.c_str(), ofstream::out);	
		
	string strLineFeed = "\n";

	string strSection, strOption;

	string strTemp;
	vector <SECTION>::iterator i;
	vector <OPTION>::iterator i2;
	
	for(i=m_vectorConfig.begin();i!=m_vectorConfig.end();++i)	
	{		
		if(i->name.length() > 0 && i->bHide == false)
		{
			
			if(i->rearComment.length() > 0)
			{
				strTemp = i->rearComment;
			} 
			else
			{
				strTemp = "";
			}
			
			if (i->bCommented)
			{
				strSection = (*i).name + (string)"\n";
			}
			else
			{
				strSection = (string)"[" + (*i).name + (string)"]" + (string)" " + strTemp + (string)"\n";
			}			
				
			try 
			{
				fout.write( strSection.c_str() , strSection.length());
			}
			catch(ios_base::failure failer)
			{	
				fout.close();
				return false;							
			}
		}
		
		for(i2=(*i).item.begin();i2!=(*i).item.end();i2++)
		{
			if(i2->rearComment.length() > 0)
			{
				strOption = (*i2).name + (string)" = " + (*i2).value + (string)" " + i2->rearComment + (string)"\n";
			}			
			else
			{
				if (StripString( (*i2).value) != "" || (*i2).bCommented == false) 
				{
					strOption = (*i2).name + (string)" = " + (*i2).value + (string)"\n";
				}
				else
				{
					strOption = (*i2).name + (string)"\n";
				}
				
			}			
			
			try
			{
				fout.write( strOption.c_str(), strOption.length());
			}
			catch(ios_base::failure failer)
                        {
                                fout.close();
                                return false;
                        }

		}
		
	}
	fout.close();
	
	m_bEditConfig = false;
	
	return true;
	
}

//! Set config file path
void		classConfigParser::SetConfigFilePath(string strPath)
{
	m_strConfigFilePath = strPath;
}

//! Set config file path
string		classConfigParser::GetConfigFilePath()
{
	return m_strConfigFilePath;
}


//! Check value
bool		classConfigParser::IsValidIndex(string section, string option)
{	
	if ( IndexSection(section) != -1 && IndexOption(section, option) != -1 )
	{
		return true;
	}
	else
	{
		return false;
	}
	
}
//! Check value
bool		classConfigParser::IsValidIndex(string section)
{
	if (  IndexSection(section) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//! Strip whitspace on both ends
string classConfigParser::StripString(string strSource, char cStrip)
{
	string strTarget;
	strTarget = StripLString(strSource, cStrip);
	strTarget = StripRString(strTarget, cStrip);
	strTarget = StripRString(strTarget, '\n');

	
	return strTarget;
}

//! Strip whitspace on left end
string classConfigParser::StripLString(string strSource, char cStrip)
{
	//printf("strSource = %s\n", strSource.c_str() );
	if(strSource.length() < 1)
	{
		return "";
	}
	string strTarget;		
	unsigned int i = 0;
	for(i=0;i<strSource.length();i++)
	{
		//printf("strSource.at(i) = %c\n", strSource.at(i));
		if(strSource.at(i) != cStrip)
		{
			break;
		}		    
	}
	strTarget.assign(strSource, i,strSource.length());
	return strTarget;	
}

//! Strip whitspace on right end
string classConfigParser::StripRString(string strSource, char cStrip)
{
	//printf("strSource = %s, size = %d \n", strSource.c_str(), strSource.length() );
	if(strSource.length() < 1)
	{
		return "";
	}
	string strTarget;
	unsigned int i = 0;
	int nLast = strSource.length()-1;		
	for(i=0;i<strSource.length();i++)
	{		
		//printf("strSource.at(i) = %c\n", strSource.at(i-1));
		if(strSource.at(nLast) != cStrip)
		{
			break;
		}		
		nLast--;
	}
	strTarget.assign(strSource, 0, nLast+1);
	return strTarget;
}

//! Return state for edit or not configuration
bool		classConfigParser::IsEditConfig()
{
	return m_bEditConfig;
}

//! Get read lines
long		classConfigParser::GetReadLines()
{
	return m_lReadLines;
}

//! Make directory recursively
bool classConfigParser::_mkdir(const char *path)
{
	char opath[MAX_STRING];
	char *p;
	size_t len;

	if (*path == '\0') return false;
	if (access(path, F_OK) == 0) return true;
	memset(opath, 0, sizeof(opath));
	strncpy(opath, path, sizeof(opath)-1);
	len = strlen(opath);
	if(opath[len - 1] == '/')
		opath[len - 1] = '\0';
	for(p = opath, p++; *p; p++)
		if(*p == '/') {
			*p = '\0';
			if(access(opath, F_OK))
			{
				mkdir(opath, S_IRWXU);
			}
			*p = '/';
		}
		if(access(opath, F_OK)){
			if(mkdir(opath, S_IRWXU) != 0){
				return false;
			}
		}
	return true;
}

void classConfigParser::split(const string& s, char c, vector<string>& v)
{
#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  int i = 0;
  int j = s.find(c);

  while (j >= 0) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);

    if (j < 0) {
      v.push_back(s.substr(i, s.length( )));
    }
  }

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

}

bool classConfigParser::parseCSV(char *path, vector<vector<string>*>& csvdata, string &errmsg)
{

#ifdef DEBUG
  TRC_PRINT_FUNC_START(stdout);
#endif

  ifstream in(path);
  bool rtv = true;
  vector<string> *p = NULL;
  string s;
  int linenum = 0;

  classLogger log;

  if (!in) {
    errmsg = strerror(errno);
    rtv = false;
    return rtv;
  }

  while (!in.eof()) {
    linenum++;
    getline(in, s, '\n');

    // SKIP: The line which starts with comment mark.
    if (s[0] == COMMENT) {
      continue;
    }

    // SKIP: The line which is null.
    if (s.size() == 0) {
      continue;
    }

    // ERROR: The line which length is over the limit.
    if (s.size() > MAX_LEN) {
      log.WriteLog_char_with_linenum(errmsg, linenum, ERR_ENV_INCMP_PARSE_OVER_LENGTH);
      log.WriteLog_syslog(errmsg);
      rtv = false;
      break;
    }

    p = new vector<string>( );
    split(s, DELIMITER, *p);

    // ERROR: The line whose number of elements is under the limit.
    if (p->size() < MIN_ELEMENT) {
      log.WriteLog_char_with_linenum(errmsg, linenum, ERR_ENV_INCMP_PARSE_UNDER_ELEMENT);
      log.WriteLog_syslog(errmsg);
      rtv = false;
      break;
    }

    // ERROR: The line whose number of elements is over the limit.
    if (p->size() > MAX_ELEMENT) {
      log.WriteLog_char_with_linenum(errmsg, linenum, ERR_ENV_INCMP_PARSE_OVER_ELEMENT);
      log.WriteLog_syslog(errmsg);
      rtv = false;
      break;
    }

    // ERROR: The line is duplicate before.
    for (vector<vector<string>*>::iterator d = csvdata.begin( ); d != csvdata.end( ); ++d) {
      if ((**d).front() == (*p).front()) {
        log.WriteLog_char_with_linenum(errmsg, linenum, ERR_ENV_INCMP_PARSE_DUPE_ELEMENT);
        log.WriteLog_syslog(errmsg);
        rtv = false;
      }
    }

    csvdata.push_back(p);

    s.clear();
  }

  // m_Logger.WriteLog_char(DEBUG_LOG, MYSELF_NAME_CUI, strTemp , NULL);
  // Writelog

#ifdef DEBUG
  TRC_PRINT_FUNC_END(stdout);
#endif

  return rtv;
}
