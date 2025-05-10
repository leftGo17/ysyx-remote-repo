CC = gcc
CFLAGS = -Wall -g

TARGET = hello
SRCS = hello.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY:all clean