CC = gcc
CFLAGS = -Wall 
DEPS = sdb.h ext.h
OBJ = sdb.o ext.c server.c
INSTALL_DEST = /usr/bin/sdb

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<

sdb: $(OBJ)
	        gcc $(CFLAGS) -o $@ $^
install: 
	cp sdb $(INSTALL_DEST)

clean:
	rm *.o
	rm sdb
