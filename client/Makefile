.PHONY=clean all
# COMPILER=gcc
CFLAGS = --std=c11 -pedantic -pedantic-errors -g -Wall -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200809L -fsanitize=address -Wno-unused-variable -lpthread
SERVER = monitor
all: $(SERVER)
clean:	
	- rm -f *.o  $(SERVER)

COMMON =  $(wildcard *.c)
$(SERVER):      
	$(CC) $(CFLAGS) -o $(SERVER) $(COMMON) 