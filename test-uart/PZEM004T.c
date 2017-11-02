#include "PZEM004T.h"
#include "my-include.h"
#include "util.h"

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
			uart0_send_bytes(set_address, 7);
			break;
		case PZEM_POWER_ALARM:
			uart0_send_bytes(set_threshold, 7); // THRESHOLD is defined in PZEM004T.h
			break;
		case PZEM_VOLTAGE:
			uart0_send_bytes(get_voltage, 7);
			break;
		case PZEM_CURRENT:
			uart0_send_bytes(get_current, 7);
			break;
		case PZEM_POWER:
			uart0_send_bytes(get_power, 7);
			break;
		case PZEM_ENERGY:
			uart0_send_bytes(get_energy, 7);
			break;
		default:
			return PZEM_ERROR_VALUE;
			break;

	}
	return PZEM_OK_VALUE;
}
/*------------------------------------------------------------------*/
int 
recv_from_PZEM(const unsigned char *data){
	return PZEM_OK_VALUE;
}
/*------------------------------------------------------------------*/
int 
crc_PZEM(const unsigned char *data){
	return PZEM_OK_VALUE;
}

