/**************************************************************************
$Header: CMD.C Thu 08-24-2000 9:18:49 am HvA V1.00 $

This file contains the command interpreter.
It will build a command from getting data from the ringbuffer
If the terminator is encountered, it will interpretate and
execute the command
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "fortify.h"
#include "agwsubs.h"
#include "ringbuf.h"
#include "tn_srv.h"
#include "cmd.h"

extern char agwpe_version[];
extern char agwpe_portinfo[];
extern char mycall[];			/*+ Callsign of this station  +*/
extern char tocall[];			/*+ Callsign of other station +*/
extern int  agwpe_status;

static char cmd[160];			/*+ Linear buffer (string) for the command +*/
static int  ptr = 0;			/*+ Pointer to postion in cmd buffer during build of command +*/


/*+ Local prototypes +*/
static int terminator(RINGBUF *buf);
static void output(char *format, ...);
static void print_usage(void);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Print command overview
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void
print_usage(void)
{
  output("\n------------------");
  output("\nAvailable commands");
  output("\n------------------");
  output("\nH = Help");
  output("\nV = AGWPE version");
  output("\nP = Port information");
  output("\nX = eXit telnet interface");
  output("\nC = Connect to station");
  output("\nD = Disconnect from station");
  output("\nR = Heard stations on port 0");
  output("\nQ = Quit program");
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Execute the command given.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int exec_cmd(char *cmd)		/*+ Pointer to the command string to execute +*/
/*+ Returns 0 on success, -1 on failure +*/
{
  char c;

  if (!(agwpe_status & CMD_MODE)) {
    printf("\nERROR: interface is not in command mode.");
	printf("\nThe interface is ignoring the command.");
  }

  strupr(cmd);

  if (strstr(cmd,"CONV")) {
    output("Going to converse mode\n");
	agwpe_status &= ~CMD_MODE;
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'V') {
    output("AGWPE version  : %s\n",agwpe_version);
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'P') {
    output("AGWPE portinfo : %s\n",agwpe_portinfo);
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'H' || cmd[0] == '?') {
    print_usage();
	return(CMD_SUCCESS);
  }

  /*--- Exit telnet interface  */
  if (cmd[0] == 'X') {
    output("Closing all connections\n");
	#ifdef TN_SRV
      tn_close_all();
	#endif
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'C') {
    sscanf(cmd,"%c %s",&c,tocall);
    output("Connecting to %s\n",tocall);
	agwpe_connect(0,tocall);
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'R') {
	agwpe_heard(0);
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'D') {
    output("Disconnecting from %s\n",tocall);
	agwpe_disconnect(0,tocall);
	return(CMD_SUCCESS);
  }

  if (cmd[0] == 'Q') {
    exit(0);
  }

  /*--- If we come here, the command was not recognized  */
  output("Unknown command %s\n",cmd);
  return(CMD_UNKNOWN);
    
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Retrieve a command from the ringbuffer. If the command building
is still in progress, this function will return a NULL pointer. If
the terminators are found, then the command will be NIL terminated
and the pointer to the command will be returned.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
char *get_cmd(RINGBUF *buf)
/*+ Returns pointer to complete command, or NULL if command was not complete yet +*/
{
  int count;		/*+ Number of bytes, retrieved from ring buffer +*/
  BYTE c;			/*+ Byte, retrieved from ring buffer +*/

  if (terminator(buf)) {
    cmd[ptr] = '\0';
	ptr = 0;
    return(cmd);
  }
  else
    count = ringbuf_get(buf,&c);
  while (count > 0) {
    cmd[ptr++] = (char) c;
	if (terminator(buf)) {
	  cmd[ptr] = '\0';		/*--- Terminate string  */
	  ptr = 0;				/*--- Reset pointer to start of string  */
	  return(cmd);
	}
	else
	  count = ringbuf_get(buf,&c);
  }
  return(NULL);			/*--- No terminator found yet  */

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Check if terminator CR is seen. If there is also a LF, it will
also be removed from the ring buffer.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int terminator(RINGBUF *buf)
/*+ Return 1 if terminator is found, 0 if not +*/
{
  int count;
  BYTE c;

  count = ringbuf_peek(buf,&c);		/*--- See if a character is available  */

  if (count != 0 && c==0x0D) {	  	/*--- If there is a CR  */
     ringbuf_get(buf,&c);			/*--- Get the CR  */
	 count = ringbuf_peek(buf,&c);	/*--- See if next char is available */
	 if (count != 0 && c==0x0A) {	/*--- If it is a LF  */
	   ringbuf_get(buf,&c);			/*--- Get the LF  */
	   return(1);		 	    	/*--- Return 'end of command found'  */
	 }
	 return(1);
  }

  return(0);						/*--- No end of command found */

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Show prompt character. This is only done when were are in command mode
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void disp_prompt(void)
{
  /*--- If not in command mode, return  */
  if (!(agwpe_status & CMD_MODE))
    return;

  /*--- We are in command mode. Print the prompt  */
  printf("\n> ");
  #ifdef TN_SRV
  if (agwpe_status & TN_CONNECTED)
    tn_printf("\n\r> ");
  #endif
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Output string to stdout and to the telnet socket
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void output(char *format, ...)
{
  va_list arg;

  #ifdef TN_SRV
  char s[180];
  #endif
 
  va_start(arg, format);

  /*--- Print to standard output  */
  vprintf(format,arg);

  #ifdef TN_SRV
  /*--- Send to telnet socket  */
  if (agwpe_status & TN_CONNECTED) {
    vsprintf(s, format, arg);
    tn_printf("%s",s);
  }
  #endif

  va_end(arg);
}

