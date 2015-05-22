#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>  
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>  

#include "serial_api.h"

int main(void) 
{
	int fd;
	char buff[BUFFER_SIZE];
	int rc = 0;

	if((fd = open_port(1)) < 0) /* 打开串口 */
	{
		perror("open_port");
		return 1;
	}

	if(fcntl(fd, F_SETFL, FNDELAY) < 0)
	{
		perror("set nonblock");
	}

	if(set_com_config(fd, 38400, 8, 'N', 1) < 0) /* 配置串口 */
	{
		perror("set_com_config");
		return 1;
	}	

//	sprintf(buff, "AT\r\n");
//	scanf(buff, "%s");				/* 为什么没有阻塞？ */

	if(fgets(buff, BUFFER_SIZE, stdin) == NULL)
	{
		perror("fgets");
	}

	strcat(buff, "\r\n");

	//清空串口输入输出缓冲区
	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCOFLUSH);

	write(fd, buff, strlen(buff));
	printf("write: %s\n", buff);

	sleep(1);

	rc = read(fd, buff, 10);

	printf("rc = %d, buff = %s\n", rc, buff);

#if 0
	do
	{
		printf("Input some words(enter 'quit' to exit):");
		memset(buff, 0, BUFFER_SIZE);
		if (fgets(buff, BUFFER_SIZE, stdin) == NULL)
		{
			perror("fgets");
			break;
		}
		write(fd, buff, strlen(buff));
	} 
	while(strncmp(buff, "quit", 4));
#endif

	close(fd);

	return 0;
}
