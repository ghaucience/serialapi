#ifndef __MTKSDK_H_
#define __MTKSDK_H_


int mtsdk_led_get(char *led);
int mtsdk_led_mode_is_none(char *led);
int mtsdk_led_on(char *led);
int mtsdk_led_off(char *led);
int mtsdk_led_blink(char *led, int delay_on, int delay_off);
int mtsdk_led_shot(char *led);

#endif
