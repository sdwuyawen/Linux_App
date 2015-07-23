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
  ������:      serial_read
  ����:        int fd,char *str,unsigned int len,unsigned int timeout
  ����ֵ:      �ڹ涨��ʱ���ڶ�ȡ���ݣ���ʱ���˳�����ʱʱ��Ϊms����
  ����:        ��fd�������Ĵ��ڽ������ݣ�����Ϊlen������str��timeout Ϊ��ʱʱ��
 *-----------------------------------------------------------------------------*/
int serial_read(int fd, char *str, unsigned int len, unsigned int timeout)
{
	fd_set rfds;
	struct timeval tv;
	int ret;								//ÿ�ζ��Ľ��
	int sret;								//select��ؽ��
	int readlen = 0;						//ʵ�ʶ������ֽ���
	char * ptr;

	struct timeval *p_tv = NULL;

	ptr = str;							//��ָ�룬ÿ���ƶ�����Ϊʵ�ʶ����ĳ��Ⱥʹ���������ܴ��ڲ���

	FD_ZERO(&rfds);						//����ļ�����������
	FD_SET(fd,&rfds);					//��fd����fds�ļ����������Դ�������select��������

	/*�����timeout��ms����ĵ�λ��������Ҫת��Ϊstruct timeval �ṹ��*/
	tv.tv_sec  = timeout / 1000;
	tv.tv_usec = (timeout%1000)*1000;

	/*��ֹ�����ݳ��ȳ���������*/
	//if(sizeof(&str) < len)
	//	len = sizeof(str);


	/*��ʼ��*/
	while(readlen < len)
	{
		sret = select(fd+1,&rfds,NULL,NULL,p_tv);		//��⴮���Ƿ�ɶ�

		if(sret == -1)										//���ʧ��
		{
			perror("select:");
			break;
		}
		else if(sret > 0)									//���ɹ��ɶ�
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

			readlen += ret;									//���¶��ĳ���
			ptr     += ret;									//���¶���λ��
		}
		else													//��ʱ
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

	/* ��ȡ��һ���ֽ�ǰ�ĳ�ʱ�����ƣ���Ϊ��֪��ʲôʱ����Խ��յ�֡ */
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
		
		/* select��ʱ��֡��� */
		if(s_rc == 0)
		{
			return msg_length;
		}

		rc = read(fd, msg + msg_length, length_to_read);

		printf("uart read %d bytes\n", rc);
		
		/* �Ӵ��ڶ���0�ֽڣ�˵���Ѿ����ֽڳ�ʱ��û���յ���һ���ֽ� */
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

		/* �ǵ�һ���ֽڣ����ֽڼ����� */
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

	if((fd = open_port(1)) < 0) /* �򿪴��� */
	{
		perror("open_port");
		return 1;
	}

	if(fcntl(fd, F_SETFL, FNDELAY) < 0)
	{
		perror("set nonblock");
	}

	if(set_com_config(fd, 9600, 8, 'N', 1) < 0) /* ���ô��� */
	{
		perror("set_com_config");
		return 1;
	}

	memset(&rs485conf, 0x0, sizeof(struct serial_rs485));

	/* ����UARTΪRS485ģʽ���Զ������շ��ź�ת������Ӧ��������s3c2416.c */
	rs485conf.flags = SER_RS485_ENABLED;
	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) 
	{
		printf("ioctl error\n");
		return -1;
	}

//	sprintf(buff, "AT\r\n");
//	scanf(buff, "%s");				/* Ϊʲôû�������� */


//	if(fgets(buff, BUFFER_SIZE, stdin) == NULL)
//	if(fgets(buff, 2, stdin) == NULL)
//	{
//		perror("fgets");
//	}

//	strcat(buff, "\r\n");

	//��մ����������������
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