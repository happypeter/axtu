/*!
@file classConfCtl.h
@brief Class header file for controling the configure file
*/

#ifndef CLASSCONFCTL_H_
#define CLASSCONFCTL_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "classConfigParser.h"

#define BUFLEN 512
#define TEMP_CONFIG_FILE "/var/tmp/axtu.conf.temp"

const mode_t g_file_mode_for_axtu = 0600;

class classConfCtl
{
public:
	classConfCtl(void);
	~classConfCtl(void);
	bool ConfigCheck(void);
	
private:
	bool NewConfCheck(void);
	bool ExConfCheck(void);
	bool ModifyExConfFile(void);
	bool MakeNewConf(void);
	bool MakeDefaultConf(void);
	bool FileCopy(char *szOrgFileName,char *szDestFileName);

	classConfigParser *m_NewConfParser;
	classConfigParser *m_OldConfParser;
	classConfigParser *m_TempConfParser;
};

#endif /* CLASSCONFCTL_H_ */
