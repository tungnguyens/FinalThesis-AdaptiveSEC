  /*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-echo-server cc2538dk UDP Echo Server Project
 *
 *  Tests that a node can correctly join an RPL network and also tests UDP
 *  functionality
 * @{
 *
 * \file
 *  An example of a simple UDP echo server for the cc2538dk platform
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "my_leds.h"
#include "net/rpl/rpl.h"

#include "reg.h"
#include "dev/gpio.h"

#define TIMEOUT         5 * CLOCK_SECOND

static  int16_t rssi, lqi;
static radio_value_t tx_pow;
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
//static char buf_send[MAX_PAYLOAD_LEN];
static uint16_t len;
/*---------------------------------------------------------------------------*/
PROCESS(udp_echo_server_process, "UDP echo server process");
AUTOSTART_PROCESSES(&udp_echo_server_process);
/*---------------------------------------------------------------------------*/
static void get_radio_parameter(void) {
  rssi = lqi = 0;
  rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  PRINTF("RSSI = %d\n",rssi); 
  lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  PRINTF("LQI = %d\n",lqi);
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tx_pow);
  PRINTF("TX power = %d\n", tx_pow);
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  static int stt_PA_t = 0;
  static int stt_LNA_t = 0;
  static int stt_HGM_t = 0;
  char r[20];

  memset(buf, 0, MAX_PAYLOAD_LEN);
  memset(r, 0, sizeof(r));
  if(uip_newdata()) {
    my_leds_on(LEDS_BLUE);
    len = uip_datalen();
    memcpy(buf, uip_appdata, len);
    PRINTF("%u bytes from [", len);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;
    get_radio_parameter();
    sprintf(r, "RSSI = %d\n", rssi);
    printf("%s\n",r);

    stt_LNA_t = GPIO_READ_PIN(GPIO_C_BASE, 4);
    stt_PA_t = GPIO_READ_PIN(GPIO_C_BASE, 8);
    stt_HGM_t = GPIO_READ_PIN(GPIO_D_BASE, 4);
    PRINTF("\n========= **** ===========\n");
    PRINTF("LNA trc khi gui = %d - %x \n", stt_LNA_t, stt_LNA_t);
    PRINTF("PA trc khi gui = %d - %x \n", stt_PA_t, stt_PA_t);
    PRINTF("HGM trc khi gui = %d - %x \n", stt_HGM_t, stt_HGM_t);

    uip_udp_packet_send(server_conn, r, sizeof(r));
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
    PRINTF("\n========= **** ===========\n");
    
    stt_LNA_t = GPIO_READ_PIN(GPIO_C_BASE, 4);
    stt_PA_t = GPIO_READ_PIN(GPIO_C_BASE, 8);
    stt_HGM_t = GPIO_READ_PIN(GPIO_D_BASE, 4);
    PRINTF("LNA = %d - %x \n", stt_LNA_t, stt_LNA_t);
    PRINTF("PA = %d - %x \n", stt_PA_t, stt_PA_t);
    PRINTF("HGM = %d - %x \n", stt_HGM_t, stt_HGM_t);
    PRINTF("\n========= **** ===========\n");
  }
  my_leds_off(LEDS_BLUE);
  return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data)
{ 
  static struct etimer et;
  static int count = 0;
  static int stt_PA = 0;
  static int stt_LNA = 0;
  static int stt_HGM = 0;

  PROCESS_BEGIN();
  PRINTF("Starting UDP echo server\n");

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);

  etimer_set(&et, TIMEOUT);
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    else if(etimer_expired(&et)){ 
        if(count % 2 == 0){
            my_leds_on(LEDS_RED);
            stt_LNA = GPIO_READ_PIN(GPIO_C_BASE, 4);
            stt_PA = GPIO_READ_PIN(GPIO_C_BASE, 8);
            stt_HGM = GPIO_READ_PIN(GPIO_D_BASE, 4);
            PRINTF("LNA = %d - %x \n", stt_LNA, stt_LNA);
            PRINTF("PA = %d - %x \n", stt_PA, stt_PA);
            PRINTF("HGM = %d - %x \n", stt_HGM, stt_HGM);
        }
        else{
            my_leds_off(LEDS_RED);
        }
        count ++;
        etimer_restart(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
