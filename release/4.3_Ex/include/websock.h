#ifndef __WEBSOCK_H_
#define __WEBSOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

	int websock_post(char *url, char *request, char *reponse);
	int websock_listen(char *url, int (*cmd_in)(char *url, char *command), int holdtime);
	int websock_stop_listen();

#ifdef __cplusplus
}
#endif

#endif
