/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Final Thesis                       |
| Version: 1.0                                                      |
| Author: ng.sontung.1995@gmail.com                                 |
| Date: 09/2017														|
| HW Support: CC2538DK 												|
|																	|
| @Compile: gcc test.c util.c aes.c -o test                         |
|-------------------------------------------------------------------|*/

#include "my-include.h"
#include "test.h"
#include "util.h"
#include "aes.h"
#include "stdint.h"
#include <stdio.h>

#include <math.h>
#include <string.h>

#include "test_cli.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...)    printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define TIMEOUT_PRINT_ALL 10

/*-----------------------------------------------------------------|*/
#define MAXBUF 64   //Max length of buffer 
#define PORT "3000"    //The port on which to listen for incoming data 

struct  pollfd fd;
int     res;
#define TIMEOUT_ACK 3000

static  int     rev_bytes;
static  struct  sockaddr_in6 rev_sin6;
static  int     rev_sin6len;

static  char    rev_buffer[MAXBUF];
static 	char 	data_rev[MAXBUF];
static  char    rev_buffer_cpy[MAXBUF];

static  int     port;

static  char    dst_ipv6addr[LEN_CHAR_IPV6];

static  char    str_port[5];
static  char    cmd[20];
static  char    arg[32];

static frame_struct_t   receive, send_device, send_encrypted;
static frame_struct_t   *receive_ptr;

static  time_t rawtime;
static  time_t time_begin;
static  time_t time_end;
static  time_t current_time;

static  struct tm * timeinfo;

static  uint16_t  hash_a, hash_b;
static  uint8_t   ipv6_bor[16] = {0xaa, 0xaa, 0, 0, 0, 0, 0, 0,\
                                              0, 0, 0, 0, 0, 0, 0, 1};
static  uint32_t  frame_counter;

static  uint8_t   ipv6_new_des[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t   my_device_pos;
node_t  my_device[MAX_DEVICE + 1];
node_t  node_alt;

/*-----------------------------------------------------------------|*/

int main(int argc, char* argv[])
{ 
    int sock;
    int status, i;
    struct addrinfo sainfo, *psinfo;
    struct sockaddr_in6 sin6;
    int sin6len;
    char buffer[MAXBUF];
    char str_app_key[32];
    unsigned char byte_array[16];
    uint8_t data[16] = "I<3U_Yori-Phuong";
    int seq = 0;
    //uint8_t device_ipaddr[LEN_CHAR_IPV6]; 

    sin6len = sizeof(struct sockaddr_in6);

    sprintf(buffer,"led_off");
    port = 3001;
    sprintf(dst_ipv6addr,"aaaa::212:7401:1:101");

    if(argc < 4) {
        printf("Specify an IPv6 addr or port number or Cmd \n"), exit(1);
    }

    else if (argc==4) {
        sprintf(dst_ipv6addr,"%s",argv[1]);      
        strcpy(str_port,argv[2]);
        strcpy(cmd,argv[3]);
        port = atoi(str_port);
        sprintf(buffer,"%s",cmd);

        /* REQ-TYPE*/
        if (strcmp(cmd,CLI_LED_ON)==0) {
            printf("led_on\n");
            prepare2send(&send_device, ipv6_bor, ipv6_new_des, \
                                    seq, STATE_NORMAL, REQUEST_LED_ON, data);
        }
        else if (strcmp(cmd,CLI_LED_OFF)==0) {
            printf("led_off\n");
            prepare2send(&send_device, ipv6_bor, ipv6_new_des, \
                                    seq, STATE_NORMAL, REQUEST_LED_OFF, data);
        } 
        else {
          printf("Unknown cmd \n");
          exit(1);
        }  
    }

    encrypt_cbc((uint8_t *)&send_device, (uint8_t *)&send_encrypted, \
              (const uint8_t *) key_begin, (uint8_t *)iv); // <<< Key begin

//     struct frame_struct_t {
//     uint8_t     sfd;
//     uint8_t     len;
//     uint32_t    seq;
//     uint8_t     state;
//     uint8_t     cmd;
//     uint8_t     payload_data[16];
//     uint8_t     ipv6_source_addr[16];
//     uint8_t     ipv6_dest_addr[16];
//     uint8_t     for_future[6];
//     uint16_t    crc;    
// }__attribute__((packed, aligned(1)));       

    sock = socket(PF_INET6, SOCK_DGRAM,0);

    memset(&sin6, 0, sizeof(struct sockaddr_in6));
    sin6.sin6_port = htons(port);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = in6addr_any;

    status = bind(sock, (struct sockaddr *)&sin6, sin6len);

    if(-1 == status)
        error("bind"), exit(1);

    memset(&sainfo, 0, sizeof(struct addrinfo));
    memset(&sin6, 0, sin6len);

    sainfo.ai_flags = 0;
    sainfo.ai_family = PF_INET6;
    sainfo.ai_socktype = SOCK_DGRAM;
    sainfo.ai_protocol = IPPROTO_UDP;
    status = getaddrinfo(dst_ipv6addr, str_port, &sainfo, &psinfo);

    status = sendto(sock, &send_encrypted, sizeof(send_encrypted), 0,(struct sockaddr *)psinfo->ai_addr, sin6len);

    printf("\nSend REQUEST (%d bytes encrypted) to [%s]:%s\n",status, dst_ipv6addr,str_port);
    printf(".......... done\n");

    /*wait for a reply */
    fd.fd = sock;
    fd.events = POLLIN;
    
    res = poll(&fd, 1, TIMEOUT_ACK); 
    if (res == -1) {
        printf(" Error !!!\n");
    }
    else if (res == 0)   {
        printf(" Wait ACK: timeout !!!\n");
    }
    else{
        rev_bytes = recvfrom((int)sock, rev_buffer, MAXBUF, 0,(struct sockaddr *)(&rev_sin6), (socklen_t *) &rev_sin6len);
        if (rev_bytes<0) {
            perror("Problem in recvfrom \n");
            exit(1);
        }
        else if(rev_bytes == 64){
            printf("Got REPLY (%d bytes):\n",rev_bytes); 
            PRINTF(" -- FROM -- ");        
            
            memcpy(data_rev, rev_buffer, 64);
            /*PRINTF("Truoc khi giai ma:   ");
            for( i = 0; i<16; i++)
                PRINTF("%02x ",data_rev[8+i]);
            PRINTF("\r\n"); */
            decrypt_cbc((uint8_t *)rev_buffer, (uint8_t *)data_rev, key_begin, iv);

            if(check_crc16(data_rev, MAXBUF)){//data_rev
                PRINTF("CRC_true\n");      
                parse_64(data_rev, &receive);//data_rev
                PRINTF("Data receive:\n");
                PRINTF_DATA(&receive);
                if(receive.cmd == REPLY_LED_ON){
                    printf("__________Da bat relay !!!\n");

                } else if (receive.cmd == REPLY_LED_OFF){
                    printf("__________Da tat relay !!!\n");
                }
                
            }
            else PRINTF("CRC_fall\n");             
        }
    }

    PRINTF("----------------------------------------------------------------\n\n");

    shutdown(sock, 2);
    close(sock); 

     // free memory
    freeaddrinfo(psinfo);
    psinfo = NULL;

    return 0; 
}  