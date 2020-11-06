#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

void pack(char* source, char* out)
{
	DIR* dp;
	int i,o,m;
	int depth=0;
	char buf[1024];
	struct dirent* entry;
	struct stat statbuf;
	if((dp=opendir(source))==NULL){
		fprintf(stderr,"Невозможно открыть директорию: %s\n",source);
		return;
	}
	chdir(source);
	while ((entry=readdir(dp))!=NULL){
		lstat(entry->d_name,&statbuf);
			if(S_ISDIR(statbuf.st_mode)){
				if((strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0))
					continue;
					
				}
	}
}
