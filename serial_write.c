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

	if((fd = open_port(1)) < 0) /* 打开串口 */
	{
		perror("open_port");
		return 1;
	}

	if(set_com_config(fd, 115200, 8, 'N', 1) < 0) /* 配置串口 */
	{
		perror("set_com_config");
		return 1;
	}	

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

	close(fd);

	return 0;
}
