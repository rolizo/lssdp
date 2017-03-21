CXX ?= g++
CXXFLAGS ?= -g -Og -std=c++11 -Wall -Werror -I../

OBJS = ../lssdp.o

all: client service 


packet_listener: $(OBJS) packet_listener.o
	$(CXX) $(CXXFLAGS) -o $@.exe $@.o $(OBJS)

client: $(OBJS) client.o
	$(CXX) $(CXXFLAGS) -o $@.exe $@.o $(OBJS)
service: $(OBJS) service.o
	$(CXX) $(CXXFLAGS) -o $@.exe $@.o $(OBJS)
clean:
	rm -rf *.o *.exe
