CC = gcc
CFLAGS = -Wall -g
DEPS = sdb.h ext.h
OBJ = sdb.o ext.c server.c

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<

sdb: $(OBJ)
	        gcc $(CFLAGS) -o $@ $^
clean:
	rm *.o
