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

static frame_struct_t 	receive, send_device;
static frame_struct_t 	*receive_ptr;

static  time_t rawtime;
static  time_t time_begin;
static  time_t time_end;
static  time_t current_time;

static  struct tm * timeinfo;

static  uint16_t  hash_a, hash_b;
static  uint8_t   ipv6_bor[16] = {0xaa, 0xaa, 0, 0, 0, 0, 0, 0,\
                                              0, 0, 0, 0, 0, 0, 0, 1};
static  uint32_t  frame_counter;

static  uint8_t   ipv6_new_des[16];

uint8_t   my_device_pos;
node_t  my_device[MAX_DEVICE + 1];
node_t  node_alt;


/*-----------------------------------------------------------------|*/
void init_main();

void process_receive_data(frame_struct_t *receive);

void process_begin(frame_struct_t *receive);

void process_normal(frame_struct_t *receive);

/*-----------------------------------------------------------------|*/
void init_main(){
    uint8_t i;
    for ( i = 0; i < MAX_DEVICE + 1; i++){
        memset(&my_device[i], 0, sizeof(node_t));
    }
    for( i=1; i < MAX_DEVICE + 1; i++){
        my_device[i].stt = i;
        my_device[i].type_device = 'S';
        my_device[i].connected = 'N';
        my_device[i].authenticated = 'N';
        my_device[i].emergency = 'N';
    }
}
/*-----------------------------------------------------------------|*/

void process_receive_data(frame_struct_t *receive){
    PRINTF("Trang thai: ");
    switch(receive->state){

        case STATE_BEGIN:
            PRINTF("STATE_BEGIN\n");
            process_begin(receive);

            break;
        case STATE_NORMAL:
            PRINTF("STATE_NORMAL\n");
            process_normal(receive);
            //node_alt.emergency = 'N';
            //memcpy(&node_alt.last_data_receive, receive, MAX_LEN);
            //node_alt.num_receive++;
            break;

        default:
            break;
    }

}


/*-----------------------------------------------------------------|*/

void process_begin(frame_struct_t *receive){
    struct addrinfo sainfo, *psinfo;    
    int status, sock;
    uint8_t data[16], i;
    uint8_t dst_ipv6addr[LEN_CHAR_IPV6];
    memset(&data, 0, 16);
    switch (receive->cmd){
        case REQUEST_JOIN:
            node_alt.last_seq = receive->seq;
            //if(node_alt.connected == 'N' || node_alt.authenticated == 'Y'){
                PRINTF("cmd = first time REQUEST_JOIN\n");
                memset(&node_alt, 0, sizeof(node_t));
                node_alt.emergency = 'N';            
                node_alt.num_receive = 0;
                node_alt.num_send = 0;
                node_alt.emergency = 'N';
                node_alt.num_emergency = 0;                

                //frame_counter = 0;
                hash_a = gen_random_num();
                PRINTF("Generate random: hash_a = 0x%04x\r\n cho ", hash_a);
                data[0] = (uint8_t )(hash_a & 0x00FF);
                data[1] = (uint8_t )((hash_a & 0xFF00)>>8);
                /*PRINTF("Truoc khi ma hoa:   ");
                for( i = 0; i<16; i++)
                    PRINTF("%02x ",data[i]);
                PRINTF("\r\n"); */
                prepare2send(&send_device, ipv6_bor, ipv6_new_des, \
                                    node_alt.num_send, STATE_BEGIN, REQUEST_HASH, data);
                AES128_ECB_encrypt(&send_device.payload_data[0], key_begin, &send_device.payload_data[0]);
                /*PRINTF("Sau khi ma hoa:   ");
                for(i = 0; i<16; i++)
                    PRINTF("%02x ",send_device.payload_data[i]);
                PRINTF("\r\n");*/
                //frame_counter++;
                //node_alt.num_send++;
                //send_packet(&send_device);
                
            // }
            // else {
            //     PRINTF("cmd = duplicate REQUEST_JOIN\n");
            //     memcpy(&send_device, &node_alt.last_data_send, MAX_LEN);
            //     send_device.seq = node_alt.num_send; // cap nhat lai seq_send
            // }

            sock = socket(PF_INET6, SOCK_DGRAM,0);
            memset(&sainfo, 0, sizeof(struct addrinfo));
            sainfo.ai_flags = 0;
            sainfo.ai_family = PF_INET6;
            sainfo.ai_socktype = SOCK_DGRAM;
            sainfo.ai_protocol = IPPROTO_UDP;
            ipv6_to_str_unexpanded(&dst_ipv6addr[0], &rev_sin6.sin6_addr);

            status = getaddrinfo(dst_ipv6addr, "3000", &sainfo, &psinfo);
            printf("status getaddrinfo = %d  ___REQUEST_JOIN",status);
            status = sendto(sock, &send_device, sizeof(send_device), 0,\
                        (struct sockaddr *)psinfo->ai_addr, sizeof(struct sockaddr_in6));
            printf("status sendto = %d  ___REQUEST_JOIN",status);
            
            node_alt.connected = 'Y';
            node_alt.num_receive++;
            node_alt.num_send++;
            memcpy(&node_alt.ipv6_addr[0], &dst_ipv6addr[0],  26);
            memcpy(&node_alt.last_data_receive, receive, MAX_LEN);
            memcpy(&node_alt.last_data_send, &send_device, MAX_LEN);
            node_alt.authenticated = 'N';
            node_alt.challenge_code = hash_a;
            //PRINTF_DATA_NODE(&node_alt);         
            /*------*/
            shutdown(sock,2);
            close(sock);
            break;

        case REPLY_HASH:
            node_alt.last_seq = receive->seq;
            if(node_alt.authenticated == 'N'){
                PRINTF("cmd = first time REPLY_HASH\n");                
                hash_a = node_alt.challenge_code;
                hash_b = (uint16_t )receive->payload_data[0] | ((uint16_t )receive->payload_data[1]<<8);            
                //PRINTF("hash_a: %04x ----- hash_b:  %04x\r\n",hash_a, hash_b);
                if (hash_b == hash(hash_a)){
                    PRINTF("SUCCESS CHALLENGE CODE <<<<<<<<<<<<<\n");
                    //memcpy(&data[0], &key_begin, 16);
                    PRINTF("Generate key16 to %s !...\n Key = ", node_alt.ipv6_addr);
                    gen_random_key_128(data);
                    for(i=0; i<16; i++){
                        PRINTF("%02x ",data[i]);
                    }
                    PRINTF("\r\n");
                    /*PRINTF("Truoc khi ma hoa:   ");
                    for(i = 0; i<16; i++)
                        PRINTF("%02x ",data[i]);
                    PRINTF("\r\n");*/
                    prepare2send(&send_device, ipv6_bor, ipv6_new_des, \
                                    node_alt.num_send, STATE_BEGIN, JOIN_SUCCESS, data);
                    AES128_ECB_encrypt(&send_device.payload_data[0], key_begin, &send_device.payload_data[0]);
                    /*PRINTF("Sau khi ma hoa:   ");
                    for( i = 0; i<16; i++)
                        PRINTF("%02x ", send_device.payload_data[i]);
                    PRINTF("\r\n");*/
                    //frame_counter++; 
                    //PRINTF("__HERE___\r\n");          
                    //send_packet(&send_device);
                    memcpy(&node_alt.key[0], data, 16);
                }
                    
            }
            else {
                PRINTF("cmd = duplicate REPLY_HASH\n");
                memcpy(&send_device, &node_alt.last_data_send, MAX_LEN);
                send_device.seq = node_alt.num_send; // cap nhat lai seq_send
            }
            sock = socket(PF_INET6, SOCK_DGRAM,0);
            memset(&sainfo, 0, sizeof(struct addrinfo));
            sainfo.ai_flags = 0;
            sainfo.ai_family = PF_INET6;
            sainfo.ai_socktype = SOCK_DGRAM;
            sainfo.ai_protocol = IPPROTO_UDP;
            ipv6_to_str_unexpanded(&dst_ipv6addr[0], &rev_sin6.sin6_addr);
            //dst_ipv6addr[25] = '\0';
            status = getaddrinfo(dst_ipv6addr, "3000", &sainfo, &psinfo);
            printf("status getaddrinfo = %d  REPLY_HASH\n",status);
            status = sendto(sock, &send_device, sizeof(send_device), 0,\
                        (struct sockaddr *)psinfo->ai_addr, sizeof(struct sockaddr_in6));
            printf("status sendto = %d  REPLY_HASH\n",status);
            // ***************************************************************
            // Tam thoi cho num_rev = last seq luc nay de tinh PRR
            node_alt.num_receive = node_alt.last_seq; //node_alt.num_receive++;

            node_alt.num_send++;
            memcpy(&node_alt.ipv6_addr[0], &dst_ipv6addr[0],  sizeof(node_alt.ipv6_addr));
            memcpy(&node_alt.last_data_receive, receive, MAX_LEN);
            memcpy(&node_alt.last_data_send, &send_device, MAX_LEN);
            node_alt.authenticated = 'Y';
            
            /*------*/
            shutdown(sock,2);
            close(sock);
            break;

        default:
            break;
    }
    
}
/*-----------------------------------------------------------------|*/
void process_normal(frame_struct_t *receive)
{
    node_alt.last_seq = receive->seq;
    node_alt.emergency = 'N';
    memcpy(&node_alt.last_data_receive, receive, MAX_LEN);
    node_alt.num_receive++;
}

/*-----------------------------------------------------------------|*/

void die(char *s) 
{ 
    perror(s); 
    exit(1); 
}

/*-----------------------------------------------------------------|*/

int main(void) 
{ 
    int sock, i;
    int status;
    struct addrinfo sainfo, *psinfo;
    struct sockaddr_in6 sin6;
    int sin6len;
    char buffer[MAXBUF];
    char str_app_key[32];
    unsigned char byte_array[16];
    struct sockaddr_in si_me, si_other;
    uint8_t device_ipaddr[LEN_CHAR_IPV6]; 

    uint8_t print_count = 0;

    printf("main\n");

    init_main(); 

    port = atoi(PORT);    

    sock = socket(PF_INET6, SOCK_DGRAM,0);
    sin6len = sizeof(struct sockaddr_in6);
    memset(&sin6, 0, sin6len);

    sin6.sin6_port = htons(port);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = in6addr_any;

    status = bind(sock, (struct sockaddr *)&sin6, sin6len);
    if(-1 == status)
        perror("bind"), exit(1);

    status = getsockname(sock, (struct sockaddr *)&sin6, &sin6len);

    
    //keep listening for data 
    time ( &time_begin );
    while(1) { 
        //printf("while\n");  
        memset(&rev_buffer, 0, MAXBUF);  
        memset(&node_alt, 0, sizeof(node_t));
        memset(&rev_buffer_cpy, 0, MAXBUF); 
        /*wait for a reply */
                
        fd.fd = sock;
        fd.events = POLLIN;
        
        res = poll(&fd, 1, 1000); 
        if (res == -1) {
            printf(" RES = -1 ... ERROR\n");
        }
        else if (res == 0)   {
            printf("...\n");
        }
        else{
            rev_bytes = recvfrom((int)sock, rev_buffer, MAXBUF+MAXBUF, 0,(struct sockaddr *)(&rev_sin6), (socklen_t *) &rev_sin6);
        

            for (i = 0; i < rev_bytes; i++)
                rev_buffer[i] = (uint8_t)(rev_buffer[i] & 0xff);
            //PRINTF("\r\n");
            time ( &rawtime );
            double seconds = difftime(rawtime, time_begin);     
            printf("Da mat %.f seconds tu khi chay chuong trinh\n", seconds);
            timeinfo = localtime ( &rawtime );
            printf ( "Current local time and date: %s\n", asctime (timeinfo) ); 
            printf("New DataComing...\nGot (%d bytes):\n",rev_bytes);
            if (rev_bytes<0) {
                perror("Problem in recvfrom \n");
                exit(1);
            }
            else if(rev_bytes == 64){

                PRINTF(" -- FROM -- ");
                memcpy(&ipv6_new_des[0], &rev_sin6.sin6_addr, 16);            
                ipv6_to_str_unexpanded(&device_ipaddr[0], &rev_sin6.sin6_addr);
                device_ipaddr[25]='\0';
                


                if(checklist_my_device(device_ipaddr) == ACCEPT){
                    memcpy(data_rev, rev_buffer, 64);
                    /*PRINTF("Truoc khi giai ma:   ");
                    for( i = 0; i<16; i++)
                        PRINTF("%02x ",data_rev[8+i]);
                    PRINTF("\r\n"); */
                    if(check_statedata_rev(rev_buffer) == STATE_BEGIN){ 
                        AES128_ECB_decrypt(&data_rev[8], key_begin, &data_rev[8]);
                        /*PRINTF("Sau khi giai ma:   ");
                        for( i = 0; i<16; i++)
                            PRINTF("%02x ",data_rev[8+i]);
                        PRINTF("\r\n"); */
                    }
                    else{
                        decrypt_cbc((uint8_t *)rev_buffer, (uint8_t *)data_rev, &node_alt.key[0], iv);
                    }   
                    if(check_crc16(data_rev, MAXBUF)){//data_rev
                        PRINTF("CRC_true\n");      
                        parse_64(data_rev, &receive);//data_rev
                        PRINTF("Data receive:\n");
                        PRINTF_DATA(&receive);
                        process_receive_data(&receive);
                        node_alt.timer = rawtime;
                        copy_node2node(&node_alt, &my_device[my_device_pos]);  
                        PRINTF_DATA_NODE(&my_device[my_device_pos]);
                    }
                    else PRINTF("CRC_fall\n");             
                }
            }
            PRINTF("----------------------------------------------------------------\n\n");
        }
        if(print_count == TIMEOUT_PRINT_ALL) {
            PRINT_ALL();
            print_count = 0;
        } else print_count++;
    }
    shutdown(sock, 2);
    close(sock); 
  
    return 0; 
}  