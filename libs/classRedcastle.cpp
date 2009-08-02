#include "classRedcastle.h"

classRedcastle::classRedcastle()
{
}

classRedcastle::~classRedcastle()
{
}


int classRedcastle::IsRCStart()
{
	int nResult = RC_MODE_NOT_INSTALL;
	char *str1, *token;
	char *strBuf = NULL;
	
	size_t len;
	ssize_t read;	
	int j;
	bool bFind = false;
	FILE * fd=NULL;
	fd = fopen(MODULES, "r");
	if(fd == NULL)
	{
		printf("file open error : %s\n", MODULES);
		return -1;
	}
	bool bExit = false;
	while( (read = getline(&strBuf, &len, fd)) != -1)	
	{			
	 	for (j = 1, str1 = strBuf; ; j++, str1 = NULL) 
	 	{
	 		token = strtok(str1, "\t \n");	 		
	 		if (token == NULL)
	 			break;	 		
	 		
	 		if(j == 3 && bFind == true)
	 		{
	 			bFind = false;
	 			if(atoi(token) == 0)
	 			{	 				
	 				nResult = RC_MODE_STOP;
					bExit = true;
					break;
	 			}
	 			else
	 			{
	 				nResult =  RC_MODE_START;
					bExit = true;
					break;
	 			}
	 		}
	 		
 			if(strcmp((const char*)token, RC_MOD_NAME) == 0)
 			{	 	
 				
 				if (token == NULL)
 					break;
 				bFind = true;
 			}	
	 	}	
		if(bExit == true)
			break;
	}
	if(strBuf)
	{
		free(strBuf);
	}
	fclose(fd);
   return nResult;
}
int classRedcastle::GetRCMode()
{
	int nResult = RC_STATUS_WRONG_INSTALL;
	char *str1, *token;
	char * strBuf = NULL;
	size_t len;
	ssize_t read;	
	int j;
	bool bFind = false;
	FILE * fd=NULL;
	fd = fopen(RC_EVRC, "r");
	if(fd == NULL)
	{		
		return RC_STATUS_NOT_INSTALL;	
	}
	bool bExit = false;
	while( (read = getline(&strBuf, &len, fd)) != -1)	
	{			
	 	for (j = 1, str1 = strBuf; ; j++, str1 = NULL) 
	 	{
	 		token = strtok(str1, "\t \n");	 		
	 		if (token == NULL)
	 			break;	 		
	 		
	 		if(j == 2 && bFind == true)
	 		{
	 			bFind = false;	 			
	 			if(strcmp(token, RC_ENABLE) == 0)
	 			{	 
					nResult = RC_STATUS_ENABLE;
					bExit = true;
					break;
	 			}
	 			else if(strcmp(token, RC_WARNING) == 0)
	 			{
					nResult = RC_STATUS_WARNING;
					bExit = true;
					break;
	 			}
	 			else
	 			{
	 				nResult = RC_STATUS_DISABLE;
					bExit = true;
					break;
	 			}	 		
	 		}
	 		
 			if(strcmp((const char*)token, RC_MODE) == 0)
 			{	
 				if (token == NULL)
 					break;
 				bFind = true;
 			}	
	 	}	
		if(bExit == true)
                        break;	
	}
	if(strBuf)
	{
		free(strBuf);
	}
	fclose(fd);
	
	return nResult;
}

int classRedcastle::GetRCStatus()
{		
	int nStart = IsRCStart();
	if(nStart < 0)
	{
		return -1;
	}
	
	if( nStart == RC_MODE_STOP)
	{
		return RC_STATUS_STOP; 
	}
	else if(nStart == RC_MODE_START)
	{
		return GetRCMode();		
	}
	else
	{
		return RC_STATUS_NOT_INSTALL;
	}	
}
