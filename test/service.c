#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday
#include "lssdp.h"


void log_callback(const char * file, const char * tag, int level, int line,
                  const char * func, const char * message) {
	char * level_name = "DEBUG";
	if (level == LSSDP_LOG_INFO)   level_name = "INFO";
	if (level == LSSDP_LOG_WARN)   level_name = "WARN";
	if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";

	printf("[%-5s][%s] %s", level_name, tag, message);
}

long long get_current_time() {
	struct timeval time = {};
	if (gettimeofday(&time, NULL) == -1) {
		printf("gettimeofday failed, errno = %s (%d)\n", strerror(errno), errno);
		return -1;
	}
	return (long long) time.tv_sec * 1000 + (long long) time.tv_usec / 1000;
}


int show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet,
                     size_t packet_len) {
	printf("%s", packet);
	return 0;
}

/*
Do something with the ssdp info


1. Keep a list of all ssdp canditates
this should be done here



*/

int main() {

	lssdp_ctx lssdp = {
		.port = 1900,
		.debug = true,           // debug

		// callback
		.packet_received_callback = show_ssdp_packet
	};

	lssdp_init(&lssdp);
	lssdp_set_log_callback(log_callback);
	lssdp.config.ADDR_LOCALHOST = "::1";
	lssdp.config.ADDR_MULTICAST = "FF02::C";



	strncpy(lssdp.header.location.prefix,"http://\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.location.domain,"test_location",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.location.suffix,":8082\0",LSSDP_FIELD_LEN);

	strncpy(lssdp.header.search_target,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.device_type,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.sm_id,"commend_switchbox\0",LSSDP_FIELD_LEN);
	strncpy(lssdp.header.unique_service_name,"Barcode",LSSDP_FIELD_LEN);
	lssdp_socket_create(&lssdp);
	lssdp_send_notify(&lssdp);

	// Main Loop
	for (;;) {
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(lssdp.sock, &fs);
		struct timeval tv = {
			.tv_usec = 500 * 1000   // 500 ms
		};

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
