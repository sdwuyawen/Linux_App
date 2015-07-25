/***************************************************************************
 *  *   Copyright (C) 2007 by qzc   *
 *   *   qzc1998@126.com   *
 *    *                                                                         *
 *     *   This program is free software; you can redistribute it and/or modify  *
 *      *   it under the terms of the GNU General Public License as published by  *
 *       *   the Free Software Foundation; either version 2 of the License, or     *
 *        *   (at your option) any later version.                                   *
 *         *                                                                         *
 *          *   This program is distributed in the hope that it will be useful,       *
 *           *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *            *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *             *   GNU General Public License for more details.                          *
 *              *                                                                         *
 *               *   You should have received a copy of the GNU General Public License     *
 *                *   along with this program; if not, write to the                         *
 *                 *   Free Software Foundation, Inc.,                                       *
 *                  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                   ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>

#define ETH_DEVICE	"eth0"

/* 得到本机的mac地址和ip地址 */
int GetLocalMac ( const char *device,char *mac,char *ip )
{
    int sockfd;
    struct ifreq req;
    struct sockaddr_in * sin;

    if ( ( sockfd = socket ( PF_INET,SOCK_DGRAM,0 ) ) ==-1 )
    {
        fprintf ( stderr,"Sock Error:%s\n\a",strerror ( errno ) );
        return ( -1 );
    }

    memset ( &req,0,sizeof ( req ) );
    strcpy ( req.ifr_name,device );
    if ( ioctl ( sockfd,SIOCGIFHWADDR, ( char * ) &req ) ==-1 )
    {
        fprintf ( stderr,"ioctl SIOCGIFHWADDR:%s\n\a",strerror ( errno ) );
        close ( sockfd );
        return ( -1 );
    }
    memcpy ( mac,req.ifr_hwaddr.sa_data,6 );

    req.ifr_addr.sa_family = PF_INET;
    if ( ioctl ( sockfd,SIOCGIFADDR, ( char * ) &req ) ==-1 )
    {
        fprintf ( stderr,"ioctl SIOCGIFADDR:%s\n\a",strerror ( errno ) );
        close ( sockfd );
        return ( -1 );
    }
    sin = ( struct sockaddr_in * ) &req.ifr_addr;
    memcpy ( ip, ( char * ) &sin->sin_addr,4 );

    return ( 0 );
}

char *mac_ntoa ( const unsigned char *mac )
{
	/* Linux 下有 ether_ntoa(),不过我们重新写一个也很简单 */
	static char buffer[18];
	memset ( buffer,0,sizeof ( buffer ) );
	sprintf ( buffer,"%02X:%02X:%02X:%02X:%02X:%02X",
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
	return ( buffer );
}

/* 根据 RFC 0826 修改*/
typedef struct _Ether_pkg Ether_pkg;
struct _Ether_pkg
{
	/* 前面是ethernet头 */
	unsigned char ether_dhost[6]; /* 目地硬件地址 */
	unsigned char ether_shost[6]; /* 源硬件地址 */
	unsigned short int ether_type; /* 网络类型 */

	/* 下面是arp协议 */
	unsigned short int ar_hrd; /* 硬件地址格式 */
	unsigned short int ar_pro; /* 协议地址格式 */
	unsigned char ar_hln; /* 硬件地址长度(字节) */
	unsigned char ar_pln; /* 协议地址长度(字节) */
	unsigned short int ar_op; /* 操作代码 */
	unsigned char arp_sha[6]; /* 源硬件地址 */
	unsigned char arp_spa[4]; /* 源协议地址 */
	unsigned char arp_tha[6]; /* 目的硬件地址 */
	unsigned char arp_tpa[4]; /* 目的协议地址 */
};

void parse_ether_package ( const Ether_pkg *pkg )
{
	unsigned char *pchar;
	
	pchar = (unsigned char *)pkg->arp_tpa;
//	printf ( "rcv dst MAC=[%s] IP=[%d.%d.%d.%d]\n",mac_ntoa ( pkg->arp_tha ), pchar[0], pchar[1], pchar[2], pchar[3]);
//	printf ( "rcv dst MAC=[%s] IP=[%s]\n",mac_ntoa ( pkg->arp_tha ),inet_ntoa ( * ( struct in_addr * ) pkg->arp_tpa ) );

	pchar = (unsigned char *)pkg->arp_spa;
	printf ( "IP=[%d.%d.%d.%d]\n", pchar[0], pchar[1], pchar[2], pchar[3]);
//	printf ( "rcv src MAC=[%s] IP=[%d.%d.%d.%d]\n",mac_ntoa ( pkg->arp_sha ), pchar[0], pchar[1], pchar[2], pchar[3]);
//	printf ( "rcv src MAC=[%s] IP=[%s]\n",mac_ntoa ( pkg->arp_sha ),inet_ntoa ( * ( struct in_addr * ) pkg->arp_spa ) );
//	printf("\n");
}

/* 源mac 
 * 目的mac
 * 源ip
 * 目的ip,字符串形式
 */
int sendpkg ( char * mac,char * broad_mac,char * src_ip,char * dest_ip )
{
	Ether_pkg pkg;
	struct hostent *host =NULL;
	
	int sockfd,len;
	char buffer[255];
	char dest_ip_n[4];
	unsigned char *pchar;
	memset ( ( char * ) &pkg,'\0',sizeof ( pkg ) );

//	struct sockaddr sa;
	
	struct sockaddr_ll toaddr;
	struct sockaddr_ll fromaddr;
	struct ifreq ifr;

	memset(&toaddr, 0x00, sizeof(toaddr));
	memset(&fromaddr, 0x00, sizeof(fromaddr));
	memset(&ifr, 0x00, sizeof(ifr));
	
	/* 填充ethernet包文 */
	memcpy ( ( char * ) pkg.ether_shost, ( char * ) mac,6 );
	memcpy ( ( char * ) pkg.ether_dhost, ( char * ) broad_mac,6 );
	pkg.ether_type = htons ( ETHERTYPE_ARP );


	/* 下面填充arp包文 */
	pkg.ar_hrd = htons ( ARPHRD_ETHER );
	pkg.ar_pro = htons ( ETHERTYPE_IP );
	pkg.ar_hln = 6;
	pkg.ar_pln = 4;
	pkg.ar_op = htons ( ARPOP_REQUEST );
	memcpy ( ( char * ) pkg.arp_sha, ( char * ) mac,6 );
	memcpy ( ( char * ) pkg.arp_spa, ( char * ) src_ip,4 );
	memcpy ( ( char * ) pkg.arp_tha, ( char * ) broad_mac,6 );

	fflush ( stdout );
	memset ( dest_ip_n,0,sizeof ( dest_ip_n ) );
#if 1
	/* 把目的ip地址字符串转换为ip地址
	 * 返回0表示转换失败
	 */
	if ( inet_aton ( dest_ip, ( struct in_addr * ) dest_ip_n ) ==0 )
	{
    		/* 把目的主机名转换成ip地址 */
		if ( ( host = gethostbyname ( dest_ip ) ) ==NULL )
		{
			fprintf ( stderr,"Fail! %s\n\a",hstrerror ( h_errno ) );
			return ( -1 );
		}
		/* 获取目的ip到dest_ip_n */
		memcpy ( ( char * ) dest_ip_n,host->h_addr,4 );
	}
#endif

	memcpy ( ( char * ) pkg.arp_tpa, ( char * ) dest_ip_n,4 );
	
	/* 把本次的目的IP，作为下一次的源IP，会造成IP冲突 */
//	memcpy ( ( char * ) src_ip,pkg.arp_tpa,4 );
//	printf ( "src_ip =[%s]\n", inet_ntoa ( * ( struct in_addr * ) src_ip ));
    
	pchar = (unsigned char *)pkg.arp_tpa;
//	printf ( "send dst MAC=[%s] IP=[%d.%d.%d.%d]\n",mac_ntoa ( pkg.arp_tha ), pchar[0], pchar[1], pchar[2], pchar[3]);

	pchar = (unsigned char *)pkg.arp_spa;
//	printf ( "send src MAC=[%s] IP=[%d.%d.%d.%d]\n",mac_ntoa ( pkg.arp_sha ), pchar[0], pchar[1], pchar[2], pchar[3]);
//	printf ( "send src MAC=[%s] IP=[%s]\n",mac_ntoa ( pkg.arp_sha ), inet_ntoa ( * ( struct in_addr * ) pkg.arp_spa ));
	
	if(0>(sockfd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP))))
	{
		fprintf ( stderr,"Socket Error:%s\n\a",strerror ( errno ) );
		return ( 0 );
	}

	strcpy(ifr.ifr_name,ETH_DEVICE);
	//获取接口索引
	if(-1 == ioctl(sockfd,SIOCGIFINDEX,&ifr)){
		printf("get dev index error:");
		return -1;
	}
	toaddr.sll_ifindex = ifr.ifr_ifindex;
	fromaddr.sll_ifindex = ifr.ifr_ifindex;
//	printf("interface Index:%d\n",ifr.ifr_ifindex);

	fromaddr.sll_family = PF_PACKET;
	fromaddr.sll_protocol=htons(ETH_P_ARP);
	fromaddr.sll_hatype=ARPHRD_ETHER;
	fromaddr.sll_pkttype=PACKET_HOST;
	fromaddr.sll_halen=ETH_ALEN;
	memcpy(fromaddr.sll_addr,mac,ETH_ALEN);

	bind(sockfd,(struct sockaddr*)&fromaddr,sizeof(struct sockaddr));

	toaddr.sll_family = PF_PACKET;
	len = sendto(sockfd, &pkg, sizeof (pkg), 0,(struct sockaddr*)&toaddr,sizeof(toaddr));
	if ( len != sizeof ( pkg ) )
	{
		fprintf ( stderr,"Sendto Error:%s\n\a",strerror ( errno ) );
		return ( 0 );
	}
	
	Ether_pkg *parse;
	parse = ( Ether_pkg * ) buffer;
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; /*100毫秒*/
	FD_ZERO ( &readfds );
	FD_SET ( sockfd, &readfds );
	
	while(1)
	{
		len = select ( sockfd+1, &readfds, 0, 0, &tv );
		if ( len == 0 )
		{
//			printf("time out\n");
//			printf("\n");
			break;
		}
		else if(len < 0)
		{
			printf("select error\n");
			break;
		}
		else
		{
			if ( FD_ISSET ( sockfd,&readfds ) )
			{
//				printf("get response\n");
				memset ( buffer,0,sizeof ( buffer ) );
				len=recvfrom ( sockfd,buffer,sizeof ( buffer ),0,NULL,&len );
				if ( ( ntohs ( parse->ether_type ) ==ETHERTYPE_ARP ) &&
				( ntohs ( parse->ar_op ) == ARPOP_REPLY ) )
				{
//					printf("get ARP replay\n");
					parse_ether_package ( parse );

					break;
				}
				else
				{
//					printf("parse->ether_type = 0x%04x, parse->ar_op = 0x%04x, parse->arp_spa[0-3] = %s\n", ntohs(parse->ether_type), ntohs(parse->ar_op), inet_ntoa (*(struct in_addr *)parse->arp_spa));
				}
			}

			{
				tv.tv_sec = 0;
				tv.tv_usec = 100000; /*100毫秒*/
			}
		}
	}

	return 1;
}


int main ( int argc,char **argv )
{
	struct timeval tvafter,tvpre;
	struct timezone tz;
	gettimeofday ( &tvpre , &tz );

	unsigned char mac[7];
	unsigned char ip[5];
	char dest[16]={0};
	unsigned char broad_mac[7]={0xff,0xff,0xff,0xff,0xff,0xff,0x00};
	//以太网帧首部的硬件地址填FF:FF:FF:FF:FF:FF表示广播
	memset ( mac,0,sizeof ( mac ) );
	memset ( ip,0,sizeof ( ip ) );
	if ( GetLocalMac ( ETH_DEVICE, mac,ip ) ==-1 )
		return ( -1 );
	//源IP地址设置为网关
//	inet_aton ( "202.194.201.254", ( struct in_addr * ) ip );

	printf ( "local Mac=[%s] Ip=[%s]\n", mac_ntoa ( mac ),inet_ntoa ( * ( struct in_addr * ) ip ) );
	printf("\n");

	int i=0;
//	while(1)
	{
		for(i=200;i<=200;i++)
		{
//			getchar();
			sprintf ( dest,"202.194.201.%d",i );
			/* 源mac 
			 * 目的mac
			 * 源ip
			 * 目的ip
			 */
			sendpkg ( mac,broad_mac,ip,dest );
//			sleep(1);
		}
	}


	gettimeofday ( &tvafter , &tz );
	printf ( "\n%dms\n", ( tvafter.tv_sec-tvpre.tv_sec ) *1000+ ( tvafter.tv_usec-tvpre.tv_usec ) /1000 );

	return 0;
} 

