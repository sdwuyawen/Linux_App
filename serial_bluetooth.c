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
		sret = select(fd+1,&rfds,NULL,NULL,&tv);		//��⴮���Ƿ�ɶ�

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

	/* ��ȡ��һ���ֽ�ǰ�ĳ�ʱ�����ƣ���Ϊ��֪��ʲôʱ����Խ��յ�֡ */
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
		
		/* select��ʱ��˵���ֽڳ�ʱ��֡���
		 * ��p_tv��NULLʱ��rfds�б仯ʱ������rc=0���������ﲻ���ж�rc==0��
		 * ��Ҫͨ��read�ķ���ֵ���ж� */
		/*if(rc == 0)
		{
			return msg_length;
		}
		*/

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

	if((fd = open_port(1)) < 0) /* �򿪴��� */
	{
		perror("open_port");
		return 1;
	}

	if(fcntl(fd, F_SETFL, FNDELAY) < 0)
	{
		perror("set nonblock");
	}

	if(set_com_config(fd, 115200, 8, 'N', 1) < 0) /* ���ô��� */
	{
		perror("set_com_config");
		return 1;
	}	

//	sprintf(buff, "AT\r\n");
//	scanf(buff, "%s");				/* Ϊʲôû�������� */


	if(fgets(buff, BUFFER_SIZE, stdin) == NULL)
	{
		perror("fgets");
	}

	strcat(buff, "\r\n");

	//��մ����������������
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
