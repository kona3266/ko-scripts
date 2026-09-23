# ko-scripts

A collection of network servers, concurrency examples, small tools and
protocol demos, organized by category.

## Directory Structure

```
ko-scripts/
├── makefile               # top-level build file (output under build/)
├── common/                # shared code
│   ├── utils.c/h          #   socket helpers (listen, peer reporting)
├── third_party/           # third-party headers
│   └── uthash.h
├── src/
│   ├── servers/           # TCP servers, same protocol, different IO models
│   │   ├── sequential_server.c   # one client at a time
│   │   ├── thread_server.c       # thread per connection
│   │   ├── select_server.c       # select() event loop
│   │   ├── epoll_server.c        # epoll event loop
│   │   ├── uv_server.c           # libuv based
│   │   └── uv_prime_server.c     # libuv, prime number protocol
│   ├── clients/           # python test clients for the servers above
│   │   ├── simple_client.py      # N concurrent clients with preset payloads
│   │   └── prime_client.py       # client for uv_prime_server
│   ├── concurrency/       # pthread synchronization examples
│   │   ├── MPMC.c                # multi-producer multi-consumer queue
│   │   └── semaphore.c           # producer/consumer with semaphores
│   ├── datastruct/
│   │   └── hash.c                # uthash usage example
│   └── tools/
│       ├── icmp_sniffer.c        # raw socket ICMP sniffer
│       └── file_counter.c        # recursive file counter
└── protobuf/              # protobuf serialization demos
    ├── target.proto
    ├── utils.py                  # varint encode/decode helpers
    ├── tcp/                      # protobuf over TCP (length-prefixed)
    │   ├── proto_tcpser.py
    │   └── proto_tcpcli.py
    └── zmq/                      # protobuf over ZeroMQ
        ├── proto_server.py
        ├── proto_client.py
        └── target_pb2.py         # generated from target.proto
```

## Building

```sh
make            # build everything into build/ (mirrors the src/ layout)
make servers    # only the TCP servers
make concurrency / tools / datastruct
make clean
```

Notes:

- `uv_server` / `uv_prime_server` require libuv development headers
  (`libuv1-dev`); they are skipped automatically when libuv is not installed.
- `thread_server`, `MPMC`, `semaphore` link with `-lpthread`; the uv based
  servers link with `-luv`.

## Running

```sh
./build/src/servers/sequential_server 9090
python3 src/clients/simple_client.py localhost 9090
```
