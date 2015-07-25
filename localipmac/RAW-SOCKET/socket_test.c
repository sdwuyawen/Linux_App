#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#define BUFLEN 60

int main(int argc,char** argv){
    int i,skfd,n;
    char buf[ETH_FRAME_LEN]={0};
    struct ethhdr *eth;
    struct ether_arp *arp;
    struct sockaddr_ll fromaddr;
    struct ifreq ifr;

    unsigned char src_mac[ETH_ALEN]={0};

    if(2 != argc){
        printf("Usage: %s netdevName\n",argv[0]);
        exit(1);
    }

    //只接收发给本机的ARP报文
    if(0>(skfd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP)))){
        perror("Create Error");
        exit(1);
    }

    bzero(&fromaddr,sizeof(fromaddr));
    bzero(&ifr,sizeof(ifr));
    strcpy(ifr.ifr_name,argv[1]);

    //获取接口索引
    if(-1 == ioctl(skfd,SIOCGIFINDEX,&ifr)){
        perror("get dev index error:");
        exit(1);
    }
    fromaddr.sll_ifindex = ifr.ifr_ifindex;
    printf("interface Index:%d\n",ifr.ifr_ifindex);

    //获取接口的MAC地址
    if(-1 == ioctl(skfd,SIOCGIFHWADDR,&ifr)){
        perror("get dev MAC addr error:");
        exit(1);
    }

    memcpy(src_mac,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    printf("MAC :%02X-%02X-%02X-%02X-%02X-%02X\n",src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);

    fromaddr.sll_family = PF_PACKET;
    fromaddr.sll_protocol=htons(ETH_P_ARP);
    fromaddr.sll_hatype=ARPHRD_ETHER;
    fromaddr.sll_pkttype=PACKET_HOST;
    fromaddr.sll_halen=ETH_ALEN;
    memcpy(fromaddr.sll_addr,src_mac,ETH_ALEN);

    bind(skfd,(struct sockaddr*)&fromaddr,sizeof(struct sockaddr));

    while(1){
        memset(buf,0,ETH_FRAME_LEN);
        n=recvfrom(skfd,buf,ETH_FRAME_LEN,0,NULL,NULL);
        eth=(struct ethhdr*)buf;
        arp=(struct ether_arp*)(buf+14);

        printf("Dest MAC:");
        for(i=0;i<ETH_ALEN;i++){
            printf("%02X-",eth->h_dest[i]);
        }
        printf("Sender MAC:");
        for(i=0;i<ETH_ALEN;i++){
            printf("%02X-",eth->h_source[i]);
        }

        printf("\n");
        printf("Frame type:%0X\n",ntohs(eth->h_proto));

        if(ntohs(arp->arp_op)==2){
            printf("Get an ARP replay!\n");
        }
    }
    close(skfd);
    return 0;
}
