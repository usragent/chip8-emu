CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = chip8

SRCS = main.c chip8.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c chip8.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all clean
