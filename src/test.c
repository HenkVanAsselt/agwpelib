/*
 $Header: /djgpp/sv2agw/test2.c 1.0 Fri 07-21-2000 3:26:47 pm $

 * SV2AGW DJGPP libsocket interface for DJGPP.
 * This is the first test version which bypasses all recommendations
 * of LU7DID and SV2AGW, but it is a quick start to see if it works
 *
 * Copyright 1997, 1998 by Indrek Mandre
 * Copyright 2000 by Henk van Asselt, PA3BTL email: pa3btl@amsat.org
 */

/*
 * News:
 * 
 * 20000707: Initial version
 *
 */

/*+ Include file  +*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <lsck/copyrite.h>

#include <fcntl.h>          /*+ RD: For fcntl() -> non-blocking I/O +*/

#include "fortify.h"
#include "agwhdr.h"
#include "agwpe_if.h"
#include "agwsubs.h"
#include "tn_srv.h"
#include "cmd.h"

/*----------------------------------------
  variables
------------------------------------------*/

unsigned char cmd[60];	/* command wich will be send to AGWPE */
BYTE data[512];

extern char agwpe_version[];
extern char agwpe_portinfo[];

static RINGBUF *tn_ringbuf;	 		/*+ Ringbuffer for incoming telnet data +*/
int telnet_sock;

/*+ External varialbles +*/
extern int agwpe_status;	  /*+ status of connection to AGWPE +*/
extern char mycall[];		  /*+ mycall is an external string +*/
extern char tocall[];		  /*+ tocall is an external string +*/

/*----------------------------------------
FUNC: main()
DESC:
HIST: 20000708 - Initial version
------------------------------------------*/
int main( int argc, char *argv[] )
{
  int ret;						/*+ A return value  +*/
  char *cmd;
  int len;

  /*--- Show compiler  */
  #ifdef DJGPP
  printf("DJGPP compiler\n");
  #endif

  #ifdef LINUX
  printf("compiled under linux\n");
  #endif

  /*--- Print the version of libsocket used  */
  printf("%s\n",__lsck_get_version());

  /*--- Set mycall  */
  strcpy(mycall,"PA3BTL-5");

#ifdef TN_SRV
  /*--- Setup telnet socket to listen on  */  
  telnet_srv_init("23");
#endif

  /*--- Create ringbuffer for incoming telnet and console data  */
  tn_ringbuf = ringbuf_create(2048);

  /*--- Setup TCP connection to AGWPE */
  printf("\nEstablishing connection to AGWPE ");
  ret = agwpe_init("127.0.0.1","8000");
  if (ret > 0) {
    printf("Initialization successfull");
  }
  else
  {
    /*--- Failure making connection. Exit this program  */
    exit(-1);
  }

  printf("AGWPE version  : %s\n",agwpe_version);
  printf("AGWPE portinfo : %s\n",agwpe_portinfo);
  
  /*--- Test registration  ---*/
  agwpe_register(mycall);
  agwpe_process();

#if defined(USE_FORTIFY)
  Fortify_ListAllMemory();
  // Fortify_OutputStatistics();
#endif

  // Do your things here
  // agwpe_monitor();

  // Start in command mode. Even if there is no telnet client
  // Connected, we still want to do something from the command prompt
  // on the console
  agwpe_status |= CMD_MODE;

  printf("\nStarting endless loop ...\n");
  for (;;) {
#ifdef TN_SRV
    if (!((agwpe_status & TN_LISTENING) || (agwpe_status & TN_CONNECTED)))
	  telnet_srv_init("23");
    else if (agwpe_status & TN_LISTENING)
      (void) check_conn_request();
	else if (agwpe_status & TN_CONNECTED) {
	  telnet_sock = get_telnet_sock();
      ret = get_telnet_data(tn_ringbuf,telnet_sock);
  	  if (ret == -1) { 
	    tn_close_all();
  	  }
	}
#endif
	get_con_data(tn_ringbuf);
    agwpe_process();
	cmd = get_cmd(tn_ringbuf);				/*--- See if there is a command available  */
	if (cmd) { 
	  if (agwpe_status & CMD_MODE) {
	    exec_cmd(cmd);						/*--- Execute the command  */
	    disp_prompt();
	  }
	  else if (agwpe_status & STATION_CONNECTED) {
		len = strlen(cmd);
		cmd[len]='\r';
		cmd[len+1]='\0';
	    agwpe_send_data(0,tocall,cmd);
	  }
	}
  }

  /*--- Close connection  ---*/
  agwpe_unregister(mycall);
  agw_tcp_close();

#if defined(USE_FORTIFY)
  Fortify_ListAllMemory();
  // Fortify_OutputStatistics();
#endif

  return 0;
}


