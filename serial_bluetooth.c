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

/*-----------------------------------------------------------------------------
  函数名:      serial_read
  参数:        int fd,char *str,unsigned int len,unsigned int timeout
  返回值:      在规定的时间内读取数据，超时则退出，超时时间为ms级别
  描述:        向fd描述符的串口接收数据，长度为len，存入str，timeout 为超时时间
 *-----------------------------------------------------------------------------*/
int serial_read(int fd, char *str, unsigned int len, unsigned int timeout)
{
	fd_set rfds;
	struct timeval tv;
	int ret;								//每次读的结果
	int sret;								//select监控结果
	int readlen = 0;						//实际读到的字节数
	char * ptr;

	ptr = str;							//读指针，每次移动，因为实际读出的长度和传入参数可能存在差异

	FD_ZERO(&rfds);						//清除文件描述符集合
	FD_SET(fd,&rfds);					//将fd加入fds文件描述符，以待下面用select方法监听

	/*传入的timeout是ms级别的单位，这里需要转换为struct timeval 结构的*/
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;

	/*防止读数据长度超过缓冲区*/
	//if(sizeof(&str) < len)
	//	len = sizeof(str);


	/*开始读*/
	while(readlen < len)
	{
		sret = select(fd+1,&rfds,NULL,NULL,&tv);		//检测串口是否可读

		if(sret == -1)										//检测失败
		{
			perror("select:");
			break;
		}
		else if(sret > 0)									//检测成功可读
		{
			ret = read(fd,ptr,1);
			printf("sec: %d,usec: %d\n",tv.tv_sec,tv.tv_usec);
			if(ret < 0)
			{
				perror("read err:");
				break;
			}
			else if(ret == 0)
				break;

			readlen += ret;									//更新读的长度
			ptr     += ret;									//更新读的位置
		}
		else													//超时
		{
			printf("timeout!\n");
			break;
		}
	}

	return readlen;
}

int receive_msg(int fd, char *msg, int length_to_read)
{
	struct timeval tv = {1, 0};
	struct timeval *p_tv = NULL;
	fd_set rfds;

	int rc = 0;
	int msg_length = 0;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	/* 读取第一个字节前的超时无限制，因为不知道什么时间可以接收到帧 */
	p_tv = NULL;

	while(length_to_read != 0)
	{
		while(rc = select(fd + 1, &rfds, NULL, NULL, p_tv) == -1)
		{
			if (errno == EINTR) 
			{
				fprintf(stderr, "A non blocked signal was caught\n");
	
				/* Necessary after an error */
				FD_ZERO(&rfds);
				FD_SET(fd, &rfds);
			}
			else 
			{
				return -1;
			}
			
			printf("select loop rc = %d\n", rc);
		}

		printf("select rc = %d\n", rc);
		
		/* select超时，说明字节超时，帧完成
		 * 当p_tv是NULL时，rfds有变化时，返回rc=0，所以这里不能判断rc==0，
		 * 而要通过read的返回值来判断 */
		/*if(rc == 0)
		{
			return msg_length;
		}
		*/

		rc = read(fd, msg + msg_length, length_to_read);

		printf("uart read %d bytes\n", rc);
		
		/* 从串口读到0字节，说明已经到字节超时仍没有收到下一个字节 */
		if (rc == 0) 				
		{
            errno = ECONNRESET;
			rc = -1;	
        }

		if(rc == -1)
		{
			return msg_length;
		}

        /* Sums bytes received */
        msg_length += rc;
        /* Computes remaining bytes */
        length_to_read -= rc;

		/* 非第一个字节，则字节间距调整 */
		if(length_to_read > 0)
		{
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			
			p_tv = &tv;	
		}	


	}

	return msg_length;
#if 0	
	select(fd + 1, &rfds, NULL, NULL, &tv);

	if(FD_ISSET(fd, &rfds))
	{
		rc = read(fd, buff, sizeof(buff));
		printf("rc = %d, buff = %s\n", rc, buff);
	}
#endif
}

int main(void) 
{
	int fd;
	char buff[BUFFER_SIZE];
	int rc;

	if((fd = open_port(1)) < 0) /* 打开串口 */
	{
		perror("open_port");
		return 1;
	}

	if(fcntl(fd, F_SETFL, FNDELAY) < 0)
	{
		perror("set nonblock");
	}

	if(set_com_config(fd, 115200, 8, 'N', 1) < 0) /* 配置串口 */
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

//	rc = receive_msg(fd, buff, sizeof(buff));
	serial_read(fd, buff, sizeof(buff), 100);

	if(rc == -1)
	{
		perror("receive_msg");
		printf("receive_msg err\n");
	}
	else
	{
		buff[rc] = 0x00;
		printf("rec length = %d, msg = %s\n", rc, buff);
	}


//	sleep(1);
	


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
