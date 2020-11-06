#include <stdio.h>
#include <string.h>
#include "arch.h"
int main(int argc, char** argv)
{
	char* action;
	char* source;
	char* out="";
	if((argc < 3) && (argc>5)){
		fprintf(stderr,"ОШИБКА. Неверное кол-во аргументов(>5 или <3)\n");
		exit(-1);
	}
	if((int f=open("Metadata",O_CREAT, S_IRUSR|S_IWUSR))==-1){
			fprintf(stderr,"ОШИБКА. Нe удалось создать файл метаданных\n");
			exit(-2);
	}
	fclose(f);
	arction=argv[1];
	source=argv[2];
	out="Arch.a";
	if(argc>3)
			out=argv[3];
	if(strcmp(action,"-p")==0){
		pack(source,out)
		printf ("%s успешно запакован в %s",source,out);
	}else{
		unpack(source,out);
		printf ("%s успешно распакован в %s",source,out);
	}
	exit(0);
}
