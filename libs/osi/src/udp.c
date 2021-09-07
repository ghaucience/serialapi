#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "udp.h"


int udp_create(int port) {
	int				reuse = 1;
	struct sockaddr_in	sa;
	int				ret;
	int				fd = -1;

	ret = socket(AF_INET, SOCK_DGRAM, 0);
	if (ret < 0) {
		return -1;
	}
	fd = ret;

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &reuse, sizeof(reuse));

	sa.sin_family = AF_INET;
	sa.sin_port   = htons((short)port);
	sa.sin_addr.s_addr = INADDR_ANY;
	//inet_aton("192.168.1.200", &(sa.sin_addr));

	ret = bind(fd, (struct sockaddr *)&sa, sizeof(sa));
	if (ret < 0) {
		udp_destroy(fd);
		return -2;
	}
	return fd;
}


int udp_destroy(int fd) {
	if (fd > 0) {
		close(fd);
		fd = -1;
	}
	return 0;
}


int udp_send(int fd, char *_buf, unsigned int _size, char *_ip, int _port, int _s, int _u) {
	fd_set			fds;
	struct timeval	tv;
	int				ret;
	int				en;
	char            es[256];
	struct sockaddr_in sa;
	socklen_t		sl = sizeof(sa);


	if (_buf == NULL || _size <= 0 || _ip == NULL ||
			_s < 0 || _u < 0)  {
		return -1;
	}

send_select_tag:
	tv.tv_sec = _s;
	tv.tv_usec = _u;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	sa.sin_family		= AF_INET;
	sa.sin_port		= htons((short)_port);
	inet_aton(_ip, &(sa.sin_addr));

	ret = select(fd + 1, NULL, &fds, NULL, &tv);
	if (ret < 0) {
		en = errno;
		if (en == EAGAIN || en == EINTR) {
			goto send_select_tag;
		} else {
			strerror_r(en, es, sizeof(es));
			log_err("[Udp::send@select]:%d,%s\n", en, es);
			return -2;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
send_tag:
		ret = sendto(fd, _buf, _size, 0, (struct sockaddr *)&sa, sl);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto send_tag;
			} else {
				//perror("error");
				strerror_r(en, es, sizeof(es));
				log_err("[Udp::send@send]:%d,%s\n", en, es);	
				return -3;
			}
		} else if (ret != (int)_size) {
			log_err("[Udp::send@]:ret != size\n");	
			return -4;
		} else {
			;
		}
	} else {
		log_err("[Udp::send@]:unknow!\n");	
		return -5;
	}

	return ret;
}

int udp_recv(int fd, char *_buf, unsigned int _size, char *_ip, int *_port, int _s, int _u) {
	fd_set				fds;
	struct timeval		tv;
	int					ret;
	int					en;
	char                es[256];
	struct sockaddr_in	sa;
	socklen_t		sl = sizeof(sa);

	if (_buf == NULL || _size <= 0 ||
			_s < 0 || _u < 0)  {
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
			strerror_r(en, es, sizeof(es));
			log_err("[Udp::recv@select]:%d,%s\n", en, es);
			return -2;
		}
	} else if (ret == 0) {
		return 0;
	} else if (FD_ISSET(fd, &fds)) {
recv_tag:
		ret = recvfrom(fd, _buf, _size, 0, (struct sockaddr *)&sa, &sl);
		if (ret < 0) {
			en = errno;
			if (en == EAGAIN || en == EINTR) {
				goto recv_tag;
			} else {
				strerror_r(en, es, sizeof(es));
				log_err("[Udp::recv@recv]:%d,%s\n", en, es);
				return -3;
			}
		} else {
			if (ret == 0) {
				log_err("[Udp::recv@]:socket failed!\n");
				return -4; //remote close the socket
			} else {
				if (_ip != NULL) {
					strcpy(_ip, inet_ntoa(sa.sin_addr));
				}

				if (_port != NULL) {
					*_port = (int)ntohs(sa.sin_port);
				}
			}
		}
	} else {
		log_err("[Udp::recv@]:unknown\n");
		return -5;
	}
	return ret;
}




