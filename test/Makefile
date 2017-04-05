CC ?= gcc
CXX ?= g++
CFLAGS ?= -g -Og -std=c99 -Wall -Werror -I../
CXXFLAGS ?= -g -Og -std=c++11 -Wall -Werror -I../

EXE = .exe
BINS = client$(EXE) service$(EXE)

all: $(BINS)

packet_listener: packet_listener.o
	$(CXX) $(CXXFLAGS) -o $@.exe $@.o

client$(EXE): client.o
	$(CXX) -pthread -static $(CXXFLAGS) -o $@ $< -L.. -llssdp -lncurses -ltinfo -lgpm

service$(EXE): service.o
	$(CXX) -static $(CXXFLAGS) -o $@ $< -L.. -llssdp

clean:
	rm -rf *.o $(BINS)
