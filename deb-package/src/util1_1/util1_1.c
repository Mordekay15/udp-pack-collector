#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define Q_PERMISSIONS 0660
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10


char* parm[4];
char* iface;
int packets = 0, packets_len = 0, sock_r, iphdrlen;

struct sockaddr saddr;
struct sockaddr_in source,dest;

pthread_mutex_t mtx;

void udp_header(unsigned char* buffer, int buflen)
{
    struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));

    iphdrlen =ip->ihl*4;

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
    int bytes = ntohs(udp->len);
    if (atoi(parm[0]) == 0 && atoi(parm[1]) == 0 && atoi(parm[2]) == 0 && atoi(parm[3]) == 0) {
	pthread_mutex_lock(&mtx);
	packets++;
        packets_len += bytes;
	pthread_mutex_unlock(&mtx);
	return;
    }

    if (parm[0] != 0 && parm[0] == inet_ntoa(source.sin_addr)) {
	pthread_mutex_lock(&mtx);
	packets++;
        packets_len += bytes;
	pthread_mutex_unlock(&mtx);
	return;
    }

    if (parm[1] != 0 && parm[1] == inet_ntoa(dest.sin_addr)) {
	pthread_mutex_lock(&mtx);
	packets++;
        packets_len += bytes;
	pthread_mutex_unlock(&mtx);
	return;
    }

    if (parm[2] != 0 && atoi(parm[2]) == ntohs(udp->source)) {
	pthread_mutex_lock(&mtx);
	packets++;
        packets_len += bytes;
	pthread_mutex_unlock(&mtx);
	return;
    }

    if (parm[3] != 0 && atoi(parm[3]) == ntohs(udp->dest)) {
	pthread_mutex_lock(&mtx);
	packets++;
	packets_len += bytes;
	pthread_mutex_unlock(&mtx);
	return;
    }
}

void set_iface(int sock_r, char* iface)
{
    struct sockaddr_ll bsock;
    struct ifreq ifr;

    bzero(&bsock, sizeof(bsock));
    bzero(&ifr, sizeof(ifr));
    strncpy((char *) ifr.ifr_name, iface, IFNAMSIZ);
    if ((ioctl(sock_r, SIOCGIFINDEX, &ifr)) == -1) {
        perror("Unabele to find iface index\n");
        exit(1);
    }

    bsock.sll_family = AF_PACKET;
    bsock.sll_ifindex = ifr.ifr_ifindex;
    bsock.sll_protocol = htons(ETH_P_IP);

    if ((bind (sock_r, (struct sockaddr *) &bsock, sizeof(bsock))) == -1) {
        perror("bind: \n");
        exit(-1);
    }
}

void data_process(unsigned char* buffer,int buflen)
{
    struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
    switch (ip->protocol)
    {
        case 17:
            udp_header(buffer,buflen);
            break;
    }
}


void *open_socket()
{
    int sock_r,saddr_len,buflen;

    unsigned char* buffer = (unsigned char *)malloc(65536);
    memset(buffer,0,65536);

    sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sock_r < 0) {
        printf("error in socket\n");
        exit(-1);
    }

    set_iface(sock_r, iface);
    printf("Reciveing packets...\n");

    while (1)
    {
        saddr_len = sizeof saddr;
        buflen = recvfrom(sock_r, buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);

        if (buflen < 0) {
            printf("error in reading recvfrom function\n");
            exit(-1);
        }

        data_process(buffer,buflen);
	sleep(2);
    }

    close(sock_r);
    pthread_exit(0);

}

int count(int num)
{
    int res;
    if (num/10 == 0) return 1;
    else {
	while (num > 0) {
	    num = num /10;
	    res++;
	}
	return res;
    }
}


char* convert_statictics(int parmtr)
{
    int v = count(parmtr);
    char* arr = (char*)malloc(v * sizeof(char));

    int i = 0;
    while (parmtr > 9) {
	arr[i++] = (parmtr % 10) + '0';
	parmtr = parmtr / 10;
    }
    arr[i++] = parmtr + '0';
    arr[i] = '\0';

    char t;
    for (int j = 0; j < i / 2; j++) {
	t = arr[j];
	arr[j] = arr[i - 1 -j];
	arr[i - 1 -j] = t;
    }

    return arr;
}

void *count_statistics()
{
    mqd_t qd_server, qd_client;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_client = mq_open ("/client", O_RDONLY | O_CREAT, Q_PERMISSIONS, &attr)) == -1) {
        perror ("Util1: mq_open (util1)");
        exit (1);
    }

    if ((qd_server = mq_open ("/server", O_WRONLY)) == -1) {
        perror ("Util1: mq_open (util2)");
        exit (1);
    }

    int bytes = 0;
    int size = 0;
    char in_buffer [MSG_BUFFER_SIZE];

    while (1) {

	if (size != packets) {
	pthread_mutex_lock(&mtx);
	    char* p_arr = convert_statictics(packets);
	    char* b_arr = convert_statictics(packets_len);

	    strcat(p_arr, " ");
	    strcat(p_arr, b_arr);

	    if (mq_send (qd_server, p_arr, strlen(p_arr) + 1, 0) == -1) {
                perror ("Util1: Not able to send message to util2");
                continue;
            }

	    size = packets;
	    pthread_mutex_unlock(&mtx);
	}
    }

    pthread_exit(0);
}

int main(int argc,char **argv)
{
    iface = argv[1];

    for(int i = 0; i < 4; i++)
	parm[i] = argv[i+2];

    pthread_t first, second;
    pthread_attr_t attr1, attr2;

    pthread_attr_init(&attr1);

    pthread_create(&first, &attr1, open_socket, NULL);

    pthread_attr_init(&attr2);

    pthread_create(&second, &attr2, count_statistics, NULL );

    pthread_join(first, NULL);
    pthread_join(second, NULL);

    return 0;
}

