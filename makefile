# CFLAGS = -I ./include
# test: ./src/main.o libstack.a
#       gcc ./src/main.o -lstack -L./lib -o test

# main.o: ./src/main.c
#       gcc $(CFLAGS) -c ./src/main.c -o ./src/main.o

# libstack.a: stack.o
#       ar -r ./lib/libstack.a ./lib/stack.o

# stack.o: ./lib/stack.c
#       gcc $(CFLAGS) -c ./lib/stack.c -o ./lib/stack.o

# clean:
#       rm test ./lib/libstack.a ./lib/stack.o src/main.o


cc = gcc
SOURCES =$(wildcard ./src/*.c)
INCLUDES =-I./include
LIB_PATH =-L./lib
OBJ =$(patsubst %.c, %.o, $(SOURCES))
#TARGET =server
EXECUTABLES = sequencial_server icmp_sniffer thread_server select_server epoll_server uv_server hash\
uv_prime_server counter MPMC semaphore
all: $(EXECUTABLES)
	@rm -r $(OBJ)
#links
sequencial_server: src/sequential_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ $(LIB_PATH) -o output/$@

icmp_sniffer: src/icmp_sniffer.o
	@mkdir -p output
	$(CC) $^ $(LIB_PATH) -o output/$@

thread_server: src/thread_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ $(LIB_PATH) -o output/$@ -lpthread

select_server: src/select_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ -lc -o output/$@

epoll_server: src/epoll_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ -lc -o output/$@

uv_server: src/uv_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ -luv -o output/$@

hash: src/hash.o
	@mkdir -p output
	$(CC) $^ -o output/$@

uv_prime_server: src/uv_prime_server.o src/utils.o
	@mkdir -p output
	$(CC) $^ -luv -o output/$@

counter: src/file_counter.o
	@mkdir -p output
	$(CC) $^ -o output/$@

MPMC: src/MPMC.o
	@mkdir -p output
	@$(CC) $^ $(LIB_PATH) -o output/$@ -lpthread

semaphore: src/semaphore.o
	@mkdir -p output
	@$(CC) $^ $(LIB_PATH) -o output/$@ -lpthread

#compile
%.o: %.c
	$(CC) -g $(INCLUDES)  -c  $< -o $@

.PHONY:clean

clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(EXECUTABLES) output
