/*
|-----------------------------------------------------------------------------|
| HCMC University of Technology                                     		  |
| Telecommunications Departments                                    		  |
| Wireless Embedded Firmware for Final Thesis                       		  |
| Version: 1.0                                                      		  |
| Author: ng.sontung.1995@gmail.com                                 		  |
| Date: 09/2017                                                     		  |
| HW support in ISM band: CC2538                                    		  |
|-----------------------------------------------------------------------------|*/
#include "my-include.h"
#include "server.h"
#include "util.h"
#include "tiny_aes.h"

/*---------------------------------------------------------------------------*/
#define TIMEOUT         1 * CLOCK_SECOND
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
/*---------------------------------------------------------------------------*/

static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *ser2bor_conn;
static char buf[MAX_LEN];
static uint16_t len;


static uip_ipaddr_t border_router_ipaddr;
static uip_ipaddr_t my_ipaddr;

//static int16_t rssi, lqi;
//static radio_value_t tx_pow;

// STATE
static uint8_t state;
static uint8_t begin_cmd;

// Frame struct
/* frame_struct_t ; */
static frame_struct_t send;//, send_encrypted;
static frame_struct_t receive;//, receive_encrypted;

static uint8_t key_begin[16] = "I<3U_Yori-Phuong";

static uint8_t  key_normal[16];

/*
static  uint8_t iv[16] =   {0x00, 0x01, 0x02, 0x03, \
                            0x04, 0x05, 0x06, 0x07, \
                            0x08, 0x09, 0x0a, 0x0b, \
                            0x0c, 0x0d, 0x0e, 0x0f};   
*/
static  uint32_t frame_counter; 

static uint8_t done = 0;

/*---------------------------------------------------------------------------*/

static void start_up(void);

static void send2bor_begin(uint8_t begin_cmd);

//static void send2bor(void);

static void tcpip_begin_handler(void); 

// static void tcpip_normal_handler(void);               
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(udp_server, "UDP SERVER");                                           //
AUTOSTART_PROCESSES(&udp_server);                                            //
/*---------------------------------------------------------------------------*/
/*
static void get_radio_parameter(void) {
  rssi = lqi = 0;
  rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  PRINTF("RSSI = %d\n",rssi); 
  lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  PRINTF("LQI = %d\n",lqi);
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tx_pow);
  PRINTF("TX power = %d\n", tx_pow);
}
*/
/*---------------------------------------------------------------------------*/
static void
start_up(void)
{
  PRINTF("__start_up_\n");

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  if(!server_conn) {
    PRINTF("udp_new server_conn error.\n");
  }
  udp_bind(server_conn, UIP_HTONS(SERVER_LISTEN_PORT));
  PRINTF("Listen port: %d, TTL=%u\n", SERVER_LISTEN_PORT, server_conn->ttl);


  PRINTF("My IPV6 address: ");  
  get_my_addrress(&my_ipaddr);
  PRINT6ADDR(&my_ipaddr);PRINTF("\n");

  PRINTF("Border router address: ");
  set_border_router_address(&border_router_ipaddr); //0xaaaa::1
  PRINT6ADDR(&border_router_ipaddr);PRINTF("\n");

  ser2bor_conn = udp_new(&border_router_ipaddr, \
                                UIP_HTONS(BORDER_ROUTER_LISTEN_PORT), NULL);
  if(!ser2bor_conn) {
    PRINTF("udp_new ser2bor_conn error.\n");
  }

  state = STATE_BEGIN;
  begin_cmd = REQUEST_JOIN;
  frame_counter = 0;

}
/*---------------------------------------------------------------------------*/
static void 
send2bor_begin(uint8_t begin_cmd)
{
  uint8_t i;
  uint8_t data[16];
  uint16_t hash_a, hash_b;  
  PRINTF("__send2bor_begin_ 0x%02x\n", begin_cmd);
  memset(data, 0, 16);
  switch(begin_cmd){
    case REQUEST_JOIN:
        for(i = 0; i<6; i++){
          send.for_future[i] = 0xff;
        }
              
        prepare2send(&send, &my_ipaddr, &border_router_ipaddr, \
                frame_counter, STATE_BEGIN, REQUEST_JOIN, data);
        //PRINTF_DATA(&send);
        AES128_ECB_encrypt((uint8_t*) &send.payload_data[0],\
            (const uint8_t*) key_begin,(uint8_t *) &send.payload_data[0]);

        uip_udp_packet_send(ser2bor_conn, &send, MAX_LEN);
        frame_counter ++;
      break;

    
    case REPLY_HASH:
          hash_a = ((uint16_t)(receive.payload_data[0]) | (uint16_t)(receive.payload_data[1] <<8));
          hash_b = hash(hash_a);
          data[0] = (uint8_t )(hash_b & 0x00FF);
          data[1] = (uint8_t )((hash_b & 0xFF00)>>8);

          for(i = 0; i<6; i++){
            send.for_future[i] = 0xff;
          }
          prepare2send(&send, &my_ipaddr, &border_router_ipaddr, \
                frame_counter, STATE_BEGIN, REPLY_HASH, data);
                AES128_ECB_encrypt((uint8_t*) &send.payload_data[0], \
                    (const uint8_t*) key_begin, (uint8_t *) &send.payload_data[0]);

          uip_udp_packet_send(ser2bor_conn, &send, MAX_LEN);
          frame_counter ++;
      break;
    
    default:
      break;
  }
}
/*---------------------------------------------------------------------------*/
/*
static void 
send2bor(void)
{
  //uint8_t i;
  uint8_t data[16];
  uint8_t data_encrypted[16];
  //uint8_t data_decrypted[16];
  //uint16_t hash_a, hash_b; 
  
  PRINTF("__send2bor_\n");
  memset(data, 0xaa, 16);

  //AES128_ECB_encrypt((uint8_t*) &data[0],\
                      (const uint8_t*) key ,(uint8_t *) &data_encrypted[0]);
  memset(&send, 0, sizeof(frame_struct_t));                    
  prepare2send(&send, &my_ipaddr, &border_router_ipaddr, \
                frame_counter, STATE_BEGIN, REQUEST_JOIN, data);//data_encrypted);

  encrypt_cbc((uint8_t *)&send, (uint8_t *)&send_encrypted, \
              (const uint8_t *) key, (uint8_t *)iv);

  //uip_udp_packet_send(ser2bor_conn, &send, MAX_LEN);
  uip_udp_packet_send(ser2bor_conn, &send_encrypted, MAX_LEN);
        
  frame_counter ++;

}
*/
/*---------------------------------------------------------------------------*/
static void
tcpip_begin_handler(void)
{
  uint8_t i;
  PRINTF("__tcpip_begin_handler_\n");
  memset(buf, 0, MAX_LEN);
  if(uip_newdata()) {
    len = uip_datalen();
    memcpy(buf, uip_appdata, MAX_LEN);

    for(i= 0; i< uip_datalen(); i++){ 
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

    AES128_ECB_decrypt((uint8_t*) &buf[8], (const uint8_t*) key_begin, \
                                (uint8_t *) &buf[8]);

    if(check_crc16((uint8_t *) buf, MAX_LEN)){
      parse_64((uint8_t *) buf, &receive);
      //PRINTF_DATA(&receive);
      switch (receive.cmd){
        case REQUEST_HASH:
          PRINTF("____receive msg__REQUEST_HASH_____\n");
          begin_cmd = REPLY_HASH;
          done = 1 ;
          break;

        case JOIN_SUCCESS:
          memcpy(&key_normal[0], &receive.payload_data[0], 16);
          PRINTF("____recieve_msg__JOIN SUCCESS_________\n");
          state = STATE_NORMAL;
          done = 0 ;
          break;

        default:
          break;
      }
    }
  }  
}
/*---------------------------------------------------------------------------*/
// static void
// tcpip_normal_handler(void)
// {

//   PRINTF("__tcpip_normal_handler_\n");
// }

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server, ev, data)                                         //
{ /*-------------------------------------------------------------------------*/
  static struct etimer et;
  static uint8_t rand_num;
  

  PROCESS_BEGIN();
  start_up();


  //rand_num = random_rand()%10 + 1;
  //PRINTF("random num = %d\n", rand_num);

  rand_num = random_rand()%5 + 1;
  etimer_set(&et, rand_num * CLOCK_SECOND);

  while(state == STATE_BEGIN){    
    if(begin_cmd == REQUEST_JOIN){
      if( done == 0 )
        send2bor_begin(begin_cmd);
      PROCESS_YIELD();
      if(etimer_expired(&et)){        
        rand_num = random_rand()%5 + 1;
        etimer_set(&et, rand_num * CLOCK_SECOND);
        done = 0;
      }
      else if(ev == tcpip_event){
        tcpip_begin_handler();
      }
    } 
    if(begin_cmd == REPLY_HASH){
      if( done == 1)
        send2bor_begin(begin_cmd);
      PROCESS_YIELD();
      if(etimer_expired(&et)){        
        rand_num = random_rand()%5 + 1;
        etimer_set(&et, rand_num * CLOCK_SECOND);
        done = 1;
      }
      else if(ev == tcpip_event){
        tcpip_begin_handler();
        //done = 0;
      }
    } 
  }


  
  while(1){
    PROCESS_YIELD();
    // if(etimer_expired(&et)){
    //   send2bor();
    //   rand_num = random_rand()%10 + 5;
    //   PRINTF("random num = %d\n", rand_num);     
    //   etimer_set(&et, rand_num * TIMEOUT);
    // }
    // else if(ev == tcpip_event){
    //   tcpip_handler();
    // }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
