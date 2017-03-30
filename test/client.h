#ifndef CLIENT_H
#define CLIENT_H


#define ALIVE_TIMEOUT 10


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

typedef std::list<device*> t_device_list;
t_device_list device_list;
t_device_list paired_device_list;

#endif
