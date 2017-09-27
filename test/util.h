/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis                       |
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com                                 |
| Date: 09/2017                                                     |
|-------------------------------------------------------------------|*/
#ifndef UTIL_H
#define UTIL_H_

#include "test.h"
#include "my-include.h"

#include <stdio.h>
#include <stdint.h>


typedef enum { false, true } bool;

#define DEBUG 1
#if DEBUG
#define PRINTF(...)    printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define POLY 0x8408

#define CBC 1
#define ECB 1
#define MAX_LEN_CBC 64
//#define MAX_DEVICE	8

static uint8_t iv[16]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
                           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

                           
static uint8_t key_begin[16] = "I<3U_Yori-Phuong";

unsigned short gen_crc16(uint8_t *data_p, unsigned short length);
bool check_crc16(uint8_t *data_p, unsigned short length);
void phex_8(uint8_t* data_8);
void phex_64(uint8_t* data_64);
void parse_64(uint8_t* data_64, frame_struct_t *receive);
//***util.c
void ipv6_to_str_unexpanded(char * str, const struct in6_addr * addr);
uint16_t hash(uint16_t a);
uint16_t gen_random_num();
void gen_random_key_128(unsigned char* key);
void
prepare2send(frame_struct_t *send, uint8_t *src_ipaddr, uint8_t *dest_ipaddr,\
            uint32_t frame_counter, uint8_t state, uint8_t cmd, uint8_t *data);
void copy_node2node(node_t *node_scr, node_t *node_des);
void PRINTF_DATA_NODE(node_t *node_id);
void PRINTF_DATA(frame_struct_t *frame);

/*-AES-*/
void decrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv);
void encrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv);


#endif /* UTIL_H_ */