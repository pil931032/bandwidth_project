CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lws2_32

# Targets
all: server.exe client.exe

server.exe: server.c
	$(CC) $(CFLAGS) server.c -o server.exe $(LIBS)

client.exe: client.c
	$(CC) $(CFLAGS) client.c -o client.exe $(LIBS)

clean:
	del /Q server.exe client.exe

.PHONY: all clean
