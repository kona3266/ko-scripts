// Sequential socket server - accepting one client at a time.
#include <stdint.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "utils.h"

typedef enum
{
    WAIT_FOR_MSG,
    IN_MSG
} InternalState;

void handle_connection(int sockfd)
{
    // do this "ack" from the server which the client expects to see before proceeding.
    if (send(sockfd, "*", 1, 0) < 1)
    {
        perror("send failed");
        exit(1);
    }

    // equal to  enum {WAIT_FOR_MSG, IN_MSG} state = WAIT_FOR_MSG;
    // define state is a  enum var and assign WAIT_FOR_MSG to it
    // the first sign in enum type is 0
    InternalState state = WAIT_FOR_MSG;
    while (1)
    {
        uint8_t buf[1024];
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if (len < 0)
        {
            perror("recv");
            exit(1);
        }
        else if (len == 0)
        {
            printf("EOF exit\n");
            break;
        }
        for (int i = 0; i < len; i++)
        {
            switch (state)
            {
            case WAIT_FOR_MSG:
                if (buf[i] == '^')
                {
                    state = IN_MSG;
                }
                break;
            case IN_MSG:
                if (buf[i] == '$')
                {
                    state = WAIT_FOR_MSG;
                }
                else
                {
                    buf[i] += 1;
                    if (send(sockfd, &buf[i], 1, 0) < 1)
                    {
                        perror("send");
                        close(sockfd);
                        return;
                    }
                }
                break;
            }
        }
    }
    close(sockfd);
}

int main(int argc, char **argv)
{
    int port = 9090;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }
    int sockfd = listen_inet_socket(port);

    while (1)
    {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        printf("waiting for connect...\n");
        int newsockfd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }

        report_peer_connected(&peer_addr, peer_addr_len);
        handle_connection(newsockfd);
        printf("peer done\n");
    }
}