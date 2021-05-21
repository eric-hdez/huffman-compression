CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Werror -g

EXEC = decode encode
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

all: $(EXEC)

encode: encode.o code.o huffman.o io.o pq.o node.o stack.o 
	$(CC) $(CFLAGS) -o $@ $^

decode: decode.o code.o huffman.o io.o pq.o node.o stack.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -rf $(EXEC) $(OBJ)

format: 
	clang-format -i -style=file *.[ch]
