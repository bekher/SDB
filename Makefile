CC = gcc
CFLAGS = -Wall
DEPS = sdb.h
OBJ = sdb.o 

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<

sdb: $(OBJ)
	        gcc $(CFLAGS) -o $@ $^
clean:
	rm *.o
