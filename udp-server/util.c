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
/*--------------------------------------------------------------------------------*/ 
void 
pack_data_PZEM(struct PZEM_t *PZEM_t_p, uint8_t *data, unsigned int len){
  data[0] = 0xFF;
  // Address 4byte
  data[1] = PZEM_t_p->addr[0];
  data[2] = PZEM_t_p->addr[1];
  data[3] = PZEM_t_p->addr[2];
  data[4] = PZEM_t_p->addr[3];
  // Power Alarm 1byte
  data[5] = PZEM_t_p->power_alarm;
  // voltage_x10 2byte
  data[6] = (uint8_t) (PZEM_t_p->voltage_x10 & 0x00FF);
  data[7] = (uint8_t) ((PZEM_t_p->voltage_x10 >> 8) & 0x00FF);
  // current_x100 2byte
  data[8] = (uint8_t) (PZEM_t_p->current_x100 & 0x00FF);
  data[9] = (uint8_t) ((PZEM_t_p->current_x100 >> 8) & 0x00FF);
  // current_x100 2byte
  data[10] = (uint8_t) (PZEM_t_p->power & 0x00FF);
  data[11] = (uint8_t) ((PZEM_t_p->power >> 8) & 0x00FF);
  // energy 4byte
  data[12] = (uint8_t) (PZEM_t_p->energy & 0x000000FF);
  data[13] = (uint8_t) ((PZEM_t_p->energy >> 8) & 0x000000FF);
  data[14] = (uint8_t) ((PZEM_t_p->energy >> 16) & 0x000000FF);
  data[15] = (uint8_t) ((PZEM_t_p->energy >> 24) & 0x000000FF);
}

void 
print_64byte(uint8_t *buf, uint8_t len)
{
  uint8_t i;
  for(i= 0; i< len; i++){ 
    //PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
    buf[i] = (uint8_t) (buf[i]&0xff);
  }

  for(i=0; i<8; i++){
    PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
  }
  PRINTF("\r\n");
  for(i=8; i<24; i++){
    PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
  }
  PRINTF("\r\n");
  for(i=24; i<40; i++){
    PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
  }
  PRINTF("\r\n");
  for(i=40; i<56; i++){
    PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
  }
  PRINTF("\r\n");
  for(i=56; i<64; i++){
    PRINTF("%02x ", (uint8_t) (buf[i]&0xff));
  }
  PRINTF("\r\n");
}
/*--------------------------------------------------------------------------------*/ 
#if DEBUG==1
void 
PRINTF_DATA(frame_struct_t *frame){ 
  int j;
  
  PRINTF("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  PRINTF("FLAG = 0x%.2x\n", frame->sfd);
  PRINTF("len = %d\n", frame->len);
  PRINTF("seq = %ld\n", frame->seq);
  PRINTF("state = 0x%.2x\n", frame->state);
  PRINTF("cmd = 0x%.2x\n", frame->cmd);

  PRINTF("\npayload_data [HEX] :\n");
  for (j =0 ; j < 16; j++){
    PRINTF(" %.2x ", frame->payload_data[j]);

  }

  PRINTF("\nipv6_source_addr [HEX] :\n");
  for (j =0 ; j < 16; j++){
    PRINTF(" %.2x ", frame->ipv6_source_addr[j]);
  }

  PRINTF("\nipv6_dest_addr [HEX] :\n");
  for (j =0 ; j < 16; j++){
    PRINTF(" %.2x ", frame->ipv6_dest_addr[j]);
  }

  PRINTF("\nfor_future [HEX] :\n");
  for (j =0 ; j < 6; j++){
    PRINTF(" %.2x ", frame->for_future[j]);
  }

  PRINTF("\ncrc16 = 0x%.4x\n", frame->crc);
    PRINTF("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
}
#else /* DEBUG */
void 
PRINTF_DATA(frame_struct_t *frame){
  ;
}
#endif /* DEBUG */    