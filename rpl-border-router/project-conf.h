#ifndef PROJECT_ROUTER_CONF_H_
#define PROJECT_ROUTER_CONF_H_

#define TARGET_HAS_CC2592   1
#define USING_SKY			0
#define USING_UART1			0

#define MY_SEC_LVL 			7

#if USING_SKY
#undef AES_128_CONF    
#define AES_128_CONF aes_128_driver
#endif /* USING_SKY */


#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     	contikimac_driver//nullrdc_driver //contikimac_driver //

#define IEEE802154_CONF_PANID		0xACBA

#ifndef WITH_NON_STORING
#define WITH_NON_STORING 0 /* Set this to run with non-storing mode */
#endif /* WITH_NON_STORING */

#if WITH_NON_STORING
#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM 40 /* Number of links maintained at the root */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES 0 /* No need for routes */
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_NON_STORING /* Mode of operation*/
#endif /* WITH_NON_STORING */

#ifndef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#endif


#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4
#endif

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    140
#endif

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  60
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif

#define LPM_CONF_ENABLE       		0		/**< Set to 0 to disable LPM entirely */
#define LPM_CONF_MAX_PM       		0

#define CC2538_RF_CONF_TX_POWER		0xFF	// +7dBm
#define CC2538_RF_CONF_AUTOACK 		1 

/* configure MAC layer */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC 				csma_driver

#undef CSMA_CONF_MAX_FRAME_RETRIES
#define CSMA_CONF_MAX_FRAME_RETRIES 	3

#undef CSMA_CONF_MAX_NEIGHBOR_QUEUES
#define CSMA_CONF_MAX_NEIGHBOR_QUEUES 	5



/* configure LLSEC layer */
#undef ADAPTIVESEC_CONF_UNICAST_SEC_LVL
#define ADAPTIVESEC_CONF_UNICAST_SEC_LVL 	MY_SEC_LVL

#undef ADAPTIVESEC_CONF_BROADCAST_SEC_LVL
#define ADAPTIVESEC_CONF_BROADCAST_SEC_LVL 	MY_SEC_LVL

#undef LLSEC802154_CONF_USES_AUX_HEADER
#define LLSEC802154_CONF_USES_AUX_HEADER 	0

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 		14

#include "net/llsec/adaptivesec/noncoresec-autoconf.h"

/* set a seeder */
#undef CSPRNG_CONF_SEEDER
#define CSPRNG_CONF_SEEDER iq_seeder

/* Guide */
#undef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 1
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER contikimac_framer
#undef CONTIKIMAC_FRAMER_CONF_ENABLED
#define CONTIKIMAC_FRAMER_CONF_ENABLED 1
#undef CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER
#define CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER adaptivesec_framer
#undef ADAPTIVESEC_CONF_DECORATED_FRAMER
#define ADAPTIVESEC_CONF_DECORATED_FRAMER framer_802154

//To enable the secrdc_driver add this to your project-conf.h:

/* configure RADIO layer */
#include "cpu/cc2538/dev/cc2538-rf-async-autoconf.h"

/* configure RDC layer */
#include "net/mac/contikimac/secrdc-autoconf.h"

//If you like to use Practical On-The-fly Rejection (POTR), also add this to your project-conf.h:
#include "net/llsec/adaptivesec/potr-autoconf.h"

//Finally, to autoconfigure FRAMERs add:
/* configure FRAMERs */
#include "net/mac/contikimac/framer-autoconf.h"

#endif /* PROJECT_ROUTER_CONF_H_ */
