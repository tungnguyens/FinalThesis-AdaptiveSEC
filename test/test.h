/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis                       |
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com                                 |
| Date: 09/2017                                                     |
|-------------------------------------------------------------------|*/
#ifndef TEST_H_
#define TEST_H_

#include <stdint.h>
#include <time.h>

enum port { // port
 		BORDER_ROUTER_LISTEN_PORT 	= 3000,
 		SERVER_LISTEN_PORT 			= 3000,
};

enum state { // state
		STATE_BEGIN 	= 1,
		STATE_NORMAL	= 2,
		STATE_EMERGENCY = 3,
};

enum cmd { 	// begin cmd
		REQUEST_JOIN 	= 1,
		REQUEST_HASH	= 2,
		REPLY_HASH		= 3,
		JOIN_SUCCESS    = 4,
		// normal cmd
		SEND_NORMAL 	= 5,
		// emergency cmd
		SEND_EMERGENCY  = 6,

};

#define	SFD 					0x7E		/* Start of NCKH frame Delimitter */
#define	MAX_LEN 				64		/* Start of NCKH frame Delimitter */

#define MAX_END_DEVICE			8
#define MAX_ROUTER				30
#define MAX_DEVICE 				9

#define TIME_OUT				2

#define LEN_CHAR_IPV6       26
/*---------------------------------------------------------------------------*/
//	sfd = 0x7E;
//	len = 64;
//	seq: frame counter;
//	type: JOIN/ KEY/ VI_TRI/ NHIP_TIM/ FOR_FUTURE
//	cmd:	command id
//	payload_data[16]: data payload
//  ipv6_source_addr[16] : source address
//	ipv6_dest_addr[16] : destination address
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
}__attribute__((packed, aligned(1)));

typedef struct frame_struct_t	frame_struct_t;

struct node_t {
	uint8_t 			stt;
	char				type_device; //'R' = router 'E'= End device
	char 				ipv6_addr[26];
	char 				connected; // 'Y', 'N'
	char 				emergency; //'Y', 'N'
	uint32_t			num_receive;
	uint32_t			num_send;
	uint32_t			num_emergency;
	uint32_t			last_seq;
	uint8_t				RSSI[MAX_ROUTER +1];

	frame_struct_t   	last_data_receive;
	frame_struct_t		last_data_send;

	uint16_t			challenge_code;
	uint8_t				last_pos;
	char				authenticated;//Y/N
	uint8_t				key[16];
	time_t				timer;

}__attribute__((packed, aligned(1)));

typedef struct node_t		node_t;
	
#endif