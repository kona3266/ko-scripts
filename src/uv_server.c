#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "utils.h"

#define N_BACKLOG 64
typedef enum
{
    INITIAL_ACK,
    WAIT_FOR_MSG,
    IN_MSG
} InternalState;
#define SENDBUF_SIZE 1024

typedef struct
{
    InternalState state;
    char sendbuf[SENDBUF_SIZE];
    int sendbuf_end;
    uv_tcp_t *client;

} peer_state_t;

void on_alloc_buffer(uv_handle_t *handle, size_t suggest_size,
                     uv_buf_t *buf)
{
    buf->base = (char *)calloc(1, suggest_size);
    if (buf->base == NULL)
    {
        printf("unable to allocate mem");
        exit(1);
    }
    buf->len = suggest_size;
}

void on_client_closed(uv_handle_t *handle)
{
    uv_tcp_t *client = (uv_tcp_t *)handle;
    if (client->data)
    {
        free(client->data);
    }
    free(client);
}

int peer_end(peer_state_t *peer_state)
{
    if (peer_state->sendbuf_end < 3)
    {
        printf("%c", peer_state->sendbuf[0]);
        return 0;
    }
    char *last, *second_to_last, *third_to_last;
    last = &(peer_state->sendbuf[peer_state->sendbuf_end - 1]);
    second_to_last = &(peer_state->sendbuf[peer_state->sendbuf_end - 2]);
    third_to_last = &(peer_state->sendbuf[peer_state->sendbuf_end - 3]);

    if (*third_to_last == 'X' && *second_to_last == 'Y' && *last == 'Z')
    {
        return 1;
    }
    printf("%c", peer_state->sendbuf[0]);
    return 0;
}

void on_wrote_buf(uv_write_t *req, int status)
{
    if (status)
    {
        printf("Write error %s\n", uv_strerror(status));
        exit(1);
    }

    peer_state_t *peer_state = (peer_state_t *)req->data;
    if (peer_end(peer_state) == 1)
    {
        free(peer_state);
        free(req);
        printf("stop default loop\n");
        uv_stop(uv_default_loop());
        return;
    }
    peer_state->sendbuf_end = 0;
    free(req);
}

void on_peer_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        }
        uv_close((uv_handle_t *)client, on_client_closed);
    }
    else if (nread == 0)
    {
    }
    else
    {
        assert(buf->len >= nread);
        peer_state_t *peer_state = (peer_state_t *)client->data;
        if (peer_state->state == INITIAL_ACK)
        {
            free(buf->base);
            return;
        }

        for (int i = 0; i < nread; ++i)
        {
            switch (peer_state->state)
            {
            case INITIAL_ACK:
                break;
            case WAIT_FOR_MSG:
                if (buf->base[i] == '^')
                {
                    peer_state->state = IN_MSG;
                }
                break;
            case IN_MSG:
                if (buf->base[i] == '$')
                {
                    peer_state->state = WAIT_FOR_MSG;
                }
                else
                {
                    assert(peer_state->sendbuf_end < SENDBUF_SIZE);
                    peer_state->sendbuf[peer_state->sendbuf_end] = buf->base[i] + 1;
                    peer_state->sendbuf_end++;
                }
                break;
            }
        }

        if (peer_state->sendbuf_end > 0)
        {
            uv_buf_t write_buf = uv_buf_init(peer_state->sendbuf, peer_state->sendbuf_end);
            uv_write_t *writereq = (uv_write_t *)calloc(1, sizeof(*writereq));
            writereq->data = peer_state;
            int rc;
            if ((rc = uv_write(writereq, (uv_stream_t *)client, &write_buf, 1,
                               on_wrote_buf)) < 0)
            {
                fprintf(stderr, "uv_write failed: %s", uv_strerror(rc));
                exit(1);
            }
        }
    }
    free(buf->base);
}

void on_wrote_init_ack(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "write err: %s\n", uv_strerror(status));
        exit(1);
    }

    peer_state_t *peer_state = (peer_state_t *)req->data;
    peer_state->state = WAIT_FOR_MSG;
    peer_state->sendbuf_end = 0;

    int rc;
    if ((rc = uv_read_start((uv_stream_t *)peer_state->client, on_alloc_buffer, on_peer_read)) < 0)
    {
        fprintf(stderr, "uv_read_start err: %s\n", uv_strerror(rc));
        exit(1);
    }
    free(req);
}

void on_peer_connected(uv_stream_t *server_stream, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "Peer connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)calloc(1, sizeof(*client));
    int rc;
    if ((rc = uv_tcp_init(uv_default_loop(), client)) < 0)
    {
        fprintf(stderr, "uv_tcp_init failed: %s", uv_strerror(rc));
        exit(1);
    }
    client->data = NULL;
    if ( uv_accept(server_stream, (uv_stream_t *)client) == 0)
    {
        struct sockaddr_storage peername;
        int namelen = sizeof(peername);
        if ((rc = uv_tcp_getpeername(client, (struct sockaddr *)&peername,
                                     &namelen)) < 0)
        {
            fprintf(stderr, "uv_tcp_getpeername failed: %s", uv_strerror(rc));
            exit(1);
        }
        report_peer_connected((const struct sockaddr_in *)&peername, namelen);

        peer_state_t *peerstate = (peer_state_t *)calloc(1, sizeof(*peerstate));
        peerstate->state = INITIAL_ACK;
        peerstate->sendbuf[0] = '*';
        peerstate->sendbuf_end = 1;
        peerstate->client = client;
        client->data = peerstate;

        uv_buf_t writebuf = uv_buf_init(peerstate->sendbuf, peerstate->sendbuf_end);
        uv_write_t *req = (uv_write_t *)calloc(1, sizeof(*req));
        req->data = peerstate;
        if ((rc = uv_write(req, (uv_stream_t *)client, &writebuf, 1,
                           on_wrote_init_ack)) < 0)
        {
            fprintf(stderr, "uv_write failed: %s", uv_strerror(rc));
            exit(1);
        }
    }
    else
    {
        uv_close((uv_handle_t *)client, on_client_closed);
    }
}

int main(int argc, const char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int portnum = 9090;
    if (argc >= 2)
    {
        portnum = atoi(argv[1]);
    }
    printf("Serving on port %d\n", portnum);

    int rc;
    uv_tcp_t server_stream;
    if ((rc = uv_tcp_init(uv_default_loop(), &server_stream)) < 0)
    {
        fprintf(stderr, "uv_tcp_init failed: %s", uv_strerror(rc));
        exit(1);
    }

    struct sockaddr_in server_address;
    if ((rc = uv_ip4_addr("0.0.0.0", portnum, &server_address)) < 0)
    {
        fprintf(stderr, "uv_ip4_addr failed: %s", uv_strerror(rc));
        exit(1);
    }

    if ((rc = uv_tcp_bind(&server_stream, (const struct sockaddr *)&server_address,
                          0)) < 0)
    {
        fprintf(stderr, "uv_tcp_bind failed: %s", uv_strerror(rc));
        exit(1);
    }

    // Listen on the socket for new peers to connect. When a new peer connects,
    // the on_peer_connected callback will be invoked.
    if ((rc = uv_listen((uv_stream_t *)&server_stream, N_BACKLOG,
                        on_peer_connected)) < 0)
    {
        fprintf(stderr, "uv_listen failed: %s", uv_strerror(rc));
        exit(1);
    }

    // Run the libuv event loop.
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    // If uv_run returned, close the default loop before exiting.
    return uv_loop_close(uv_default_loop());
}