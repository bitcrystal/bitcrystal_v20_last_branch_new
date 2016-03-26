#include "windows_defs.h"
#ifndef OS_WIN
#define IS_NOT_WINDOWS
#ifdef IS_NOT_WINDOWS
#ifndef IS_LINUX_OS_DEFINED
	#ifdef __linux__
		#define IS_LINUX_OS_DEFINED;
	#endif
#endif
#endif
#ifdef IS_LINUX_OS_DEFINED
#include "linux_os.h"
#include "linux_os_2.h
#endif
#ifdef IS_NOT_WINDOWS
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
		int x = getline(&line, &s, stream);
		unsigned long long ss;
		unsigned long long ss_;
		unsigned long long counter = 0;
		while(x>0)
		{
			if((my___find(keyword,line,&ss,&ss_))==1)
			{
					line[ss_++]='\r';
					line[ss_++]='\n';
					line[ss_++]=0;
					fwrite(line,1,(ss_),stream2);
					ss_=0;
					counter++;
			}
			x = getline(&line, &s, stream);
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

LPVOID WINAPI VirtualAlloc(LPVOID lpAddress,SIZE_T dwSize,DWORD flAllocationType,DWORD flProtect)
{
	#ifdef IS_LINUX_OS_DEFINED
		void * address = mmap((void*)lpAddress, (size_t)dwSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		return (LPVOID)address;
	#endif
	return NULL;
}
BOOL WINAPI VirtualProtect(LPVOID lpAddress,SIZE_T dwSize,DWORD  flNewProtect,PDWORD lpflOldProtect) {
	#ifdef IS_LINUX_OS_DEFINED
		return TRUE;
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
SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength) {
	#ifdef IS_LINUX_OS_DEFINED
		if(lpAddress==NULL||lpBuffer==NULL||dwLength<=0)
			return (SIZE_T)0;
			
		memset((void*)lpBuffer,0,sizeof((*lpBuffer)));
		#include <errno.h>
		#if defined(ENOMEM) && !defined(EFAULT)
			#define EFAULT ENOMEM
		#elif defined(EFAULT) && !defined(ENOMEM)
			#define ENOMEM EFAULT
		#endif
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
		x = msync(address, (size_t)(nsize_), 0);
		errno_=errno;
		if(x==-1&&errno_==ENOMEM)
		{
			zz.complete_free_region=1;
		} else {
			zz.complete_free_region=0;
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
					return 0;
				}
				x=1;
			}
			if(x==1)
			{
				zz.start_address=(unsigned long long)address;
				zz.end_address=(unsigned long long)(address+nsize_);
				zz.size=(unsigned long long)dwLength;
				zz.page_size=page_size;
				zz.page_alignment_size=nsize_;
				zz.free_pages=0;
				zz.free_pages_size=0;
				zz.reserved_pages_size=nsize_;
				zz.reserved_pages=(reserved_pages_size/page_size);
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
				zz.page_size=page_size;
				zz.page_alignment_size=psp;
				zz.free_pages=0;
				zz.free_pages_size=0;
				zz.reserved_pages_size=psp;
				zz.reserved_pages=(reserved_pages_size/page_size);
			}
		} else {
			zz.start_address=(unsigned long long)address;
			zz.end_address=(unsigned long long)(address+nsize_);
			zz.size=(unsigned long long)dwLength;
			zz.page_size=page_size;
			zz.page_alignment_size=nsize_;
			zz.free_pages=(nsize_/page_size);
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
		if(zz.complete_free_region==1)
		{
			lpBuffer->BaseAddress=zz.start_address;
			lpBuffer->RegionSize=zz.page_alignment_size;
			lpBuffer->State=MEM_FREE;
		} else {
		{
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
		return (SIZE_T)sizeof((*lpBuffer));
	#endif
}
#endif