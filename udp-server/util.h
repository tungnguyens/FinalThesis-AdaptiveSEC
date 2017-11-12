/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis  						|
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com									|
| Date: 09/2017                                                     |
| HW support in ISM band: CC2538									|
|-------------------------------------------------------------------|*/
#ifndef UTIL_H_
#define UTIL_H_

#include "my-include.h"
#include "server.h"
#include "tiny_aes.h"
#include "PZEM004T.h"

#include <stdio.h>
#include <stdint.h>

typedef enum { false, true } bool;

#define CBC 1
#define ECB 1

// #define DEBUG 0
// #if DEBUG
// #include <stdio.h>
// #define PRINTF(...) printf(__VA_ARGS__)
// #else /* DEBUG */
// #define PRINTF(...)
// #endif /* DEBUG*/

#define POLY 0x8408
#define MAX_LEN_CBC 64

unsigned short gen_crc16(uint8_t *data_p, unsigned short length);
bool check_crc16(uint8_t *data_p, unsigned short length);

void get_my_addrress(uip_ipaddr_t *ipaddr);
void set_border_router_address(uip_ipaddr_t *ipaddr);

/*-AES-*/
void decrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv);
void encrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv);


void prepare2send(frame_struct_t *send, uip_ipaddr_t *src_ipaddr, uip_ipaddr_t *dest_ipaddr,\
      							uint32_t frame_counter, uint8_t state, uint8_t cmd, uint8_t *data);

void parse_64(uint8_t* data_64, frame_struct_t *receive);
uint16_t hash(uint16_t a);

unsigned int uart0_send_bytes(const unsigned  char *s, unsigned int len);
unsigned int uart1_send_bytes(const unsigned  char *s, unsigned int len);

void pack_data_PZEM(struct PZEM_t *PZEM_t_p, uint8_t *data, unsigned int len);

void print_64byte(uint8_t *buf, uint8_t len);
void PRINTF_DATA(frame_struct_t *frame);
#endif /* UTIL_H_ */