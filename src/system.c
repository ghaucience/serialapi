#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<net/if.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <regex.h>
#include <time.h>


#include "schedule.h"
#include "log.h"

int system_cmd(char *cmd, char *out) {
	FILE *fp = popen(cmd, "r");
	if (fp == NULL) {
		out[0] = 0;
		return -1;
	}

	char buf[256];
	if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
		pclose(fp);
		return -2;
	}
	pclose(fp);

	int len = strlen(buf);
	if (buf[len-1] == 0x0A) {
		buf[len-1] = 0;
		len--;
	} 

	strcpy(out, buf);
	return 0;
}


int system_get_lan_ip(char *ip, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.lan status | grep \"address\" | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}'", buf);
	if (ret != 0) {
		ip[0] = 0;
		return 0;
	}
	strcpy(ip, buf);
	return 0;
}

int system_get_lan_mask(char *mask, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.lan status | grep \"mask\" | grep -oE '[0-9]{1,2}' | head -n 1", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	if (strcmp(buf, "24") == 0) {
		strcpy(mask, "255.255.255.0");
	} else if (strcmp(buf, "8") == 0) {
		strcpy(mask, "255.0.0.0");
	} else if (strcmp(buf, "16") == 0) {
		strcpy(mask, "255.255.0.0");
	} else if (strcmp(buf, "32") == 0)  {
		strcpy(mask, "255.255.255.255");
	} else {
	}

	return 0;
}

int system_get_wan_ip(char *ip, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.wan status | grep \"address\" | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}'", buf);
	if (ret != 0) {
		ip[0] = 0;
		return 0;
	}
	strcpy(ip, buf);
	return 0;
}

int system_get_wan_gateway(char *gw, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.wan status |grep nexthop | tail -n 1 | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}'", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(gw, buf);
	return 0;
}

int system_get_wan_mask(char *mask, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.lan status | grep \"mask\" | grep -oE '[0-9]{1,2}' | head -n 1", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	if (strcmp(buf, "24") == 0) {
		strcpy(mask, "255.255.255.0");
	} else if (strcmp(buf, "8") == 0) {
		strcpy(mask, "255.0.0.0");
	} else if (strcmp(buf, "16") == 0) {
		strcpy(mask, "255.255.0.0");
	} else if (strcmp(buf, "32") == 0)  {
		strcpy(mask, "255.255.255.255");
	} else {
	}

	return 0;
}

int system_get_wan_dns1(char *dns1, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.wan status | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}' | tail -n 2 | head -n 1", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(dns1, buf);
	return 0;
}

int system_get_wan_dns2(char *dns2, int size) {
	char buf[256];
	int ret = system_cmd((char *)"ubus call network.interface.wan status | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}' | tail -n 2 | tail -n 1", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(dns2, buf);
	return 0;
}


int system_get_wifi_onoff(int *onoff) {
	char buf[256];
	//int ret = system_cmd((char *)"ifconfig wlan0 | grep MTU | wc -l", buf);
	int ret = system_cmd((char *)"ifconfig wlan0 | wc -l", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	if (strcmp(buf, "1") == 0) {
		*onoff = 0;
	} else {
		*onoff = 1;
	}

	return 0;
}

int system_get_wifi_mode(char *mode, int size) {
	//uci get wireless.@wifi-iface[0].mode
	
	char buf[256];
	int ret = system_cmd((char *)"uci get wireless.@wifi-iface[0].mode", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(mode, buf);

	return 0;

}

int system_get_wifi_ssid(char *ssid, int size) {

	char buf[256];
	int ret = system_cmd((char *)"uci get wireless.@wifi-iface[0].ssid", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(ssid, buf);

	return 0;
}

int system_get_wifi_encryption(char *encryption, int size) {

	char buf[256];
	int ret = system_cmd((char *)"uci get wireless.@wifi-iface[0].encryption", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(encryption, buf);


	return 0;
}

int system_get_wifi_key(char *key, int size) {

	char buf[256];
	memset(buf, 0, sizeof(buf));
	int ret = system_cmd((char *)"uci get wireless.@wifi-iface[0].key", buf);
	if (ret != 0) {
		key[0] = 0;
		return 0;
	}
	char encryption[64];
	system_get_wifi_encryption(encryption, sizeof(encryption));
	if (strcmp(encryption, "none") == 0) {
		strcpy(key, "none");
	} else {
		strcpy(key, buf);
	}

	return 0;
}

int system_set_wifi(int onoff, char *mode, char *ssid, char *encryption, char *key) {
	printf("[%s] setting wifi\n", __func__);
	if (onoff == 0) {
		printf("[%s] setting wifi off\n", __func__);
		char buf[256];
		system_cmd("uci set wireless.@wifi-device[0].disabled=1;uci commit wireless;", buf);
		system_cmd("wifi down;", buf);
		system_cmd("uci set network.wan.ifname=\'eth0.2\';uci commit network;", buf);
	} else if (onoff == 1) {
		printf("[%s] setting wifi on\n", __func__);
		if (strlen(key) < 8) {
			return -1;
		}
		if (strlen(ssid) < 5 || strlen(ssid) > 32) {
			return -2;
		}
		if (strcmp(encryption, "psk2") != 0 && strcmp(encryption, "psk2+tkip") != 0) {
			return -3;
		}
		if (strcmp(mode, "ap") == 0) {
			char cmd[256];
			char buf[256];
			system_cmd("uci set wireless.@wifi-iface[0].device=radio0", buf);
			system_cmd("uci set wireless.@wifi-iface[0].network=lan", buf);
			system_cmd("uci set wireless.@wifi-iface[0].mode=ap", buf);

			sprintf(cmd, "uci set wireless.@wifi-iface[0].ssid=%s", ssid);
			system_cmd(cmd, buf);

			if (strcmp(key, "none") == 0) {
				system_cmd("uci set wireless.@wifi-iface[0].encryption=none", buf);
			} else {
				sprintf(cmd, "uci set wireless.@wifi-iface[0].encryption=%s", encryption);
				system_cmd(cmd, buf);
				sprintf(cmd, "uci set wireless.@wifi-iface[0].key=%s", key);
				system_cmd(cmd, buf);
			}
			system_cmd("uci set network.wan.ifname=\'eth0.2\';uci commit network;", buf);
			system_cmd("uci set wireless.@wifi-device[0].disabled=0;", buf);
			system_cmd("uci commit wireless; wifi;", buf);
		} else if (strcmp(mode, "sta") == 0) {
			char wanip[32];
			memset(wanip, 0, sizeof(wanip));
			system_get_wan_ip(wanip, sizeof(wanip));
			if (strlen(wanip) >= 10) {
				printf("wan has connectted(%s), don't use wifi sta!\n", wanip);
				return -4;
			}
	
			char cmd[256];
			char buf[256];
			system_cmd("uci set wireless.@wifi-iface[0].device=radio0", buf);
			system_cmd("uci set wireless.@wifi-iface[0].network=wan1", buf);
			system_cmd("uci set wireless.@wifi-iface[0].mode=sta", buf);

			sprintf(cmd, "uci set wireless.@wifi-iface[0].ssid=%s", ssid);
			system_cmd(cmd, buf);

			if (strcmp(key, "none") == 0) {
				system_cmd("uci set wireless.@wifi-iface[0].encryption=none", buf);
			} else {
				sprintf(cmd, "uci set wireless.@wifi-iface[0].encryption=%s", encryption);
				system_cmd(cmd, buf);
				sprintf(cmd, "uci set wireless.@wifi-iface[0].key=%s", key);
				system_cmd(cmd, buf);
			}
			system_cmd("uci set network.wan1=interface;uci set network.wan1.proto=dhcp;uci commit network;", buf);
			system_cmd("uci set wireless.@wifi-device[0].disabled=0;", buf);
			system_cmd("uci commit wireless; wifi;", buf);
		}
	}
	return 0;
}

int system_get_4g_onoff(int *onoff) {
	/* lsusb  | grep 2c7c:0296 | wc -l */
	char buf[256];
	int ret = system_cmd((char *)"lsusb  | grep 2c7c:0296 | wc -l", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	if (strcmp(buf, "1") == 0) {
		*onoff = 1;
	} else {
		*onoff = 0;
	}
	return 0;
}

int system_get_4g_device(char *device, int size) {
	/* find /sys/bus/usb/devices/1-1.4:1.3/ -name "ttyUSB*" -maxdepth 1 */
	

	char buf[256];
	int ret = system_cmd((char *)"basename `find /sys/bus/usb/devices/1-1.4:1.3/ -name \"ttyUSB*\" -maxdepth 1`", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}

	strcpy(device, buf);
	return 0;
}

int system_get_4g_simcard(int *simcard) {
	*simcard = 0;
	return 0;
}

int system_get_4g_ip(char *ip, int size) {
	/* ifconfig eth0.2 | grep "inet addr:" | xargs | cut -d " " -f 2 | cut -d ":" -f 2 */

	char buf[256];
	memset(buf, 0, sizeof(buf));
	int ret = system_cmd((char *)"ifconfig ppp0 | grep \"inet addr:\" | xargs | cut -d \" \" -f 2 | cut -d \":\" -f 2", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}

	strcpy(ip, buf);

	return 0;
}


int system_get_ble_onoff(int *onoff) {
	/* lsusb  | grep 0a12 | grep 0001 | wc -l */
	char buf[256];
	int ret = system_cmd((char *)"lsusb  | grep 0a12 | grep 0001 | wc -l", buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	if (strcmp(buf, "1") == 0) {
		*onoff = 1;
	} else {
		*onoff = 0;
	}
	return 0;
}

int system_get_ble_mac(char *mac, int size) {
	/* hciconfig | grep "BD Address" | xargs | cut -d " " -f 3 */
	char *cmd = "hciconfig | grep \"BD Address\" | xargs | cut -d \" \" -f 3";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(mac, buf);

	return 0;
}

int system_get_ble_scan(int *scan) {
	/* TODO */
	*scan = 0;
	return 0;
}

int system_get_ble_pair(int *pair) {
	/* TODO */
	*pair = 0;
	return 0;
}

int system_set_ble_scan(int scan) {
	/* TODO */
	return 0;
}

int system_set_ble_enpair(int enpair) {
	/* TODO */
	return 0;
}

int system_get_ble_list() {
	/* TODO */
	return 0;
}

int system_set_ble_remove(char *mac) {
	/* TODO */
	return 0;
}

int system_get_system_version(char *version, int size) {
	char *cmd = "cat /etc/dusun_build | grep VERSION | cut -d \"=\" -f 2";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(version, buf);


	return 0;
}

int system_get_model(char *model, int size) {
	char *cmd = "cat /tmp/sysinfo/model";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(model, buf);


	return 0;
}

static void _system_model_get(char *model, int size) {
	int ret = system_cmd("cat /tmp/sysinfo/model", model);
	if (ret != 0) {
		strcpy(model, "UNKNOWN");
		return;
	}
	return;
}
const char *system_model_get() {
	static char model[64] = "UNKNOWN";
	if (strcmp(model, "UNKNOWN") == 0) {
		_system_model_get(model, sizeof(model));
	}
	//AMBER_LOG_DEBUG("model is %s\n", model);
	return (const char *)model;
}

int system_get_mac(char *mac, int size) {

#if 0
	//char *cmd = "ifconfig eth0.1  | grep HWaddr | xargs | cut -d \" \" -f 5";
	char *cmd = "ifconfig br-lan  | grep HWaddr | xargs | cut -d \" \" -f 5";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0 || strlen(buf) != 17) {
		cmd = "ifconfig eth0  | grep HWaddr | xargs | cut -d \" \" -f 5";
		int ret = system_cmd(cmd, buf);
		if (ret != 0 || strlen(buf) != 17) {
			buf[0] = 0;
			return -1;
		}
	}
	strcpy(mac, buf);
	{
		int i = 0; 
		for (i = 0; i < strlen(mac); i++) {
			if (mac[i] >= 'A' && mac[i] <= 'F') {
				mac[i] = mac[i] - 'A' + 'a';
			}
		}
	}
#else
	static char __mac[32] = {0};
	if (__mac[0] == 0) {
		char *cmd = "cat /sys/class/net/eth0/address";
		char buf[256];
		system_cmd(cmd, buf);
		strcpy(__mac, buf);
	} 
	strcpy(mac, __mac);
#endif

	return 0;
}

int system_get_coord_mac(char *mac, int size) {
	//root@Hiwifi:/tmp/cryptdata/zgateway/roombanker# cat /etc/config/dusun/amber/netinfo 
	//channel:13, panid:0x85A9, extaddr:0xBAEC763BC4532C16
	
	static char cmac[48] = {0};
	if (cmac[0] != 0) {
		strcpy(mac, cmac);
		return 0;
	}
		
	FILE *fp = fopen("/etc/config/dusun/amber/netinfo", "r");
	if (fp == NULL) {
		//log_warn("can't find netinfo");
		return -1;
	}
	char buf[256];
	char *p = fgets(buf, sizeof(buf), fp);
	if (p == NULL) {
		//log_warn("read error !");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	char *p1 = strstr(buf, "extaddr:");
	if (p1 == NULL) {
		//log_warn("parse error!");
		return -1;
	}

	char *p2 = p1 + strlen("extaddr:0x");
	
	strncpy(mac, p2, 16);
	mac[16] = 0;

	strcpy(cmac, mac);
	
	return 0;
}

int system_get_zwversion(char *zwver, int size) {
	char *cmd = "cat /etc/config/dusun/zwdev/version | cut -d \",\" -f 1 | cut -d \":\" -f 2";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(zwver, buf);


	return 0;
}

int system_get_zbversion(char *zbver, int size) {
	char *cmd = "cat /etc/config/dusun/amber/version | cut -d \",\" -f 1 | cut -d \":\" -f 2";
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(zbver, buf);

	return 0;
}



int system_get_mqtt_keepalive() {
	char *cmd = "uci get mqtt-gw.server.heartbeat;";
		
	char buf[256];
	int ret = system_cmd(cmd, buf);
	int keepalive = 20;
	if (ret != 0) {
		return keepalive;
	}

	if (sscanf(buf, "%d", &keepalive) != 1) {
		return keepalive;
	}

	return keepalive;
}

int system_get_mqtt_username_password(char *name, char *pass) {
  char *cmd = "uci get /etc/config/mqtt-gw.server.username;";

  char buf[256];
  int ret = system_cmd(cmd, buf);
  if (ret != 0) {
    name[0] = 0;
    pass[0] = 0;
    return -1;
  }
  strcpy(name, buf);


  cmd = "uci get /etc/config/mqtt-gw.server.password;";
  ret = system_cmd(cmd, buf);
  if (ret != 0) {
    name[0] = 0;
    pass[0] = 0;
    return -2;
  }
  strcpy(pass, buf);


  return 0;
}

int system_set_mqtt_username_password(char *name, char *pass) {
  char cmd[1024];
  char buf[256];
  sprintf(cmd, "uci set /etc/config/mqtt-gw.server.username=%s;uci set /etc/config/mqtt-gw.server.password=%s; uci commit mqtt-gw;", name, pass);
  int ret = system_cmd(cmd, buf);

  if (ret != 0) {
    return -1;
  }

  return 0;
}

char *system_get_uplink_type(char *type) {
	strcpy(type, "Ethernet");
	return type;
}

char * system_get_uplink_rssi(char *rssi) {
	char upType[32];
	system_get_uplink_type(upType);
	if (strcmp(rssi, "Ethernet") == 0) {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	} else if (strcmp(upType, "wifi") == 0) {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	} else if (strcmp(upType, "3G") == 0) {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	} else if (strcmp(upType, "4G") == 0) {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	} else if (strcmp(upType, "2G") == 0) {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	} else {
		/**> TODO */
		strcpy(rssi, "-100dBm");
	}

	return rssi;
}

int system_get_interface_ip(char *ifname, char *ip_buf) {
	int fd = 0;
	if((fd=socket(AF_INET, SOCK_STREAM, 0))<0) {
		strcpy(ip_buf, "0.0.0.0");
		return -1;
	}

	int ret = -1;
	struct ifreq temp;
	strcpy(temp.ifr_name, ifname);
	ret = ioctl(fd, SIOCGIFADDR, &temp);
	close(fd);
	if(ret < 0) {
		strcpy(ip_buf, "0.0.0.0");
		return -2;
	}

	struct sockaddr_in *myaddr;
	myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
	strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));
	return 0;
}

/*
 * led stuff 
 */
static int  write_led_attribute(char * led, char * att, char * value) {
    char path[100];
    char str[20];
    sprintf(path, "/sys/class/leds/%s/%s", led, att);
    sprintf(str, "%s\n", value);
    FILE * fp = fopen(path, "w");
    if (!fp)
        return -1;

    fputs(str, fp);
    fclose(fp);
    return 0;
}

int system_led_on(char * led)
{
  write_led_attribute(led, "trigger", "none");
  write_led_attribute(led, "brightness", "1");
	return 0;
}

int system_led_off(char * led)
{
	//log_debug(" [%s] %s\r\n", __func__, led);
	if (access("/tmp/sysinfo/mtksdk", F_OK) == 0) {
		mtsdk_led_off(led);
		return 0;
	}
	if (access("/tmp/sysinfo/item", F_OK) == 0) {
		mtsdk_led_off(led);
		return 0;
	}



  write_led_attribute(led, "trigger", "none");
  write_led_attribute(led, "brightness", "0");
	return 0;
}

int system_led_blink(char * led, int delay_on, int delay_off)
{
	log_debug(" [%s] %s\r\n", __func__, led);
	if (access("/tmp/sysinfo/mtksdk", F_OK) == 0) {
		log_debug(" ---------- ");
		mtsdk_led_blink(led, delay_on, delay_off);
		return 0;
	}
	if (access("/tmp/sysinfo/item", F_OK) == 0) {
		mtsdk_led_blink(led, delay_on, delay_off);
		return 0;
	}


  char delay_on_str[16];
  char delay_off_str[16];

  sprintf(delay_on_str, "%d", delay_on);
  sprintf(delay_off_str, "%d", delay_off);

  write_led_attribute(led, "trigger", "timer");
  write_led_attribute(led, "delay_on", delay_on_str);
  write_led_attribute(led, "delay_off", delay_off_str);
	return 0;
}

int system_led_shot(char * led)
{
	if (access("/tmp/sysinfo/mtksdk", F_OK) == 0) {
		mtsdk_led_shot(led);
		return 0;
	}
	if (access("/tmp/sysinfo/item", F_OK) == 0) {
		mtsdk_led_shot(led);
		return 0;
	}
	
	
  write_led_attribute(led, "trigger", "oneshot");
  write_led_attribute(led, "shot", "1");
	return 0;
}


int system_uci_get(char *config, char *section, char *name, char *value, int size) {
	//uci get wireless.@wifi-iface[0].mode
	
	char cmd[512];

	sprintf(cmd, "uci get %s.%s.%s", config, section, name);
	log_info("uci cmd [%s]", cmd);
	char buf[256];
	int ret = system_cmd(cmd, buf);
	if (ret != 0) {
		buf[0] = 0;
		return 0;
	}
	strcpy(value, buf);

	return 0;
}

/** crond */
#if 1
//#define crond_au_log_debug(...) (printf(__VA_ARGS__), printf("\n"))
#define crond_au_log_debug(...) 
#define crond_au_log_warn(...) (printf(__VA_ARGS__), printf("\n"))
#define crond_au_log_info(...) (printf(__VA_ARGS__), printf("\n"))
//#define crond_au_printf(...) printf(__VA_ARGS__)
#define crond_au_printf(...) 
#else
#define crond_au_log_debug(...) 
#define crond_au_log_warn(...)
#define crond_au_log_info(...) 
#define crond_au_printf(...)
#endif

#if 0
/*  crond valid char * / , - x
 *  *
 *  */x
 *  x-x
 *  x-x/x
 *  x,x,x
 *  ?
 *  ?/x
 *  x
 */
#endif

typedef struct stCrondItemType {
	int		type;
	char	*pattern;
} stCrondItemType_t;
typedef struct stCrondRegFmt {
	char							name[32];
	int								cnt;
	stCrondItemType_t	fmts[10];
} stCrondRegFmt_t;
typedef struct stCrondItem {
	int		type;
	char	*value;
	char	*name;
	unsigned long long bitval;
	int		min;
	int		max;
} stCrondItem_t;

static stCrondRegFmt_t crfs[] = {
	[0] = { "sec", 5, {
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1-5][0-9])$"},
					[2] = {2, "^([0-9]|[1-5][0-9])-([0-9]|[1-5][0-9])$"},
					[3] = {3, "^([0-9]|[1-5][0-9])-([0-9]|[1-5][0-9])/([1-9]|[1-5][0-9])$"},
					[4] = {4, "^([0-9]|[1-5][0-9])(,([0-9][1-5][0-9])){0,59}$"},
				}
	},
	[1] = { "min", 5, {
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1-5][0-9])$"},
					[2] = {2, "^([0-9]|[1-5][0-9])-([0-9]|[1-5][0-9])$"},
					[3] = {3, "^([0-9]|[1-5][0-9])-([0-9]|[1-5][0-9])/([1-9]|[1-5][0-9])$"},
					[4] = {4, "^([0-9]|[1-5][0-9])(,([0-9][1-5][0-9])){0,59}$"},
				}
	},
	[2] = { "hour", 5, {
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1][0-9]|[2][0-3])$"},
					[2] = {2, "^([0-9]|[1][0-9]|[2][0-3])-([0-9]|[1][0-9]|[2][0-3])$"},
					[3] = {3, "^([0-9]|[1][0-9]|[2][0-3])-([0-9]|[1][0-9]|[2][0-3])/([1-9]|[1][0-9]|[2][0-3])$"},
					[4] = {4, "^([0-9]|[1][0-9]|[2][0-3])(,([0-9]|[1][0-9]|[2][0-3])){0,23}$"},
				}
	},
	[3] = { "day", 7, {
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1-2][0-9]|[3][0])$"},
					[2] = {2, "^([1-9]|[1-2][0-9]|[3][0-1])-([1-9]|[1-2][0-9]|[3][0-1])"},
					[3] = {3, "^([1-9]|[1-2][0-9]|[3][0-1])-([1-9]|[1-2][0-9]|[3][0-1])/([1-9]|[1-2][0-9]|[3][0])$"},
					[4] = {4, "^([1-9]|[1-2][0-9]|[3][0-1])(,([1-9]|[1-2][0-9]|[3][0-1])){0,30}$"},
					[5] = {5, "^\\?$"},
					[6] = {6, "^\\?/([1-9]|[1-2][0-9]|[3][0-1])$"},
				}
	},
	[4] = { "month", 5, {
#if 0
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1][0-1])$"},
					[2] = {2, "^([0-9]|[1][0-1])-([0-9]|[1][0-1])$"},
					[3] = {3, "^([0-9]|[1][0-1])-([0-9]|[1][0-1])/([1-9]|[1][0-1]))$"},
					[4] = {4, "^([0-9]|[1][0-1])(,([0-9]|[1][0-1])){0,11}$"},
#else
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-9]|[1][0-1])$"},
					[2] = {2, "^([1-9]|[1][0-2])-([1-9]|[1][0-2])$"},
					[3] = {3, "^([1-9]|[1][0-2])-([1-9]|[1][0-2])/([1-9]|[1][0-1]))$"},
					[4] = {4, "^([1-9]|[1][0-2])(,([1-9]|[1][0-2])){0,11}$"},
#endif
				}
	},
	[5] = { "week", 7, {
#if 0
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-6])$"},
					[2] = {2, "^([0-6])-([0-6])"},
					[3] = {3, "^([0-6])-([0-6])/([1-6])$"},
					[4] = {4, "^([0-6])(,([0-6])){0,6}$"},
					[5] = {5, "^\\?$"},
					[6] = {6, "^\\?/([1-6])$"},
#else
					[0] = {0, "^\\*$"},
					[1] = {1, "^\\*/([1-6])$"},
					[2] = {2, "^([1-7])-([1-7])"},
					[3] = {3, "^([1-7])-([1-7])/([1-6])$"},
					[4] = {4, "^([1-7])(,([1-7])){0,6}$"},
					[5] = {5, "^\\?$"},
					[6] = {6, "^\\?/([1-6])$"},
#endif
				}
	},
	[6] = { "year", 3, {
					[0] = {0, "^\\*$"},
					[1] = {2, "^(2[0-9][0-9][0-9])-(2[0-9][0-9][0-9])"},
					[2] = {4, "^(2[0-9][0-9][0-9])(,(2[0-9][0-9][0-9])){0,20}$"},
				}
	},
};

int crond_au_valid_item(stCrondItem_t *item, int i) {
	if (item == NULL || (i < 0 || i > sizeof(crfs)/sizeof(crfs[0])) ){
		return 0;
	}

	stCrondRegFmt_t *crf = &crfs[i];
	int cnt = crf->cnt;
	int j = 0;
	crond_au_log_debug("  item:%d([%s],[%s]), cnt:%d", i, item->value, crf->name, crf->cnt);
	for (j = 0; j < cnt; j++) {
		const char *pattern = crf->fmts[j].pattern;
		regex_t			reg;
		crond_au_log_debug("  regcom [%s]...", pattern);
		int ret = regcomp(&reg, pattern, REG_EXTENDED);
		if (ret < 0) {
			crond_au_log_debug("  regcomp error :%d", ret);
			continue;
		}

		regmatch_t	pmatch[10];
		memset(pmatch, 0, sizeof(pmatch));
		crond_au_log_debug("  regexec [%s], pattern:[%s]", item->value, pattern);
		ret = regexec(&reg, item->value, sizeof(pmatch)/sizeof(pmatch[0]), pmatch, 0);
		crond_au_log_debug("  regexec over");
		if (ret == REG_NOMATCH) {
			crond_au_log_debug("  no match!");
			continue;
		}

		item->type = crf->fmts[j].type;
		return 1;
	}

	return 0;
}

unsigned long long  crond_au_cacl_bitval(stCrondItem_t *ci, int *_min) {
	int min = ci->min;
	int max = ci->max;
	int step = 1;
	int cnt = 0;
	int value[60];
	unsigned long long bitval = 0;
	switch (ci->type) {
		case 0: { // *
			;
		}
		break;


		case 1: { // */x
			sscanf(ci->value, "*/%d", &step);
		}
		break;

		case 2: { // x-x
			sscanf(ci->value, "%d-%d", &min, &max);
		}
		break;

		case 3: { // x-x/x
			sscanf(ci->value, "%d-%d/%d", &min, &max, &step);
		}
		break;

		case 4: { // x,x,x,
			/* 1, 2, 3, 4...*/
			char *s = ci->value;
			while (*s != '\0' && cnt < sizeof(value)/sizeof(value[0])) {
				char buf[64];
				char *e = strchr(s, ',');
				if (e != NULL) {
					strncpy(buf, s, e-s);
					buf[e-s] = 0;
					s = e+1;
				} else {
					strcpy(buf, s);
					s += strlen(s);
				}
				sscanf(buf, "%d", &value[cnt]);
				cnt++;
				
			} 
		}
		break;

		case 5: { // ?
			;
		}
			
		break;

		case 6: { // ?/x
			sscanf(ci->value, "?/%d", &step);
		}
		break;

		case 7: { // x
			sscanf(ci->value, "%d", &value[0]);
			cnt = 1;
		}
		break;
	}
	if (min > max) {
		int tmp = min;
		min = max;
		max = tmp;
	}
	
	crond_au_log_debug("====================================================");
	crond_au_log_debug("name:%s, value:%s, ci->type:%d", ci->name, ci->value, ci->type);
	crond_au_log_debug("min:%d, max:%d, step:%d, cnt:%d", min, max, step, cnt);
	if (cnt > 0) {
		int i = 0;
		for (i = 0; i < cnt; i++) {
			crond_au_printf("%d,", value[i]);
		}
		crond_au_printf("\n");
	}
	if (cnt == 0) {
		int i = 0;
		for (i = min; i < max; i += step) {
			bitval |= 1LL << i;
		}

		if (strcmp(ci->name, "year") == 0) {
			*_min = min + 2000 - 1900;
		} else {
			*_min = min;
		}
	} else {
		/**> VALUE 排顺序 */
		int i = 0;
		if (strcmp(ci->name, "year") == 0) {
			*_min = value[0] - 1900;
		} else if (strcmp(ci->name, "mon") == 0) {
			*_min = value[0] - 1;
		} else if (strcmp(ci->name, "wday") == 0) {
			*_min = value[0] - 1;
		} else {
			*_min = value[0];
		}
		for (i = 0; i < cnt; i++) {
			if (strcmp(ci->name, "year") == 0) {
				bitval |= 1LL << (value[i] - 1900 - 100);
			} else if (strcmp(ci->name, "mon") == 0) {
				bitval |= 1LL << (value[i] - 1);
			} else if (strcmp(ci->name, "wday") == 0) {
				bitval |= 1LL << (value[i] - 1);
			} else {
				bitval |= 1LL << value[i];
			}
		}
	}

	return bitval;
}

int crond_au_view_bitval(const char *name, unsigned long long bitval) {
	char bitbuf[128]={0};
	int j = 0;
	for (j = 63; j >= 0; j--) {
		sprintf(bitbuf + (63-j), "%lld", (bitval >> j)&0x1);
	}
	crond_au_log_debug("name:%s, bit:%llx, bitstr:%s", name, bitval, bitbuf);
	return 0;
}

int crond_au_valid(const char *cdstr, long *next, int *timeout) {
	char *s = (char *)cdstr;
	if (s == NULL) {
		crond_au_log_warn("null crond str!");
		return 0;
	}

	crond_au_log_info("crond valid : %s", cdstr);

	char _sec[256];
	char _min[256];
	char _hour[128];
	char _day[128];
	char _month[128];
	char _week[128];
	char _year[256];
	stCrondItem_t sec		= {-1, _sec,	"sec",		0, 0,59};
	stCrondItem_t min		= {-1, _min,	"min",		0, 0,59};
	stCrondItem_t hour	= {-1, _hour,	"hour",		0, 0,23};
	stCrondItem_t day		= {-1, _day,	"mday",		0, 1,31};
	stCrondItem_t month	= {-1, _month,"mon",		0, 0,11};
	stCrondItem_t week	= {-1, _week,	"wday",		0, 0,6};
	stCrondItem_t year	= {-1, _year,	"year",		0, 2000-1900-100,2000-1900+63-100};
	stCrondItem_t *ss[] = {&sec, &min, &hour, &day, &month, &week, &year};

	int i = 0; 
	char *e = NULL;
	for (i = 0, s = s; i < sizeof(ss)/sizeof(ss[0]); i++, s = e + 1) {
		/**> skip head space */
		crond_au_log_debug(" --------------------------------------------");
		crond_au_log_debug("  skip head space  for %s, left:%s", ss[i]->name, s);
		while (*s == ' ') {
			s++;
		}

		/**> check empty str */
		crond_au_log_debug("  check empty str  for %s", ss[i]->name);
		if (*s == '\0') {
			crond_au_log_debug("crond no %s content!", ss[i]->name);
			return 0;
		}

		/**> search end space */
		crond_au_log_debug("  search end space %s", ss[i]->name);
		e = NULL;
		if (i < sizeof(ss)/sizeof(ss[0]) - 1) {
			e = strchr(s, ' ');
		} else {
			e = s + strlen(s);
			while (*e == ' ') {
				e--;
			}
		}

		if (e == NULL || (e-s) < 1) {
			crond_au_log_warn("crond no %s", ss[i]->name);
			return 0;
		}
	
		/**> copy content */
		crond_au_log_debug("  copy item str for %s", ss[i]->name);
		strncpy(ss[i]->value, s, e - s);
		ss[i]->value[e-s] = 0;

		/**> valid item */
		crond_au_log_debug("  valid item str for %s", ss[i]->name);
		crond_au_log_debug(" %s:%s", ss[i]->name, ss[i]->value);
		if (!crond_au_valid_item(ss[i], i)) {
			crond_au_log_warn("crond error %s!", ss[i]->name);
			return 0;
		}
	}

	crond_au_log_info("[AT] sec:%s, min:%s, hour:%s, day:%s, month:%s, week:%s, year:%s",
		sec.value, min.value, hour.value, day.value, month.value, week.value, year.value);

	struct tm tv;
	int *tvs[] = {&tv.tm_sec, &tv.tm_min, &tv.tm_hour, &tv.tm_mday, &tv.tm_mon, &tv.tm_wday, &tv.tm_year};
	for (i = 0, s = s; i < sizeof(ss)/sizeof(ss[0]); i++, s = e + 1) {
		ss[i]->bitval = crond_au_cacl_bitval(ss[i], tvs[i]);
		crond_au_view_bitval(ss[i]->name, ss[i]->bitval);
	}


	if (timeout == NULL && next == NULL) {
		return 1;
	}

	long now = time(NULL);
	struct tm tt;
	struct tm *tnow = &tt;
	localtime_r(&now, tnow);
	crond_au_log_info("[NW] sec:%d, min:%d, hour:%d, day:%d, month:%d, week:%d, year:%d",
		tt.tm_sec, tt.tm_min,tt.tm_hour, tt.tm_mday, tt.tm_mon+1, tt.tm_wday+1, tt.tm_year+1900);


	long dtime = now;
	struct tm td;
	localtime_r(&dtime, &td);

	{
		int *val = &td.tm_mday;
		stCrondItem_t *ci = ss[3];

		int *wval = &td.tm_wday;
		stCrondItem_t *ciw = ss[5];

		int *mval = &td.tm_mon;
		stCrondItem_t *cim = ss[4];

		int *yval = &td.tm_year;
		stCrondItem_t *ciy = ss[6];

		int cnt = 0;
		crond_au_log_debug("mday:%d, wday:%d, mon:%d, year:%d, bitval:%llx", *val, *wval+1, *mval+1, *yval+1900, ci->bitval);

		crond_au_view_bitval("day", (1LL << (*val)));
		crond_au_view_bitval(ci->name,	ci->bitval);

		crond_au_view_bitval("wday", (1LL << (*wval)));
		crond_au_view_bitval(ciw->name, ciw->bitval);

		crond_au_view_bitval("mon", (1LL << (*mval)));
		crond_au_view_bitval(cim->name, cim->bitval);

		crond_au_view_bitval("year", (1LL << (*yval+1900-2000)));
		crond_au_view_bitval(ciy->name, ciy->bitval);

		while (	ci->bitval != 0 && (*yval+1900-2000) < ciy->max && (
						((1LL << (*val))  & ci ->bitval) == 0 ||
						((1LL << (*wval)) & ciw->bitval) == 0 ||
						((1LL << (*mval)) & cim->bitval) == 0 ||
						((1LL << (*yval + 1900 - 2000)) & ciy->bitval) == 0) ) {
			crond_au_view_bitval("day", (1LL << (*val)));
			crond_au_view_bitval(ci->name,	ci->bitval);

			crond_au_view_bitval("wday", (1LL << (*wval)));
			crond_au_view_bitval(ciw->name, ciw->bitval);

			crond_au_view_bitval("mon", (1LL << (*mval)));
			crond_au_view_bitval(cim->name, cim->bitval);

			crond_au_view_bitval("year", (1LL << (*yval+1900-2000)));
			crond_au_view_bitval(ciy->name, ciy->bitval);

			dtime += 3600*24;
			localtime_r(&dtime, &td);
			td.tm_hour = 0;
			td.tm_min = 0;
			td.tm_sec = 0;
			dtime = mktime(&td);

			cnt++;
			if (cnt >= 2) { // only check today and next day
				break;
			}
			crond_au_log_debug("mday:%d, wday:%d, mon:%d, year:%d, bitval:%llx, hour:%d", *val, *wval+1, *mval+1, *yval+1900, ci->bitval, td.tm_hour);
		}
		if (cnt >= 2) {
			crond_au_log_warn("next time is not in today & next day!");
			return 0;
		}
	}

	{
		int *val = &td.tm_hour;
		stCrondItem_t *ci = ss[2];
		crond_au_log_debug("hour, val:%d, bitval:%llx", *val, ci->bitval);
		while (ci->bitval != 0 &&  ((1LL << (*val)) & ci->bitval) == 0) {
			dtime += 3600;
			localtime_r(&dtime, &td);
			td.tm_min = 0;
			td.tm_sec = 0;
			dtime = mktime(&td);
		}
	}

	{
		int *val = &td.tm_min;
		stCrondItem_t *ci = ss[1];
		crond_au_log_debug("min, val:%d, bitval:%llx", *val, ci->bitval);
		while (ci->bitval != 0 &&  ((1LL << (*val)) & ci->bitval) == 0) {
			dtime += 60;
			localtime_r(&dtime, &td);
			td.tm_sec = 0;
			dtime = mktime(&td);
		}
	}

	{
		int *val = &td.tm_sec;
		stCrondItem_t *ci = ss[0];
		crond_au_log_debug("sec, val:%d, bitval:%llx", *val, ci->bitval);
		while (ci->bitval != 0 &&  ((1LL << (*val)) & ci->bitval) == 0) {
			dtime += 1;
			localtime_r(&dtime, &td);
		}
	}

	crond_au_log_info("[NT] sec:%d, min:%d, hour:%d, day:%d, month:%d, week:%d, year:%d",
		td.tm_sec, td.tm_min,td.tm_hour, td.tm_mday, td.tm_mon+1, td.tm_wday+1, td.tm_year+1900);
	{
		int *val = &td.tm_mday;
		stCrondItem_t *ci = ss[3];

		int *wval = &td.tm_wday;
		stCrondItem_t *ciw = ss[5];

		int *mval = &td.tm_mon;
		stCrondItem_t *cim = ss[4];

		int *yval = &td.tm_year;
		stCrondItem_t *ciy = ss[6];

		int *hval = &td.tm_hour;
		stCrondItem_t *cih = ss[2];
		
		int *Mval = &td.tm_min;
		stCrondItem_t *ciM = ss[1];
	
		int *sval = &td.tm_sec;
		stCrondItem_t *cis = ss[0];
	

		crond_au_log_debug("mday:%d, wday:%d, mon:%d, year:%d, hour:%d, min:%d, sec:%d, bitval:%llx", *val, *wval+1, *mval+1, *yval+1900, *hval, *Mval, *sval, ci->bitval);

		crond_au_view_bitval("day", (1LL << (*val)));
		crond_au_view_bitval(ci->name,	ci->bitval);

		crond_au_view_bitval("wday", (1LL << (*wval)));
		crond_au_view_bitval(ciw->name, ciw->bitval);

		crond_au_view_bitval("mon", (1LL << (*mval)));
		crond_au_view_bitval(cim->name, cim->bitval);

		crond_au_view_bitval("year", (1LL << (*yval+1900-2000)));
		crond_au_view_bitval(ciy->name, ciy->bitval);

		crond_au_view_bitval("hour", (1LL << (*hval)));
		crond_au_view_bitval(cih->name, cih->bitval);

		crond_au_view_bitval("min", (1LL << (*Mval)));
		crond_au_view_bitval(ciM->name, ciM->bitval);

		crond_au_view_bitval("sec", (1LL << (*sval)));
		crond_au_view_bitval(cis->name, cis->bitval);

		if (ci->bitval != 0 && (*yval+1900-2000) < ciy->max && (
						((1LL << (*val))  & ci ->bitval) == 0 ||
						((1LL << (*wval)) & ciw->bitval) == 0 ||
						((1LL << (*mval)) & cim->bitval) == 0 ||
						((1LL << (*yval + 1900 - 2000)) & ciy->bitval) == 0 || 
						((1LL << (*hval)) & cih->bitval) == 0 ||
						((1LL << (*Mval)) & ciM->bitval) == 0 ||
						((1LL << (*sval)) & cis->bitval) == 0 ) ) {
		crond_au_log_info("[FD] sec:%d, min:%d, hour:%d, day:%d, month:%d, week:%d, year:%d",
			td.tm_sec, td.tm_min,td.tm_hour, td.tm_mday, td.tm_mon+1, td.tm_wday+1, td.tm_year+1900);
			crond_au_log_warn("next time is not in today & next day! 1");
			return 0;
		}
	}

	crond_au_log_info("[OK] sec:%d, min:%d, hour:%d, day:%d, month:%d, week:%d, year:%d",
		td.tm_sec, td.tm_min,td.tm_hour, td.tm_mday, td.tm_mon+1, td.tm_wday+1, td.tm_year+1900);

	if (next != NULL) {
		*next = dtime;
	}
	if (timeout != NULL) {
		*timeout = dtime - now;
	}

	return 1;
}

void test_crond() {	
	//const char *cdstr = "* 33-44/2 */5 * 8 4 2019";
	//const char *cdstr = "1-59/12 33-44/2 10 * 7 4 2019";
	const char *cdstr = "1-59/12 33-44/2 16 22 7 * 2019";
	long next = 0;
	int timeout = 0;

	long last = schedue_current();
	int ret = crond_au_valid(cdstr, &next, &timeout);
	long delt = schedue_current() - last;
	if (ret != 1) {
		log_debug("not valid !");
	} else {
		struct tm td;
		localtime_r(&next, &td);
		log_debug("[NEXT] sec:%d, min:%d, hour:%d, day:%d, month:%d, week:%d, year:%d",
				td.tm_sec, td.tm_min,td.tm_hour, td.tm_mday, td.tm_mon+1, td.tm_wday+1, td.tm_year+1900);

		log_debug("[%s]-> ret:%d, next:%d, timeout:%d, use:%ld ms", cdstr, ret, timeout, ret, delt);
	}
}


int system_upgrade(const char *target, const char *url, const char *keep) {
	log_info("upgrade %s ....", target);
	char cmd[512];
	sprintf("/usr/bin/updemo.sh %s %s %s > /dev/null &", target, url, keep);
	system(cmd);
	return 0;
}
int system_delete_netinfo(const char *nifile) {
	char buf[256];
	sprintf(buf, "rm -rf %s", nifile);
	system(buf);
	return 0;
}
int system_write_netinfo(const char *nifile, int HomeID, char NodeID) {
	FILE *fp = fopen(nifile, "w");
	if (fp == NULL) {
		return 0;
	}

	char buf[256];
	sprintf(buf, "HomeID:0x%08X, NodeID:%02X\n", HomeID, NodeID);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	return 0;
}


int system_get_zwave_region() {
	FILE *fp = fopen("/etc/config/dusun/zwdev/region", "r");
	if (fp == NULL) {
		return -1;
	}
	
	char line[1024];
	memset(line, 0, sizeof(line));
	if (fgets(line, sizeof(line), fp) == NULL) {
		fclose(fp);
		return -2;
	}
	int region;
	if (sscanf(line, "0x%02x", &region) != 1) {
		fclose(fp);
		return -3;
	}
	fclose(fp);

	switch (region&0xff) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x20:
		case 0x21:
			return region&0xff;
		default:
			break;
	}
	
	return -4;
}

int system_set_zwave_region(int region) {
	FILE *fp = fopen("/etc/config/dusun/zwdev/region", "w");
	if (fp == NULL) {
		return -1;
	}
	
	char line[1024];
	sprintf(line, "0x%02X\n", region&0xff);
	fputs(line, fp);
	fclose(fp);

	return 0;
}

