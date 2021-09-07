
#include <curl/curl.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "websock.h"
#include "log.h"


#if 1
#define websock_log_info	log_info
#define websock_log_warn	log_warn
#define websock_log_debug	log_debug
#else
#define websock_log_info	printf
#define websock_log_warn	printf
#define websock_log_debug	printf
#endif

typedef struct stWebSocketEnv {
	int		listen_flag;
	char	url[1024];
	int		holdtime;
	pthread_t tid;

	pthread_t monitor_tid;
	CURL *cmd_curl;
	int		cmd_count;

	int   run_flag;

	int		(*callback)(char *url, char *command);
} stWebSocketEnv_t;

static stWebSocketEnv_t wse = {0};


static size_t websock_thread_func_writedata(void *buffer, size_t size, size_t nmemb, 
		void *userp) {
	char * buf = (char * ) userp;
	int len = size * nmemb;
	if (len >= 2048) {
		len = 2048;
	}

	memcpy(buf, buffer, len);
	buf[len] = 0;

	if (buf[0] == '0') {
		int len = strlen(buf);
		if (buf[len-1] == '\n') {
			buf[len-1] = 0;
		}
		log_debug("server heart->[%s]", buf);
		wse.cmd_count = 0;
		return size * nmemb;
	}

	websock_log_info("RecvCmd: %s", buf);
	if (wse.callback != NULL) {
		wse.callback(wse.url, buf);
	}

	return size * nmemb;
}

void * websock_monitor_thread_func() {
	while(1){
		usleep(100 * 1000);
		wse.cmd_count += 100;

		if (wse.cmd_count >  2 * 60 * 1000) {
			if (wse.cmd_curl != NULL) {
				CURL *c = wse.cmd_curl;
				wse.cmd_curl = NULL;
				curl_easy_cleanup(c);
			}
			wse.cmd_count = 0;
		}

		if (wse.cmd_curl == NULL) {
			websock_log_info("Command Monitor thread Exit");
			return NULL;
		}
	}

	return NULL;
}


static void *websock_thread_func(void *arg) {
	while (wse.run_flag) {

		wse.cmd_curl = curl_easy_init();
		if (wse.cmd_curl == NULL) {
			websock_log_warn("Can't Create Curl Client!,Sleep and continue");
			sleep(2);
			continue;
		}

		websock_log_info("CMD URL=%s", wse.url);

		curl_easy_setopt(wse.cmd_curl, CURLOPT_URL, wse.url);
		curl_easy_setopt(wse.cmd_curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(wse.cmd_curl, CURLOPT_WRITEFUNCTION, websock_thread_func_writedata);

		char  response_data[2048];
		response_data[0] = '\0';
		curl_easy_setopt(wse.cmd_curl, CURLOPT_WRITEDATA, response_data);

		curl_easy_setopt(wse.cmd_curl, CURLOPT_CONNECTTIMEOUT, 15L);
		curl_easy_setopt(wse.cmd_curl, CURLOPT_NOSIGNAL, 1L);

		pthread_create(&wse.monitor_tid, NULL, websock_monitor_thread_func, NULL);
		CURLcode res = curl_easy_perform(wse.cmd_curl);  // TODO command link error process.
		if (wse.cmd_curl != NULL) {
			CURL *c = wse.cmd_curl;
			wse.cmd_curl = NULL;

			curl_easy_cleanup(c);
		}
		

		void * status = NULL;
		pthread_join(wse.monitor_tid, &status);

		websock_log_info("Connect disconnected: %d", res);
	}

	return NULL;
}

int websock_listen(char *url, int (*cmd_in)(char *url, char *command), int holdtime) {
	if (wse.listen_flag != 0) {
		return 0;
	}
	
	wse.listen_flag = 1;
	strcpy(wse.url, url);
	wse.holdtime = holdtime;
	wse.tid = 0;
	wse.run_flag = 1;
	wse.callback = cmd_in;

	wse.cmd_count = 0;
	wse.cmd_curl = NULL;
	wse.monitor_tid = 0;

	pthread_create(&wse.tid, NULL, websock_thread_func, NULL);

	return 0;
}


int websock_stop_listen() {
#if 1
	wse.listen_flag = 0;
	wse.cmd_count = 0;
#else
	wse.listen_flag = 0;
	if (wse.cmd_curl != NULL) {
		CURL *c = wse.cmd_curl;
		wse.cmd_curl = NULL;
		curl_easy_cleanup(c);
	}
	usleep(200);
#endif

	return 0;
}


static size_t curl_post_write_data(void *buffer, size_t size, size_t nmemb,void *userp){
	char * buf = (char * ) userp;

	int len = size * nmemb;
	if (len >= 2048) {
		len = 2048;
	}

	memcpy(buf, buffer, len);
	buf[len] = 0;

	websock_log_debug("RECV:%s", buf);

	return size * nmemb;
}

int websock_post(char *url, char *request, char *response) {
	log_info("url : %s, request:%s", url, request);

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		websock_log_warn("Can't Create Curl Client!");
		return -1;
	}

	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request) + 1 );
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_post_write_data);


	char  response_data[2048];
	response_data[0] = '\0';
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if(res != CURLE_OK) {
		websock_log_warn("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return -2;
	}

	strcpy(response, response_data);

	return 0;
}

