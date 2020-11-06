#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
int packfile(struct dirent* entry,struct stat statbuf,int output,int Meta);
int pack(char* source, int output,int Meta);
int sdir(char* dir,int output,int Meta,int depth);

int main(int argc, char** argv)
{
	char* action;
	char* source;
	char* out="";
	int output,Meta;
	if((argc < 3) && (argc>5)){
		fprintf(stderr,"ОШИБКА. Неверное кол-во аргументов(>5 или <3)\n");
		exit(-1);
	}
	
	action=argv[1];
	source=argv[2];
	out="Arch.a";
	if(argc>3)
		out=argv[3];
	if((output=open(out,O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR))==-1)
		return -3;
	if((Meta=open("Metadata",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))==-1)
		return -2;
	if(strcmp(action,"-p")==0){
		pack(source,output,Meta);
		printf ("%s успешно запакован в %s\n",source,out);
	}else{
		//unpack(source,out);
		printf ("%s успешно распакован в %s",source,out);
	}
	exit(0);
}



int pack(char* source, int output,int Meta)
{
	printf("IN PACK\n");
	int i,o,m;
	int depth=0;
	sdir(source,output,Meta,depth);
}

int sdir(char* dir,int output,int Meta,int depth)
{
	printf("IN SDIR\n");
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;
	if((dp=opendir(dir))==NULL){
		fprintf(stderr,"Невозможно открыть директорию: %s\n",dir);
		return -1;
	}
	chdir(dir);
	while ((entry=readdir(dp))!=NULL){
		lstat(entry->d_name,&statbuf);
			if(S_ISDIR(statbuf.st_mode)){
				if((strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0))
					continue;
					depth++;
					packfile(entry,statbuf,output,Meta);
					sdir(entry->d_name,output,Meta,depth);
				}else
					packfile(entry,statbuf,output,Meta);
	}
	depth--;
	chdir("..");
	closedir(dp);
	return 0;
}

int packfile(struct dirent* entry,struct stat statbuf,int output,int Meta)
{
	printf("IN PACKFILE\n");
	unsigned char buf[1024];
	unsigned int md[257];
	int in;
	int nread;
	memset(md,0,257);
	unsigned int i=0;
	for(int i=0;i<strlen(entry->d_name);i++)
		md[i]=(entry->d_name)[i];
	if(!S_ISDIR(statbuf.st_mode)){
		if((in=open(entry->d_name,O_RDONLY))==-1)
			return -1;
		while ((nread=read(in,buf,sizeof(buf)))>0){
			if(write(output,buf,nread)!=nread)
				return -4;
				md[256]=nread+md[256];
		}
	}
	else
		md[256]=0;
	write(Meta,md,sizeof(md));
	return 0;
}
