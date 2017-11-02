#include "PZEM004T.h"
#include "my-include.h"
#include "util.h"


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
bool 
crc_PZEM(const unsigned char *data){
	return PZEM_OK_VALUE;
}

