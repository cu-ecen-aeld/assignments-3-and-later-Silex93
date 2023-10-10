#Daniel Mendez
#ECEN 5713
#Assignment 5

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -g #Usually we do this for all assignments

OUTPUT = aesdsocket

SOURCES = aesdsocket.c

OBJS = $(SOURCES:.c=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJS)

clean:
	rm -f $(OUTPUT) $(OBJS)
