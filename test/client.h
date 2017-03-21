#ifndef CLIENT_H
#define CLIENT_H


#include <string>
#include <list>

typedef enum {PAIRED, UNREACHABLE} t_state;


typedef struct device
{
	t_state state;
	std::string barcode;
	std::string location;
	long long update_time;
} device;

std::list<device*> device_list;
std::list<device*> paired_device_list;

#endif
