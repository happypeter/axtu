/*!
@file classConfigParser.h
@brief Class header file for parsing a configure file' contents
*/
// Copyright 2006 HAANSOFT, INC  

#ifndef CLASSCONFIGPARSER_H_
#define CLASSCONFIGPARSER_H_

#include <string>
#include <vector>

using namespace std;

// option
typedef struct structOPTION
{
	string name;
	string value;	
	string rearComment;
	bool bCommented;
}OPTION;


// section
typedef struct structSECTION
{	
	string 			name;
	vector<OPTION> 	item;	
	string rearComment;
	bool bCommented;
	bool bHide;
	// constructor
	structSECTION()
	{
		//item.setAutoDelete(true);		
	}
	
	// destructor
	~structSECTION()
	{
		//item.clear();
		// I have to make that the member of vector will be clear. 
	}	
}SECTION;

//! This class parser *.conf file.  this class is poted to c++ from python class.
class	classConfigParser
{
public:
	classConfigParser();
	~classConfigParser();
private:	
	string 			m_strConfigFilePath;
	vector<SECTION> 	m_vectorConfig;
	bool m_bEditConfig;
	long m_lReadLines;
	
public:    

	////////////////////////////////////////
	// Read file and it puts in m_vectorConfig
	bool Read(string strPath="");
	
	
	////////////////////////////////////////
	// Get sections
	vector<string> 	GetSections(bool bHide = false);
	
	////////////////////////////////////////
	// Get options
	vector<string>	GetOptions(string	section);
	
	////////////////////////////////////////
	// Add section
	bool	AddSection(string	strSection);	

	////////////////////////////////////////
	// Get index section
	int		IndexSection(string section);
	
	////////////////////////////////////////
	// Get index option
	int		IndexOption(string section, string option);

	////////////////////////////////////////
	// Is has section?
	bool	HasSection(string	strSection);
		
	////////////////////////////////////////
	// Is has option?
	bool	HasOption(string section,string option);
	
	
	
	////////////////////////////////////////
	// Get the option value
	string		GetOption(string section, string option);
	
	////////////////////////////////////////
	// Set the option value
	bool	SetOption(string section, string option, string value);
	
	////////////////////////////////////////
	// Remove option
	bool	RemoveOption(string section, string option);
	
	
	////////////////////////////////////////
	// Remove section
	bool		RemoveSection(string section);
	
	
	////////////////////////////////////////
	// Write contens to file.
	bool		Write(string strPath="");
	
	void		SetConfigFilePath(string strPath);
	string	GetConfigFilePath();
	
	////////////////////////////////////////
	// Check value
	bool		IsValidIndex(string section, string option);
	bool		IsValidIndex(string section);
	
	bool		IsEditConfig();

    ////////////////////////////////////////
  // Strip whitspace
  string StripString(string strSource, char cStrip=' ');
  string StripLString(string strSource, char cStrip=' ');
  string StripRString(string strSource, char cStrip=' ');
		
	long		GetReadLines();
	
	bool _mkdir(const char *path);
  void split(const string& s, char c, vector<string>& v);
  bool parseCSV(char *path, vector<vector<string>*>& csvdata, string &errmsg);
	
};

#endif /*CLASSCONFIGPARSER_H_*/
