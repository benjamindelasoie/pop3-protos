# compiler flags:
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -std=c99 -lpthread -Wall

# the build target executable:
TARGET = pop3.out
default: all

all: $(TARGET)

$(TARGET): *.c
	$(CC) *.c $(CFLAGS) -o $(TARGET) 

clean:
	-rm -f $(TARGET)