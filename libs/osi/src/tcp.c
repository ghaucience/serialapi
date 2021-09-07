#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "tcp.h"

int		tcp_create(int type, const char *ip, int port) {
	int 				reuse = 1;
	struct sockaddr_in 	sa;
	int 				ret;
	int					fd = -1;

	ret = socket(AF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		return -1;
	}
	fd = ret;

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	//TODO: set more, if need

	if (type == TCP_CLIENT) {
		sa.sin_family = AF_INET;
		sa.sin_port   = htons((short)port);
		inet_aton(ip, &(sa.sin_addr));
		ret = connect(fd, (struct sockaddr *)&sa, sizeof(sa));
		if (ret < 0) {
			tcp_destroy(fd);
			return -2;
		}
	} else if (type == TCP_SERVER) {
		sa.sin_family = AF_INET;
		sa.sin_port   = htons((short)port);
		inet_aton(ip, &(sa.sin_addr));
		//sa.sin_addr.s_addr = INADDR_ANY;
		ret = bind(fd, (struct sockaddr *)&sa, sizeof(sa));
		if (ret < 0) {
			tcp_destroy(fd);
			return -3;
		}

		ret = listen(fd, 5);
		if (ret != 0) {
			tcp_destroy(fd);
			return -4;
		}
	}
	return fd;
}

void	tcp_destroy(int fd) {
	if (!(fd > 0)) {
		return;
	}
	struct linger {
		int   l_onoff;
		int    l_linger;
	};
	char buf[128];
	struct linger linger = { 1, 0}; 
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *) &linger, sizeof(linger));
	shutdown(fd, SHUT_RDWR);
	do { 
		int ret = recv(fd, buf, sizeof(buf), 0);
		if (ret == 0)  break;
		if (ret == -1) {	
			if (errno == EAGAIN) break;
			if (errno == EWOULDBLOCK) break;
			if (errno == 9) break;
		}
	} while (1);
	close(fd);
	fd = -1;
}
int		tcp_recv(int fd, char *_buf, unsigned int _size, int _s, int _u) {
	fd_set	fds;
	struct timeval	tv;
	int ret;
	int en;
	char es[256];

	if (_buf == NULL || _size <= 0 || _s < 0 || _u < 0 || fd <= 0)  {
		return -1;
	}
recv_select_tag:
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = _s;
	tv.tv_usec = _u;
	ret = select(fd + 1, &fds, NULL, NULL, &tv);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto recv_select_tag;
		} else {
			int x = strerror_r(en, es, sizeof(es));
			x = x;
			log_err("[Tcp::recv@select]:%d,%s\n", en, es);
			return -2;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
recv_tag:
		ret = recv(fd, _buf, _size, 0);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto recv_tag;
			} else {
				int x = strerror_r(en, es, sizeof(es));
				x = x;
				log_err("[Tcp::recv@recv]:%d,%s\n", en, es);
				return -3;
			}
		} else {
			if (ret == 0) {
				log_err("[Tcp::recv@]:remote socket closed!\n");
				return -4; //remote close the socket
			} else {
				;
			}
		}
	} else {
		log_err("[Tcp::recv@]:unknown\n");
		return -5;
	}
	return ret;

}

int		tcp_send(int fd, char *_buf, unsigned int _size, int _s, int _u) {
	fd_set	fds;
	struct timeval tv;
	int ret;
	int en;
	char es[256];
	int try_count = 5;


	if (_buf == NULL || _size <= 0 || _s < 0 || _u < 0 || fd <= 0)  {
		return -1;
	}

send_select_tag:
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = _s;
	tv.tv_usec = _u;
	ret = select(fd + 1, NULL, &fds, NULL, &tv);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto send_select_tag;
		} else {
			int x = strerror_r(en, es, sizeof(es));
			x = x;
			log_err("[Tcp::send@select]:%d,%s\n", en, es);	
			return -2;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
send_tag:
		ret = send(fd, _buf, _size, 0);
		if (ret < 0) {
			en = errno;
			if ((en == EAGAIN || en == EINTR) && (try_count-- >= 0)) {
				usleep(10);
				goto send_tag;
			} else {
				int x = strerror_r(en, es, sizeof(es));
				x = x;
				log_err("[Tcp::send@send]:%d,%s\n", en, es);	
				return -3;
			}
		} else if (ret != (int)_size) {
			log_err("[Tcp::send@]:ret != size\n");	
			return -4;
		} else {
			;
		}
	} else {
		log_err("[Tcp::send@]:unknow!\n");	
		return -5;
	}
	return ret;
}


int		tcp_accept(int fd, int _s, int _u) {
	fd_set	fds;
	struct timeval	tv;
	int ret;
	struct sockaddr_in sa;
	socklen_t		sl = sizeof(sa);
	int en;
	char es[256];

accept_select_tag:
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = _s;
	tv.tv_usec = _u;

	ret = select(fd + 1, &fds, NULL, NULL, &tv);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto accept_select_tag;
		} else {
			int x = strerror_r(en, es, sizeof(es));
			x = x;
			log_err("[Tcp::accept@select]:%d,%s\n", en, es);	
			return -1;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
accept_tag:
		ret = accept(fd, (struct sockaddr *)&sa, &sl);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto accept_tag;
			} else {
				int x = strerror_r(en, es, sizeof(es));
				x = x;
				log_err("[Tcp::accept@accept]:%d,%s\n", en, es);	
				return -2;
			}
		} else{
			;
		}
	} else {
		log_err("[Tcp::accept@]:unknown!\n");	
		return -3;
	}
	return ret;
}
