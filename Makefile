CC ?= gcc
CFLAGS ?= -fPIC

all: lib
	$(MAKE) -C test

lib: lssdp.o
	gcc -o liblssdp.so lssdp.o  -fPIC -shared

clean:
	rm -rf *.o *.so
	$(MAKE) -C test clean
