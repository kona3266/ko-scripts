#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/select.h>
#include <unistd.h>

#include "utils.h"

#define MAXFDS 1000
#define SENDBUF_SIZE 1024

typedef enum
{
    INITIAL_ACK,
    WAIT_FOR_MSG,
    IN_MSG
} InternalState;

typedef struct{
    InternalState state;
    uint8_t sendbuf[SENDBUF_SIZE];
    int sendbuf_end;
    int sendptr;
} peer_state_t;

peer_state_t global_state[MAXFDS];

typedef struct 
{
    bool want_read;
    bool want_write;
} fd_status_t;

const fd_status_t fd_status_R = {.want_read = true, .want_write = false};
const fd_status_t fd_status_W = {.want_read = false, .want_write = true};
const fd_status_t fd_status_RW = {.want_read= true, .want_write = true};
const fd_status_t fd_status_NORW = {.want_read = false, .want_write = false};

fd_status_t on_peer_connected(int sockfd, const struct sockaddr_in* peer_addr, socklen_t peer_addr_len)
{
    assert(sockfd < MAXFDS);
    report_peer_connected(peer_addr, peer_addr_len);
    peer_state_t* peerstate = &global_state[sockfd];
    peerstate->state = INITIAL_ACK;
    peerstate->sendbuf[0] = '*';
    peerstate->sendptr = 0;
    peerstate->sendbuf_end = 1;

    return fd_status_W;
}

fd_status_t on_peer_ready_recv(int sockfd)
{
    assert(sockfd < MAXFDS);
    peer_state_t* peerstate = &global_state[sockfd];
    if (peerstate->state == INITIAL_ACK || peerstate->sendptr < peerstate->sendbuf_end){
        // receive data until all data staged is sent
        return fd_status_W;
    }

    uint8_t buf[1024];
    int nbytes = recv(sockfd, buf, sizeof(buf), 0);
    if (nbytes == 0){
        return fd_status_NORW;
    } else if (nbytes < 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK){
            return fd_status_R;
        }else{
            perror("recv");
            exit(1);
        }
    }
    bool ready_to_send = false;
    for (int i=0; i< nbytes; ++i){
        switch (peerstate->state)
        {
        case INITIAL_ACK:
            assert(0 && "cant reach here");
            break;
        case WAIT_FOR_MSG:
            if(buf[i] == '^'){
                peerstate->state = IN_MSG;
            }
            break;
        case IN_MSG:
            if (buf[i] == '$'){
                peerstate->state = WAIT_FOR_MSG;
            } else {
                assert(peerstate->sendbuf_end < SENDBUF_SIZE);
                peerstate->sendbuf[peerstate->sendbuf_end] = buf[i] + 1;
                peerstate->sendbuf_end++;
                ready_to_send = true;
            }
            break;
        }
    }
    return (fd_status_t){.want_read = !ready_to_send,
                         .want_write = ready_to_send};

}

fd_status_t on_peer_ready_send(int sockfd)
{
    assert(sockfd < MAXFDS);
    peer_state_t* peer_state = &global_state[sockfd];
    if (peer_state->sendptr >= peer_state->sendbuf_end){
        return fd_status_RW;
    }
    int sendlen = peer_state->sendbuf_end - peer_state->sendptr;
    int nsent = send(sockfd, &peer_state->sendbuf[peer_state->sendptr], sendlen, 0);
    if (nsent == -1){
        if (errno == EAGAIN || errno == EWOULDBLOCK){
            return fd_status_W;
        }else {
            perror("send");
            exit(1);
        }
    }

    if (nsent < sendlen){
        peer_state->sendptr += nsent;
        return fd_status_W;
    }else {
        peer_state->sendptr = 0;
        peer_state->sendbuf_end = 0;

        if (peer_state->state == INITIAL_ACK) {
            peer_state->state = WAIT_FOR_MSG;
        }
        return fd_status_R;
    }
}

void make_sockfd_non_blocking(int sockfd){
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1){
        perror("fcntl F_GETFL");
        exit(1);
    }
    if ( fcntl(sockfd, F_SETFL, flags| O_NONBLOCK) == -1){
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(1);
    }

}

int main(int argc, char** argv){
    setvbuf(stdout, NULL, _IONBF, 0);
    int portnum = 9090;
    if (argc >= 2){
        portnum = atoi(argv[1]);
    }
    printf("serve on port %d\n", portnum);

    int listener_fd = listen_inet_socket(portnum);
    make_sockfd_non_blocking(listener_fd);
    if (listener_fd >= FD_SETSIZE){
        fprintf(stderr, "listner socket fd %d >= FD_SETSIZE %d\n", listener_fd, FD_SETSIZE);
        exit(1);
    }

    fd_set readfd_set;
    FD_ZERO(&readfd_set);
    fd_set writefd_set;
    FD_ZERO(&writefd_set);

    FD_SET(listener_fd, &readfd_set);
    int fdset_max = listener_fd;
    while (1){
        fd_set readfds = readfd_set;
        fd_set writefds = writefd_set;
        // select modified the fd_sets pass to it, so use copies to pass
        int nready = select(fdset_max + 1, &readfds, &writefds, NULL, NULL);
        if (nready < 0) {
            perror("select");
            exit(1);
        }

        for (int fd = 0; fd <= fdset_max && nready > 0; fd++){
            if (FD_ISSET(fd, &readfds))
            {
                nready--;
                if ( fd == listener_fd) 
                {
                    struct sockaddr_in peer_addr;
                    socklen_t peer_addr_len = sizeof(peer_addr);
                    int newfd = accept(listener_fd, (struct sockaddr*)&peer_addr, &peer_addr_len);
                    if (newfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK){
                            printf("accept return EAGAIN OR EWOULDBLOCK\n");
                        } else {
                            perror("accept");
                            exit(1);
                        }
                    }else {
                        make_sockfd_non_blocking(newfd);
                        if (newfd > fdset_max) {
                            if (newfd >= FD_SETSIZE) {
                                fprintf(stderr, "socket fd %d >= FD_SETSIZE %d\n", newfd, FD_SETSIZE);
                                exit(1);
                            }
                            fdset_max = newfd;
                        }
                        fd_status_t status = on_peer_connected(newfd, &peer_addr, peer_addr_len);
                        if (status.want_read){
                            FD_SET(newfd, &readfd_set);
                        }else{
                            FD_CLR(newfd, &readfd_set);
                        }

                        if (status.want_write){
                            FD_SET(newfd, &writefd_set);
                        }else {
                            FD_CLR(newfd, &writefd_set);
                        }
                    }
                }else
                {
                    fd_status_t status = on_peer_ready_recv(fd);
                    if (status.want_read){
                        FD_SET(fd, &readfd_set);
                    }else{
                        FD_CLR(fd, &readfd_set);
                    }

                    if (status.want_write){
                        FD_SET(fd, &writefd_set);
                    }else {
                        FD_CLR(fd, &writefd_set);
                    }
                    if (!status.want_read && !status.want_write){
                        printf("socket %d closing\n", fd);
                        close(fd);
                    }
                }
            }

            if (FD_ISSET(fd, &writefds)){
                nready--;
                fd_status_t status = on_peer_ready_send(fd);
                if (status.want_read){
                    FD_SET(fd, &readfd_set);
                }else{
                    FD_CLR(fd, &readfd_set);
                }

                if (status.want_write){
                    FD_SET(fd, &writefd_set);
                }else {
                    FD_CLR(fd, &writefd_set);
                }
                if (!status.want_read && !status.want_write){
                    printf("socket %d closing\n", fd);
                    close(fd);
                }
            }
        }
    }
    return 0;
}