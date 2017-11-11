/*******************************************************************
$Header: ringbuf.h 1.0 Sun 07-23-2000 10:36:48 pm $

This is the header file for ringbuf.c which contains functions
for a circular buffer, for character storage and retrieval.
********************************************************************/

#ifndef _RINGBUF_H_	   /*+ Avoid multiple inclusion +*/
#define _RINGBUF_H_

#include <stdio.h>
#include <stdlib.h>

/*+ Define a BYTE if this is not already done +*/
#ifndef BYTE
#define BYTE unsigned char
#endif

/*+ Set an absolute maximum buffer size +*/
#define MAX_RINGBUF_SIZE 4096

/*+ This is the structure definition of RINGBUF +*/
typedef struct _RINGBUF_
{
  BYTE *buf;   /*+ Pointer to the created buffer +*/
  int spos;    /*+ Store position +*/
  int rpos;	   /*+ Retrieval position +*/
  int size;	   /*+ Defined ringbuffer size +*/
  size_t cnt;	   /*+ Number of bytes actually stored +*/
} RINGBUF;

/*+ Create a ringbuffer with the requested size +*/
RINGBUF *ringbuf_create(int size);

/*+ Store a byte in the ringbuffer +*/
void ringbuf_put(RINGBUF *b, BYTE c);

/*+ Get a byte from the ringbuffer +*/
int ringbuf_get(RINGBUF *b, BYTE *c);

/*+ Peek ahead and Get a byte from the ringbuffer +*/
int ringbuf_peek(RINGBUF *b, BYTE *c);

/*+ Delete the ringbuffer from memory +*/
void ringbuf_delete(RINGBUF *buf);

/*+ Return the number of available bytes in the ringbuffer +*/
size_t ringbuf_cnt(RINGBUF *b);

#endif


