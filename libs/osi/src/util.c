#include "util.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>


char * get_exe_path( char * buf, int count){
  int i;
  int rslt = readlink("/proc/self/exe", buf, count - 1);/////注意这里使用的是self
  if (rslt < 0 || (rslt >= count - 1))
  {
    return NULL;
  }
  buf[rslt] = '\0';
  for (i = rslt; i >= 0; i--)
  {
    //printf("buf[%d] %c\n", i, buf[i]);
    if (buf[i] == '/')
    {
      buf[i + 1] = '\0';
      break;
    }
  }
  return buf;
}


void view_buf(char *buf, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    printf("[%02X] ", buf[i]&0xff);
    if ( (i + 1) % 20 == 0) {
      printf("\n");
    }
  }
}

long current_system_time_us() {
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

