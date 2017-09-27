/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis             			|
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com                 				|
| Date: 09/2017                                                     |
| HW support in ISM band: CC2538                  					|
|-------------------------------------------------------------------|*/
#ifndef MY_INCLUDE_H
#define MY_INCLUDE_H

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdio.h>
#include <string.h>
#include "lib/random.h"
#include "random.h"

#include "net/ip/uip.h"
#include "net/ip/uip-udp-packet.h"

#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-route.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"

#include "dev/my_leds.h"


#endif /* MY_INCLUDE_H */