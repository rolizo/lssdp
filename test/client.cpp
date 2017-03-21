#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday
#include "lssdp.h"
#include <list>
#include <iostream>
#include <algorithm>
#include "client.h"


void print_devices();
int ping_device(struct device*target);
int add_device(lssdp_packet &packet);
int remove_device(std::string barcode);
device* find_device(std::string barcode);

void log_callback(const char * file, const char * tag, int level, int line,
                  const char * func, const char * message) {
	std::string level_name = "DEBUG";
	if (level == LSSDP_LOG_INFO)   level_name = "INFO";
	if (level == LSSDP_LOG_WARN)   level_name = "WARN";
	if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";

	printf("[%-5s][%s] %s", level_name.c_str(), tag, message);
}

long long get_current_time() {
	struct timeval time = {};
	if (gettimeofday(&time, NULL) == -1) {
		printf("gettimeofday failed, errno = %s (%d)\n", strerror(errno), errno);
		return -1;
	}
	return (long long) time.tv_sec * 1000 + (long long) time.tv_usec / 1000;
}

void print_lssdp_packet(lssdp_packet &parsed_packet) {

	printf("---------------------------------------------------\n");
	printf("METHOD: %s\n",parsed_packet.method);
	printf("ST: %s\n",parsed_packet.st);
	printf("USN: %s\n",parsed_packet.usn);
	printf("LOCATION: %s\n",parsed_packet.location);
	printf("SM_ID: %s\n",parsed_packet.sm_id);
	printf("DEVICE: %s\n",parsed_packet.device_type);
	printf("NTS: %s\n",parsed_packet.nts);
	printf("---------------------------------------------------\n");
}

int parse_packet(struct lssdp_ctx * lssdp, const char * packet,
                 size_t packet_len) {

	lssdp_packet parsed_packet = {};
	if (lssdp_packet_parser(packet, packet_len, &parsed_packet) != 0) {
		printf("Failed to parse packet");
		return -1;
	}

	print_lssdp_packet(parsed_packet);

	if(strcmp("ssdp:alive",parsed_packet.nts) == 0) {
		add_device(parsed_packet);
	} else if(strcmp("ssdp:byebye",parsed_packet.nts) == 0) {
		std::string check_barcode = parsed_packet.usn;
		remove_device(check_barcode);
	} else if (strcmp("ssdp:all",parsed_packet.nts) == 0) {
	} else if (strcmp( parsed_packet.method,"RESPONSE") == 0) {
		add_device(parsed_packet);
	} else 	{
		printf("UNSUPPORTED PACKET\n\n");
	}

	return 0;

	return -1;
}

//returns 0 if device was removed
//returns -1 if no device was removed
int remove_device(std::string barcode) {

	device*match = find_device(barcode);

	if (match) {
		device_list.remove(match);
		print_devices();
		return 0;
	}

	return -1;

}


//Find device in device list with matching barcode
//Returns Null if not found
device* find_device(std::string barcode) {

	std::list<struct device*>::iterator iter;
	for(iter = device_list.begin(); iter != device_list.end(); iter++) {
		if((*iter)->barcode == barcode) {
			return (*iter);
		}
	}
	return NULL;
}


bool is_device_unique(std::string barcode) {

	if(find_device(barcode))
		return false;
	return true;
}

int add_device(lssdp_packet &packet) {

	std::string check_barcode = packet.usn;
	if(!is_device_unique(check_barcode)) {
		return -1;
	}

	device *ptr = new device();
	ptr->state=UNREACHABLE;
	ptr->barcode=packet.usn;
	ptr->update_time=packet.update_time;
	device_list.push_back(ptr);

	print_devices();
	return 0;
}


int ping_device(struct device*target) {


	return 0;
}

void ping_devices() {

	std::list<struct device*>::iterator iter;
	for(iter = device_list.begin(); iter != device_list.end(); iter++) {
		ping_device(*iter);
	}

}

void print_devices() {


	std::cout << "--Device List---" << std::endl;
	std::list<struct device*>::iterator iter;
	for(iter = device_list.begin(); iter != device_list.end(); iter++) {
		std::string status;
		if( (*iter)->state ==  PAIRED) {
			status = "PAIRED";
		} else {
			status = "UNREACHABLE";
		}
		std::cout << (*iter)->barcode << "    " << status << std::endl;
	}
	std::cout << "--End Device List---" << std::endl;
}


void connect_device(struct device *ptr)
{
	paired_device_list.push_back(ptr);
}

void disconnect_device(struct device *ptr) {

	std::list<struct device*>::iterator iter;
	for(iter = paired_device_list.begin(); iter != paired_device_list.end();
	        iter++) {
		if((*iter)->barcode == ptr->barcode) {
			paired_device_list.remove(*iter);
		}
	}
}


int main() {

	lssdp_ctx lssdp;
	lssdp.port = 1900;
	lssdp.debug = true;
	lssdp.packet_received_callback  = parse_packet;

	lssdp_init(&lssdp);

	lssdp.config.ADDR_LOCALHOST = "::1";
	lssdp.config.ADDR_MULTICAST = "FF02::C";
	lssdp_set_log_callback(log_callback);

	strncpy(lssdp.header.location.prefix,"http://\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.location.domain,"test_location",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.location.suffix,":8082\0",LSSDP_FIELD_LEN);

	strncpy(lssdp.header.search_target,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.device_type,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.sm_id,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.unique_service_name,"Barcode",LSSDP_FIELD_LEN);
	lssdp_socket_create(&lssdp);
	lssdp_send_msearch(&lssdp);

	// Main Loop
	for (;;) {
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(lssdp.sock, &fs);
		struct timeval tv;
		tv.tv_usec = 500 * 1000;   // 500 ms


		//TODO: Remove devices which timed out from list

		int ret = select(lssdp.sock + 1, &fs, NULL, NULL, &tv);
		if (ret < 0) {
			printf("select error, ret = %d\n", ret);
			break;
		}

		if (ret > 0) {
			lssdp_socket_read(&lssdp);
		}
	}

	return EXIT_SUCCESS;
}
