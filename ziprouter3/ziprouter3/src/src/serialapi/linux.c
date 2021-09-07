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

int SerialInit(const char* port) {
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

void SerialClose() {
  close(fd);
  return;
}

int SerialGetByte() {
	unsigned char c;
	int n;
	n = read(fd,&c,1);
//	printf("R %x\n", (unsigned char)c);
	if(n < 0) return n;
	return c;
}

void SerialPutByte(unsigned char c) {
	int n;
	n=EAGAIN;
	while(n==EAGAIN) {
//		printf("W %x\n", (unsigned char)c);
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



void timer_set(struct timer *t, int interval) {
  t->interval = interval;
  timer_restart(t);
}

void timer_restart(struct timer *t){
  struct timeval now;
  struct timeval d;
  gettimeofday(&now,0);
  d.tv_sec = (t->interval / 1000);
  d.tv_usec = (t->interval % 1000) *1000;
  timeradd(&d, &now, &(t->timeout));
}

int timer_expired(struct timer *t){
  struct timeval now;
  gettimeofday(&now,0);
  return (timercmp(&t->timeout, &now, <));
}

