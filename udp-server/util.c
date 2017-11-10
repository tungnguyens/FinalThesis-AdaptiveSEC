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


#include "util.h"
#include "tiny_aes.h"

/*---------------------------------------------------------------------------*/
unsigned short 
gen_crc16(uint8_t *data_p, unsigned short length)
{
  unsigned char i;
  unsigned int data;
  unsigned int crc = 0xffff;

  if (length == 0)
    return (~crc);

  do
  {
    for (i=0, data=(unsigned int)0xff & *data_p++; i < 8; i++, data >>= 1)
    {
      if ((crc & 0x0001) ^ (data & 0x0001))
        crc = (crc >> 1) ^ POLY;
      else  crc >>= 1;
    }
  } while (--length);

  crc = ~crc;
  data = crc;
  crc = (crc << 8) | (data >> 8 & 0xff);

  return (crc);
}


/*---------------------------------------------------------------------------*/
bool check_crc16(uint8_t *data_p, unsigned short length)
{
  unsigned short crc16_check;
  uint8_t c0, c1;

  c0 = c1 = 0;
  crc16_check = gen_crc16(data_p, length - 2);
  c0 = (uint8_t)(crc16_check & 0x00ff);
  c1 = (uint8_t)((crc16_check & 0xff00) >> 8);

  if( c0 == data_p[length -2] && c1 == data_p[length -1]){
      return true;
  }
  else return false;
}
/*---------------------------------------------------------------------------*/
void 
get_my_addrress(uip_ipaddr_t *ipaddr) {
  uint8_t longaddr[8];
  uint16_t ip[4];

  memset(longaddr, 0, sizeof(longaddr));
  linkaddr_copy((linkaddr_t *)&longaddr, &linkaddr_node_addr);
  //printf("My MAC =  %02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
         //longaddr[0], longaddr[1], longaddr[2], longaddr[3],
         //longaddr[4], longaddr[5], longaddr[6], longaddr[7]);
  ip[0] = ((uint16_t)longaddr[0])<<8|(uint16_t)longaddr[1];
  ip[1] = ((uint16_t)longaddr[2])<<8|(uint16_t)longaddr[3];
  ip[2] = ((uint16_t)longaddr[4])<<8|(uint16_t)longaddr[5];
  ip[3] = ((uint16_t)longaddr[6])<<8|(uint16_t)longaddr[7];
  
	uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, ip[1], ip[2], ip[3]);
}
/*---------------------------------------------------------------------------*/
void 
set_border_router_address(uip_ipaddr_t *ipaddr) {
  // change this IP address depending on the node that runs the server!
  uip_ip6addr(ipaddr, 0xaaaa,0x0000,0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001);
}
/*---------------------------------------------------------------------------*/
// ma hoa 64 bytes
void 
encrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv) 
{ 
  uint8_t data_temp[MAX_LEN_CBC];
  memcpy(data_temp, data_input, MAX_LEN_CBC);

  AES128_CBC_encrypt_buffer(data_temp, data_output, 64, key, iv);
}
/*---------------------------------------------------------------------------*/
// giai ma 64 bytes
void 
decrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv) 
{
  uint8_t data_temp[MAX_LEN_CBC];
  memcpy(data_temp, data_input, MAX_LEN_CBC);

  AES128_CBC_decrypt_buffer(data_temp+0,  data_output+0,  16, key, iv);;
  AES128_CBC_decrypt_buffer(data_temp+16, data_output+16, 16, 0, 0);
  AES128_CBC_decrypt_buffer(data_temp+32, data_output+32, 16, 0, 0);
  AES128_CBC_decrypt_buffer(data_temp+48, data_output+48, 16, 0, 0);
}
/*---------------------------------------------------------------------------*/
void
prepare2send(frame_struct_t *send, uip_ipaddr_t *src_ipaddr, uip_ipaddr_t *dest_ipaddr,\
      uint32_t frame_counter, uint8_t state, uint8_t cmd, uint8_t *data){

  send->sfd = SFD;
  send->len = MAX_LEN;
  send->seq = frame_counter;
  send->state = state;
  send->cmd = cmd;

  memcpy(send->payload_data, data, 16);
  memcpy(send->ipv6_source_addr, src_ipaddr->u8, 16);
  memcpy(send->ipv6_dest_addr, dest_ipaddr->u8, 16);
  //memset(send->for_future, 0, 6);

  send->crc = gen_crc16((uint8_t *)send, MAX_LEN - 2);
}
/*-----------------------------------------------------------------------------------*/

void 
parse_64(uint8_t* data_64, frame_struct_t *receive) 
{ // in chuoi hex 64 bytes
    int j;
    unsigned char i = 0;
    uint32_t b0, b1, b2, b3;
    uint16_t c0, c1;
    b0 = b1 = b2 = b3 = 0;
    c0 = c1 = 0;

    receive->sfd = *(data_64 + i++);
    receive->len = *(data_64 + i++);

    b0 = (uint32_t) *(data_64 + i++);
    receive->seq = b0;
    b1 = ((uint32_t) *(data_64 + i++)) << 8;
    receive->seq += b1;
    b2 = ((uint32_t) *(data_64 + i++)) << 16;
    receive->seq += b2;
    b1 = ((uint32_t) *(data_64 + i++)) << 24;
    receive->seq += b3;
    
    receive->state = *(data_64 + i++);
    receive->cmd = *(data_64 + i++);

    for(j=0; j<16; j++){
        receive->payload_data[j] = *(data_64 + i++);
    }

    for(j=0; j<16; j++){
        receive->ipv6_source_addr[j] = *(data_64 + i++);
    }

    for(j=0; j<16; j++){
        receive->ipv6_dest_addr[j] = *(data_64 + i++);
    }

    for(j=0; j<6; j++){
        receive->for_future[j] = *(data_64 + i++);
    }

    c0 = (uint16_t) *(data_64 + i++);
    c1 = (uint16_t) *(data_64 + i++) << 8;

    receive->crc = c0 + c1;
}

/*--------------------------------------------------------------------------------*/ 

uint16_t hash(uint16_t a) {
  uint32_t tem;
  tem =a;
  tem = (a+0x7ed55d16) + (tem<<12);
  tem = (a^0xc761c23c) ^ (tem>>19);
  tem = (a+0x165667b1) + (tem<<5);
  tem = (a+0xd3a2646c) ^ (tem<<9);
  tem = (a+0xfd7046c5) + (tem<<3);
  tem = (a^0xb55a4f09) ^ (tem>>16);
   return tem & 0xFFFF;
}

/*--------------------------------------------------------------------------------*/ 
unsigned int 
uart0_send_bytes(const unsigned  char *s, unsigned int len) {
  unsigned int i;
  for (i = 0; i<len; i++) {
    uart_write_byte(0, (uint8_t) (*(s+i)));
  }   
  return 1;
}
/*--------------------------------------------------------------------------------*/ 
unsigned int 
uart1_send_bytes(const unsigned  char *s, unsigned int len) {
  unsigned int i;
  for (i = 0; i<len; i++) {
    uart_write_byte(1, (uint8_t) (*(s+i)));
  }   
  return 1;
}