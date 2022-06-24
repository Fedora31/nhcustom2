CC = gcc
SRC = src/*.c
CFLAGS = -Wall

ifeq ($(OS), Windows_NT)
	CFLAGS += -lregex
endif

build:
	$(CC) -o nhcustom2 $(SRC) $(CFLAGS)
	
