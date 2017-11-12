#include "util.h"
#include "aes.h"


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
          for (i=0, data=(unsigned int)0xff & *data_p++;
               i < 8; 
               i++, data >>= 1)
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

    PRINTF("Kiem tra crc ...\n");
    if( c0 == data_p[length -2] && c1 == data_p[length -1]){
        return true;
    }
    else return false;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// ma hoa 64 bytes
void 
encrypt_cbc(uint8_t* data_input, uint8_t* data_output, const uint8_t* key, const uint8_t* iv) 
{ // ma hoa 64 bytes
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
phex_8(uint8_t* data_8) 
{ // in chuoi hex 16 bytes
    unsigned char i;
    printf("\n");
    for(i = 0; i < 8; i++)
        printf("%.2x  ", data_8[i]);
    printf("\n");
}
/*---------------------------------------------------------------------------*/
void 
phex_64(uint8_t* data_64) 
{ // in chuoi hex 64 bytes
    unsigned char i;
    printf("phex_64\n");
    for(i = 0; i < 8; i++) 
        phex_8(data_64 + (i*8));
    printf("\n");
}
/*---------------------------------------------------------------------------*/
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

void copy_node2node(node_t *node_scr, node_t *node_des){
    node_des->stt = node_scr->stt;
    //PRINTF("STT: %d\r\n", node_des->stt);
    node_des->type_device = node_scr->type_device;
    //PRINTF("Type: %d\r\n", node_des->type_device);
    memcpy(&node_des->ipv6_addr, &node_scr->ipv6_addr, 26);
    //PRINTF("IPv6 Addr: %s\r\n", node_des->ipv6_addr);
    node_des->connected = node_scr->connected;
    //PRINTF("connected??? : %c\r\n", node_des->connected);
    node_des->emergency = node_scr->emergency;

    node_des->num_receive = node_scr->num_receive;
    //PRINTF("Receive: %d\r\n", node_des->num_receive);
    node_des->num_send = node_scr->num_send;
    //PRINTF("Send: %d\r\n", node_des->num_send);
    node_des->num_emergency = node_scr->num_emergency;
    //PRINTF("Emergercy: %d\r\n", node_des->num_emergency);
    node_des->last_seq = node_scr->last_seq;

    memcpy(&node_des->RSSI, &node_scr->RSSI, MAX_DEVICE +1);

    memcpy(&node_des->last_data_receive, &node_scr->last_data_receive, sizeof(frame_struct_t));

    memcpy(&node_des->last_data_send, &node_scr->last_data_send, sizeof(frame_struct_t));
    node_des->challenge_code = node_scr->challenge_code;
    //PRINTF("STT: %d", node_des->stt);
    node_des->last_pos = node_scr->last_pos;
    //PRINTF("STT: %d", node_des->stt);
    node_des->authenticated = node_scr->authenticated;
    //PRINTF("STT: %d", node_des->stt);
    memcpy(&node_des->key, &node_scr->key, 16);
    //PRINTF("STT: %d", node_des->stt);
    node_des->timer = node_scr->timer;
}

void PRINTF_DATA_NODE(node_t *node_id){
    uint8_t i;
    PRINTF(">>>>>>>>>>>------------------------------------------<<<<<<<<<<<\n");
    PRINTF(">>>>>>>>>>>------------------------------------------<<<<<<<<<<<\n");
    PRINTF("1. Thu tu Device: %d\r\n", node_id->stt);
    PRINTF("2. Device la: my Device\r\n");
    PRINTF("3. IPv6 address la: %s\r\n",node_id->ipv6_addr);
    PRINTF("4. Trang thai device: %c\r\n",node_id->connected);
    PRINTF("5. So packet da nhan: %d\r\n", node_id->num_receive);
    PRINTF("6. So packet da gui: %d\r\n", node_id->num_send);
    PRINTF("7. So packet khan cap: %d\r\n", node_id->num_emergency);
    PRINTF("8. Last seq receive: %d\r\n", node_id->last_seq);
    PRINTF("9. Last packet receive: \r\n");
    PRINTF_DATA(&node_id->last_data_receive);
    PRINTF("\r\n");
    PRINTF("10. Last packet send:  \r\n");
    PRINTF_DATA(&node_id->last_data_send);
    PRINTF("\r\n"); 
    PRINTF("11 .Challenge code: %04x",node_id->challenge_code);
    PRINTF("\r\n");
    PRINTF("12. Vi tri: %02x\r\n", node_id->last_pos);
    PRINTF("13. Xac thuc Device: %c\r\n",node_id->authenticated);
    PRINTF("14. Key cung cap cho Device: \r\n\t");
    for (i = 0; i < 16; i++)
        PRINTF("%02x ",node_id->key[i]);
    PRINTF("\r\n");
    PRINTF(">>>>>>>>>>>------------------------------------------<<<<<<<<<<<\n");
    PRINTF(">>>>>>>>>>>------------------------------------------<<<<<<<<<<<\n");

}

//-------------------------------------------------------------
void ipv6_to_str_unexpanded(char * str, const struct in6_addr * addr) {
    sprintf(str, "%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x",
        (int)addr->s6_addr[0], (int)addr->s6_addr[1],
        (int)addr->s6_addr[8], (int)addr->s6_addr[9],
        (int)addr->s6_addr[10], (int)addr->s6_addr[11],
        (int)addr->s6_addr[12], (int)addr->s6_addr[13],
        (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
    PRINTF("IPV6_TO_STR = %s\r\n", str);
}
void
prepare2send(frame_struct_t *send, uint8_t *src_ipaddr, uint8_t *dest_ipaddr,\
            uint32_t frame_counter, uint8_t state, uint8_t cmd, uint8_t *data){

    send->sfd = SFD;
    send->len = MAX_LEN;
    send->seq = frame_counter;
    send->state = state;
    send->cmd = cmd;

    memcpy(send->payload_data, data, 16);
    memcpy(send->ipv6_source_addr, src_ipaddr, 16);
    memcpy(send->ipv6_dest_addr, dest_ipaddr, 16);
    memset(send->for_future, 0, 6);

    send->crc = gen_crc16((uint8_t *)send, MAX_LEN - 2);

    //PRINTF_DATA(send);

}

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
/*---------------------------------------------------------------------------*/
uint16_t gen_random_num() {
    uint16_t random1, random2;
    random1 = rand() % 255;
    random2 = rand() % 255;    
    return (random1<<8) | (random2);   
}
/*---------------------------------------------------------------------------------*/
void gen_random_key_128(unsigned char* key){
    int i;
    unsigned char byte_array[16];
    for (i=0; i<16; i++) {
        byte_array[i] = gen_random_num() & 0xFF;
    }
    strcpy(key,byte_array); 
}
/*--------------------------------------------------------------------------------*/ 
#if DEBUG==1
void 
PRINTF_DATA(frame_struct_t *frame){ 
	int j;
	
    PRINTF("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    PRINTF("FLAG = 0x%.2x\n", frame->sfd);
	PRINTF("len = %d\n", frame->len);
	PRINTF("seq = %d\n", frame->seq);
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
/*-----------------------------------------------------------------------------------*/
bool checklist_my_device(uint8_t *device_ipaddr) 
{ 
    my_device_pos = check_my_device_list(device_ipaddr);
    if(my_device_pos){
        PRINTF("My device is the %d\n", my_device_pos);
        copy_node2node(&my_device[my_device_pos], &node_alt);
        //PRINTF_DATA_NODE(&node_alt);
        return ACCEPT;
    }      
    else{
        PRINTF("This is not my device\n");
        return DENY;
    }
}
/*-----------------------------------------------------------------------------------*/
uint8_t check_my_device_list(uint8_t *device_ipaddr){
    int c, i, j;
    static uint8_t my_device_ipaddr[LEN_CHAR_IPV6];
    static const char device[] = "device.txt";
    FILE *device_list; 

    memset(my_device_ipaddr, 0, LEN_CHAR_IPV6);

    device_list = fopen ( device, "r" );

    i=0; j=1;

    if ( device_ipaddr != NULL ) {
        while((c = fgetc(device_list)) != EOF) {
            if (c != 0x0a){
                my_device_ipaddr[i] = c;
                //PRINTF("%c",my_device_ipaddr[i]);
                i++;
            }
            else {  
                i = 0;
                //PRINTF("\nmy Device = %s\n", my_device_ipaddr);           
                if(strcmp ( my_device_ipaddr, device_ipaddr) == 0){
                    //PRINTF("vi tri Device = %d\n", j);
                    fclose (device_list );
                    return j;
                }
                j++;
            }
        }        
        fclose (device_list );            
    }
    else {
        perror ( device ); /* why didn't the file open? */
        fclose ( device_list ); 
    }
    //PRINTF("Ko thuoc MyDevice list\n");
    return 0;
}
/*-----------------------------------------------------------------------------------*/
bool check_seq(uint32_t new_seq,uint32_t old_seq){
    if(new_seq<=old_seq)
        return DENY;
    else
        return ACCEPT;
}
/*-----------------------------------------------------------------------------------*/

bool check_statedata_rev (char *rev_buffer)
{
    uint8_t i,count=0;
    for(i=56;i<62;i++){
        if(rev_buffer[i]==0xffffffff){
            count++;
        }
    }
    if(count==6){
        return STATE_BEGIN;
    }
    else{
        return 0;
    }
}
/*-----------------------------------------------------------------------------------*/
void PRINT_ALL()
{
    uint8_t i, j;
    double total_rev = 0;
    double total_last_seq = 0;

    printf("|----|---------------------------|-----|-----|-----|-----|-----|-------|----------------------------------|---------------------------------|\n");
    printf("|node|       ipv6 address        | con/| num | num | num | last| chall |             key[16]              |          Last time seen         |\n");
    printf("| id |  (prefix: aaaa::0/64)     | auth| send| recv| emer| seq | -code |              (hex)               |              (GMT+7)            |\n");
    printf("|----|---------------------------|-----|-----|-----|-----|-----|-------|----------------------------------|---------------------------------|\n");

    for(i = 1; i < MAX_DEVICE + 1; i++){
        printf("| %2d | %25s | %c/%c |%5d|%5d|%5d|%5d| 0x%04x| ",\
                my_device[i].stt, \
                my_device[i].ipv6_addr, \
                my_device[i].connected, \
                my_device[i].authenticated, \
                my_device[i].num_send, \
                my_device[i].num_receive, \
                my_device[i].num_emergency, \
                my_device[i].last_seq,\
                my_device[i].challenge_code);
        for(j=0; j<16; j++){
            printf("%02x",my_device[i].key[j]);
        }
        printf(" |     %.*s    |\n", 24, asctime (localtime ( &my_device[i].timer)));

    printf("|----|---------------------------|-----|-----|-----|-----|-----|-------|----------------------------------|---------------------------------|\n");
    }

    for(i = 1; i < MAX_DEVICE + 1; i++){
        total_rev = total_rev + (double) my_device[i].num_receive;
        total_last_seq = total_last_seq + (double) my_device[i].last_seq;
    }

    printf("\n--- >>>>>>> PRR = %.0lf / %.0lf = %lf \n", total_rev, total_last_seq, total_rev/total_last_seq);
}
/*-----------------------------------------------------------------------------------*/
void PRINTF_PZEM(frame_struct_t *frame)
{
    struct PZEM_t PZEM_data_t;
    uint8_t j;

    PRINTF("\npayload_data [HEX] :\n");
    for (j =0 ; j < 16; j++){
        PRINTF(" %.2x ", frame->payload_data[j]);

    }
    PRINTF("\n");

    PZEM_data_t.addr[0] = frame->payload_data[1];
    PZEM_data_t.addr[1] = frame->payload_data[2];
    PZEM_data_t.addr[2] = frame->payload_data[3];
    PZEM_data_t.addr[3] = frame->payload_data[4];

    PZEM_data_t.power_alarm = frame->payload_data[5];
    
    PZEM_data_t.voltage_x10 = (uint16_t)frame->payload_data[6] | (uint16_t) (frame->payload_data[7] << 8);

    PZEM_data_t.current_x100 =(uint16_t)frame->payload_data[8] | (uint16_t) (frame->payload_data[9] << 8);
 
    PZEM_data_t.power = (uint16_t)frame->payload_data[10] | (uint16_t) (frame->payload_data[11] << 8);

    PZEM_data_t.energy = 0;
    PZEM_data_t.energy |= (uint32_t)frame->payload_data[12];
    PZEM_data_t.energy |= ((uint32_t)frame->payload_data[13]) << 8;
    PZEM_data_t.energy |= ((uint32_t)frame->payload_data[14]) << 16;
    PZEM_data_t.energy |= ((uint32_t)frame->payload_data[15]) << 24;

    PRINTF(" PZEM_data_t: \n");
    PRINTF(" 1. Address: %d.%d.%d.%d\n", \
        PZEM_data_t.addr[0], PZEM_data_t.addr[1], PZEM_data_t.addr[2], PZEM_data_t.addr[3]);
    PRINTF(" 2. Power_alarm: %d kW\n", PZEM_data_t.power_alarm);
    PRINTF(" 3. Voltage_x10: %d V\n", PZEM_data_t.voltage_x10);
    PRINTF(" 4. Current_x100: %d A\n", PZEM_data_t.current_x100);
    PRINTF(" 5. Power: %d W\n", PZEM_data_t.power);
    PRINTF(" 6. Energy: %d Wh\n", PZEM_data_t.energy);
    PRINTF("\n");
}