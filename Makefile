TARGET = jogo

CC = gcc

CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRCS = main.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) $(TARGET)