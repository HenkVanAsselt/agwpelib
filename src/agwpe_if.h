/**************************************************************************
$Header: AGEPE_IF.H Sun 08-06-2000 11:10:59 am HvA V1.00 $
***************************************************************************/


#ifndef _AGWPE_IF_H_
#define _AGWPE_IF_H_

#include "ringbuf.h"
#include "agwhdr.h"

#ifndef BYTE
#define BYTE unsigned char
#endif

/*--- Setup the TCP connection */
int agw_tcp_init(char *address, char *port);

/*--- Close the TCP connection  ---*/
void agw_tcp_close(void);

/*--- Send a command over TCP/IP connection to AGWPE  ---*/
void agw_tcp_send(BYTE *data, size_t size);

/*--- Check if AGW header is available in the ring buffer  ---*/
int agwhdr_available(RINGBUF *buf);

/*--- Get AGW header out of ring buffer  ---*/
int agwhdr_get(RINGBUF *buf, AGWPE_HDR *hdr);

/*--- Get cnt data out of ring buffer and store it in array 'data' ---*/
int agwdata_get(RINGBUF *ringbuf, size_t cnt, BYTE *data);

/*--- Get data from socket and store it in the ring buffer */
void get_tcp_data(RINGBUF *buf);


#endif
