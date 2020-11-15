#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
int packfile(struct dirent* entry,struct stat statbuf,int output,int Meta,int depth);
int pack(char* source, int output,int Meta);
int sdir(char* dir,int output,int Meta,int depth);
int unpack(char* source,int Meta);

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
	if((Meta=open("Metadata",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR))==-1)
		return -2;
	if(strcmp(action,"-p")==0){
		if(argc>3){
			out=argv[3];
			strcat(out,".a");
		}
		if((output=open(out,O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR))==-1)
			return -3;
		if(pack(source,output,Meta)!=0){
			fprintf(stderr,"Ошибка архивации\n");
			remove("Metadata");
			remove(out);
			exit(-2);
		}
		printf ("%s успешно запакован в %s\n",source,out);
		rmdir(source);
	}else
	if(strcmp(action,"-u")==0){
		if(unpack(source,Meta)!=0){
			fprintf(stderr,"Ошибка распаковки\n");
			exit(-4);
		}
		printf ("%s успешно распакован в %s\n",source,out);
	}else{
		fprintf(stderr,"ОШИБКА. Неверный флаг\n");
		exit(-1);
	}
	exit(0);
}


int pack(char* source, int output,int Meta)
{
	printf("IN PACK\n");
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;
	int depth=1;
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
				if(packfile(entry,statbuf,output,Meta,depth)!=0)
					return -1;
				break;
			}
		}	
	}
	printf("Out while\n");
	closedir(dp);
	if(sdir(source,output,Meta,depth)!=0)
		return -2;
	return 0;
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
		printf("%s\n",entry->d_name);
		if(S_ISDIR(statbuf.st_mode)){
			if((strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0))
				continue;
				depth++;
				if(packfile(entry,statbuf,output,Meta,depth)!=0)
					return -1;
				if(sdir(entry->d_name,output,Meta,depth)!=0)
					return -2;
				rmdir(entry->d_name);
			}else{
				if(packfile(entry,statbuf,output,Meta,depth)!=0)
					return -3;
				remove(entry->d_name);
			 }
	}
	chdir("..");
	depth--;
	closedir(dp);
	return 0;
}

int packfile(struct dirent* entry,struct stat statbuf,int output,int Meta,int depth)
{
	printf("IN PACKFILE\n");
	unsigned char buf[1024];
	unsigned int md[3];
	char outname[256];
	int nread,in;
	memset(outname,0,256);
	memset(md,0,3);
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
			printf("buf=%s\n",buf);
			if(nread==-1)
				return -5;
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
	md[2]=depth;
	printf("md[0]=%d\n",md[0]);
	if(write(Meta,md,sizeof(md))!=sizeof(md))
		return -4;

	return 0;
}

int unpack(char* source,int Meta)
{
	printf("IN UNPACK\n");
	DIR* dp;
	int count=0;
	unsigned char buf[1024];
	int depth=0;
	int in,nread,out,i=0;
	unsigned int md[3];
	char name[256];
	memset(name,0,256);
	memset(md,0,3);
	if((in=open(source,O_RDONLY))==-1)
		return -1;
	if(read(Meta,md,sizeof(md))==-1)
		return -2;
	printf("md[0]=%d\n",md[0]);
	if(read(in,name,sizeof(name))==-1)
		return -3;
	printf("name=%s\n",name);
	if((mkdir(name,O_CREAT|S_IRUSR|S_IWUSR))==-1){
	fprintf(stderr,"Невозможно создать директорию: %s\n",name);
		return -4;
	}
	if((dp=opendir(name))==NULL){
		fprintf(stderr,"Невозможно открыть директорию: %s\n",name);
		return -5;
	}
	chdir(name);
	depth++;
	printf("While\n");
	while ((nread=read(in,name,sizeof(name)))>0){
		i++;
		printf("In while:%d",i);
		memset(buf,0,1024);
		printf("name=%s\n",name);
		printf("depth=%d\n",md[2]);
		if(read(Meta,md,sizeof(md))==-1)
			return -6;
		printf("md[0]=%d\n",md[0]);
		if(md[1]==1){
			while(depth!=md[2]){
				chdir("..");
				depth--;
			}
			if((out=open(name,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))==-1)
				return -7;
			if(md[0]>1024)
				while ((nread=read(in,buf,sizeof(buf)))>0){
				printf("buf=%s\n",buf);
					if(nread==-1)
					return -8;
				count+=nread;
				if(write(out,buf,nread)!=nread)	
					return -9;
				if(count!=md[0]-1)
					return -10;
			}
			else{
				if((nread=read(in,buf,md[0]))==-1)
					return -11;
				printf("buf=%s\n",buf);
				if(write(out,buf,nread)!=nread)	
					return -12;
			}
					
		}
		else{
			if((mkdir(name,O_CREAT|S_IRUSR|S_IWUSR))==-1)
				return -4;
			if((dp=opendir(name))==NULL){
				fprintf(stderr,"Невозможно открыть директорию: %s\n",name);
				return -5;
			}
			printf("namedir=%s\n",name);
			chdir(name);
			depth++;
		}
	}
	if(nread==-1)
		return -7;
	closedir(dp);	
	return 0;
}
