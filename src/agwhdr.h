/***********************************************************
 
 $Header: /djgpp/sv2agw/agwhdr.h 1.0 Fri 07-21-2000 4:06:09 pm $

 This header file contains the structure definition of the
 AGWPE TCP header
**************************************************************/

#ifndef __agwhdr_h__	/*+ Avoid multiple inclusions +*/
#define __agwhdr_h__

#ifndef BYTE
#define BYTE unsigned char
#endif

/*
 * Until C compilers support C++ namespaces, we use this
 * prefix for our namespace.
 */
#ifndef NAMESPACE
#define NAMESPACE(x)    _w32_ ## x     
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

typedef struct _agwpe_hdr_ {
  BYTE port;		      /*+ Radio port  +*/
  BYTE reserved[3];	      /*+ Reserved  +*/
  char datakind;	      /*+ Type of AGWPE message +*/
  BYTE reserved2;	      /*+ Reserved  +*/
  BYTE pid;			      /*+ PID of the received AGWPE message  +*/
  BYTE reserved3;	      /*+ Reserved  +*/
  char callfrom[10];	  /*+ From callsign  +*/
  char callto[10];		  /*+ To callsign +*/
  unsigned long datalen;  /*+ Indicates how much data will follow  +*/
  unsigned long user;	  /*+ ???  +*/ 
} 									 
AGWPE_HDR;

#endif
