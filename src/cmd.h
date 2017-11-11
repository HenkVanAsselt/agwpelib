/**************************************************************************
$Header: CMD.H Thu 08-24-2000 9:20:40 am HvA V1.00 $

This is the header file for the CMD.C which contains the command
interpreter.
***************************************************************************/

#ifndef _CMD_H_
#define _CMD_H_

#include "ringbuf.h"

#define CMD_SUCCESS 0
#define CMD_UNKNOWN -1

char *get_cmd(RINGBUF *buf); 	/*+ Retrieve command from ring buffer +*/

int exec_cmd(char *cmd);		/*+ Execute a command +*/
 
void disp_prompt(void);			/*+ Display > prompt +*/

#endif

