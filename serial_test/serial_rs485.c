#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#include <unistd.h>  
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>  
#include "string.h"

#include "serial_api.h"

#ifndef TIOCSRS485
#define TIOCSRS485	0x542F
#endif

struct serial_rs485 {
	unsigned int	flags;			/* RS485 feature flags */
#define SER_RS485_ENABLED		(1 << 0)	/* If enabled */
#define SER_RS485_RTS_ON_SEND		(1 << 1)	/* Logical level for
							   RTS pin when
							   sending */
#define SER_RS485_RTS_AFTER_SEND	(1 << 2)	/* Logical level for
							   RTS pin after sent*/
#define SER_RS485_RX_DURING_TX		(1 << 4)
	unsigned int	delay_rts_before_send;	/* Delay before send (milliseconds) */
	unsigned int	delay_rts_after_send;	/* Delay after send (milliseconds) */
	unsigned int	padding[5];		/* Memory is cheap, new structs
					   are a royal PITA .. */
//	unsigned int pin_txen;	/* txen pin for 485 mode */
};

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

	struct timeval *p_tv = NULL;

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
		sret = select(fd+1,&rfds,NULL,NULL,p_tv);		//检测串口是否可读

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

		{
			p_tv = &tv;
			tv.tv_sec  = timeout / 1000;
			tv.tv_usec = (timeout%1000)*1000;
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
	int s_rc = 0;
	int msg_length = 0;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	/* 读取第一个字节前的超时无限制，因为不知道什么时间可以接收到帧 */
	p_tv = NULL;

	while(length_to_read != 0)
	{
#if 1
		while((s_rc = select(fd + 1, &rfds, NULL, NULL, p_tv)) == -1)
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
			
			printf("select loop s_rc = %d\n", s_rc);
		}
#endif

#if 0
		do
		{
			s_rc = select(fd + 1, &rfds, NULL, NULL, p_tv);
		}
		while(s_rc == -1);
#endif

#if 0
		while((s_rc = select(fd + 1, &rfds, NULL, NULL, p_tv)) == -1)
		while(s_rc = select(fd + 1, &rfds, NULL, NULL, p_tv) == -1)
		{

		}
#endif
		printf("sec: %d,usec: %d\n",tv.tv_sec,tv.tv_usec);
		
		printf("select s_rc = %d\n", s_rc);
		
		/* select超时，帧完成 */
		if(s_rc == 0)
		{
			return msg_length;
		}

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
			tv.tv_usec = 50000;
			
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
	struct serial_rs485 rs485conf;
	int rc;
	struct timeval tvafter,tvpre;
	struct timezone tz;

	if((fd = open_port(1)) < 0) /* 打开串口 */
	{
		perror("open_port");
		return 1;
	}

	if(fcntl(fd, F_SETFL, FNDELAY) < 0)
	{
		perror("set nonblock");
	}

	if(set_com_config(fd, 9600, 8, 'N', 1) < 0) /* 配置串口 */
	{
		perror("set_com_config");
		return 1;
	}

	memset(&rs485conf, 0x0, sizeof(struct serial_rs485));

	/* 设置UART为RS485模式，自动控制收发信号转换，对应串口驱动s3c2416.c */
	rs485conf.flags = SER_RS485_ENABLED;
	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) 
	{
		printf("ioctl error\n");
		return -1;
	}

//	sprintf(buff, "AT\r\n");
//	scanf(buff, "%s");				/* 为什么没有阻塞？ */


//	if(fgets(buff, BUFFER_SIZE, stdin) == NULL)
//	if(fgets(buff, 2, stdin) == NULL)
//	{
//		perror("fgets");
//	}

//	strcat(buff, "\r\n");

	//清空串口输入输出缓冲区
//	tcflush(fd, TCIFLUSH);
//	tcflush(fd, TCOFLUSH);

//	write(fd, buff, strlen(buff));
//	printf("write: %s\n", buff);

	rc = receive_msg(fd, buff, sizeof(buff));

//	while(read(fd, buff, 1) == -1);

//	serial_read(fd, buff, sizeof(buff), 100);

	if(rc == -1)
	{
		perror("receive_msg");
		printf("receive_msg err\n");
	}
	else
	{
		buff[rc] = 0x00;
		printf("rec length = %d, msg = %s\n", rc, buff);

		gettimeofday ( &tvpre , &tz );
		write(fd, buff, strlen(buff));
		gettimeofday ( &tvafter , &tz );
		printf("write takes %d ms\n", ( tvafter.tv_sec-tvpre.tv_sec ) *1000+ ( tvafter.tv_usec-tvpre.tv_usec ) /1000 );

		gettimeofday ( &tvpre , &tz );
		write(fd, buff, strlen(buff));
		gettimeofday ( &tvafter , &tz );
		printf("write takes %d ms\n", ( tvafter.tv_sec-tvpre.tv_sec ) *1000+ ( tvafter.tv_usec-tvpre.tv_usec ) /1000 );
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

	while(1)
	{
		sleep(1);
	}

	close(fd);

	return 0;
}
