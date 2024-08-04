#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>


int main(int argc, char**argv) {
    int n, soc;
    char buf[1500];
    struct in_addr insaddr;
    struct in_addr indaddr;
    struct icmphdr icmphdr;
    struct iphdr *recv_iphdr;
    struct icmphdr *recv_imcphdr;

    soc = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    struct sockaddr_ll server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sll_family = AF_PACKET;
    server_addr.sll_protocol = htons(ETH_P_ALL);
    server_addr.sll_ifindex = if_nametoindex("eth0");
    bind(soc, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
    while(1) {
        n = recv(soc, buf, sizeof(buf), 0);
        if (n < 1) {
            return 1;
        }
        recv_iphdr = (struct iphdr *)(buf + ETH_HLEN);
        if (recv_iphdr->protocol == IPPROTO_ICMP){
            printf("receive length %d, ", n);
            printf("ip header length %d\n", recv_iphdr->ihl << 2);
            recv_imcphdr = (struct icmphdr *)(buf + ETH_HLEN + (recv_iphdr->ihl << 2));
            // recv_iphdr->hdl 代表ip 头部的长度。由于ip头部长度的单位是32位字，一个32位字，等于4个字节。所以ihl乘以4后代表ip首部的字节长度。
            char *type[2] = {"request", "reply"};
    
            if (recv_imcphdr->type == ICMP_ECHOREPLY) {
                    printf("ICMP %s, ", type[1]);
            } else if (recv_imcphdr->type == ICMP_ECHO)  {
                    printf("ICMP %s, ", type[0]);
            }
            printf("msgtype=%d, code=%d ", recv_imcphdr->type, recv_imcphdr->code);
            insaddr.s_addr = recv_iphdr->saddr;
            indaddr.s_addr = recv_iphdr->daddr;
            printf("source addr %s ", inet_ntoa(insaddr));
            printf("dst addr %s\n", inet_ntoa(indaddr));
        }
    }
    close(soc);
    return 0;
}
