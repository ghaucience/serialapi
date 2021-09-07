#ifndef __ZWAVE_H_
#define __ZWAVE_H_

int zwave_start(int argc, char *argv[]);
int zwave_include();
int zwave_exclude();
int zwave_ep_class_cmd(unsigned char nid, int ep,  unsigned char class, unsigned char cmd, char *data, int len, void (*completed)(unsigned char x, unsigned char *status));
int zwave_class_cmd(unsigned char nid, unsigned char class, unsigned char cmd, char *data, int len,void (*completed)(unsigned char x, unsigned char *status));
int zwave_version(char *version);
int zwave_info(int *homeid, int *nodeid);
int zwave_remove_failed_node(char *mac);
#endif


