/*=======================================================================
 dictproc.h
 
 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

======================================================================*/
#include   <stdlib.h>
#include   <stdio.h>
#include   <string.h>

#define    MAX_PHONE    100      /* English has 46 */
#define    MAX_PRON     100      /* max # of phones in a word */
#define    MAX_ENTRY    300000
#define    MAX_STR      1024
#define    DELIMIT      " \n\t"
#define    MAX_POS      62       /* max number of phone alignment in a word */

#define    ALIGN        (char) 0xff   /* nothing else can be 0xff */
#define    MASK         (char) 0xc0   /* MSB two bits */
#define    MATCH        (char) 0x00
#define    SUB          (char) 0x40
#define    DEL          (char) 0x80
#define    INS          (char) 0xc0

#define    LARGE        9999

extern int epd_make_string(char *, char *, int);
extern void encode_entry( char * );
extern void encode_entry_pron( char * );
extern void expand_str(char *, int);
extern int get_strlen( char * );
extern void copy_str(char *, char *);
extern void read_phone( char * );
extern int get_phone_idx(char *);
extern void pron2bin(char *, char *);
extern void pron2err( char * );
extern void err2pron( char *, char * );
extern int equal_pron(char *, char *);
extern int equal_str(char *, char *);
extern int compare_str(char *, char *);
extern void match(int, int, int, int, int, char *, int);
extern void map_pron(char *, char *);

