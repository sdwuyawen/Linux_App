/*
 * =====================================================================================
 *
 *       Filename:  serial.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/19/2015 10:23:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include     <stdio.h>
#include     <stdlib.h> 
#include     <unistd.h>  
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h> 
#include     <termios.h>
#include     <errno.h>

main()
{
	int fd;
	int i;
	int len;
	int n = 0;      
	char read_buf[256];
	char write_buf[256];
	struct termios opt; 

	fd = open("/dev/s3c2410_serial1", O_RDWR | O_NOCTTY);    //默认为阻塞读方式
	if(fd == -1)
	{
		perror("error: open serial 0\n");
		exit(0);
	}

	tcgetattr(fd, &opt);      
	cfsetispeed(&opt, B9600);
	cfsetospeed(&opt, B9600);

	if(tcsetattr(fd, TCSANOW, &opt) != 0 )
	{     
		perror("tcsetattr error");
		return -1;
	}

	opt.c_cflag &= ~CSIZE;  
	opt.c_cflag |= CS8;   
	opt.c_cflag &= ~CSTOPB; 
	opt.c_cflag &= ~PARENB; 
	opt.c_cflag &= ~INPCK;
	opt.c_cflag |= (CLOCAL | CREAD);

	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	opt.c_oflag &= ~OPOST;
	opt.c_oflag &= ~(ONLCR | OCRNL);    //添加的

	opt.c_iflag &= ~(ICRNL | INLCR);
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //添加的

	opt.c_cc[VTIME] = 0;
	opt.c_cc[VMIN] = 0;

	tcflush(fd, TCIOFLUSH);

	printf("configure complete\n");

	if(tcsetattr(fd, TCSANOW, &opt) != 0)
	{
		perror("serial error");
		return -1;
	}
	printf("start send and receive data\n");

	while(1)
	{    
		n = 0;
		len = 0;
		bzero(read_buf, sizeof(read_buf));    //类似于memset
		bzero(write_buf, sizeof(write_buf));

		while( (n = read(fd, read_buf, sizeof(read_buf))) > 0 )
		{
			for(i = len; i < (len + n); i++)
			{
				write_buf[i] = read_buf[i - len];
			}
			len += n;
		}
		write_buf[len] = '\0';

		printf("Len %d \n", len);
		printf("%s \n", write_buf);

		sprintf(write_buf, "write test\n");
		n = write(fd, write_buf, strlen(write_buf));
		printf("write %d chars\n",n);

		sleep(2);
	}
}
