CC = gcc
CFLAGS = -Wall -Werror

all: AS US

AS: server.c
	$(CC) $(CFLAGS) server.c -o AS

US: user.c
	$(CC) $(CFLAGS) user.c -o US

clean:
	rm -f AS US