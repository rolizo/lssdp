#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday
#include <list>
#include <algorithm>
#include <thread>
#include <iostream>
#include <curses.h>

#include "lssdp.h"
#include "client.h"

using namespace std;
char in_buf[255];
char out_buf[255];

void print_devices();
void render_screen();
int ping_device(struct device*target);
int add_device(lssdp_packet &packet);
int remove_device(std::string barcode);
void update_device_timestamp(std::string barcode);
void remove_old_devices();
device* find_device(t_device_list *container, std::string barcode);

void log_callback(const char * file, const char * tag, int level, int line,
                  const char * func, const char * message) {
    std::string level_name = "DEBUG";
    if (level == LSSDP_LOG_INFO)   level_name = "INFO";
    if (level == LSSDP_LOG_WARN)   level_name = "WARN";
    if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";

//	printf("[%-5s][%s] %s", level_name.c_str(), tag, message);
}

long long get_current_time() {
    struct timeval time = {};
    if (gettimeofday(&time, NULL) == -1) {
        //printf("gettimeofday failed, errno = %s (%d)\n", strerror(errno), errno);
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

    //print_lssdp_packet(parsed_packet);

    if(strcmp("ssdp:alive",parsed_packet.nts) == 0) {
        add_device(parsed_packet);
        update_device_timestamp(parsed_packet.usn);
    } else if(strcmp("ssdp:byebye",parsed_packet.nts) == 0) {
        std::string check_barcode = parsed_packet.usn;
        remove_device(check_barcode);
    } else if (strcmp("ssdp:all",parsed_packet.nts) == 0) {
    } else if (strcmp( parsed_packet.method,"RESPONSE") == 0) {
        add_device(parsed_packet);
    } else 	{
        return -1;
        //printf("UNSUPPORTED PACKET\n\n");
    }

    return 0;
}


void  update_device_timestamp(std::string barcode) {

    device*match = find_device(&device_list,barcode);

    long long timestamp = get_current_time();
    if (match) {
        match->update_time = timestamp;
        render_screen();
    }
}


//returns 0 if device was removed
//returns -1 if no device was removed
int remove_device(std::string barcode) {

    device*match = find_device(&device_list,barcode);

    if (match) {
        device_list.remove(match);
        render_screen();
        return 0;
    }

    return -1;

}


//Find device in device list with matching barcode
//Returns Null if not found
device* find_device(t_device_list *container, std::string barcode) {

    t_device_list::iterator iter;
    for(iter = container->begin(); iter != container->end(); iter++) {
        if((*iter)->barcode == barcode) {
            return (*iter);
        }
    }
    return NULL;
}


bool is_device_unique(t_device_list *container,std::string barcode) {

    if(find_device(container,barcode))
        return false;
    return true;
}

int add_device(lssdp_packet &packet) {

    std::string check_barcode = packet.usn;
    if(!is_device_unique(&device_list,check_barcode)) {
        return -1;
    }

    device *ptr = new device();
    ptr->state=UNREACHABLE;
    ptr->barcode=packet.usn;
    ptr->update_time=packet.update_time;
    device_list.push_back(ptr);

    render_screen();
    return 0;
}


int ping_device(struct device*target) {


    return 0;
}

void ping_devices() {

    t_device_list::iterator iter;
    for(iter = device_list.begin(); iter != device_list.end(); iter++) {
        ping_device(*iter);
    }

}



void print_paired_devices() {

    int count = 0;
    t_device_list::iterator iter;
    for(iter = paired_device_list.begin(); iter != paired_device_list.end(); iter++) {
        count++;
        std::string status;
        if( (*iter)->state ==  PAIRED) {
            status = "PAIRED";
        } else {
            status = "UNREACHABLE";
        }
        printw("%d: %s %s\n" ,count ,(*iter)->barcode.c_str()  ,status.c_str()) ;
    }
}

struct device * get_device(t_device_list *container ,unsigned int index) {

    if (container->size() > index)
    {
        t_device_list::iterator it = container->begin();
        std::advance(it, index);
        return *it;    // 'it' points to the element at index 'N'
    }


    return NULL;

}


void print_devices() {

    int count = 0;
    t_device_list::iterator iter;
    for(iter = device_list.begin(); iter != device_list.end(); iter++) {
        count++;
        int age = get_current_time() - (*iter)->update_time;
        printw("%d: %s Age: %d\n" ,count ,(*iter)->barcode.c_str(), age/1000) ;
    }
}

void render_screen() {

    clear();
    printw("--Avaliable Device List---\n");
    print_devices();
    printw("-- End Device List---\n");


    printw("\n\n\n");

    printw("--Paired Device List---\n");
    print_paired_devices();
    printw("-- End Device List---\n");
    //print other list

    printw("Last command: %s\n", out_buf);
    refresh();			/* Print it on to the real screen */
}


void connect_device(struct device *ptr)
{
    if(is_device_unique(&paired_device_list,ptr->barcode)) {
        paired_device_list.push_back(ptr);
    }
}

void disconnect_device(struct device *ptr) {

    if(!ptr)
        return;

    t_device_list::iterator iter;
    for(iter = paired_device_list.begin(); iter != paired_device_list.end();
            iter++) {
        if((*iter)->barcode == ptr->barcode) {
            paired_device_list.remove(*iter);
            return;
        }
    }
}

void remove_old_devices()
{
    t_device_list::iterator iter;
    for(iter = device_list.begin(); iter != device_list.end();) {
        int age = (get_current_time() - (*iter)->update_time)/1000;
        if(age > ALIVE_TIMEOUT) {
            iter = device_list.erase(iter);
            continue;
        }
        iter++;
    }

}
void mainLoop() {

    lssdp_ctx lssdp;
    lssdp.port = 1900;
    lssdp.debug = false;
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

    for (;;) {
        fd_set fs;
        FD_ZERO(&fs);
        FD_SET(lssdp.sock, &fs);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500 * 1000;   // 500 ms

        remove_old_devices();
        render_screen();

        int ret = select(lssdp.sock + 1, &fs, NULL, NULL, &tv);
        if (ret < 0) {
            //printf("select error, ret = %d\n", ret);
            break;
        }

        if (ret > 0) {
            lssdp_socket_read(&lssdp);
        }
    }
}



int main() {


    initscr();			/* Start curses mode 		  */
    clear();
    //leaveok(stdscr,true);
    std::thread t_MainThread(mainLoop);
    while(true) {
        std::string input;
        getstr(in_buf);
        char*pos;
        if((pos = strstr(in_buf,"add")) != 0) {
            struct device *tmp = get_device(&device_list, atoi(pos+3)-1);
            if (tmp) {
                connect_device(tmp);
                snprintf(out_buf,255,"Added device %d",atoi(pos+3));
            }
            else {
                snprintf(out_buf,255,"Unknown device number");
            }
        } else if ((pos = strstr(in_buf,"rm")) != 0) {
            struct device *tmp = get_device(&paired_device_list, atoi(pos+2)-1);
            if(tmp) {
                disconnect_device(tmp);
                snprintf(out_buf,255,"Removed device %d",atoi(pos+2));
            }
            else {
                snprintf(out_buf,255,"Unknown device number");
            }
        } else {
            snprintf(out_buf,255,"Unknown command");
        }
        render_screen();
    }

    t_MainThread.join();

    // Main Loop

    endwin();

    return EXIT_SUCCESS;
}
