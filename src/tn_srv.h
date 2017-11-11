/**************************************************************************
$Header: TN_SRV.H Mon 08-21-2000 11:10:59 am HvA V1.00 $

***************************************************************************/


#ifndef _TN_SRV_H_
#define _TN_SRV_H_

#include "ringbuf.h"

#ifndef BYTE
#define BYTE unsigned char
#endif


int get_con_data (RINGBUF *ringbuf);		/*+ Pointer to ringbuf +*/

int get_telnet_data (RINGBUF *ringbuf, int tn_sock);		/*+ Pointer to ringbuf +*/

int telnet_srv_init(char *port);		/*+ Port nr to listen on +*/

void tn_close(int socket);				/*+ Close telnet socket +*/

int check_conn_request(); 	/*+ Check for incoming connection requests +*/

int get_telnet_sock(void);  /*+ Get current telnet socket number +*/

int get_listen_sock(void);  /*+ Get current telnet listen socket +*/

void send_telnet_data(BYTE *buf, int len);		/*+ Data to send +*/

void tn_close_all(void);		/*+ Close all open sockets +*/

void tn_printf(char *format, ...);   /*+ printf for the telnet connection +*/

#endif
