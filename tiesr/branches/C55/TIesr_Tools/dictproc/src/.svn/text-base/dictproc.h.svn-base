/*=======================================================================

 *
 * dictproc.h
 *
 * Header for dictproc dictionary compression program.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *

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

/* Maximum number of letter characters making up words in the dictionary. Can
 only have 127, since letter indices in the compressed dictionary must be
 0 < index <= 127. We must reserve the index 0 for the NULL character, and no
 index can have a 1 in the msb of the byte. */
#define    MAX_LETTERS  127

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
extern void expand_pron(char *buf, int idx);

