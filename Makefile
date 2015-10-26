CC = gcc
CFLAGS = -Wall -g
DEPS = sdb.h
OBJ = sdb.o testdriver.c

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<

sdb: $(OBJ)
	        gcc $(CFLAGS) -o $@ $^
clean:
	rm *.o
