#ifndef __UTIL_H_
#define __UTIL_H_


#ifdef __cplusplus 
extern "C" {
#endif


char * get_exe_path( char * buf, int count);
void view_buf(char *buf, int len);
long current_system_time_us();



#ifdef __cplusplus
}
#endif

#endif
