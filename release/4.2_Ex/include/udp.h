#ifndef __UDP_H
#define __UDP_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* return < 0 error, else return fd */
int udp_create(int port);

/* close udp socket */
int udp_destroy(int fd);

/* return < 0 error, else the send data len */ 
int udp_send(int fd, char *_buf, unsigned int _size, char *_ip, int _port, int _s, int _u);

/* return < 0 error, else the recv data len */
int udp_recv(int fd, char *_buf, unsigned int _size, char *_ip, int *_port, int _s, int _u);



#ifdef __cplusplus
}
#endif

#endif
