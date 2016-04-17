#ifndef WINDOWS_DEFS_C
#define WINDOWS_DEFS_C
#include "windows_defs.h"
#ifdef CO_MODE
#include "linux_defs.c"
#endif
#ifndef OS_WIN
#include "linux_os_2.h"
#include "linux_os_2.c"
#include <stdio.h>
static int my___find(const char * search, const char * string, unsigned long long * size_search_, unsigned long long * size_string_)
	{
		unsigned long long size_search = size_search_==NULL?0:(*size_search_);
		unsigned long long size_string = size_string_==NULL?0:(*size_string_);
		
		unsigned long long i=0;
		if(size_search==0)
		{
			while(1)
			{
				if(search[i]!=0)
				{
					size_search++;
				} else {
					break;
				}
			}
			*size_search_=size_search;
		}
		if(size_string==0)
		{
			while(1)
			{
				if(string[i]!=0)
				{
					size_string++;
				} else {
					break;
				}
			}
			*size_string_=size_string;
		}
		if(size_search>size_string)
		{
			return 0;
		} else if (size_search==size_string)
		{
			for(i=0; i < size_search; i++)
			{
				if(string[i]!=search[i])
				{
					return 0;
				}
			}
			return 1;
		}
		char string_target[size_search];
		unsigned long long string_target_i=0;
		for(i=0;i<size_string;i++)
		{
			if(string_target_i==size_search)
			{
				return 1;
			} else if(string[i]==search[string_target_i])
			{
				string_target_i++;
			} else {
				string_target_i=0;
			}
		}
		if(string_target_i==size_search)
		{
			return 1;
		} else {
			return 0;
		}
	}
	
static int my___grep(const char * filename, const char * keyword, const char * grepfile)
	{
		size_t s = 450;
		char line[500];
		char * r = NULL;
		unsigned long long cv = (unsigned long long)(&line[0]);
                r = (char*)cv;
		FILE * stream = NULL;
		FILE * stream2 = NULL;
		stream = fopen(filename,"r");
		if(stream==NULL)
			return 0;
		memset(line,0,500);
		stream2 = fopen(grepfile,"wb");
		if(stream2==NULL)
			return 0;
		memset(line,0,500);
		int x = getline(&r, &s, stream);
		unsigned long long ss;
		unsigned long long ss_;
		unsigned long long counter = 0;
		while(x>0)
		{
			if((my___find(keyword,r,&ss,&ss_))==1)
			{
					line[ss_++]='\r';
					line[ss_++]='\n';
					line[ss_++]=0;
					fwrite(line,1,(ss_),stream2);
					ss_=0;
					counter++;
			}
			x = getline(&r, &s, stream);
		}
		fclose(stream);
		fclose(stream2);
		return counter;
	}
#endif
int vma_iterate_func(void *data,unsigned long long start, unsigned long long end,unsigned int flags){
	vma_it_func * x = (vma_it_func*)data;
	if((x->start_address>=start))
	{
		x->base_start_address=start;
		x->base_end_address=end;
		x->base_flags=flags;
		x->ret=1;
		return 1;
	} else {
		x->ret=0;
		return 0;
	}
}

int vma_iterate_full_addressing_func(void *data,unsigned long long start, unsigned long long end,unsigned int flags){
	vma_it_func * x = (vma_it_func*)data;
	if((x->start_address>=start)&&(end<=x->end_address))
	{
		x->base_start_address=start;
		x->base_end_address=end;
		x->base_flags=flags;
		x->ret=1;
		return 1;
	} else {
		x->ret=0;
		return 0;
	}
}
#ifndef OS_WIN

LPVOID WINAPI VirtualAlloc(LPVOID lpAddress,SIZE_T dwSize,DWORD flAllocationType,DWORD flProtect)
{
	#ifdef OS_UNIX_STRUCT
		void * address = (void*)lpAddress;
		size_t size = (size_t)dwSize;
		int prot=0;
		 switch (flProtect & ~(PAGE_GUARD|PAGE_NOCACHE))
              {
              case PAGE_READONLY:
                prot = PROT_READ;
                break;
              case PAGE_READWRITE:
              case PAGE_WRITECOPY:
                prot = PROT_READ | PROT_WRITE;
                break;
              case PAGE_EXECUTE:
               prot = PROT_EXEC;
                break;
              case PAGE_EXECUTE_READ:
                prot = PROT_READ | PROT_EXEC;
                break;
              case PAGE_EXECUTE_READWRITE:
              case PAGE_EXECUTE_WRITECOPY:
				prot = PROT_READ | PROT_WRITE | PROT_EXEC;
                break;
              case PAGE_NOACCESS:
              default:
                prot = PROT_NONE;
                break;
              }
		if(address!=NULL)
		{
			if((sysconf(_SC_PAGESIZE)==-1))
					return NULL;
			unsigned long pagesize = (unsigned long long)sysconf(_SC_PAGESIZE);
			address = (void *)(((unsigned long long)address) & ~(pagesize - 1));
			unsigned long nsize_ = (((unsigned long long)size) / pagesize);
			if(((unsigned long long)(nsize_ % pagesize))!=0)
			{
				nsize_++;
			}
			nsize_ = nsize_ * pagesize;
			size=(size_t)nsize_;
		}
		
		if(flAllocationType&MEM_RESERVE)
		{
			address=mmap(address, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
			if(address==NULL)
				return NULL;
			msync(address, size, MS_SYNC|MS_INVALIDATE);
		}
		if(flAllocationType&MEM_COMMIT)
		{
			address=mmap(address, size, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED|MAP_ANON, -1, 0);
			if(address==NULL)
				return NULL;
			msync(address, size, MS_SYNC|MS_INVALIDATE);
		}
		int r = mprotect(address,size,prot);
		if(r==-1)
			return NULL;
		return address;
		
	#endif
	return NULL;
}
BOOL WINAPI VirtualFree(LPVOID lpAddress,SIZE_T dwSize,DWORD  dwFreeType)
{
	#ifdef OS_UNIX_STRUCT
		void * address=(void*)lpAddress;
		size_t size=(size_t)dwSize;
		if(address!=NULL)
		{
			if((sysconf(_SC_PAGESIZE)==-1))
					return FALSE;
			unsigned long pagesize = (unsigned long long)sysconf(_SC_PAGESIZE);
			address = (void *)(((unsigned long long)address) & ~(pagesize - 1));
			unsigned long nsize_ = (((unsigned long long)size) / pagesize);
			if(((unsigned long long)(nsize_ % pagesize))!=0)
			{
				nsize_++;
			}
			nsize_ = nsize_ * pagesize;
			size=(size_t)nsize_;
		}
		if(dwFreeType&MEM_DECOMMIT)
		{
			mmap(address, size, PROT_NONE, MAP_FIXED|MAP_PRIVATE|MAP_ANON, -1, 0);
			msync(address, size, MS_SYNC|MS_INVALIDATE);
		}
		if(dwFreeType&MEM_RELEASE)
		{
			 msync(address, size, MS_SYNC);
			munmap(address, size);
		}
		return TRUE;
	#endif
	return FALSE;
}

BOOL WINAPI VirtualProtect(LPVOID lpAddress,SIZE_T dwSize,DWORD  flNewProtect,PDWORD lpflOldProtect) {
	#ifdef OS_UNIX_STRUCT
		if(lpAddress==NULL||dwSize<=0||flNewProtect<=0)
			return FALSE;
		if(lpflOldProtect==NULL)
			return FALSE;
		MEMORY_BASIC_INFORMATION lpBuffer;
		SIZE_T my_ret = VirtualQuery(lpAddress,&lpBuffer,dwSize);
		if(my_ret==0||lpBuffer.State==MEM_FREE)
			return FALSE;
		void * address = (void*)lpAddress;
		size_t size = (size_t)dwSize;
		int prot=0;
		 switch (flNewProtect & ~(PAGE_GUARD|PAGE_NOCACHE))
              {
              case PAGE_READONLY:
                prot = PROT_READ;
                break;
              case PAGE_READWRITE:
              case PAGE_WRITECOPY:
                prot = PROT_READ | PROT_WRITE;
                break;
              case PAGE_EXECUTE:
               prot = PROT_EXEC;
                break;
              case PAGE_EXECUTE_READ:
                prot = PROT_READ | PROT_EXEC;
                break;
              case PAGE_EXECUTE_READWRITE:
              case PAGE_EXECUTE_WRITECOPY:
				prot = PROT_READ | PROT_WRITE | PROT_EXEC;
                break;
              case PAGE_NOACCESS:
              default:
                prot = PROT_NONE;
                break;
              }
		if(address!=NULL)
		{
			if((sysconf(_SC_PAGESIZE)==-1))
					return NULL;
			unsigned long pagesize = (unsigned long long)sysconf(_SC_PAGESIZE);
			address = (void *)(((unsigned long long)address) & ~(pagesize - 1));
			unsigned long nsize_ = (((unsigned long long)size) / pagesize);
			if(((unsigned long long)(nsize_ % pagesize))!=0)
			{
				nsize_++;
			}
			nsize_ = nsize_ * pagesize;
			size=(size_t)nsize_;
		}
		
		int ret = mprotect(address, size, prot);
		if(ret!=-1)
		{
			return TRUE;
		}
		return FALSE;
	#endif
	return FALSE;
}
/*SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength) {
	#ifdef IS_LINUX_OS_DEFINED
		vector_c free_blocks;
		int r = GET_FREE_BLOCKS_EX(lpAddress, 0, (&free_blocks));
		if(r==0)
		{
			return (SIZE_T)r;
		}
		VECTOR_C_CAP_TYPE total = VECTOR_TOTAL_C(free_blocks);
		VECTOR_C_CAP_TYPE i;
		struct_saver * ss;
		my_memory_block * mem;
		for(i=0; i < total; i++)
		{
			ss = VECTOR_GET_C_STRUCT(free_blocks,i);
			mem = (my_memory_block*)ss->item;
			
		}
		my_memory_block blockx;
		r = TO_ONE_BLOCK_EX((&free_blocks),(&blockx));
		if(r==0)
		{
			return (SIZE_T)r;
		}
		
	#endif
	return (SIZE_T)0;
}*/
#endif
SIZE_T WINAPI VirtualQueryUnix(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength)
{
	#ifndef OS_UNIX_STRUCT
		return (SIZE_T)VirtualQuery(lpAddress,lpBuffer,dwLength);
	#else
		MEMORY_BASIC_INFORMATION old_basic;
		if(lpBuffer!=NULL)
		{
			memcpy((void*)&old_basic,lpBuffer,sizeof(MEMORY_BASIC_INFORMATION));
		} else {
			memset((void*)&old_basic,0,sizeof(MEMORY_BASIC_INFORMATION));
		}
		SIZE_T xy=VirtualQuery(lpAddress,lpBuffer,dwLength);
		if(xy==0)
		{
			return (SIZE_T)0;
		}
		SIZE_T ia = 40;
		if(old_basic.State==MEM_FREE&&lpBuffer->RegionSize>0&&lpBuffer->State!=MEM_FREE&&lpBuffer->RegionSize>(dwLength+(ia*2)))
		{
			if(lpBuffer->AllocationProtect==PAGE_READWRITE||lpBuffer->AllocationProtect==PAGE_EXECUTE_READWRITE)
			{
				SIZE_T ix=0;
				SIZE_T rs=lpBuffer->RegionSize;
				SIZE_T ixp=0;
				SIZE_T rsp=dwLength+(ia*2);
				char * lxx = (char*)lpBuffer->BaseAddress;
				BOOL allok=FALSE;
				for(ix=0; ix < rs; ix++)
				{
					if(ixp>=rsp)
					{
						allok=TRUE;	
						break;
					}
					if(lxx[ix]==0)
					{
						ixp++;
					} else {
						ixp=0;
					}
				}
				//char * nb=(char*)&lxx[(ix-ixp)];
				//SIZE_T nrs=old_basic.RegionSize;
				//memcpy((void*)&nb[64],(void*)lpBuffer,sizeof(MEMORY_BASIC_INFORMATION));
				//memcpy((void*)&old_basic,(void*)lpBuffer,sizeof(MEMORY_BASIC_INFORMATION));
				//old_basic.BaseAddress = (void*)&nb[128];
				//old_basic.RegionSize = nrs;
				//memset((void*)lpBuffer,0,sizeof(MEMORY_BASIC_INFORMATION));
				//memcpy((void*)lpBuffer,(void*)&old_basic,sizeof(MEMORY_BASIC_INFORMATION));
				if(allok==TRUE)
				{
					lpBuffer->BaseAddress=(LPVOID)&lxx[((ix-ixp)+ia)];
				}
				return (SIZE_T)xy;
			} else {
				return (SIZE_T)VirtualQuery(lpAddress,lpBuffer,dwLength);
			}
		} else {
			return (SIZE_T)VirtualQuery(lpAddress,lpBuffer,dwLength);
		}
	#endif
	return (SIZE_T)VirtualQuery(lpAddress,lpBuffer,dwLength);
}
BOOL WINAPI VirtualQueryUnixAdjustment(PMEMORY_BASIC_INFORMATION lpBuffer,PMEMORY_BASIC_INFORMATION newBuffer,SIZE_T dwLength)
{
	if(lpBuffer==NULL||newBuffer==NULL)
	{
		return FALSE;
	}
	memcpy((void*)newBuffer,(void*)lpBuffer,sizeof(MEMORY_BASIC_INFORMATION));
#ifdef OS_UNIX_STRUCT
	if(lpBuffer->BaseAddress!=lpBuffer->AllocationBase&&lpBuffer->AllocationProtect==PAGE_READWRITE||lpBuffer->AllocationProtect==PAGE_EXECUTE_READWRITE)
	{
		newBuffer->State=MEM_FREE;
		newBuffer->AllocationBase=newBuffer->BaseAddress;
		newBuffer->RegionSize=dwLength;
		return TRUE;
	} else {
		return FALSE;
	}
#else
	return FALSE;
#endif
}

BOOL WINAPI VirtualQueryUnixGetFreeMemoryRegion(PMEMORY_BASIC_INFORMATION lpBuffer,PMEMORY_BASIC_INFORMATION newBuffer,SIZE_T dwLength)
{

	return VirtualQueryUnixAdjustment(lpBuffer,newBuffer,dwLength);
}
#ifndef OS_WIN
SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength) {
	#ifdef OS_UNIX_STRUCT
		if(lpAddress==NULL||lpBuffer==NULL||dwLength<=0)
			return (SIZE_T)0;
		if((sysconf(_SC_PAGESIZE))==-1)
			return (SIZE_T)0;
		memset((void*)lpBuffer,0,sizeof(MEMORY_BASIC_INFORMATION));
		unsigned long long pagesize;
		vma_it_func zz;
		int x=0;
		int y=0;
		int errno_ = 0;
		void * address=NULL;
		void * test_address=NULL;
		unsigned long long nsize_ =0;
		unsigned  long long psp = 0;
		unsigned  long long rpsp = 0;
		pagesize = (unsigned long long)sysconf(_SC_PAGESIZE);
		address = (void *)(((unsigned long long)lpAddress) & ~(pagesize - 1));
		nsize_ = (((unsigned long long)dwLength) / pagesize);
		if(((unsigned long long)(nsize_ % pagesize))!=0)
		{
			nsize_++;
		}
		nsize_ = nsize_ * pagesize;
		void * testx_address = NULL;
		testx_address=(void*)sbrk(0);
		if(testx_address!=NULL&&(((unsigned long long)(address)) > ((unsigned long long)(testx_address))))
		{
			zz.complete_free_region=1;
		} else {
			x = msync(address, (size_t)(nsize_), 0);
			errno_=errno;
			if(x==-1&&errno_==ENOMEM)
			{
				zz.complete_free_region=1;
			} else {
				zz.complete_free_region=0;
			}
		}
		if(zz.complete_free_region==0)
		{
			x = 0;
			zz.start_address=(unsigned long long)lpAddress;
			zz.end_address=(unsigned long long)lpAddress;
			zz.base_start_address=0;
			zz.base_end_address=0;
			vma_iterate(((vma_iterate_callback_fn)&vma_iterate_func),(void*)&zz);
			if(zz.ret!=1)
			{
				zz.ret=0;
				vma_iterate(((vma_iterate_callback_fn)&vma_iterate_full_addressing_func),(void*)&zz);
				if(zz.ret!=0)
				{
					return (SIZE_T)0;
				}
				x=1;
			}
			if(x==1)
			{
				zz.start_address=(unsigned long long)address;
				zz.end_address=(unsigned long long)(address+nsize_);
				zz.size=(unsigned long long)dwLength;
				zz.page_size=pagesize;
				zz.page_alignment_size=nsize_;
				zz.free_pages=0;
				zz.free_pages_size=0;
				zz.reserved_pages_size=nsize_;
				zz.reserved_pages=(zz.reserved_pages_size/zz.page_size);
			} else {
				/*
			    test_address=(void*)zz.base_start_address;
				for(rpsp = 0; rpsp < nsize_; rpsp+=pagesize)
				{
					x = msync(test_address, (size_t)(rpsp+pagesize), 0);
					errno_=errno;
					if(x==-1&&errno_=ENOMEM)
					{
						break;
					} else {
						continue;
					}
				}
				test_address=(void*)(((unsigned long long)test_address)+rpsp);
				for(psp = 0; psp < nsize_; psp+=pagesize)
				{
					x = msync(test_address, (size_t)(psp+pagesize), 0);
					errno_=errno;
					if(x=-1&&errno_=ENOMEM)
					{
						continue;
					} else {
						break;
					}
				}
			}
			
				int partwise_free=0;
				if(rpsp==0)
				{
					partwise_free=1;
				} else {
					partwise_free=0;
				}*/
				if(zz.base_end_address<=zz.base_start_address)
					return (SIZE_T)0;
				psp=(zz.base_end_address-zz.base_start_address);
				psp+=(psp%pagesize);
				//vma_it_func zz;
				zz.start_address=(unsigned long long)zz.base_start_address;
				zz.end_address=(unsigned long long)(zz.base_end_address);
				zz.size=(unsigned long long)dwLength;
				zz.page_size=pagesize;
				zz.page_alignment_size=psp;
				zz.free_pages=0;
				zz.free_pages_size=0;
				zz.reserved_pages_size=psp;
				zz.reserved_pages=(zz.reserved_pages_size/zz.page_size);
			}
		} else {
			zz.start_address=(unsigned long long)address;
			zz.end_address=(unsigned long long)(address+nsize_);
			zz.size=(unsigned long long)dwLength;
			zz.page_size=pagesize;
			zz.page_alignment_size=nsize_;
			zz.free_pages=(nsize_/zz.page_size);
			zz.free_pages_size=nsize_;
			zz.reserved_pages_size=0;
			zz.reserved_pages=0;
		}
		/*int y = vma_iterate((void*)&zz,&vma_iterate_func);
		x = msync(address, (size_t)(nsize_), 0);
		errno_=errno;
		if(x==-1&&errno_=ENOMEM)
		{
			zz.complete_free_region=1;
		} else {
			zz.complete_free_region=0;
		}
		if(zz.complete_free_region==1)
		{
			lpBuffer->BaseAddress=zz.base_start_address;
			lpBuffer->RegionSize=nsize_;
			lpBuffer->State=MEM_FREE;
			
		} else if(free_pages_size>0)
		{
			
		}*/
		int flags=zz.base_flags;
		if(zz.complete_free_region==1)
		{
			lpBuffer->BaseAddress=(void*)zz.start_address;
			lpBuffer->RegionSize=zz.page_alignment_size;
			lpBuffer->State=MEM_FREE;
		} else {
			lpBuffer->BaseAddress=(void*)zz.start_address;
			lpBuffer->AllocationBase=(void*)zz.base_start_address;
			if((VMA_PROT_READ | VMA_PROT_WRITE | VMA_PROT_EXECUTE)==flags)
			{
				lpBuffer->AllocationProtect=PAGE_EXECUTE_READWRITE;
			} else if((VMA_PROT_READ | VMA_PROT_EXECUTE)==flags)
			{
				lpBuffer->AllocationProtect=PAGE_EXECUTE_READ;
			} else if((VMA_PROT_READ | VMA_PROT_WRITE)==flags)
			{
				lpBuffer->AllocationProtect=PAGE_READWRITE;
			} else if(VMA_PROT_READ==flags)
			{
				lpBuffer->AllocationProtect=PAGE_READONLY;
			} else if(flags==VMA_PROT_EXEC)
			{
				lpBuffer->AllocationProtect=PAGE_EXECUTE;
			} else if(flags > 0) {
				lpBuffer->AllocationProtect=PAGE_READWRITE;
			} else {
				lpBuffer->AllocationProtect=PAGE_NOACCESS;
			}
			lpBuffer->RegionSize=zz.page_alignment_size;
			lpBuffer->State=(flags&VMA_PRIVATE)?MEM_RESERVE:MEM_COMMIT;
			lpBuffer->Type=(flags&VMA_PRIVATE)?MEM_PRIVATE:MEM_MAPPED;
		}
		return (SIZE_T)sizeof(MEMORY_BASIC_INFORMATION);
	#endif
}
BOOL WINAPI FlushInstructionCache(HANDLE  hProcess, LPCVOID lpBaseAddress, SIZE_T  dwSize)
{
	#ifdef OS_UNIX_STRUCT
		int xyz___=sizeof(int);
		if(xyz___==2)
		{
			xyz___=0x7FFF;
		} else if(xyz___==4)
		{
			xyz___=0x7FFFFFFF;
		} else if(xyz___==8) {
			xyz___=0x7FFFFFFFFFFFFFFF;
		} else {
			return FALSE;
		}
		unsigned long long x=(dwSize/xyz___);
		int y=(dwSize%xyz___);
		BOOL z = FALSE;
		char * pointer=(char*)lpBaseAddress;
		for(;x>0;x--)
		{
			z = (cacheflush(pointer, xyz___,  ICACHE) == 0);
			if((x-1)!=0)
			{
				pointer = (char*)(((unsigned long long)pointer)+((unsigned long long)xyz___));
			}
		}
		if(y>0)
		{
			z = (cacheflush(pointer, y,  ICACHE) == 0);
		}
		return z;
	#endif
	return FALSE;
}
HANDLE WINAPI GetCurrentProcess()
{
	#ifdef OS_UNIX_STRUCT
	#endif
	return NULL;
}
#endif
#ifdef _WINDOWS_HELPER_TO_BOOL_IS_DEFINED
#endif
#ifdef _WINDOWS_HELPER_TO_HEX_IS_DEFINED
char * _WINDOWS_HELPER_TO_HEX_STRING(unsigned long long x)
{
	static char res[17];
        res[16] = '\0';
        res[0] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF0000000000000000)) >> 60));   
        res[1] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF000000000000000)) >> 56));   
        res[2] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF0000000000000)) >> 52));   
        res[3] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF000000000000)) >> 48));   
        res[4] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF00000000000)) >> 44));   
        res[5] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF0000000000)) >> 40));   
        res[6] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF000000000)) >> 36));   
        res[7] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF00000000)) >> 32));   
        res[8] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF0000000)) >> 28));   
        res[9] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF000000)) >> 24));   
        res[10] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF00000)) >> 20));   
        res[11] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF0000)) >> 16));   
    	res[12] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0xF000)) >> 12));   
    	res[13] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0x0F00)) >> 8));
    	res[14] = _WINDOWS_HELPER_TO_HEX(((x & ((unsigned long long)0x00F0)) >> 4));
        res[15] = _WINDOWS_HELPER_TO_HEX((x & ((unsigned long long)0x000F)));
	char * p = (char*)&res[0];
	while(*p!=0)
	{
		if(*p!='0')
		{
			break;
		}
		p++;
	}
	if(*p==0)
	{
		p--;
	}
        return p;
}
#endif
#endif
