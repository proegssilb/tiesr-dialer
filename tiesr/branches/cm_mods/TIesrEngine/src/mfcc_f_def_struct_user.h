/*=======================================================================

 *
 * mfcc_f_def_struct_user.h
 *
 * Header for mfcc structures.
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
 
 This header file encapsulates and exposes the portion of the mfcc
 feature processor that contains global constants and structures.
 These are split from mfcc_f_user, since these are needed by
 the gmhmm_type structure, and that structure is used within the 
 function interface of the mfcc feature processor.

======================================================================*/

#ifndef MFCC_F_DEF_STRUCT_USER_H
#define MFCC_F_DEF_STRUCT_USER_H 


/* Actually defines FRAME_LEN for application designer */
#include "winlen.h"

/*--------------------------------*/
/* Constants that need to be exposed so they can be used elsewhere */

/* overlap between two frames */
#define   OVERLAP  (WINDOW_LEN - FRAME_LEN) 

#define   N_FILTER20     20

#define   N_FILTER26     26

/* for JAC */
#define ACC_MEMORY (N_FILTER26 * 2 * 2)  

/* maximum number of mfcc */
#define   MAX_DIM_MFCC10 10         

/* maximum number of mfcc */
#define   MAX_DIM_MFCC16 16

/* maximum number of mfcc */
#define MAX_DIM_MFCC13 13

/* static + dynamic feature vector size*/
#define   MAX_DIM  (2 * MAX_DIM_MFCC16) 

/* regression span */
#define   REG_SPAN     2                  

/* #define   MFCC_BUF_SZ  (2 * REG_SPAN + 1) */
#define   MFCC_BUF_SZ  (2 * REG_SPAN + 10)


/*--------------------------------*/

/* Structure used in both mfcc and uttdet */
typedef struct 
{
  short norm0;  /* preemphasis */
  short norm1;  /* Hamming */
  short norm2;  /* power spectrum */
} NormType;


/* mel filter structure used in gmhmm_type structure */
typedef struct {   /* mel filter, mel scale, triangular square */
  short num;
  short x1;
  short x2;
} mel_filter_type;


#endif
