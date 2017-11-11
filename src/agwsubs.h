/**************************************************************************
$Header: AGWSUBS.H Sun 08-06-2000 11:11:21 am HvA V1.00 $
***************************************************************************/


#ifndef __agwsubs_h__
#define __agwsubs_h__

#include "agwhdr.h"

/*+ Bit masks for AGWPE interface status +*/
#define AGWPE_ACTIVE		0x0001	/*+ AGWPE is active and connection is made +*/
#define STATION_CONNECTED	0x0002	/*+ We have a connection to another station +*/
#define TN_LISTENING    	0x0008	/*+ No tn client connected but server is listening +*/
#define TN_CONNECTED    	0x0010	/*+ A tn client is connectd +*/
#define CMD_MODE			0x0020	/*+ This interface is in command mode +*/

/*+ Initialize agwpe interface +*/
int agwpe_init(char *address, char *portnr);

/*+ Request version information from AGWPE +*/
void  agwpe_get_ver(void);

/*+ Register 'callsign' to AGWPE  +*/
int   agwpe_register(char *callsign);

/*+ Unregister 'callsign' from AGWPE  +*/
int   agwpe_unregister(char *callsign);

/*+ Enable monitored frames to be send from AGWPE +*/
int   agwpe_monitor(void);

/*+ Print header of S,U,I,T frames  +*/
void  agwpe_printhdr(BYTE *data);

/*+ Process incoming pctcp headers and data frames +*/
void  agwpe_process(void);

/*+ Connect to another station  +*/
int agwpe_connect(int port, char *callsign);

/*+ Disconnect from another station  +*/
int agwpe_disconnect(int port, char *callsign);

/*+ Ask for list of heard stations  +*/
int agwpe_heard(int port);

/*+ Send data over the connection to the other station +*/
int agwpe_send_data(int port, char *callsign, char *data);

#endif
