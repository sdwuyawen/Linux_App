
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define ONLINE_TIME_THRESHOLD	3	

typedef enum
{
	OFFLINE = 0,
	ONLINE,
}ConnectStatus;

/* system函数调用/bin/sh  执行特定的shell命令，阻塞当前的进程知道shell命令执行完毕。
 * #include
 * int system(const char *command);
 * 执行system实际上是调用了fork函数(产生新进程)、exec函数(在新进程中执行新任务)、waitpid函数(等待新进程结束)。
 * 如果system()在调用/bin/sh时失败则返回127，其他失败原因返回-1。
 * 若参数string为空指针(NULL)，则返回非零值。如果system()调
 * 用成功则最后会返回执行shell命令后的返回值，但是此返回值也有
 * 可能为system()调用/bin/sh失败所返回的127，因此最好能再检查
 * errno 来确认执行成功。
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
//		printf("father process\n");
		/* 关闭写描述符 */
		close(fd[1]);     
		close(fd_ret[1]);     
		count = 0;
		while ((n = read(fd[0], buf + count, len)) > 0 && n > 0 && count < len)
			count += n;
		close(fd[0]);
		n = read(fd_ret[0], &buf_ret, 10);
//		printf("fd_set[0] get %d bytes. buf_set = %d\n", n, buf_ret);
		close(fd_ret[0]);
		if (waitpid(pid, NULL, 0) < 0)
			return -3;
//		printf("father process end\n");

		if(buf_ret == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else                  /* 子进程 */ 
	{
//		printf("child process\n");
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
//		printf("child process stdout\n");
		/* 分别使用system()和execl()调用shell */
		ret = system(cmdstring);
		write(fd_ret[1], &ret, 4);
		close(fd_ret[1]);
		/* The exec() family of functions replaces the current process 
		 * image with a new process image 
		 */
//		if (execl("/bin/sh", "sh", "-c", cmdstring, (char*)0) == -1)
//			return -4;
//		printf("child process end\n");
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
	char timebuf[128];
	time_t timep;
	struct tm *tm;
	char *path="./pingout.log";
	FILE *fp;
	unsigned int count = 0;
	int online_counts = 0;
	int offline_counts = 0;
	ConnectStatus last_status = OFFLINE;

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

	while(1)
	{
		//    ret = system(cmdbuf);  //调用shell命令 ls -l
		ret = mysystem(cmdbuf, outbuf, sizeof(outbuf));
		//	printf("output:%s\n", outbuf);

//		printf("ret = %d\n",ret);

//		printf("[%d] ", ++count);
		/* host is online */
		if(ret == 0)
		{
			if(online_counts < 10)
				online_counts++;
			offline_counts = 0;
//			printf("host %s is online.\n", argv[1]);
			sleep(1);
		}
		else
		{
			if(offline_counts < 10)
				offline_counts++;
			online_counts = 0;
//			printf("host %s can't be found.\n", argv[1]);
		}

		/* 把父进程得到的outbuf写入文件，证明父进程读取到管道的数据 */
		fp = fopen(path, "a");		/* 追加写入 */
		fseek(fp, 0, SEEK_END);
		
		/* 获取当前时间 */
		time(&timep);
		tm = localtime(&timep); /* 取得当地时间*/ 
		sprintf(timebuf, "%d%d%d %d:%d:%d", (1900 + tm->tm_year),(1 + tm->tm_mon), tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec);
		/* 上线记录 */
		if(online_counts == ONLINE_TIME_THRESHOLD)
		{
			if(last_status == OFFLINE)
			{
				last_status = ONLINE;
				sprintf(outbuf, "%s host %s online just now\n", timebuf, argv[1]);
				fprintf(stdout, outbuf);
				fprintf(fp, outbuf);
			}
		}
		/* 下线记录 */
		else if(offline_counts == ONLINE_TIME_THRESHOLD)
		{
			if(last_status == ONLINE)
			{
				last_status = OFFLINE;
				sprintf(outbuf, "%s host %s offline just now\n", timebuf, argv[1]);
				fprintf(stdout, outbuf);
				fprintf(fp, outbuf);
			}
		}
		fclose(fp);
	}
    return 0;
}

