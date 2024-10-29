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
TARGET =app

#links
$(TARGET):$(OBJ)
        @mkdir -p output
        $(CC) $(OBJ) $(LIB_PATH) -o output/$(TARGET)
        @rm -rf $(OBJ)

#compile
%.o: %.c
        $(CC) $(INCLUDES) -c  $< -o $@

.PHONY:clean
clean:
        @echo "Remove linked and compiled files......"
        rm -rf $(OBJ) $(TARGET) output
