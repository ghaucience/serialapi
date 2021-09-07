/* © 2014 Silicon Laboratories Inc.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <termios.h>

#include "port.h"

static int fd;

static char port[128] = {0};

#ifdef LIBDEBUG
#define LIBSERIAL_PRINTF printf
#else
#define LIBSERIAL_PRINTF(...) 
#endif

int SerialAPI_GetFd() {
	return fd;
}

int SerialInit(const char* _port) {
		strcpy(port, _port);
	
    struct termios tp;
    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd == -1) {
            return 0;
    }
    // attributes
    tcgetattr(fd, &tp);
    tp.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;
    tp.c_iflag = IGNBRK | IGNPAR;
    tp.c_oflag = 0;
    tp.c_lflag = 0;

    cfsetspeed(&tp, 115200);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSAFLUSH, &tp);
    return fd;
}

int SerialReOpen() {
	return SerialInit(port);

}

void SerialClose() {
  close(fd);
  return;
}

int SerialGetByte() {
#if 0
	unsigned char c;
	int n;
	//LIBSERIAL_PRINTF("read a ch...\n");
	n = read(fd,&c,1);
	//LIBSERIAL_PRINTF("read a ch over...\n");
	//LIBSERIAL_PRINTF("R %x\n", (unsigned char)c);
	if(n < 0) return n;
	
	//LIBSERIAL_PRINTF("receive: [%02X]\n", c&0xff);
	return c;
#else
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 5000;

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if (retval < 0) {
			SerialReOpen();
			return 0;
		} else if (retval == 0){
			return 0;
		} else {
			unsigned char c;
			int n = read(fd,&c,1);
			if (n < 0) return n;
			return c;
		}
 
#endif
}

void SerialPutByte(unsigned char c) {
	int n;
	n=EAGAIN;
	while(n==EAGAIN) {
//		LIBSERIAL_PRINTF("W %x\n", (unsigned char)c);
		n = write(fd,&c,1);
	}
}

int SerialCheck() {
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */
    if (retval == -1) {
        perror("select()");
		return 0;
    } else if (retval>0) {
        return 1;
    } else {
    	return 0;
    }
}

void SerialFlush() {
	tcdrain(fd);
}

int SerialGetBuffer(unsigned char* c, int len) {
	LIBSERIAL_PRINTF("%s..., len:%08X\n", __func__, len);
	int i = 0;
	unsigned char *p = c;
	int errcnt = 0;
	for (i = 0; i < len; i++) {
		//LIBSERIAL_PRINTF("i:%d\n", i);
		int ch= SerialGetByte();
		//LIBSERIAL_PRINTF("ch is %02X\n", ch&0xff);
		if (ch < 0) {
			#warning -----------------------------------------------------
			errcnt++;
			LIBSERIAL_PRINTF("serial read failed, errcnt:%d\n", errcnt);
			if (errcnt > 200) {
				LIBSERIAL_PRINTF("%s failed\n", __func__);
				return -1;
			}
			//continue;
			break;
		}
		*p = ch;
		p++;
	}
	LIBSERIAL_PRINTF("%s ok\n", __func__);
	return 0;
}
void SerialPutBuffer(unsigned char* c, int len) {
	int i = 0;
	LIBSERIAL_PRINTF("[SEND]: ");
	for (i = 0; i < len; i++) {
		LIBSERIAL_PRINTF("[%02X] ", (*c)&0xff);
		SerialPutByte(*c);
		c++;
	}
	LIBSERIAL_PRINTF("\n");
	return;
}

void xtimer_set(struct timer *t, int interval) {
  t->interval = interval;
  xtimer_restart(t);
}

void xtimer_restart(struct timer *t){
  struct timeval now;
  struct timeval d;
  gettimeofday(&now,0);
  d.tv_sec = (t->interval / 1000);
  d.tv_usec = (t->interval % 1000) *1000;
  timeradd(&d, &now, &(t->timeout));
}

int xtimer_expired(struct timer *t){
  struct timeval now;
  gettimeofday(&now,0);
  return (timercmp(&t->timeout, &now, <));
}

