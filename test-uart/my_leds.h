#ifndef MY_LED_H_
#define MY_LED_H_

#include "contiki.h"
#include "reg.h"
#include "dev/gpio.h"


void my_leds_on(unsigned char leds);
void my_leds_off(unsigned char leds);

unsigned char my_leds_get(unsigned char port_base);
void my_leds_toggle(unsigned char leds);

#endif /* MY_LED_H_ */
