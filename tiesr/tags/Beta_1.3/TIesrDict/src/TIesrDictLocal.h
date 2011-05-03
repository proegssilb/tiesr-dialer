/*=======================================================================

 * 
 * TIesrDictLocal.h
 *
 * Header file for local implementation of TIesrDict API.
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
 

This header contains structures and constants that are local to the
TIesrDict class, but are not exposed by the class.

======================================================================*/

#ifndef _TIESRDICTLOCAL_H
#define _TIESRDICTLOCAL_H


//--------------------------------
// Constants


#if defined (WIN32) || defined (WINCE)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif


#define TRUE 1
#define FALSE 0



/* Max word string length */
#define    MAX_STR      256

/* max # of phones in a word */
#define    MAX_PRON     30       


//--------------------------------
//  mapping_pairs
//
//  The mapping_pairs structure type that does context sensitive
//  mappings, eg, stop consonants.  At this time it is a static member
//  of the class, but probably should be static in local parameters.
//
typedef struct mapping_pairs
{
    char *from;
    char *to;
} mapping_pairs;


// These are the context sensitive stops for our WSJ trained triphones
// Eventually this should be transferred to data, not in code.
static const int N_PAIR = 8;

static const mapping_pairs mapping_pair[] = 
{
    { "b", "bcl b" },
    { "d", "dcl d" },
    { "g", "gcl g" },
    { "jh", "dcl jh" },
    { "p", "pcl p" },
    { "t", "tcl t" },
    { "k", "kcl k" },
    { "ch", "tcl ch"}
};

static const char* BOUNDARY = "sp";
static const char* DELIMIT =  " \n\t";


//--------------------------------
// Filenames that are fixed within the API
static const char* PHONELIST = "phone.lis";
static const char* DICTFILE = "dict.bin";


//--------------------------------
// Encoded dictionary character definitions
//
// lower 7 bits is the pron len
static const char MASK_2 = (char) 0x7f;
static const char MASK_3 = (char) 0x80;
static const char ALIGN =  (char) 0xff;
// MSB 2 bits 
static const char MASK =   (char) 0xc0;
static const char MATCH =  (char) 0x00;
static const char SUB =    (char) 0x40;
static const char DEL =    (char) 0x80;
static const char INS =    (char) 0xc0;

//#define LEFT_PART	0
//#define MATCH_PART	1
//#define RIGHT_PART	2
//#define OUT_PART	3

#endif // _TIESRDICTLOCAL_H 
