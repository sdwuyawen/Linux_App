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
	int fd_ret[2];
	pid_t pid;
	int n, count; 
	int ret;
	int buf_ret;

	memset(buf, 0, len);
	/* fd[0]是读端，fd[1]是写端 */
	if (pipe(fd) < 0 || pipe(fd_ret) < 0)
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
		/* 关闭写描述符 */
		close(fd[1]);     
		close(fd_ret[1]);     
		count = 0;
		while ((n = read(fd[0], buf + count, len)) > 0 && n > 0 && count < len)
			count += n;
		close(fd[0]);
		n = read(fd_ret[0], &buf_ret, 10);
		printf("fd_set[0] get %d bytes. buf_set = %d\n", n, buf_ret);
		close(fd_ret[0]);
		if (waitpid(pid, NULL, 0) < 0)
			return -3;
		printf("father process end\n");
	}
	else                  /* 子进程 */ 
	{
		printf("child process\n");
		close(fd[0]);     
//		write(fd[1], "123", 4);
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
		printf("child process stdout\n");
		/* 分别使用system()和execl()调用shell */
//		ret = system("ls -l");
		ret = system(cmdstring);
		write(fd_ret[1], &ret, 4);
		close(fd_ret[1]);
		/* The exec() family of functions replaces the current process 
		 * image with a new process image 
		 */
//		if (execl("/bin/sh", "sh", "-c", cmdstring, (char*)0) == -1)
//			return -4;
		printf("child process end\n");
		exit(0);
	} 
	return 0;
}

int main(int argc,char **argv)
{
    int ret;
	int i;
	char cmdbuf[100];
	char outbuf[1024];
	char *path="./testout.txt";
	FILE *fp;

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
    
	sprintf(cmdbuf, "ping %s -c 1 -W 1", argv[1]);
//    ret = system(cmdbuf);  //调用shell命令 ls -l
	ret = mysystem(cmdbuf, outbuf, sizeof(outbuf));
	printf("output:%s\n", outbuf);
	
    printf("ret = %d\n",ret);

	/* 把父进程得到的outbuf写入文件，证明父进程读取到管道的数据 */
	fp = fopen(path, "w");
	fputs(outbuf, fp);
	fclose(fp);

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

