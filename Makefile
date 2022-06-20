CC = gcc
SRC = src/*.c
CFLAGS = -Wall

build:
	$(CC) -o nhcustom2 $(SRC) $(CFLAGS)
	
