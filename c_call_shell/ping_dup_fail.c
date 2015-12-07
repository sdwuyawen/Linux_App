#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


/* system函数调用/bin/sh  执行特定的shell命令，阻塞当前的进程知道shell命令执行完毕。
 * #include
 * int system(const char *command);
 * 执行system实际上是调用了fork函数(产生新进程)、exec函数(在新进程中执行新任务)、waitpid函数(等待新进程结束)。
 */

int mysystem(char *cmdstring, char *buf, int len)
{
	int fd[2];
	pid_t pid;
	int n, count; 
	memset(buf, 0, len);
	if (pipe(fd) < 0)
	{
		printf("pipe error.\n");
		return -1;
	}
	if ((pid = fork()) < 0)
	{
		printf("fork error.\n");
		return -2;
	}
	else if (pid > 0)		/* 父进程 */ 
	{
		printf("father process\n");
		close(fd[1]);     
		count = 0;
		while ((n = read(fd[0], buf + count, len)) > 0 && n > 0 && count < len)
			count += n;
		close(fd[0]);
		if (waitpid(pid, NULL, 0) < 0)
			return -3;
	}
	else                  /* 子进程 */ 
	{
		close(fd[0]);     
		/* 把管道f[1]写入STDOUT，即标准输出重定向至管道 */
		if (fd[1] != STDOUT_FILENO)
		{
			/* 把fd[1]复制至STDOUT_FILENO */
			if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				return -1;
			}
			close(fd[1]);
		} 
		/* 标准输出已经重定向至fd[1] */
		printf("child process\n");
		printf("123\n");
//		write(fd[1], "123", 4);
//		if (execl("/bin/sh", "sh", "-c", cmdstring, (char*)0) == -1)
//			return -4;
	} 
	return 0;
}

int main(int argc,char **argv)
{
    int ret;
	int i;
	char cmdbuf[100];
	char outbuf[1024];

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
	sprintf(cmdbuf, "ping %s -c 1 -w 1", argv[1]);
//    ret = system(cmdbuf);  //调用shell命令 ls -l
	ret = mysystem(cmdbuf, outbuf, sizeof(outbuf));
	printf("output:%s\n", outbuf);
	
    printf("ret = %d\n",ret);
//
//	/* host is online */
//	if(ret == 0)
//	{
//		printf("host %s is online.\n", argv[1]);
//	}
//	else
//	{
//		printf("host %s can't be found.\n", argv[1]);
//	}
    return 0;
}

