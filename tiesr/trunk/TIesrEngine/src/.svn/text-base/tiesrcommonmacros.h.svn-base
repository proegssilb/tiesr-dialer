/*=======================================================================

 *
 * tiesrcommonmacros.h
 *
 * Common macros and parameters used by TIesr engine.
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

 This header file contains macros that may be used in several APIs.

======================================================================*/

#ifndef TIESRCOMMONMACROS_H
#define TIESRCOMMONMACROS_H


/*--------------------------------*/
/* Types */

/* The Codesourcery gnu ARM toolchain typedefs ushort in <stdlib>, causing
 the #define to generate an error, so convert #define to typedef 
 for consistency. */

/*#define ushort unsigned short */
typedef unsigned short ushort;

#define uchar  unsigned char
#define Boolean short

/*--------------------------------*/
/* Macros that are commonly used constants */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* log10(2) = 0.30103 in Q 15 */
#define   LOG2TOLOG10  9864               
/* comment: in order to use LOG2TOLOG10, use q15_x(LOG2TOLOG10, value) so that the returned 
   value has the same Q number as that of value */

/* log2(10) = 3.3219 in Q 13 */
#define   LOG10TOLOG2  27213              

#define LZERO -32767

/*  log2(10)/ln(10) in Q13 */
#define LC   11818  

/* ln 10 in Q13 */
#define LN10 18863 

/* 1/log2(exp(1)) in Q9 */
#define LOG2Q9 355

/* 1.0 in Q15 */
#define ONE_Q15  32768

/* 1.0 in Q9 */
#define ONE_Q9   512

/*--------------------------------*/
/* Macros that act as functions */

/*
** un scaled: W_B in Q 15
*/
#define SMOOTHING(A, W_B, B) ((long) ( 32767 - W_B ) * A + (long) W_B * B)

#ifndef MAX
#define   MAX(a,b)  ((a) > (b) ? (a): (b))
#endif
#ifndef MIN
#define   MIN(a,b)  ((a) < (b) ? (a): (b))
#endif

/* right shift with rounding: */
#define LONG_RIGHT_SHIFT_N(_val_,_n_) (((_val_) + ((long)1<<((_n_)-1))) >>(_n_))

/* clip */
#define LONG2SHORT(x) (short)(MIN(MAX(x, -32768),32767))

/* macros for search space */
/* These are now in headers specific to the particular recognition task */
//#define BEAM_R   9851
//#define STATE_R  9209
//#define TIME_Z   200 /* words, MIN 118 */

// specific code value used in grammar parsing
#define MCODE 65534

#define IS_WORD_END_TIME( time_at ) (time_at & 0x8000)
#define MARK_WORD_END_TIME( time_at) (time_at | 0x8000)
#define CLR_MARK_WORD_END_TIME( time_at ) (time_at & 0x7fff)

/* log(2) in Q12 */
#define LOG2Q12 2839

/* log(2*pi) in Q21 */
#define LOG_2_PI 3854306


/* Alignment for memory management */
#define SHORTALIGN 0
#define LONGALIGN 1
#define INTALIGN 2


/* Architecture's sizes for shorts and longs.  This assumes char, short,
 int, long, all have sizes that are a power of 2 bytes. */

#define SHORTLEN  ( sizeof(short) )

/* Number of >> shifts to turn byte size to short size, assuming
 number of bytes is multiple of number of shorts. */
#define SHORTSHIFT ( SHORTLEN >> 1 )

#define SHORTSPERLONG ( sizeof(long) >> SHORTSHIFT )

/* Number of << shifts to turn long size to short size, assuming
longs are multiple of shorts. */
#define LONGSHIFT ( SHORTSPERLONG >> 1 )
#endif
