CC ?= gcc
CFLAGS ?= -fPIC

all: liblssdp.so liblssdp.a
	$(MAKE) -C test

liblssdp.so: lssdp.o
	$(CC) -o liblssdp.so lssdp.o  -fPIC -shared

liblssdp.a: lssdp.o
	$(AR) rcv liblssdp.a lssdp.o

install: all

clean:
	rm -rf *.o *.so *.a
	$(MAKE) -C test clean
