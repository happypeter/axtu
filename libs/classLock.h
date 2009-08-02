#include <sys/stat.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "semun.h"
#include <stdio.h>

//#include <fstream.h>

using namespace std;

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */
static int sem_id;

static int set_semvalue(void)
{
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
    	return(0);
    }
    return(1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */
static int del_semvalue(void)
{
    union semun sem_union;

    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    {        
        return (0);
    }
    return (1);
}

/* semaphore_p changes the semaphore by -1 (waiting). */

static int semaphore_p(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {        
        return 0;
    }
    return 1;
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

static int semaphore_v(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {        
        return 0;
    }
    return 1;
}

/*
static int isproc(int pid, char *exe)
	{
		char procfile[80];
	  	char null[40];
	    char exename[80];
	    FILE *fp;
	    sprintf(procfile, "/proc/%d/stat", pid);
	
	    if (access(procfile, F_OK) != 0)
	    {
	        return 0;
	    }
	    if (exe == NULL)
	    {
	        return 1;
	    }
	    if ((fp = fopen(procfile, "r")) == NULL)
	        return 0;
	    fscanf(fp, "%s %s",null, exename);
	    fclose(fp);
	    if (strstr(exename, exe) != NULL)
	        return 1;
	    return 0;
	}
	static bool lock(const char * strFilePath, const char * strExe)
	{
        sem_id = semget((key_t) 2007, 1, 0666 | IPC_CREAT);
        if (!set_semvalue()) {        
            return true;
        }
        
        if (!semaphore_p()) 
        {
            return true; 
        }

		uint 		nOldPID;
		int 		nResult=0;
		uint	 	nPID;
		
		char *oldPID = NULL;
		size_t sizeOldPid;
		char strPID[256];
		// Get current pid
		nPID = (int)getpid();	
		sprintf(strPID, "%d",nPID);
	
		if(access(strFilePath, F_OK) == 0)
		{			
			FILE * fin;
			fin = fopen(strFilePath, "r");			
			if( getline(&oldPID, &sizeOldPid, fin) < 0)
			{	
				fclose(fin);
				return true;
			}
			
			nOldPID = atoi(oldPID);		
			// File is empty
			if (nOldPID == 0)
			{
				nResult = -1;
			}			
			// Try to check pid
			else
			{
				//nResult = kill(nOldPID, 0);
				if (isproc(nOldPID, (char *)strExe) == 0)
				{
					nResult = -1;
				}
			}
			fclose(fin);	
		}
		else
		{	
			nResult = -1;
		}       
        	
		// can execute
		if(nResult == -1)
		{				
			FILE * fout;
			remove(strFilePath);
			fout = fopen(strFilePath, "w");
			fwrite(strPID , sizeof(char) , strlen(strPID), fout);
			if(chmod(strFilePath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) != 0)
			{
				return true;				
			}
			fclose(fout);			
	
		}
		// cannot execute
		else
		{	
			return true;
		}			

        if (!semaphore_v())
        {
            return true;
        }
        //sleep(1);
        if (del_semvalue() == 0)
            return true;
        
        return false;
	}
*/
class CHSLock
{
public:
	static int isproc(int pid, char *exe)
	{
		
		char procfile[80];
	  	char null[40];
	    char exename[80];
	    FILE *fp;
	    sprintf(procfile, "/proc/%d/stat", pid);
	
	    if (access(procfile, F_OK) != 0)
	    {
	        return 0;
	    }
	    if (exe == NULL)
	    {
	        return 1;
	    }
	    if ((fp = fopen(procfile, "r")) == NULL)
	        return 0;
	    if( fscanf(fp, "%s %s",null, exename)  == 0)
	    {
	    	return 1;
	    }
	    fclose(fp);
	    if (strstr(exename, exe) != NULL)
	        return 1;
	    return 0;
	}
	static bool lock(const char * strFilePath)
	{
		uint	 	nPID;
		char strPID[256];
		// Get current pid
		nPID = (int)getpid();	
		sprintf(strPID, "%d",nPID);
						
		FILE * fout;
		if(access(strFilePath, F_OK) == 0)
		{
			if(remove(strFilePath) != 0)
			{
				return false;
			}
		}
		fout = fopen(strFilePath, "w");
		if( fwrite(strPID , sizeof(char) , strlen(strPID), fout) == 0)
		{
			return false;
		}
		if(chmod(strFilePath, 0600) != 0)
		{
			return false;				
		}
		
		fclose(fout);
		
		return true;
	}
	static bool Islock(const char * strFilePath, const char * strExe)
	{
        sem_id = semget((key_t) 2007, 1, 0666 | IPC_CREAT);
        if (!set_semvalue()) {        
            return true;
        }
        
        if (!semaphore_p()) 
        {
            return true; 
        }

		uint 		nOldPID;
		int 		nResult=0;
		uint	 	nPID;
		
		char *oldPID = NULL;
		size_t sizeOldPid;
		char strPID[256];
		// Get current pid
		nPID = (int)getpid();	
		sprintf(strPID, "%d",nPID);
	
		if(access(strFilePath, F_OK) == 0)
		{			
			FILE * fin;
			fin = fopen(strFilePath, "r");			
			if( getline(&oldPID, &sizeOldPid, fin) < 0)
			{	
				fclose(fin);
				return true;
			}
			
			nOldPID = atoi(oldPID);		
			// File is empty
			if (nOldPID == 0)
			{
				nResult = 1;
			}			
			// Try to check pid
			else
			{
				//nResult = kill(nOldPID, 0);
				if (isproc(nOldPID, (char *)strExe) == 0)
				{
					nResult = 1;
				}
				else
				{
					nResult = 3;
				}
			}
			fclose(fin);			
	
		}
		else
		{	
			nResult = 1;
		}       
		

        if (!semaphore_v())
        {
            return true;
        }
        //sleep(1);
        if (del_semvalue() == 0)
            return true;
        
        
        if(nResult == 0)
        {	
        	return false;						
        }        
        else if(nResult == 1)
        {	
        	return false;
        }			
        else if(nResult == 2)
        {	
        	return false;
        }
        else
        {
        	return true;
        }
	}
};
