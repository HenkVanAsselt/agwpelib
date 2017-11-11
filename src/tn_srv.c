/***************************************************************
$Header: /djgpp/sv2agw/tn_srv.c 1.0 Mon 08-21-2000 3:53:22 pm $

Telnet interface for AGWPE/TCP
****************************************************************/

#ifdef TN_SRV

/*+ Normal includes  +*/
#include <stdio.h>
#include <stdlib.h>

/*+ Includes for network functionality  +*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef DJGPP
#include <lsck/lsck.h>
#else
#define lsck_perror perror
#endif

#include <fcntl.h>		/*+ For fnctl() -> non-blocking  +*/


/*+ Include own header files  +*/
#include "fortify.h"
#include "ringbuf.h"
#include "tn_srv.h"
#include "agwsubs.h"		/*--- For STATUS definitions  */

/*+ Local variables +*/
static int telnet_sock;				/*+ Socket number which will be assigned to telnet client +*/
static int listen_sock;

/*+ External variables +*/
extern int agwpe_status;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Initialization function for listening on a port (23) for incoming
connection requests.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
telnet_srv_init(char *port)		/*+ Port nr to listen on +*/
{
  struct sockaddr_in in;

  memset ( &in, 0, sizeof ( struct sockaddr_in ) );

  listen_sock = socket ( AF_INET, SOCK_STREAM, 0 );

  in.sin_family = AF_INET;
  in.sin_addr.s_addr = INADDR_ANY;
  in.sin_port = htons ( atoi ( port ) );

  /*--- The `bind()' function assigns a local socket-address to socket S that */
  /*--- has no local socket-address assigned.  When a socket is created with */
  /* Always check return values! */
  if (bind(listen_sock, (struct sockaddr *) &in, sizeof (struct sockaddr_in)) == -1) {
      lsck_perror("bind");
      exit(1);
  }
  
  /*--- Set the maximum number of connection that can wait to be handled by accept */
  /*--- Always check return values! */
  if (listen (listen_sock, 1) == -1) {
      lsck_perror("listen");
      exit(1);
  }

  /*--- Use fcntl to accept calls in a non-blocking way wo we still
        can use CTRL+C or do something else */
  fcntl(listen_sock, F_SETFL, O_NONBLOCK);
  printf("\nListening on port %s for incoming connection requests ",port);
  agwpe_status |= TN_LISTENING;
  
  return(0);

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Check if a connection request is made to the socket we listen on
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
check_conn_request(void) 	
/*+ Returns TN_DISCONNECTED if no request was made, return TN_CONNECTED if a conncetion was received +*/
{
  size_t len;
  struct sockaddr_in peer_in;

  /*--- Check if we are in listening status */
  if (!(agwpe_status & TN_LISTENING)) {
    printf("\n Not listening");
	return(agwpe_status);
  }

  /*--- Listen on port 23  */
  /*--- The `accept()' function returns the first completed connection from the */
  /*--- the pending connection queue form a listening socket. */
  len = sizeof ( struct sockaddr_in );
  telnet_sock = accept ( listen_sock, (struct sockaddr *)&peer_in, &len );

  /*--- Do we have a connection or is it actually an error? */
  /*--- If marked non-blocking and no pending connections are present  */
  /*--- it returns -1 and set ERRNO to EWOULDBLOCK. */
  if (telnet_sock == -1) {
    if (errno == EWOULDBLOCK) {
      return(agwpe_status);	/*--- No connection request made */
    } else {
      lsck_perror("check_con_request()");
      return(agwpe_status);
    }
  }

  /*--- If we come here, we have an incoming connection  */
  printf ("Connection made from %s:\n", inet_ntoa(peer_in.sin_addr));
  agwpe_status &= ~TN_LISTENING;	/*--- Reset listening flag */
  agwpe_status |= TN_CONNECTED;		/*--- We have a connection now  */
  tn_printf("Welcome to PA3BTL telnet interface to SV2AGW's PE Engine\r\n");
  tn_printf("Enter ? for help\r\n");
  tn_printf("> ");
  agwpe_status |= CMD_MODE;			/*--- Go in command mode  */
  return(agwpe_status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Get data from the telnet client socket and store it
in the ring buffer. The socket number used is a global variable.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
get_telnet_data (RINGBUF *ringbuf,  /*+ Pointer to ringbuf to store data in +*/
				 int tn_sock) 		/*+ Socket to get data from +*/
/*+ Returns number of bytes received (>=0) or -1 if socket is closed +*/
{
  #define BUFSIZE 256 	/*+ This is a static size for now +*/
  BYTE buf[BUFSIZE];    
  int t = 0;         
  int i;
  size_t size = (size_t)(BUFSIZE-1);

  /*@+matchanyintegral@*/  /*--- Avoid LCLINT warning  */
  /* Do receive as non-blocking */
  fcntl(telnet_sock, F_SETFL, O_NONBLOCK);
  t = recv ( tn_sock, buf, (size_t) size, 0 );
  if (t == -1 && errno == EWOULDBLOCK)
    return(0);	  /*--- No bytes received */
  if (t == 0)
    return(-1);	  /*--- Disconnected  */

  buf[t] = '\0';


  /*--- Store incoming data in own ring buffer  ---*/
  for (i=0 ; i<t ; i++) {
    ringbuf_put(ringbuf,buf[i]);
  }

  /*--- Echo incoming data to standard output  */
  for (i=0 ; i<t ; i++) {
    /*--- Check for the special character to go back to command mode  */
	if (buf[i] == 0x03) {			/*--- CTRL-C will go back to cmd mode  */
	  agwpe_status |= CMD_MODE;		/*--- Set command mode flag */
	  return(t);					/*--- Return now  */
	}
    printf("%c",buf[i]);			/*--- Echo to screen  */
	tn_printf("%c",buf[i]);			/*--- Echo back to telnet client  */
  }

  return(t); 	/*--- Return number of bytes received  */
  /*@=matchanyintegral@*/	/*--- Restore original LCLINT value */
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Send telnet data over the current connection. This assumes that
only 1 connection exists at a time
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void send_telnet_data(BYTE *buf,	/*+ Pointer to buffer +*/
					  int len)		/*+ Number of bytes to send +*/
{
  /*--- Check if really a telnet client is connected  */
  if (!(agwpe_status & TN_CONNECTED)) 
    return;

  /*--- Echo back to telnet client  */
  if ( send ( telnet_sock, buf, len, 0 ) <= 0 ) {
    perror ( "agw_telnet_send()" );
    exit (EXIT_FAILURE);
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Close socket used for the connection to AGWPE
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void 
tn_close(int socket)		/*+ Socket to close +*/
{
  /*--- Check if really a telnet client is connected  */
  if (!(agwpe_status & TN_CONNECTED)) 
    return;

  /*--- Close the socket and exit this program  ---*/
  fflush(stdout);
  (void) close (socket);
  fflush(stdout);
}


/**************************************************************************
Following functions return information, so we do not use global variables.
***************************************************************************/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get the socket number of the current telnet connection
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
get_telnet_sock(void)
/*+ Returns socket number +*/
{
  return(telnet_sock);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get the socket number of were we listen on
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
get_listen_sock(void)
/*+ Returns socket number +*/
{
  return(listen_sock);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Close all telnet sockets
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void
tn_close_all(void)
{
  tn_close(get_telnet_sock());		/*--- Close incoming telnet socket  */
  agwpe_status &= ~TN_CONNECTED;	/*--- Adjust status  */
  tn_close(get_listen_sock());		/*--- Close socket we listen on  */
  agwpe_status &= ~TN_LISTENING;	/*--- Adjust status  */

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
A printf function for the telnet connection
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void
tn_printf(char *format, ...)
{
  va_list arg;
  char s[180];
  int len;

  va_start(arg, format);

  /*--- Send to telnet socket  */
  if (agwpe_status & TN_CONNECTED) {
    vsprintf(s, format, arg);
	len = strlen(s);
    send_telnet_data(s,len);
  }

  va_end(arg);
}

#endif






