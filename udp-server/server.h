/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis                       |
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com                                 |
| Date: 09/2017                                                     |
| HW support in ISM band: CC2538                                    |
|-------------------------------------------------------------------|*/

#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdint.h>

enum port { // port
 			BORDER_ROUTER_LISTEN_PORT 		= 3000,
 			SERVER_LISTEN_PORT 				= 3000,
 			BORDER_ROUTER_RELAY_LISTEN_PORT	= 3001,
 			SERVER_RELAY_LISTEN_PORT		= 3001,
};

enum state { // state
			STATE_BEGIN 	= 1,
			STATE_NORMAL	= 2,
			STATE_EMERGENCY = 3,
};

enum cmd { 	// begin cmd ///////////
			REQUEST_JOIN 		= 1,
			REQUEST_HASH		= 2,
			REPLY_HASH			= 3,
			JOIN_SUCCESS    	= 4,

			// normal cmd //////////
			SEND_PZEM	 		= 5,
			REQUEST_LED_ON 		= 6,
			REPLY_LED_ON		= 7,
			REQUEST_LED_OFF		= 8,
			REPLY_LED_OFF		= 9,

			// emergency cmd ///////
			SEND_EMERGENCY  	= 10,
};
/*---------------------------------------------------------------------------*/
#define	SFD 		0x7E	/* Start of MY_THESIS frame Delimitter */
#define	MAX_LEN 	64		/* Always 64 */

struct frame_struct_t {
	uint8_t  	sfd;
	uint8_t 	len;
	uint32_t 	seq;
	uint8_t		state;
	uint8_t		cmd;
	uint8_t		payload_data[16];
	uint8_t 	ipv6_source_addr[16];
	uint8_t 	ipv6_dest_addr[16];
	uint8_t		for_future[6];
	uint16_t	crc;	
}__attribute__((packed, aligned(1)));;

typedef struct frame_struct_t	frame_struct_t;
/*---------------------------------------------------------------------------*/
#endif /* SERVER_H_ */