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
#include "PZEM004T.h"

#define LEN_PZEM_RESP 7

extern struct PZEM_t PZEM_data_t;
static unsigned char rx[LEN_PZEM_RESP];
static uint16_t rx_count = 0;
//static uint16_t last_rx_count = 0;
/*---------------------------------------------------------------------------*/
#define TIMEOUT         1 * CLOCK_SECOND
#define TIMEOUT_NORMAL  5 * CLOCK_SECOND

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120

#define MAX_BUF         64
/*---------------------------------------------------------------------------*/

static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *server_conn_relay;
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
static frame_struct_t send, send_encrypted;
static frame_struct_t receive;//, receive_encrypted;

static uint8_t key_begin[16] = "I<3U_Yori-Phuong";

static uint8_t  key_normal[16];


static  uint8_t iv[16] =   {0x00, 0x01, 0x02, 0x03, \
                            0x04, 0x05, 0x06, 0x07, \
                            0x08, 0x09, 0x0a, 0x0b, \
                            0x0c, 0x0d, 0x0e, 0x0f};   

static  uint32_t frame_counter; 

static uint8_t done = 0;

/*---------------------------------------------------------------------------*/

void relay_on(unsigned char relays);

void relay_off(unsigned char relays);

static void start_up(void);

static void send2bor_begin(uint8_t begin_cmd);

static void send2bor_normal(void);

static void send2bor_emergency(void);
//static void send2bor(void);

static void tcpip_begin_handler(void); 

static void timeout_normal_handler(void);

static void tcpip_normal_handler(void);               
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(udp_server, "UDP SERVER");                                           //
AUTOSTART_PROCESSES(&udp_server);                                            //
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void 
relay_on(unsigned char relays)
{
  my_leds_off(relays);
}

/*---------------------------------------------------------------------------*/
void 
relay_off(unsigned char relays)
{
  my_leds_on(relays);
}
/*---------------------------------------------------------------------------*/
int 
uart0_callback(unsigned char c)
{
    uart_write_byte(0, c);
    //printf(" 0x%02x", c);
    return 1;
}
/*---------------------------------------------------------------------------*/
int 
uart1_callback(unsigned char c)
{
    //uart_write_byte(1, c);
    //printf(" 0x%02x", c);
    static uint8_t i = 0;

  if (i == LEN_PZEM_RESP-1){
      rx[i] = c;
    printf(" 0x%02x", rx[i]);
    i=0;
    printf("\nrecv_from_PZEM\n");
    if(recv_from_PZEM(rx) == PZEM_OK_VALUE){
      rx_count ++;
      printf("ok %d\n", rx_count);      

    }
    else {
      printf("fail\n");
    }
    return 1;
  }
  else if(i>0){
    rx[i] = c;
    printf(" 0x%02x", rx[i]);
    i++;
    return 1;
  }
  if (is_PZEM_1st_byte(c)){
      rx[i] = c;
      printf(" 0x%02x", rx[i]);
    i++;
    return 1;
  }
  return 1;
}
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


  server_conn_relay = udp_new(NULL, UIP_HTONS(0), NULL);
  if(!server_conn_relay) {
    PRINTF("udp_new server_conn_relay error.\n");
  }
  udp_bind(server_conn_relay, UIP_HTONS(SERVER_RELAY_LISTEN_PORT));
  PRINTF("Listen port: %d, TTL=%u\n", SERVER_LISTEN_PORT, server_conn_relay->ttl);


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
static void 
send2bor_normal(void)
{
  uint8_t data[16];
  //uint8_t data_encrypted[16];
  PRINTF("__send2bor_normal_\n");

  //memcpy(&data[0], &key_normal[0], 16);

  pack_data_PZEM(&PZEM_data_t, data, 16);
  memset(&send, 0, sizeof(frame_struct_t));

  prepare2send(&send, &my_ipaddr, &border_router_ipaddr, \
                frame_counter, STATE_NORMAL, SEND_PZEM, data);//data_encrypted);

  encrypt_cbc((uint8_t *)&send, (uint8_t *)&send_encrypted, \
              (const uint8_t *) key_normal, (uint8_t *)iv); 

  uip_udp_packet_send(ser2bor_conn, &send_encrypted, MAX_LEN);

  frame_counter ++;
}
/*---------------------------------------------------------------------------*/
static void 
send2bor_emergency(void){
  PRINTF("__send2bor_emergency_\n");
  PRINTF("*** Mat Dien toan mien Nam ***\n");
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
  //uint8_t i;
  PRINTF("__tcpip_begin_handler_\n");
  memset(buf, 0, MAX_LEN);
  if(uip_newdata()) {
    len = uip_datalen();
    memcpy(buf, uip_appdata, MAX_LEN);

    print_64byte((uint8_t *)buf, len);

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
static void
tcpip_normal_handler(void)
{
  char data_decrypted[64];
  PRINTF("__tcpip_normal_handler_\n");
  if(uip_newdata() && uip_datalen() == MAX_LEN) {    
    len = uip_datalen();
    memcpy(buf, uip_appdata, MAX_LEN);

    //print_64byte((uint8_t *)buf, len);

    decrypt_cbc((uint8_t *) &buf[0], (uint8_t *)data_decrypted, \
              (const uint8_t *) key_begin, (uint8_t *)iv); // <<< Key begin

    print_64byte((uint8_t *)data_decrypted, len);

    if(check_crc16((uint8_t *) data_decrypted, MAX_LEN)){//data_rev
      PRINTF("CRC_true\n");      
      parse_64((uint8_t *) data_decrypted, &receive);//data_rev
      PRINTF("Data receive:\n");
      PRINTF_DATA(&receive);
      //PRINTF_DATA_NODE(&my_device[my_device_pos]);
      if(receive.cmd == REQUEST_LED_ON){
        printf("__________Yeu cau bat relay !!!\n");
        relay_on(RELAY_1);
      } else {
        printf("__________Yeu cau tat relay !!!\n");
        relay_off(RELAY_1);
      }
    }
    else PRINTF("CRC_fall\n");    
  }

}
/*---------------------------------------------------------------------------*/
static void timeout_normal_handler(void)
{
  PRINTF("__timeout_normal_handler_\n");
  static uint8_t count = 1;
  //uint8_t i;

  printf(" PZEM_data_t: \n");
  // printf(" 1. Address: %d.%d.%d.%d\n", PZEM_data_t.addr[0], PZEM_data_t.addr[1], PZEM_data_t.addr[2], PZEM_data_t.addr[3]);
  // printf(" 2. Power_alarm: %d kW\n", PZEM_data_t.power_alarm);
  // printf(" 3. Voltage_x10: %d V\n", PZEM_data_t.voltage_x10);
  // printf(" 4. Current_x100: %d A\n", PZEM_data_t.current_x100);
  // printf(" 5. Power: %d W\n", PZEM_data_t.power);
  // printf(" 6. Energy: %ld Wh\n", PZEM_data_t.energy);
  // printf(" 7. Last_data_recv: ");
  // for(i=0; i<7; i++){
  //  printf("0x%02x ",PZEM_data_t.last_data_recv[i]); 
  // }
  // printf("\n");

  switch(count){
    case 1:
      rx_count = 0;
      send_to_PZEM(PZEM_SET_ADDRESS);
      count = 2;
      break;
    case 2:
      send_to_PZEM(PZEM_POWER_ALARM); // THRESHOLD is defined in PZEM004T.h
      count = 3;
      break;
    case 3:
      send_to_PZEM(PZEM_VOLTAGE);
      count = 4;
      break;
    case 4:
      send_to_PZEM(PZEM_CURRENT);
      count = 5;
      break;
    case 5:
      send_to_PZEM(PZEM_POWER);
      count = 6;
      break;
    case 6:
      send_to_PZEM(PZEM_ENERGY);
      count = 7;
      break;
    case 7:
      if(rx_count == 6){
        send2bor_normal();
      }
      else {
        send2bor_emergency();
      }
      count = 1;
    default:
      //return PZEM_ERROR_VALUE;
      break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server, ev, data)                                         //
{ /*-------------------------------------------------------------------------*/
  static struct etimer et;
  static uint8_t rand_num;
  uint8_t i;
  static uint8_t  relay_count = 0;
  // static int stt_PA = 0;
  // static int stt_LNA = 0;
  // static int stt_HGM = 0;

  PROCESS_BEGIN();

  uart_set_input(0, uart0_callback);
  uart_set_input(1, uart1_callback);

  start_up();

  //rand_num = random_rand()%10 + 1;
  //PRINTF("random num = %d\n", rand_num);

  rand_num = random_rand()%5 + 1;
  etimer_set(&et, rand_num * CLOCK_SECOND);

  while(state == STATE_BEGIN){    
    if(begin_cmd == REQUEST_JOIN){
      if( done == 0 )
        send2bor_begin(begin_cmd);
        // stt_LNA = GPIO_READ_PIN(GPIO_C_BASE, 4);
        // stt_PA = GPIO_READ_PIN(GPIO_C_BASE, 8);
        // stt_HGM = GPIO_READ_PIN(GPIO_D_BASE, 4);
        // PRINTF("LNA = %d - %x \n", stt_LNA, stt_LNA);
        // PRINTF("PA = %d - %x \n", stt_PA, stt_PA);
        // PRINTF("HGM = %d - %x \n", stt_HGM, stt_HGM);
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
    if(etimer_expired(&et)){
      //send2bor();
      //rand_num = random_rand()%5 + 1;
      //PRINTF("random num = %d\n", rand_num);     
      etimer_set(&et, TIMEOUT_NORMAL);
      printf("My key [hex] = ");
      for(i=0; i<16; i++){
        printf("%02x ", key_normal[i]);
      }
      printf("\n");
      //send2bor_normal();
      timeout_normal_handler();



      /////////////////////////////////////
      // if(relay_count%2 == 0){
      //   printf("bat 2 relay\n");
      //   relay_on(RELAY_1);
      // //   my_leds_off(RELAY_2);
      // //   my_leds_off(RELAY_3);
      // //   my_leds_off(RELAY_4);
      // }
      // else{
      //   printf("tat 2 relay\n");
      //   relay_off(RELAY_1);
      // //   my_leds_on(RELAY_2);
      // //   my_leds_on(RELAY_3);
      // //   my_leds_on(RELAY_4);
      // }
      relay_count++;
      ////////////////////////////////////
    }
    else if(ev == tcpip_event){
      tcpip_normal_handler();
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
