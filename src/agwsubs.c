/*******************************************************************
$Header: agwsubs.c 1.0 Sun 07-23-2000 10:08:34 pm $

AGW Packet Engine subroutines
These are devellopped under DJGPP environment

********************************************************************/

/*+ Default include files +*/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <pc.h>			/*+ For kbhit(), which is not ANSI unfortunately +*/
#include <conio.h>		/*+ For getch(), which is not ANSI unfortunately +*/

/*+ The AGWPE include files +*/
#include "fortify.h"
#include "ringbuf.h"
#include "agwhdr.h"
#include "agwpe_if.h"
#include "agwsubs.h"
#include "tn_srv.h"

/*--- Global variables  ---*/
char agwpe_version[20];		/*+ String which will be filled with the AGWPE version +*/
char agwpe_portinfo[264];	/*+ String which will be filled with AGWPE port information +*/
int agwpe_status = 0; 		/*+ Start status with everything off +*/

/*+ Strings to store mycall and tocall in. This is possible in this
    way as we only support one connection at a time +*/
char mycall[16];			/*+ Space to store mycall in +*/
char tocall[16];			/*+ Space to store other stations callsign in +*/

/*--- Local variables  ---*/
static AGWPE_HDR cmdbuffer;			 /*+ AGWPE header which will be used to send commands +*/
static AGWPE_HDR *cmd = &cmdbuffer;	 /*+ A pointer to the AGWPE header defined above +*/
static BYTE databuffer[4096];		 /*+ Buffer to store incoming data in +*/
static BYTE *data = databuffer;		 /*+ A pointer to the buffer, defined above +*/
static RINGBUF *ringbuf;    	 	 /*+ Ringbuffer to store incoming data  +*/


/*--- Local prototypes  */
static int store_version(BYTE *d);	   	  /*+ Store version string +*/
static char agwhdr_type(AGWPE_HDR *hdr);  /*+++ Return the agwpe 'datakind' +*/
static unsigned long agwhdr_cnt(AGWPE_HDR *hdr);		/*+ Pointer to AGWPE header +*/
static void agwpe_get_portinfo(void);				/*+++ Request port information from AGWPE +*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Initialize the AGWPE interface by 
setting up tcpip interface,
allocating a ringbuffer for incoming data
get version number and port information of the AGWPE

Returns always 1 for success
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_init(char *address,    /*+ Host name or IP address to use +*/
           char *portnr)     /*+ Port number to use +*/   
/*+ returns 1 for success, 0 on failure +*/
{
  int ret;

  /*--- Initialize tcpip  */
  ret = agw_tcp_init(address,portnr);
  if (ret > 0) {
    agwpe_status |= AGWPE_ACTIVE;
    (void) atexit(agw_tcp_close);
  }
  else 
    return(0);

  /*--- Allocate and setup the ring buffer */
  ringbuf = ringbuf_create(4096);

  /*--- Initialize some local variables so they can be used in  */
  /*--- loop to test if they are initialized */
  agwpe_version[0] = '\0';
  agwpe_portinfo[0] = '\0';

  /*--- By default, get version number and port information  */
  agwpe_get_ver();
  while (agwpe_version[0] == '\0') 
    agwpe_process();

  agwpe_get_portinfo();
  while (agwpe_portinfo[0] == '\0') 
    agwpe_process();

  return(1);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get the number which is stored in the AGWPE header as the number
bytes which will follow the header
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static unsigned long 
agwhdr_cnt(AGWPE_HDR *hdr)		/*+ Pointer to AGWPE header +*/
/*+ Returns the number of bytes indicated by the header +*/
{
  return(hdr->datalen);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
This function will return the header type (datakind)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static char 
agwhdr_type(AGWPE_HDR *hdr)		/*+ Pointer to AGWPE header +*/
/*+ Returns the 'datakind' indicated by the header +*/
{
  return(hdr->datakind);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
This function will calculate the AGWPE version and store
it in the global string
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int 
store_version(BYTE *d)	/*+ Pointer to buffer, containing the data  +*/
/*+ Returns 1 for success, 0 on failure +*/
{
  int highver;
  int lowver;								  

  if (d == NULL)
    return(0);

  highver = (int) (d[1]*256+d[0]);	   
  lowver = (int) (d[5]*256+d[4]);
  sprintf(agwpe_version,"%d.%d",highver,lowver);
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Inquire AGWPE for version number. The version number will
be stored by agw_process in the global string agwpe_version.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void 
agwpe_get_ver(void)
{
  /* Ask for AGWPE Version Info ('R' frame) */
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind='R';									  
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Request AGWPE port information. agw_process will store
it in the global string agwpe_portinfo;
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void 
agwpe_get_portinfo(void )
{
  /*--- Ask for AGWPE Port information ('G' frame) */
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind='G';									  
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Register a callsign at AGWPE
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_register(char *callsign)  /*+ Callsign to register +*/
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind='X';
  strcpy(cmd->callfrom,callsign);
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));

  /*--- Return failure or success */
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Unregister the given callsign from AGWPE

Returns always 1 for success
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* History: 20000713 V1.0 */
int
agwpe_unregister(char *callsign    /*+ Callsign to unregister +*/
                           )
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind='x';
  strcpy(cmd->callfrom,callsign);
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Enables the monitor function. There will be no confirmation,
but all incoming 'S','U'and 'I' frames will be send to the application.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_monitor(void)
/*+ Always returns 1 on success +*/
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind='m';
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Start a connection to another station
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_connect(int port, char *callsign)
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind = 'C';
  cmd->port = port;	
  cmd->pid = 0xF0;
  strcpy(cmd->callfrom,mycall);
  strcpy(cmd->callto,callsign);
  printf("agw_connect(%d,%s,%s)",port,mycall,callsign);
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Disconnect station
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_disconnect(int port, char *callsign)
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind = 'd';
  cmd->port = port;	
  cmd->pid = 0xF0;
  strcpy(cmd->callfrom,mycall);
  strcpy(cmd->callto,callsign);
  printf("agw_disconnect(%d,%s,%s)",port,mycall,callsign);
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Ask for list of heard stations
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_heard(int port)
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind = 'H';
  cmd->port = port;	
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  return(1);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Send data over the connection to another station
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwpe_send_data(int port, char *callsign, char *data)
{
  memset(cmd,0,sizeof(AGWPE_HDR));
  cmd->datakind = 'D';
  cmd->port = port;	
  cmd->pid = 0xF0;
  strcpy(cmd->callfrom,mycall);
  strcpy(cmd->callto,callsign);
  cmd->datalen = strlen(data);
  agw_tcp_send((BYTE *)cmd,sizeof(AGWPE_HDR));
  agw_tcp_send((BYTE *)data,cmd->datalen);
  return(1);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Print header part of an received S,U,I or T frame
by determining were the CR is, and the print everything
in front of this as a regular string, terminated with a '\n'
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000716 V1.0 */
void
agwpe_printhdr(BYTE *data)
{
  BYTE *p = NULL;

  p = data;
  while (*p != 0x0D)
    printf("%c",(char)*p++);
  printf("\n");
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Print data=r part of an received S,U,I or T frame
by determining were the CR is, and the print everything
after this, terminated with a '\n'
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000716 V1.0 */
void
agwpe_printdata(BYTE *p, 	/*+ Pointer to data to print +*/
 		        int len)   	/*+ Number of databytes to print +*/
{
  int i = 0;

  /*--- Check if there is anything to print  */
  if (len == 0) {
    return;
  }

  /*--- If there is a CR, then point just after that  */
  /*--- This was only intended to be used in a monitored frame  */
//  p = strchr((char *)data,0x0D);  /*--- Is there a Carriage Return ? */
//  if (p) {				  /*--- Yes, found CR  */
//	p++; 				  /*--- Point to next character  */
//  }

  /*--- Print the data  */
  for (i=0 ; i<len ; i++) {
    printf("%c",*p);
	if ( (*p == 0x0D) && (*(p+1) != 0x0A))
	  printf("\n");
	p++;
  }

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Return the length of the data, as indicated in the AGWPE header like
1:Fm PA3BTL-1 To BEACON <UI pid=F0 Len=64 >[10:34:24]
will return 64

Returns length of the following data
If the string "Len=" is not found, function will return 0.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000716 V1.0 */

int 
agwpe_datalen(BYTE *data)		/*+ Pointer to incoming message +*/
{
  char *p = NULL;
  int len = 0;

  p = strstr((char *) data,"Len=");
  if (p) {
    len = atoi(p+4);
	return(len);
  }
  else
    return(0);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Process all incoming data from AGWPE
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000714 V1.0 */
void
agwpe_process(void)
{
  static AGWPE_HDR agwheader; 	 /*+ Incoming AGWPE header +*/
  static AGWPE_HDR *hdr = &agwheader;  /*+ Pointer to header defined above +*/
  size_t cnt = 0;		/*+ Number of databytes which will follow the header +*/
  char type;
  int  datalen = 0;
  int  ret = 0;
  char tempstr[256];
  int  i;

  /*--- Check if there is incoming TCP data. If so, store it in the ringbuffer */
  get_tcp_data(ringbuf);

  /*--- As long as there is a agwpe header in the ringbuffer  */
  while (agwhdr_available(ringbuf))
  {
    ret = agwhdr_get(ringbuf,hdr);     /*--- Retrieve the header */
	if (ret <= 0)
	  break;
	type = agwhdr_type(hdr);

	/*--- Number of databytes, following header  */
	cnt = agwhdr_cnt(hdr);			
	if (cnt > 0)
	{
	  /*--- Get specified number of databytes and store it in local buffer */
	  agwdata_get(ringbuf,cnt,data);  
    }

    // Do something with the data

    switch(hdr->datakind) {

	  /*--- Connection  */
      case 'C': agwpe_printdata(data,cnt);
	            #ifdef TN_SRV
				  tn_printf("%s\r\n",data);
				#endif
				/*--- Set flag that we are connected to another station  */
				agwpe_status |= STATION_CONNECTED;
				/*--- Go out of command mode in converse mode  */
				agwpe_status &= ~CMD_MODE;			
	            break;

      /*--- Disconnection  */
      case 'd': agwpe_printdata(data,cnt);
	  			#ifdef TN_SRV
				  tn_printf("%s\r\n",data);
				#endif
				/*--- Reset flag as we are not connected to another station anymore  */
				agwpe_status &= ~STATION_CONNECTED;
				/*--- Go out of converse in command mode  */
				agwpe_status |= CMD_MODE;			
	            break;

	  /*--- Data frame  */
	  case 'D':	memset(tempstr,'\0',256);
				memcpy(tempstr,data,cnt);
				agwpe_printdata(tempstr,cnt);
				#ifdef TN_SRV
  				  tn_printf("\r\n<%s>\r\n",tempstr);
				#endif
				break;

	  /*--- Heard stations list  */
	  case 'H':	i = 0;
				/*--- Empty heardlist entry ?  */
				if (data[0] == ' ' && data[1] == ' ')
				  break;
				/*--- Print station, data and time information  */
				while (isprint(data[i])) {
				  printf("%c",data[i]);
				  #ifdef TN_SRV
				    tn_printf("%c",data[i]);
				  #endif
				  i++;
				}
				printf("\n");
				#ifdef TN_SRV
				  tn_printf("\r\n");
				#endif
				break;


                /*--- version number response  */
      case 'R': store_version(data);
                break;

                /*--- port information response  */
      case 'G': strcpy(agwpe_portinfo, (char *)data);
                break;

                /*--- registration response  */
      case 'X': if (*data == 0)
                  printf("Failed to register %s\n",hdr->callfrom);
                else
                  printf("Successfully registered %s\n",hdr->callfrom);
                break;

      case 'S':
	  case 'I':										 
	  case 'U':	/*--- Monitor frames  */
	  			printf("\n--------------------------------------------\n");
				agwpe_printhdr(data);
				datalen = agwpe_datalen(data);
				agwpe_printdata(data,datalen);
				break;

	  case 'T': /*--- Frames which are sent by our AGWPE engine  */
	  			/*--- Don't print these frames  */
	  			break;		

                /*--- Unimplemented response  */
      default:  printf("Unimplemented header type %c\n",hdr->datakind);
                break;
    }					   

	/*--- See if more data is coming in  */
    get_tcp_data(ringbuf);
  }

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Get AGWPE header out of the ringbuffer
Space for the header has to be allocated before calling this function.

Returns 1 on success, 0 if no header was available
++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
agwhdr_get(RINGBUF *buf,   /*+ Pointer to ringbuffer to get data from +*/
           AGWPE_HDR *hdr  /*+ Pointer to header to store data in +*/)
{
  int i;
  BYTE c;
  BYTE *p = (BYTE *)hdr;

  /*--- Check if memory was allocated  ---*/
  if (hdr == NULL) {
    perror("No AGWPE header allocated");
	exit(EXIT_FAILURE);
  }

  /*--- Check if there is a header available  ---*/
  if (ringbuf_cnt(buf) < sizeof(AGWPE_HDR)) 
  {
    return(0);		/*--- There was no header available  */
  }

  /*--- If a header is available, get it out  ---*/
  if (agwhdr_available(buf)) 
  {
    for (i=0 ; i < (int)sizeof(AGWPE_HDR) ; i++) {
      ringbuf_get(buf,&c);
      p[i] = (BYTE)c;
    }
    return(1);
  }

  return(0);	// Just in case
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description: Get data from the console, and handle it as if it
came from the telnet client.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
get_con_data (RINGBUF *ringbuf)		/*+ Pointer to ringbuf +*/
/*+ Returns number of bytes received (>=0) +*/
{
  #define BUFSIZE 256 	/*+ This is a static size for now +*/
  BYTE buf[BUFSIZE];    
  int t = 0;         
  int i;
  int c;

  if (!kbhit())
    return(0);

  /*@+matchanyintegral@*/  /*--- Avoid LCLINT warning  */

  /*--- Store incoming data in own ring buffer  ---*/
  t = 0;
  while (kbhit() && (c=getch()) != EOF) {
    ringbuf_put(ringbuf,c);
	buf[t++] = c;
  }
  buf[t] = '\0';

  /*--- Echo incoming data to standard output  */
  for (i=0 ; i<t ; i++) {
    /*--- Check for the special character to go back to command mode  */
	if (buf[i] == 0x01) {			/*--- CTRL-C will go back to cmd mode  */
	  agwpe_status |= CMD_MODE;		/*--- Set command mode flag */
	  return(t);					/*--- Return now  */
	}
    printf("%c",buf[i]);	
	#ifdef TN_SRV
	  tn_printf("%c",buf[i]);		/*--- Echo back to telnet client  */
	#endif
  }

  return(t); 	/*--- Return number of bytes received  */
  /*@=matchanyintegral@*/	/*--- Restore original LCLINT value */
}


