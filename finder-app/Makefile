#Daniel Mendez
#ECEN 5713
#With assistance from CHATGPT for doing the clean and object file variable

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra #Usually we do this for all assignments

OUTPUT = writer

SOURCES = writer.c

OBJS = $(SOURCES:.c=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJS)

clean:
	rm -f $(OUTPUT) $(OBJS)
