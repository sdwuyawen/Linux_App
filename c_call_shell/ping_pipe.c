#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1024

int main()
{
	FILE *fstream = NULL;
	int error=0;
	char buff[MAX_SIZE]={0};

	if(NULL == (fstream=popen("ping 202.194.201.252 -c 1 -W 1","w")))//这个应该是写方式的管道
	{
		fprintf(stderr,"execute command failed:%s",strerror(error));
		return -1;
	}

	if(NULL != fgets(buff,sizeof(buff),fstream))
	{
		printf("%s",buff);
	}
	else
	{
		pclose(fstream);
		return -1;
	}
	pclose(fstream);
	printf("Hello world!\n");
	return 0;
}
