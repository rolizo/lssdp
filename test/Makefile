CXX ?= g++
CXXFLAGS ?= -g -Og -std=c++11 -Wall -Werror -I../

OBJS = ../lssdp.o
EXE = .exe
BINS = client$(EXE) service$(EXE)

all: $(BINS)

packet_listener: $(OBJS) packet_listener.o
	$(CXX) $(CXXFLAGS) -o $@.exe $@.o $(OBJS)

client$(EXE): client.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

service$(EXE): service.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJS)

clean:
	rm -rf *.o $(BINS)
