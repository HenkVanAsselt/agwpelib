/***************************************************************
$Header: /djgpp/sv2agw/agwpe_if.c 1.0 Fri 07-21-2000 3:53:22 pm $

FILE: agwpe_if.c
DESC: Interface between libsock and AGWPE/TCP
HIST: 20000712 V0.1
****************************************************************/

/*+ Normal includes  +*/
#include <stdio.h>
#include <stdlib.h>

/*+ Includes for network functionality  +*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifdef DJGPP
#include <lsck/lsck.h>
#else
#define lsck_perror perror
#endif

#include <fcntl.h>		/*+ For fnctl() -> non-blocking  +*/

/*+ Include own header files  +*/
#include "fortify.h"
#include "ringbuf.h"
#include "agwhdr.h"
#include "agwpe_if.h"
#include "agwsubs.h"

/*+ Local variables +*/
static unsigned short port;		 /*+ TCP IP portnumber to use  +*/
static unsigned long int host;	 /*+ Host address  +*/
static int agwpe_sock;			 /*+ Socket number which will be assigned for communcation to AGWPE +*/

/*+ External variables +*/
extern int agwpe_status;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DESC: setup the tcp connection to AGWPE							   
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
agw_tcp_init(char *address,    	/*+ Host name or IP address to use +*/
             char *p)			/*+ Port number to use +*/
/*+ Returns 1 on success, 0 on failure +*/
{
  struct sockaddr_in in;
  struct hostent *hostlookup;

  printf("\nagw_tcp_init()");

  /* Host name lookup */
  port = (unsigned short) atoi ( p );
  hostlookup = gethostbyname(address);
  if (hostlookup == NULL) {
    herror("agw_tcp_init()");
    exit(EXIT_FAILURE);        
  }
  host = ( (struct in_addr *) hostlookup->h_addr)->s_addr;
  free(hostlookup);

  /* Initialize sockets */
  memset ( &in, 0, sizeof ( struct sockaddr_in ) );

  if ( ( agwpe_sock = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
    perror("agw_tcp_init");
    return(0);
  }

  in.sin_family = AF_INET;
  in.sin_port = htons ( port );
  in.sin_addr.s_addr = host;

  if ( connect ( agwpe_sock, (struct sockaddr *)&in, sizeof ( struct sockaddr_in ) ) == -1 )
    {
      perror ( "connect" );
      return(0);
    }

  return(1);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Close socket used for the connection to AGWPE
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void 
agw_tcp_close(void)
{
  static int first_time = 1;

  /*--- Check if this function is called fro the first time  */
  if (!first_time)
    return;

  /*--- Close the socket and exit this program  ---*/
  (void) close (agwpe_sock);
  first_time = 0;

#if defined(USE_FORTIFY)
  Fortify_ListAllMemory();
  // Fortify_OutputStatistics();
#endif


}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++
Send a command to the AGW Packet Engine over the tcpip socket.
This version knows of sending the header only.

Returns 1 on success,
        exits on a failure.
++++++++++++++++++++++++++++++++++++++++++++++++++*/
void 
agw_tcp_send(BYTE *data,	/*+ AGWPE header, containing the command +*/
             size_t size)	/*+ Size of the data to send (inclusive header +*/
{
  if (!(agwpe_status & AGWPE_ACTIVE)) {
    printf("agw_tcp_send(): No connection to AGWPE");
    return;
  }

  /*--- No data to send  */
  if (size == 0)
    return; 

  /*--- Send command over the socket  ---*/
  if ( send ( agwpe_sock, data, size, 0 ) <= 0 ) {
    perror ( "agw_tcp_send()" );
    exit (EXIT_FAILURE);
  }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Get data from the socket and store it
in the ring buffer. The socket number used is a global 
variable.
++++++++++++++++++++++++++++++++++++++++++++++++++*/
void
get_tcp_data (RINGBUF *ringbuf)		/*+ Pointer to ringbuf +*/
{
  #define BUFSIZE 256 	/*+ This is a static size for now +*/
  BYTE buf[BUFSIZE];    
  int t = 0;         
  int i;
  size_t size = (size_t)(BUFSIZE-1);

  /*@-unrecog@*/	/*--- Suppress LCLINT warning message  */
  /*--- Only now set socket in nonblock mode  */
  (void) fcntl(agwpe_sock,F_SETFL,O_NONBLOCK);

  /*@+matchanyintegral@*/  /*--- Avoid LCLINT warning  */
  while ((t = recv ( agwpe_sock, buf, (size_t) size, 0 )) > 0) {
    if (t == -1) {
	  if (errno == EWOULDBLOCK) 
	    continue; 
	  else
	    break;
	}
    buf[t] = '\0';
    /*--- Store incoming data in own ring buffer  ---*/
    for (i=0 ; i<t ; i++) {
	  ringbuf_put(ringbuf,buf[i]);
	}
  }
  /*@=matchanyintegral@*/	/*--- Restore original LCLINT value */
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++
Description: This function checks if ther are enough
databytes in the ringbuffer to contain a AGWPE header

Returns: 0 if no header available,
         1 if there is enough data to contain a header
++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
agwhdr_available(RINGBUF *ringbuf)	/*+ Pointer to ringbuffer to use +*/
/*+ Returns 0 if no header was available, 1 if there is enough data
in the buffer to contain a header +*/
{
  if (ringbuf_cnt(ringbuf) < sizeof(AGWPE_HDR)) {
    return(0);
  }
  else
    return(1);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Get AGWPE data out of the ringbuffer
++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
agwdata_get(RINGBUF *buf,	/*+ Pointer to the rignbuffer +*/
			size_t  cnt, 	/*+ Number of bytes to get out of buffer +*/
		    BYTE *d)	    /*+ Pointer to buffer to store the data in +*/
/*+ Returns 1 on success, 0 if no data was available +*/
{
  int i;
  BYTE c;

  if (!(agwpe_status & AGWPE_ACTIVE)) {
    printf("agwdata_get(): No connection to AGWPE");
    return(0);
  }
												 
  /*--- Check if there is a header available  ---*/
  if (ringbuf_cnt(buf) < cnt) {
    perror("agwdata_get() Less then requested bytes in ringbuf"); 
    return(0);
  }

  for (i=0 ; i < (int)cnt ; i++) {
    (void) ringbuf_get(buf,&c);  
	d[i] = c;
  }
  return((int)cnt);
}


