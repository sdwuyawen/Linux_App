#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


/* system函数调用/bin/sh  执行特定的shell命令，阻塞当前的进程知道shell命令执行完毕。
 * #include
 * int system(const char *command);
 * 执行system实际上是调用了fork函数(产生新进程)、exec函数(在新进程中执行新任务)、waitpid函数(等待新进程结束)。
 */

int main(int argc,char **argv)
{
    int ret;
	int i;
	char buf[100];

	if(argc != 2)
	{
		printf("Usage: %s <IP>\n", argv[0]);
		exit(-1);
	}
	else
	{
		for(i = 0; i < argc; i++)
		{
			printf("arg[%d] = %s\n", i, argv[i]);
		}
	}
    
    printf("当前进程的进程号为%d\n",getpid());
	sprintf(buf, "ping %s -c 1 -W 1 > tmpfile.txt", argv[1]);
    ret = system(buf);  //调用shell命令 ls -l
    printf("ret = %d\n",ret);

	/* host is online */
	if(ret == 0)
	{
		printf("host %s is online.\n", argv[1]);
	}
	else
	{
		printf("host %s can't be found.\n", argv[1]);
	}
    return 0;
}
