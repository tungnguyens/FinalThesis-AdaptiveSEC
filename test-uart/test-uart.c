/*
 * Copyright (c) 2016, Indian Institute of Science <http://www.iisc.ernet.in>
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-test-uart Test the CC2538 UART
 *
 * Demonstrates the use of the CC2538 UART
 *
 * @{
 *
 * \file
 *         A quick program for testing the UART1 on the cc2538 based
 * platform which can be used to interface with sensor with UART
 * interface and to communicate with other development boards
 * \author
 *         Akshay P M <akshaypm@ece.iisc.ernet.in>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include <stdio.h>
#include <stdint.h>

#include "my-include.h"
#include "util.h"
#include "PZEM004T.h"

#define TIMEOUT 3 * CLOCK_SECOND
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS(cc2538_uart_demo_process, "cc2538 uart demo");
AUTOSTART_PROCESSES(&cc2538_uart_demo_process);
/*---------------------------------------------------------------------------*/
int uart0_callback(unsigned char c){
  uart_write_byte(0, c);
  //printf(" 0x%02x", c);
  return 1;
}
/*---------------------------------------------------------------------------*/
int uart1_callback(unsigned char c){
  uart_write_byte(1, c);
  //printf(" 0x%02x", c);
  return 1;
}
/*---------------------------------------------------------------------------*/
void timeout_handler(void){
	static uint8_t count = 1;
	switch(count){
		case 1:
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
			count = 1;
			break;
		default:
			//return PZEM_ERROR_VALUE;
			break;
	}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2538_uart_demo_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  uart_set_input(0, uart0_callback);
  uart_set_input(1, uart1_callback);

  etimer_set(&et, TIMEOUT);
  while(1) {
    PROCESS_YIELD();
	// if(etimer_expired(&et)){
	// 	printf("\n Send to PZEM004T:\n");
	// 	timeout_handler();				
	// 	etimer_restart(&et);
	// }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
