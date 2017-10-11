#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define TARGET_HAS_CC2592   1
#define USING_SKY			0

#define MY_SEC_LVL 			7

#if USING_SKY
#undef AES_128_CONF    
#define AES_128_CONF aes_128_driver
#endif /* USING_SKY */


#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC   contikimac_driver//nullrdc_driver //contikimac_driver //

/*--------------------------------------------------------------------------------*/
#define DEBUG_CC2538_RF		0
#define DEBUG_NULLRDC		0 // chi DEBUG loi
#define DEBUG_NONCORESEC	0
#define DEBUG_FRAMER_802154 0
#define DEBUG_MAC 			0
#define DEBUG_CSMA 			0 // csma
#define DEBUG_SICSLOWPAN	0 // vo nghia do PRINTF0
/*--------------------------------------------------------------------------------*/

#define IEEE802154_CONF_PANID		0xACBA

#define CC2538_RF_CONF_TX_POWER		0xFF	// +7dBm
/*
#if TARGET_HAS_CC2592
#define CC2538_RF_CONF_CHANNEL    	26
#else
#define CC2538_RF_CONF_CHANNEL    	26
#endif
*/

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

/*--------------------------------------------------------------------------------*/
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

/* Moi them */
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

/* disable TCP */
// #undef UIP_CONF_TCP
// #define UIP_CONF_TCP 0
/*--------------------------------------------------------------------------------*/

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

#endif /* PROJECT_CONF_H_ */