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
    unsigned char arp_tha[6]; /* 目地硬件地址 */
    unsigned char arp_tpa[4]; /* 目地协议地址 */
};

void parse_ether_package ( const Ether_pkg *pkg )
{
    printf ( "源   IP=[%s] MAC=[%s]\n",inet_ntoa ( * ( struct in_addr * ) pkg->arp_spa ),mac_ntoa ( pkg->arp_sha ) );
    printf ( "目地 IP=[%s] MAC=[%s]\n",inet_ntoa ( * ( struct in_addr * ) pkg->arp_tpa ),mac_ntoa ( pkg->arp_tha ) );
}

int sendpkg ( char * mac,char * broad_mac,char * ip,char * dest )
{
    Ether_pkg pkg;
    struct hostent *host =NULL;
    struct sockaddr sa;
    int sockfd,len;
    char buffer[255];
    memset ( ( char * ) &pkg,'\0',sizeof ( pkg ) );

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
    memcpy ( ( char * ) pkg.arp_spa, ( char * ) ip,4 );
    memcpy ( ( char * ) pkg.arp_tha, ( char * ) broad_mac,6 );

    /*printf ( "Resolve [%s],Please Waiting...",dest );
     以欺骗方式发包，会造成IP冲突错误 */
    fflush ( stdout );
    memset ( ip,0,sizeof ( ip ) );
    if ( inet_aton ( dest, ( struct in_addr * ) ip ) ==0 )
    {
        if ( ( host = gethostbyname ( dest ) ) ==NULL )
        {
            fprintf ( stderr,"Fail! %s\n\a",hstrerror ( h_errno ) );
            return ( -1 );
        }
        memcpy ( ( char * ) ip,host->h_addr,4 );
    }
    memcpy ( ( char * ) pkg.arp_tpa, ( char * ) ip,4 );
	/*unsigned char tip[5];
    memset(tip,0,sizeof(tip));
	inet_aton(dest,(struct in_addr *)tip);
	memcpy((char *)pkg.arp_tpa,(char *)tip,4);*/
    /* 实际应该使用PF_PACKET */
    if ( ( sockfd = socket ( PF_INET,SOCK_PACKET,htons ( ETH_P_ALL ) ) ) ==-1 )
    {
        fprintf ( stderr,"Socket Error:%s\n\a",strerror ( errno ) );
        return ( 0 );
    }

    memset ( &sa,'\0',sizeof ( sa ) );
    strcpy ( sa.sa_data,"eth0" );

    len = sendto ( sockfd,&pkg,sizeof ( pkg ),0,&sa,sizeof ( sa ) );
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
    tv.tv_usec = 500000; /*500毫秒*/
    FD_ZERO ( &readfds );
    FD_SET ( sockfd, &readfds );
    len = select ( sockfd+1, &readfds, 0, 0, &tv );
    if ( len>-1 )
    {
        if ( FD_ISSET ( sockfd,&readfds ) )
        {
            memset ( buffer,0,sizeof ( buffer ) );
            len=recvfrom ( sockfd,buffer,sizeof ( buffer ),0,NULL,&len );
            if ( ( ntohs ( parse->ether_type ) ==ETHERTYPE_ARP ) &&
                    ( ntohs ( parse->ar_op ) == ARPOP_REPLY ) )
            {
                parse_ether_package ( parse );                
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
    //char dip[16]={0};
    char dest[16]={0};
    unsigned char broad_mac[7]={0xff,0xff,0xff,0xff,0xff,0xff,0x00};
    //以太网帧首部的硬件地址填FF:FF:FF:FF:FF:FF表示广播
    memset ( mac,0,sizeof ( mac ) );
    memset ( ip,0,sizeof ( ip ) );
    if ( GetLocalMac ( "eth0",mac,ip ) ==-1 )
        return ( -1 );

    printf ( "本地 Mac=[%s] Ip=[%s]\n", mac_ntoa ( mac ),inet_ntoa ( * ( struct in_addr * ) ip ) );

    //if ( argc==1 ) return ( -1 );
    /*sprintf ( dest,"192.168.0.%d",64 );
    uint32_t dip= inet_addr("192.168.0.66");
    sendpkg (mac,broad_mac,(char *)&dip,dest);
	/*/
    int i=0;
    for(i=10;i<76;i++){
         sprintf ( dest,"192.168.0.%d",i );
             sendpkg ( mac,broad_mac,ip,dest );
     }
/**/
    gettimeofday ( &tvafter , &tz );
    printf ( "\n程序执行完毕：%d毫秒\n", ( tvafter.tv_sec-tvpre.tv_sec ) *1000+ ( tvafter.tv_usec-tvpre.tv_usec ) /1000 );

    return 0;
} 

