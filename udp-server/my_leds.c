#include "contiki.h"
#include "reg.h"
#include "my_leds.h"
#include "dev/leds.h"
#include "dev/gpio.h"

#define LEDS_GPIO_PIN_MASK   0x03
/*---------------------------------------------------------------------------*/
void
my_leds_on(unsigned char leds)
{
#if TARGET_HAS_CC2592 == 1
	switch(leds){
		case LEDS_ORANGE:
			GPIO_WRITE_PIN(GPIO_C_BASE, 1, 1);
			break;
		case LEDS_RED:
			GPIO_WRITE_PIN(GPIO_C_BASE, 2, 2);//2);
			break;
		case LEDS_GREEN:
			GPIO_WRITE_PIN(GPIO_B_BASE, 1, 1);
			break;
		case LEDS_BLUE:
			GPIO_WRITE_PIN(GPIO_B_BASE, 2, 2);//2);
			break;
		///////////////////////////////////////////////////////
		case RELAY_1:
			GPIO_WRITE_PIN(GPIO_D_BASE, 1, 1);
			break;
		case RELAY_2:
			GPIO_WRITE_PIN(GPIO_D_BASE, 2, 2);//2);
			break;
		case RELAY_3:
			GPIO_WRITE_PIN(GPIO_D_BASE, 16, 16);
			break;
		case RELAY_4:
			GPIO_WRITE_PIN(GPIO_D_BASE, 32, 32);//2);
			break;
		///////////////////////////////////////////////////////
	}
#else 
	leds_on(leds);
#endif  
}
/*---------------------------------------------------------------------------*/
void
my_leds_off(unsigned char leds)
{
#if TARGET_HAS_CC2592 == 1
  	switch(leds){
		case LEDS_ORANGE:
			GPIO_WRITE_PIN(GPIO_C_BASE, 1, 0);
			break;
		case LEDS_RED:
			GPIO_WRITE_PIN(GPIO_C_BASE, 2, 0);
			break;
		case LEDS_GREEN:
			GPIO_WRITE_PIN(GPIO_B_BASE, 1, 0);
			break;
		case LEDS_BLUE:
			GPIO_WRITE_PIN(GPIO_B_BASE, 2, 0);
			break;
		///////////////////////////////////////////////////////
		case RELAY_1:
			GPIO_WRITE_PIN(GPIO_D_BASE, 1, 0);
			break;
		case RELAY_2:
			GPIO_WRITE_PIN(GPIO_D_BASE, 2, 0);//2);
			break;
		case RELAY_3:
			GPIO_WRITE_PIN(GPIO_D_BASE, 16, 0);
			break;
		case RELAY_4:
			GPIO_WRITE_PIN(GPIO_D_BASE, 32, 0);//2);
			break;		
		///////////////////////////////////////////////////////
	}
#else 
	leds_off(leds);
#endif 
}
/*---------------------------------------------------------------------------*/
unsigned char
my_leds_get(unsigned char port_base)
{
	if(port_base){ // Port C
		return GPIO_READ_PIN(GPIO_C_BASE, LEDS_GPIO_PIN_MASK);
	}
	else{ // Port B
		return GPIO_READ_PIN(GPIO_B_BASE, LEDS_GPIO_PIN_MASK);
	}  
}
void
my_leds_toggle(unsigned char leds)
{
#if TARGET_HAS_CC2592 == 1
	switch(leds){
		case LEDS_ORANGE:
			GPIO_WRITE_PIN(GPIO_C_BASE, 1, 1^my_leds_get(1));
			break;
		case LEDS_RED:
			GPIO_WRITE_PIN(GPIO_C_BASE, 2, 2^my_leds_get(1));
			break;
		case LEDS_GREEN:
			GPIO_WRITE_PIN(GPIO_B_BASE, 1, 1^my_leds_get(0));
			break;
		case LEDS_BLUE:
			GPIO_WRITE_PIN(GPIO_B_BASE, 2, 2^my_leds_get(0));
			break;
	}
#else 
	leds_toggle(leds);
#endif 
}


/*---------------------------------------------------------------------------*/