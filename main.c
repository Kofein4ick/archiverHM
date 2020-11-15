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
int sdir(char* dir,int output,int Meta);

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
		if(pack(source,output,Meta)!=0){
			fprintf(stderr,"Ошибка архивации\n");
			exit(-2);
		}
		printf ("%s успешно запакован в %s\n",source,out);
		rmdir(source);
	}else{
		//unpack(source,out);
		printf ("%s успешно распакован в %s",source,out);
	}
	exit(0);
}



int pack(char* source, int output,int Meta)
{
	printf("IN PACK\n");
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;
	if((dp=opendir("."))==NULL){
		fprintf(stderr,"Невозможно открыть директорию: %s\n",".");
		return -1;
	}
	while ((entry=readdir(dp))!=NULL){
		printf("%s\n",entry->d_name);
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode)){
			if((strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0))
				continue;
			if(strcmp(source,entry->d_name)==0)
			{
				printf("In if:%s\n",entry->d_name);
				if(packfile(entry,statbuf,output,Meta)!=0)
					return -1;
				break;
			}
		}	
	}
	printf("Out while\n");
	closedir(dp);
	if(sdir(source,output,Meta)!=0)
		return -2;
	return 0;
}

int sdir(char* dir,int output,int Meta)
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
		printf("%s\n",entry->d_name);
		if(S_ISDIR(statbuf.st_mode)){
			if((strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0))
				continue;
				if(packfile(entry,statbuf,output,Meta)!=0)
					return -1;
				if(sdir(entry->d_name,output,Meta)!=0)
					return -2;
				rmdir(entry->d_name);
			}else{
				if(packfile(entry,statbuf,output,Meta)!=0)
					return -1;
				remove(entry->d_name);
			 }
	}
	chdir("..");
	closedir(dp);
	return 0;
}

int packfile(struct dirent* entry,struct stat statbuf,int output,int Meta)
{
	printf("IN PACKFILE\n");
	unsigned char buf[1024];
	unsigned int md[2];
	char outname[256];
	int nread,in;
	memset(outname,0,256);
	memset(md,0,2);
	unsigned int i=0;
	for(int i=0;i<strlen(entry->d_name);i++)
		outname[i]=(entry->d_name)[i];
	printf("%s\n",entry->d_name);
	if(!S_ISDIR(statbuf.st_mode)){
		if(write(output,outname,sizeof(outname))!=sizeof(outname))
			return -5;
		if((in=open(entry->d_name,O_RDONLY))==-1)
			return -1;
		while ((nread=read(in,buf,sizeof(buf)))>0){
			if(write(output,buf,nread)!=nread)	
				return -2;
				md[0]+=nread;
		}
		md[1]=1;
	}
	else{
		md[0]=0;
		md[1]=2;
		if(write(output,outname,sizeof(outname))!=sizeof(outname))
			return -3;
	}
	if(write(Meta,md,sizeof(md))!=sizeof(md))
		return -4;

	return 0;
}
