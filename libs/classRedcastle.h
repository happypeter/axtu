#ifndef CLASSREDCASTLE_H_
#define CLASSREDCASTLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RC_EVRC "/etc/.evrc"
#define MODULES "/proc/modules"
#define RC_MOD_NAME "RedCastle"
#define RC_MODE "RC_MODE"

#define RC_STATUS_ENABLE 0
#define RC_STATUS_WARNING 1
#define RC_STATUS_DISABLE 2 
#define RC_STATUS_STOP 3
#define RC_STATUS_NOT_INSTALL 4
#define RC_STATUS_WRONG_INSTALL 5

#define RC_MODE_START 0
#define RC_MODE_STOP 1
#define RC_MODE_NOT_INSTALL 2

#define RC_ENABLE "ENABLE"
#define RC_WARNING "WARNING"


#define REDCASTLE_ENABLE_MSG_TR tr("RedCastle is currently in Enabled mode. \nAXTU cannot function properly unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function.")

#define REDCASTLE_ENABLE_MSG "RedCastle is currently in Enabled mode. \nAXTU cannot function properly unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function."

#define REDCASTLE_WARNING_MSG_TR tr("RedCastle is currently in Warning mode. \nAXTU cannot function properly unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function.")

#define REDCASTLE_WARNING_MSG "RedCastle is currently in Warning mode. \nAXTU cannot function properly unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function."

#define REDCASTLE_WARNING_CONTINUE_MSG_TR tr("RedCastle is currently in Warning mode. \nAXTU will cause RedCastle to issue Warnings about updated files unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function.\nDo you want to continue ?")

#define REDCASTLE_WARNING_CONTINUE_MSG "RedCastle is currently in Warning mode. \nAXTU will cause RedCastle to issue Warnings about updated files unless RedCastle is Disabled.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function.\nDo you want to continue ?"

#define REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG_TR tr("The Kernel package(s) below will NOT be installed because it requires a matching version of \nthe RedCastle kernel module, which is not currently available to download and install.\nRedCastle must be disabled in order to update the kernel package(s) in this situation. \nRedCastle will not funtion with the updated kernel until the RedCastle kernel module has also been updated.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function.")

#define REDCASTLE_CAN_NOT_FIND_REQUIRED_MSG "The Kernel package(s) below will NOT be installed because it requires a matching version of \nthe RedCastle kernel module, which is not currently available to download and install.\nRedCastle must be disabled in order to update the kernel package(s) in this situation. \nRedCastle will not funtion with the updated kernel until the RedCastle kernel module has also been updated.\nNote: Kernel updates require matching RedCastle kernel modules to also be updated, or RedCastle will not function."


class classRedcastle
{
public:
	classRedcastle();
	virtual ~classRedcastle();
	int GetRCStatus();
	
protected:	
	int GetRCMode();
	int IsRCStart();	
};

#endif /*CLASSREDCASTLE_H_*/
