CC      = gcc
CFLAGS  = -Wall -Wextra -pthread $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lSDL2_ttf -lpthread
TARGET  = restaurante

SRCS = main.c buffer.c render.c restaurante.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
