#ifndef __TCP_H
#define __TCP_H

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

enum {
	TCP_SERVER = 0,
	TCP_CLIENT = 1,
	TCP_PROXY  = 2,
};


/* return < 0 error, else return fd */
int		tcp_create(int _type, const char *ip, int port);

/* close socket */
void	tcp_destroy(int fd);

/* return < 0 error, else the recv data len */
int		tcp_recv(int fd, char *_buf, unsigned int _size, int _s, int _u);

/* return < 0 error, else the send data len */
int		tcp_send(int fd, char *_buf, unsigned int _size, int _s, int _u);

/* return < 0 error, else the client fd */
int		tcp_accept(int fd, int _s, int _u);
#ifdef __cplusplus
}
#endif

#endif
