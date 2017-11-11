/********************************************************************
$Header: ringbuf.c v1.0 Sun 07-23-2000 10:39:46 pm $

Functions for a circular buffer

********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "fortify.h"
#include "ringbuf.h"

/*******************************************************************
* RINGBUF CREATE AND DELETE FUNCTIONS
********************************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Create a circular buffer and initialize it
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000709 V1.0 */

RINGBUF *
ringbuf_create(int size)	 /*+ Size of the ringbuffer to create +*/
{
  RINGBUF *b = NULL;

  /*+ Also check the absolute maximum size +*/
  if (size > MAX_RINGBUF_SIZE) {
    perror("maximum ringbuf size would be  exceeded");
	exit(EXIT_FAILURE);
  }

  /*--- Allocate new ringbuffer  ---*/
  b = (RINGBUF *) calloc(1,sizeof(RINGBUF));
  if (!b) {
    perror("No memory to allocate ringbuf ringbuf_init()");
    exit(EXIT_FAILURE);
  }

  /*--- Allocate memory in the newly created ring buffer  ---*/
  b->buf = (BYTE *) calloc((size_t)size+1,sizeof(BYTE));
  if (!b->buf) {
    perror("No memory in ringbuf_init()");
    exit(EXIT_FAILURE);
  }
  b->size = size;	/*--- Store the size  ---*/

  /*--- Initialize variables  ---*/
  b->spos = 0;
  b->rpos = 0;
  b->cnt = 0;
  return(b);
}				   

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Delete a ringbuffer from mememory (regardless if it is empty or not
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 20000709 V1.0 */

void
ringbuf_delete(RINGBUF *b)		/*+ Pointer to ringbuffer to delete +*/
{
  /*--- Is there really a buffer defined ?  ---*/
  if (b==NULL){
    perror("trying to delete a non-existing ringbuffer");
	exit(EXIT_FAILURE);
  }

  /*--- Deallocate memory  ---*/
  if (b->buf)
    free(b->buf);
  free(b);
}

/*******************************************************************
* RINGBUF INPUT COMMANDS
********************************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Put a BYTE in the ringbuffer

Returns nothing. Exits when the ringbuffer is full
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 200000709 V1.0 */

void 
ringbuf_put(RINGBUF *b, 	/*+ Pointer to the buffer +*/
 		    BYTE c) 		/*+ Byte to store in the buffer +*/
{
  #ifdef RINGBUF_DEBUG
  printf("spos=%4d rpos=%4d size=%4d ",b->spos,b->rpos,b->size);
  if (!iscntrl(c))
    printf("%c",c);
  else
    printf(".");
  printf("\n");
  #endif

  if (b->spos+1 == b->rpos) {	   	/*--- Is the buffer full ?  */
    perror("ringbuffer full ");	/*--- Yes, signal error  */
  }
  else {							/*--- No,  */				   
    b->buf[b->spos] = c;			/*--- Store character  */
	b->spos++;						/*--- Increment storage pointer  */
	if (b->spos >= b->size)			/*--- Are we at the end of the circular buffer ?  */
	  b->spos = 0;					/*--- Yes, set pointer to start of buffer */
	b->cnt++;
  }
}


/*******************************************************************
* RINGBUF OUTPUT COMMANDS
********************************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get a character from the ring buffer

Returns the number of characters retrieved (0 or 1)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 200000709 V1.0 */

int
ringbuf_get(RINGBUF *b, /*+ Pointer to the ring buffer +*/
			BYTE *c)	/*+ The retrieved byte has to be stored here +*/
{
  if (b->rpos == b->size)			/*--- Do we have to wrap ?  */
    b->rpos = 0;					/*--- Yes, wrap to start of ringbuf  */
  if (b->rpos == b->spos) {			/*--- Are ther any characters in the buffer ? */
    *c = '\0';						/*--- No, Set character to NIL character  */
   	return(0);						/*--- No events in the queue */
  }	 
  else {							/*--- Yes, there are characters in the buffer  */
    b->rpos++;						/*--- Increment pointer */  
	*c = b->buf[b->rpos-1];			/*--- Get character from buffer */
	b->cnt--;
	return(1);						/*--- 1 character retrieved  */
  }
}			 


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Get the number of characters actually stored in the ring buffer
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
size_t
ringbuf_cnt(RINGBUF *b)	  /*+ pointer to the ringbuffer +*/
/*+ Returns the number of characters stored +*/
{
  return(b->cnt);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Look ahead what is the next character in the rinbuffer,
but do not adjust the pointers

Returns the number of characters retrieved (0 or 1)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* 200000709 V1.0 */

int
ringbuf_peek(RINGBUF *b,  /*+ Pointer to the ring buffer +*/
			BYTE *c)	  /*+ The retrieved byte will be stored here +*/
{
  int rpos = b->rpos;
  int spos = b->spos;
  int size = b->size;

  if (rpos == size)					/*--- Do we have to wrap ?  */
    rpos = 0;						/*--- Yes, wrap to start of ringbuf  */
  if (rpos == spos) {				/*--- Are ther any characters in the buffer ? */
    *c = '\0';						/*--- No, Set character to NIL character  */
   	return(0);						/*--- No events in the queue */
  }	 
  else {							/*--- Yes, there are characters in the buffer  */
    rpos++;							/*--- Increment pointer */  
	*c = b->buf[rpos-1];			/*--- Get character from buffer */
	return(1);						/*--- 1 character retrieved  */
  }
}			 


