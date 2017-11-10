#include "PZEM004T.h"
#include "my-include.h"
#include "util.h"

struct PZEM_t PZEM_data_t;

// Set the communication address: 192.168.1.1???
unsigned char set_address[7] = {0xB4, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1E}; 

unsigned char set_threshold[7] = {0xB5, 0xC0, 0xA8, 0x01, 0x01, PZEM_POWER_THRESHOLD, 0x33};

unsigned char get_voltage[7] = {0xB0, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1A};

unsigned char get_current[7] = {0xB1, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1B};

unsigned char get_power[7] = {0xB2, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1C};

unsigned char get_energy[7] = {0xB3, 0xC0, 0xA8, 0x01, 0x01, 0x00, 0x1D};

unsigned char PZEM_resp_address[7] = {0xA4, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA4};

unsigned char PZEM_resp_power_threshold[7] = {0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA5};
/*------------------------------------------------------------------*/
int 
send_to_PZEM(uint8_t cmd){
	switch(cmd){
		case PZEM_SET_ADDRESS:
			uart1_send_bytes(set_address, 7);
			PZEM_data_t.addr[0] = set_address[1];
			PZEM_data_t.addr[1] = set_address[2];
			PZEM_data_t.addr[2] = set_address[3];
			PZEM_data_t.addr[3] = set_address[4];
			break;
		case PZEM_POWER_ALARM:
			uart1_send_bytes(set_threshold, 7); // THRESHOLD is defined in PZEM004T.h
			PZEM_data_t.power_alarm = set_threshold[5];
			break;
		case PZEM_VOLTAGE:
			uart1_send_bytes(get_voltage, 7);
			break;
		case PZEM_CURRENT:
			uart1_send_bytes(get_current, 7);
			break;
		case PZEM_POWER:
			uart1_send_bytes(get_power, 7);
			break;
		case PZEM_ENERGY:
			uart1_send_bytes(get_energy, 7);
			break;
		default:
			return PZEM_ERROR_VALUE;
			break;

	}
	return PZEM_OK_VALUE;
}
/*------------------------------------------------------------------*/
int is_PZEM_1st_byte(unsigned char c){
	switch(c){
		case RESP_SET_ADDRESS:
		case RESP_POWER_ALARM:
		case RESP_VOLTAGE:
		case RESP_CURRENT:
		case RESP_POWER:
		case RESP_ENERGY:
			return 1;
			break;
		default:
			return 0;
	}
}
/*------------------------------------------------------------------*/
int 
recv_from_PZEM(const unsigned char *data){
	//printf("\nrecv_from_PZEM\n");
	if(crc_PZEM(data)){
		parse_PZEM(data);
		return PZEM_OK_VALUE;
	}
	return PZEM_ERROR_VALUE;
}
/*------------------------------------------------------------------*/
int 
crc_PZEM(const unsigned char *data){
	//printf("\ncrc_PZEM\n");
	uint8_t i;
	uint8_t checksum = 0;
	for(i=0; i<6; i++){
		checksum = (uint8_t) (checksum + *(data+i));
	}
	if(checksum == data[6]){
		return PZEM_OK_VALUE;
	}
	return PZEM_ERROR_VALUE;
}
/*------------------------------------------------------------------*/
int parse_PZEM(const unsigned char *data){
	uint8_t i;
	for(i=0; i<7; i++){
		PZEM_data_t.last_data_recv[i] = data[i]; 
	}
	switch(data[0]){
		case RESP_SET_ADDRESS:
			return 1;

		case RESP_POWER_ALARM:
			return 1;

		case RESP_VOLTAGE:
		    PZEM_data_t.voltage_x10 = 0; // clear
			PZEM_data_t.voltage_x10 = ((uint16_t) data[1]) << 8;
			PZEM_data_t.voltage_x10 += ((uint16_t) data[2]);
			PZEM_data_t.voltage_x10 *= 10;
			PZEM_data_t.voltage_x10 += ((uint16_t) data[3]);
			return 1;

		case RESP_CURRENT:
			PZEM_data_t.current_x100 = 0; // clear
			PZEM_data_t.current_x100 = ((uint16_t) data[2]);
			PZEM_data_t.current_x100 *= 100;
			PZEM_data_t.current_x100 += ((uint16_t) data[3]);
			return 1;

		case RESP_POWER:
			PZEM_data_t.power = 0; // clear
			PZEM_data_t.power = ((uint16_t) data[1]) << 8;
			PZEM_data_t.power += ((uint16_t) data[2]);
			return 1;

		case RESP_ENERGY:
			PZEM_data_t.energy = 0; // clear
			PZEM_data_t.energy = ((uint16_t) data[1]) << 16;
			PZEM_data_t.energy |= ((uint16_t) data[2]) << 8;
			PZEM_data_t.energy |= ((uint16_t) data[3]);
			return 1;

		default:
			return 0;
	}
}

