#include WINDOWS_DEFS_2_C
#define WINDOWS_DEFS_2_C
#include "windows_defs2.h"
#include "windows_defs.c"

HANDLE WINAPI CreateFile(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	/*char pp[3];
	if((dwDesiredAccess&(FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE)))
	{
		pp[0]='w';
		pp[1]='+';
		pp[2]=0;
	} else if((dwDesiredAccess&(FILE_SHARE_READ|FILE_SHARE_DELETE))) {
		pp[0]='w';
		pp[1]='+';
		pp[2]=0;
	} else if((dwDesiredAccess&(FILE_SHARE_WRITE|FILE_SHARE_DELETE))) {
		pp[0]='w';
		pp[1]='+';
		pp[2]=0;
	} else if((dwDesiredAccess&(FILE_SHARE_READ|FILE_SHARE_WRITE)))
	{
		pp[0]='w';
		pp[1]='+';
		pp[2]=0;
	} else if((dwDesiredAccess&(FILE_SHARE_READ)))
		pp[0]='r';
		pp[1]=0;
		pp[2]=0;
	} else if((dwDesiredAccess&(FILE_SHARE_WRITE))) {
		pp[0]='w';
		pp[1]=0;
		pp[2]=0;
	}*/
	
	FILE * file = NULL;
	switch(dwCreationDisposition)
	{
		case CREATE_ALWAYS:
		{
			SetLastError(NO_ERROR);
			file = fopen((const char*)lpFileName, "rb");
			if(file!=NULL)
			{
				fclose(file);
				SetLastError(ERROR_ALREADY_EXISTS);
			}
			file = fopen((const char*)lpFileName, "wb+");
			if(file==NULL)
			{
				return NULL;
			}
			return (HANDLE)file;
		}
		break;
		
		case CREATE_NEW:
		{
			SetLastError(NO_ERROR);
			file = fopen((const char*)lpFileName, "rb");
			if(file!=NULL)
			{
				fclose(file);
				SetLastError(ERROR_FILE_EXISTS);
				return NULL;
			}
			file = fopen(const char*)lpFileName, "wb+");
			if(file==NULL)
			{
				return NULL;
			}
			return (HANDLE)file;
		}
		break;
		
		case OPEN_ALWAYS:
		{
			SetLastError(NO_ERROR);
			file = fopen((const char*)lpFileName, "rb");
			if(file==NULL)
			{
				file = fopen((const char*)lpFileName, "wb+");
				if(file==NULL)
					return NULL;
				return (HANDLE)file;
			}
			fclose(file);
			file = fopen(const char*)lpFileName, "ab+");
			if(file==NULL)
				return NULL;
			SetLastError(ERROR_ALREADY_EXISTS);
			return (HANDLE)file;
		}
		
		case OPEN_EXISTING:
		{
			SetLastError(NO_ERROR);
			file = fopen((const char*)lpFileName, "rb");
			if(file==NULL)
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
				return NULL;
			}
			fclose(file);
			file = fopen(const char*)lpFileName, "ab+");
			if(file==NULL)
				return NULL;
			return (HANDLE)file;
		}
		
		case TRUNCATE_EXISTING:
		{
			SetLastError(NO_ERROR);
			file = fopen((const char*)lpFileName, "rb");
			if(file==NULL)
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
				return NULL;
			}
			fclose(file);
			file = fopen(const char*)lpFileName, "wb+");
			if(file==NULL)
				return NULL;
			return (HANDLE)file;
		}
		break;
	}
	return NULL;
}

BOOL WINAPI ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	if(___Win32Helper___IsFileHandle(hFile)==FALSE)
	{
		return FALSE;
	}
	if(nNumberOfBytesToRead<=0)
	{
		return FALSE;
	}
	
	if(lpNumberOfBytesRead==NULL)
	{
		return FALSE;
	}
	*lpNumberOfBytesRead = 0;
	char mystring[4096];
	memset((void*)mystring,255,4096);
	FILE* pFile=(FILE*)hFile;
	DWORD filepp=0;
	DWORD filexx=0;
	DWORD llength=0;
	char * buff_pointer=(char*)lpBuffer;
	char * curr_pointer = NULL;
	curr_pointer=(char*)fgets ((char*)mystring , 255 , pFile);
	if(curr_pointer==NULL)
		return FALSE;
	 while ( curr_pointer != NULL )
	 {
	   for(filepp=4095,filepp>=0;filepp--)
	   {
			if(mystring[filepp]==255||mystring[filepp]==-127)
			{
				continue;
			}
			llength=(filepp+1);
			*lpNumberOfBytesRead+=llength;
	   }
       for(filepp=0;filepp<llength&&filexx<nNumberOfBytesToRead;filepp++,filexx++)
	   {
			buff_pointer[filexx]=mystring[filepp];
	   }
	   memset(mystring,255,4096);
	   if(feof(pFile)!=0)
	   {
		return TRUE;
	   }
	   curr_pointer=(char*)fgets ((char*)mystring , 4096 , pFile);
	 }
	 if(*lpNumberOfBytesRead>0)
		return TRUE;
	return FALSE;
}

BOOL WINAPI WriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped)
{
	if(___Win32Helper___IsFileHandle(hFile)==FALSE)
	{
		return FALSE;
	}
	if(nNumberOfBytesToWrite<=0)
	{
		return FALSE;
	}
	
	if(lpNumberOfBytesWritten==NULL)
	{
		return FALSE;
	}
	*lpNumberOfBytesWritten=0;
	char mystring[4096];
	memset((void*)mystring,0,4096);
	FILE* pFile=(FILE*)hFile;
	DWORD filepp=0;
	DWORD filexx=0;
	DWORD x = 0;
	DWORD y = 0;
	int nwrited=0;
	char * curr_pointer=NULL;
	curr_pointer=(char*)lpBuffer;
	x = (DWORD)(nNumberOfBytesToWrite / (DWORD)4096);
	y = (DWORD)(nNumberOfBytesToWrite % (DWORD)4096);
	DWORD i = 0;
	for(x=x;x>0;x--)
	{
		for(filepp=0;filepp<4096&&filexx<nNumberOfBytesToWrite;filepp++,filexx++)
		{
			mystring[filepp]=curr_pointer[filexx];
		}
		nwrited=fputs(mystring,pFile);
		if(nwrited<0)
		{
			if(*lpNumberOfBytesWritten==0)
			{
				return FALSE;
			}
			return TRUE;
		}
		*lpNumberOfBytesWritten+=nwrited;
		memset((void*)mystring,0,4096);
	}
	if(y>0)
	{
		memset((void*)mystring,0,4096);
		for(filepp=0;filepp<y&&filexx<nNumberOfBytesToWrite;filepp++,filexx++)
		{
			mystring[filepp]=curr_pointer[filexx];
		}
		nwrited=fputs(mystring,pFile);
		if(nwrited<0)
		{
			if(*lpNumberOfBytesWritten==0)
			{
				return FALSE;
			}
			return TRUE;
		}
		*lpNumberOfBytesWritten+=nwrited;
	}
	*lpNumberOfBytesWritten=nNumberOfBytesToWrite;
	return TRUE;
}

DWORD WINAPI SetFilePointer(HANDLE hFile,LONG lDistanceToMove,PLONG lpDistanceToMoveHigh,DWORD dwMoveMethod)
{
	if(___Win32Helper___IsFileHandle(hFile)==FALSE)
	{
		return INVALID_SET_FILE_POINTER;
	}
	unsigned long long distance=0;
	char * distance_p = (char*)&distance[0];
	char * p = NULL;
	if(lpDistanceToMoveHigh!=NULL)
	{
		p=(char*)lpDistanceToMoveHigh;
		distance_p[4]=p[0];
		distance_p[5]=p[1];
		distance_p[6]=p[2];
		distance_p[7]=p[3];
	}
	p=(char*)&lDistanceToMove[0];
	distance_p[0]=p[0];
	distance_p[1]=p[1];
	distance_p[2]=p[2];
	distance_p[3]=p[3];
	long * x = (long*)&distance_p[0];
	long * y = (long*)&distance_p[4];
	long rx=0;
	long ry=0;
	if(*x>0)
	{
		rx=*x;
	}
	if(*y>0)
	{
		ry=*y;
	}
	if(dwMoveMethod==FILE_BEGIN)
	{
		fseek ( pFile , 0 , SEEK_SET );
		if(rx!=0)
		{
			fseek ( pFile , rx, SEEK_CUR );
		}
		if(ry!=0)
		{
			fseek ( pFile , ry , SEEK_CUR );
		}
	} else if(dwMoveMethod==FILE_END)
	{
		char c;
		#define XP_LENGTH 1024
		#define MY_INT_MAX 0x7FFFFFFF
		long xxx[XP_LENGTH];
		int xp = 0;
		fseek ( pFile , 0 , SEEK_SET );
		for(xp=0;xp<XP_LENGTH;xp++)
		{
			xxx[xp]=0;
		}
		xp=0;
		do
		{
			c=fgetc(pFile);
			if(xp<XP_LENGTH)
			{
				if(xxx[xp]<MY_INT_MAX)
				{
					xxx[xp]++;
				} else {
					xp++;
					if(xp<XP_LENGTH)
					{
						xxx[xp]++;
					} else {
						return INVALID_SET_FILE_POINTER;
					}
				}
			} else {
				break;
			}
		} while(c != EOF);
		long i = 0;
		fseek(pFile,0,SEEK_SET);
		for(i=0;i<xp;i++)
		{
			if((i+1)==xp)
			{
				xxx[i]-=1;
				xxx[i]-=rx;
				xxx[i]-=ry;
				if(xxx[i]<0)
				{
					return INVALID_SET_FILE_POINTER;
				}
				if(xxx[i]>0)
				{
					fseek(pFile, xxx[i], SEEK_CUR);
				}
				return 1;
			}
			fseek(pFile, xxx[i], SEEK_CUR);
		}
	} else if(dwMoveMethod==FILE_CURRENT)
	{
		if(rx !=0)
		{
			fseek(pFile, rx, SEEK_CUR);
		}
		if(ry !=0)
		{
			fseek(pFile, ry, SEEK_CUR);
		}
		return 1;
	}
	return INVALID_SET_FILE_POINTER;
}

BOOL ___Win32Helper___IsFileHandle(HANDLE hObject)
{
	if(hObject==NULL)
		return FALSE;
	if(sizeof((*hobject))==sizeof(FILE))
	{
		return TRUE;
	}
	return FALSE;
}
 
BOOL CloseHandle(HANDLE hObject)
{
	if(___Win32Helper___IsFileHandle(hObject)==TRUE)
	{
		fclose((FILE*)hObject);
		return TRUE;
	}
	return FALSE;
}

void WINAPI SetLastError(DWORD set)
{
	______MY_LAST_ERROR______=set;
}

DWORD WINAPI GetLastError()
{
	return (DWORD)______MY_LAST_ERROR______;
}

#endif
