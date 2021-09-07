#ifndef __SYSTEM_H_
#define __SYSTEM_H_

	int system_get_lan_ip(char *ip, int size);

	int system_get_lan_mask(char *mask, int size);


	int system_get_wan_ip(char *ip, int size);
	
	int system_get_wan_gateway(char *gw, int size);

	int system_get_wan_mask(char *mask, int size);

	int system_get_wan_dns1(char *dns1, int size);

	int system_get_wan_dns2(char *dns2, int size);


	int system_get_wifi_onoff(int *onoff);

	int system_get_wifi_mode(char *mode, int size);

	int system_get_wifi_ssid(char *ssid, int size);

	int system_get_wifi_encryption(char *encryption, int size);

	int system_get_wifi_key(char *key, int size);

	int system_set_wifi(int onoff, char *mode, char *ssid, char *encryption, char *key);


	int system_get_4g_onoff(int *onoff);
	
	int system_get_4g_device(char *device, int size);

	int system_get_4g_simcard(int *simcard);

	int system_get_4g_ip(char *ip, int size);

	
	int system_get_ble_onoff(int *onoff);

	int system_get_ble_mac(char *mac, int size);

	int system_get_ble_scan(int *scan);

	int system_get_ble_pair(int *pair);

	int system_set_ble_scan(int scan);

	int system_set_ble_enpair(int enpair);

	int system_set_ble_remove(char *mac);

	int system_get_ble_list();

	int system_get_system_version(char *version, int size);
	
	int system_get_model(char *model, int size);

	int system_get_mac(char *mac, int size);

	int system_get_zwversion(char *zwver, int size);

	int system_get_zbversion(char *zbver, int size);
	

	int system_get_mqtt_keepalive();
	int system_get_mqtt_username_password(char *name, char *pass);
	int system_set_mqtt_username_password(char *name, char *pass);

	char *system_get_uplink_type(char *type);
	char *system_get_uplink_rssi(char *rssi);


	int system_uci_get(char *config, char *section, char *name, char *value, int size);

	int crond_au_valid(const char *cdstr, long *next, int *timeout);

	int system_cmd(char *cmd, char *out);

	int system_upgrade(const char *target, const char *url, const char *keep);
	int system_delete_netinfo(const char *nifile);
	int system_write_netinfo(const char *nifile, int HomeID, char NodeID);
	int system_get_zwave_region();
	int system_set_zwave_region(int region);
	const char *system_model_get();
#endif
