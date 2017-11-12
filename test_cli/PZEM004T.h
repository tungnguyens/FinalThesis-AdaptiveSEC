#ifndef PZEM004T_H
#define PZEM004T_H

#include <stdio.h>
#include <stdint.h>

struct PZEMCommand {
    uint8_t command;
    uint8_t addr[4];
    uint8_t data;
    uint8_t crc;
};

struct PZEM_t {
	uint8_t addr[4];// = {0xC0, 0xA8, 0x01, 0x01};
	uint8_t power_alarm; 
	uint16_t voltage_x10;
	uint16_t current_x100;
	uint16_t power;
	uint32_t energy;
	uint8_t last_data_recv[7];
}__attribute__((packed, aligned(1)));

#define RESPONSE_SIZE sizeof(PZEMCommand)
#define RESPONSE_DATA_SIZE RESPONSE_SIZE - 2

#define PZEM_SET_ADDRESS (uint8_t)0xB4
#define RESP_SET_ADDRESS (uint8_t)0xA4

#define PZEM_POWER_ALARM (uint8_t)0xB5
#define RESP_POWER_ALARM (uint8_t)0xA5

#define PZEM_VOLTAGE (uint8_t)0xB0
#define RESP_VOLTAGE (uint8_t)0xA0

#define PZEM_CURRENT (uint8_t)0xB1
#define RESP_CURRENT (uint8_t)0xA1

#define PZEM_POWER   (uint8_t)0xB2
#define RESP_POWER   (uint8_t)0xA2

#define PZEM_ENERGY  (uint8_t)0xB3
#define RESP_ENERGY  (uint8_t)0xA3

#define PZEM_POWER_THRESHOLD (uint8_t)0x14 // power threshold = 20kW

#define PZEM_BAUD_RATE 9600
#define PZEM_DEFAULT_READ_TIMEOUT 1000

#define PZEM_ERROR_VALUE  0
#define PZEM_OK_VALUE     1

int is_PZEM_1st_byte(unsigned char c);
int send_to_PZEM(uint8_t cmd);
int recv_from_PZEM(const unsigned char *data);
int crc_PZEM(const unsigned char *data);
int parse_PZEM(const unsigned char *data);

#endif // PZEM004T_H