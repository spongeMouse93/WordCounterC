CC = gcc
CFLAGS = -Wall -std=c99

SRC = words.c
TARGET = word_counter

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJ)


.PHONY: all clean
