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

int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg,old_cfg;
	int speed;

	/* 保存并测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息 */
	if (tcgetattr(fd, &old_cfg) != 0) 
	{
		perror("tcgetattr");
		return -1;
	}
	new_cfg = old_cfg;
	cfmakeraw(&new_cfg); /* 配置为原始模式 */
	new_cfg.c_cflag &= ~CSIZE;
	/* 设置波特率 */
	switch (baud_rate)
	{
		case 2400:
			{
				speed = B2400;
			}
			break;
		case 4800:
			{
				speed = B4800;
			}
			break;
		case 9600:
			{
				speed = B9600;
			}
			break;
		case 19200:
			{
				speed = B19200;
			}
			break;
		case 38400:
			{
				speed = B38400;
			}
			break;

		default:
		case 115200:
			{
				speed = B115200;
			}
			break;
	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);

	switch (data_bits) /* 设置数据位 */
	{
		case 7:
			{
				new_cfg.c_cflag |= CS7;
			}
			break;

		default:
		case 8:
			{
				new_cfg.c_cflag |= CS8
					;             }
			break;
	}

	switch (parity) /* 设置奇偶校验位 */
	{
		default:
		case 'n':
		case 'N':
			{
				new_cfg.c_cflag &= ~PARENB; 
				new_cfg.c_iflag &= ~INPCK; 
			}
			break;

		case 'o':
		case 'O':
			{
				new_cfg.c_cflag |= (PARODD | PARENB); 
				new_cfg.c_iflag |= INPCK; 
			}
			break;

		case 'e':
		case 'E':
			{
				new_cfg.c_cflag |= PARENB; 
				new_cfg.c_cflag &= ~PARODD; 
				new_cfg.c_iflag |= INPCK; 
			}
			break;

		case 's': /* as no parity */
		case 'S':
			{
				new_cfg.c_cflag &= ~PARENB;
				new_cfg.c_cflag &= ~CSTOPB;
			}
			break;
	}

	switch (stop_bits) /* 设置停止位 */
	{
		default:
		case 1:
			{
				new_cfg.c_cflag &= ~CSTOPB;
			}
			break;

		case 2:
			{
				new_cfg.c_cflag |= CSTOPB;
			}
	}

	/* 设置等待时间和最小接收字符 */
	new_cfg.c_cc[VTIME] = 0;
	new_cfg.c_cc[VMIN] = 1;
	tcflush(fd, TCIFLUSH); /* 处理未接收字符 */
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0) /* 激活新配置 */
	{
		perror("tcsetattr");
		return -1;
	}	
	return 0;
}

/*打开串口函数*/
int open_port(int com_port)
{
	int fd;
	char *dev[] = {"/dev/s3c2410_serial0", "/dev/s3c2410_serial1", "/dev/s3c2410_serial2"};

	if ((com_port < 0) || (com_port > MAX_COM_NUM))
	{
		return -1;
	}
	/* 打开串口 */
	fd = open(dev[com_port], O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0)
	{
		perror("open serial port");
		return(-1);
	}

	if (fcntl(fd, F_SETFL, 0) < 0) /* 恢复串口为阻塞状态 */
	{
		perror("fcntl F_SETFL\n");
	}

	if (isatty(fd) == 0) /* 测试打开的文件是否为终端设备 */
	{
		perror("This is not a terminal device");
	}	
	return fd;
}


