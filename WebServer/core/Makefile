CC=gcc
Master=web4c
CFLAGS= -Wall -g -DLINUX

mobjs = web4c.o 

all: $(Master)

%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $<
$(Master): $(mobjs)
	$(CC) $(mobjs) $(LDFLAGS) -o $@  
clean:
	rm -f *.o $(Master) 
