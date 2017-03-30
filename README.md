# lssdp
light weight SSDP library

### What is SSDP

The Simple Service Discovery Protocol (SSDP) is a network protocol based on the Internet Protocol Suite for advertisement and discovery of network services and presence information.

====

### Support Platform

* Linux Ubuntu
* MAC OSX
* Android
* iOS

====

### How To Build and Test

```
make clean
make

cd test
./client.exe

on a other machine
./service.exe <identification_id>
```

====

#### lssdp_ctx:

lssdp context

**port** - SSDP UDP port, 1900 port is general.

**sock** - SSDP socket, created by `lssdp_socket_create`, and close by `lssdp_socket_close`

**debug** - SSDP debug mode, show debug message.

**header.search_target** - SSDP Search Target (ST). A potential search target.

**header.unique_service_name** - SSDP Unique Service Name (USN). A composite identifier for the advertisement.

**header.location = prefix + domain + suffix** - [http://] + IP + [:PORT/URI]

**header.sm_id** - Optional field.

**header.device_type** - Optional field.

**network_interface_changed_callback** - when interface is changed, this callback would be invoked.

**neighbor_list_changed_callback** - when neighbor list is changed, this callback would be invoked.

**packet_received_callback** - when received any SSDP packet, this callback would be invoked. It callback is usally used for debugging.

====

#### Function API (8)

##### lssdp_socket_create

create SSDP socket.

```
- SSDP port must be setup ready before call this function. (lssdp.port > 0)

- if SSDP socket is already exist (lssdp.sock > 0), the socket will be closed, and create a new one.

- SSDP neighbor list will be force clean up.
```

##### lssdp_socket_close

close SSDP socket.

```
- if SSDP socket <= 0, will be ignore, and lssdp.sock will be set -1.
- SSDP neighbor list will be force clean up.
```

##### lssdp_socket_read

read SSDP socket.

```
1. if read success, packet_received_callback will be invoked.

2. if received SSDP packet is match to Search Target (lssdp.header.search_target),
   - M-SEARCH: send RESPONSE back
   - NOTIFY/RESPONSE: add/update to SSDP neighbor list
```

```
- SSDP socket and port must be setup ready before call this function. (sock, port > 0)
- if SSDP neighbor list has been changed, neighbor_list_changed_callback will be invoked.
```
##### lssdp_send_byebye

##### lssdp_send_msearch

send SSDP M-SEARCH packet to multicast address

```
- SSDP port must be setup ready before call this function. (lssdp.port > 0)
```

##### lssdp_send_notify

send SSDP NOTIFY packet to multicast address

```
- SSDP port must be setup ready before call this function. (lssdp.port > 0)
```

##### lssdp_set_log_callback

setup SSDP log callback. All SSDP library log will be forward to here.
