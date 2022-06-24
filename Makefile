CC = gcc
SRC = src/*.c
CFLAGS = -Wall

ifeq ($(OS), Windows_NT)
	CFLAGS += -l:libregex.a -l:libtre.a -l:libintl.a -l:libtre.a -l:libiconv.a
endif

build:
	$(CC) -o nhcustom2 $(SRC) $(CFLAGS)
	
