CFLAGS = -Og -Wall -I../

OBJS = ../lssdp.o

all: client service 


packet_listener: $(OBJS) packet_listener.o
	$(CC) $(CFLAGS) -o $@.exe $@.o $(OBJS)

client: $(OBJS) client.o
	$(CC) $(CFLAGS) -o $@.exe $@.o $(OBJS)
service: $(OBJS) service.o
	$(CC) $(CFLAGS) -o $@.exe $@.o $(OBJS)
clean:
	rm -rf *.o *.exe
