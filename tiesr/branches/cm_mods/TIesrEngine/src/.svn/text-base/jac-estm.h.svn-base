/*=======================================================================

 *
 * jac-estm.h
 *
 * Header for jac estimation implementation.
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


 This header provides the information that is specific to the
 jac_estm API.

======================================================================*/

#ifndef _JAC_ESTM_H
#define _JAC_ESTM_H

#include "tiesr_config.h"

/*--------------------------------*/
/* Constants by define */

/*for ISVA*/
#define ISVA_NOISE_TH  14000
#define C0LIMIT 200

/* notice: since gtm modified after EM */
#define MAX_EM_ITR 1 

#define MAX_NEWTON_ITR 1

#define NBR_TRANSFO 1

/* 1.0 in Q15 */
#define ONE_Q15  32768

/* 1.0 in Q9 */
#define ONE_Q9   512

/* -1e6 in Q9 */
#define MINUS_1E6_Q9 -512000000


/* This is the minimum value for the SVA log rho to ensure that the
 * variance does not get too small, (-2 in Q9).  Thus variance is
 * never multiplied by any number less than exp(-2).  Or since we use
 * inverse variance, the inverse variance is not multiplied by anything
 * greater than exp(2).*/
#define MIN_SVA_RHO  -1024


#ifdef USE_IJAC

/* Limit to avoid zero division sensitivity, 0.005 in Q14 */
#define MIN_ACCUM 81

#else

/* Limit to avoid zero division sensitivity, 0.005 in Q16 */
#define MIN_ACCUM 328

#endif

/*
** MEMORY REQUIRMENT: total about 2Kwords, in two parts: 
** A: for forward/backward variables  B: for accumulators 
*/
/*
** part A: Collect (EM) statistics for a training segment
**         Total memory requirement: 2 x N (T+1) + 1. e.g. N = 5, T = 30 -> 311 words
*/
#define MAX_EM_MEMORY 1000  

/*
** part B: accumulators:
** objective function accumulators 
** funct + derivative funct for numerator and denominator
** Total: N_FILTER * 2 * 2 + NBR_TRANSFO. e.g. N_FILTER = 20, NBR_TRANSFO = 1 -> 161 words
*/
/* #define ACC_MEMORY (N_FILTER * 2 * 2)  */


/*--------------------------------*/
/* JAC SVA parameter defaults */

/* Number of mean vectors to update per input frame */
static const short VEC_PER_FRAME_DEFAULT = 80;

/* Threshold of component log likelihood required to accum in JAC */
/* This was originally -1600 and had the note: "-25 in Q6, can
   be smaller later".  But that made the Q15 gamma = exp(Lr) zero much
   of the time, which was wasted processing.  It was changed to -667
   so that wasted processing is not likely to be done. */
static const short COMPONENT_LIKELIHOOD_DEFAULT = -667;

//#define MAX_SEG_SIZE 50 
static const short MAX_SEG_SIZE_DEFAULT = 15;


#ifdef USE_IJAC

/* Channel correction temporal smoothing filter coefficient, 0.6 in Q15 */
static const short  SMOOTH_COEF_DEFAULT = 19660;

#else

/* Channel correction temporal smoothing filter coefficient, 0.3 in Q15 */
static const short SMOOTH_COEF_DEFAULT = 9830;

#endif

/* IJAC channel discount factor */
static const short DISCOUNT_FACTOR_DEFAULT = 26214;

/* Channel correction deweighting coefficient */
static const short DEWEIGHT_FACTOR_DEFAULT = 9830;

/* IJAC Bayesian weighting for inverse variance prior */
/* Specify it as zero in order not to do Bayesian weighting */
static const short BAYESIAN_WEIGHT_DEFAULT = 327;

/* Q9, inverse variance of channel distortion for Bayesian weighting */
static const long pPriorChannelInvvar[N_FILTER26] = {
4179, 7023, 8192, 7574, 8192, 8888, 10579, 9678, 8192, 8192, 12800, 12800, 12800, 12800, 14183, 14183, 8192, 17716, 10579, 7574, 7574, 9678, 10579,   9678,   8192,   568890};


/* SVA forgetting factor.  Comments said:  */
/* forgetting factor = 0.25 */
/* But this is the power of two to divide by */
static const short VAR_FORGET_FACTOR_DEFAULT = 4;


/*--------------------------------*/
/* function macros */

/* 
** The two functions go together (because the storage: top and bottom accumulators)
** 1. Get the derivative part from objective function
** 2. Iteration on obj-func + derivative term
*/
#define GET_D_OBJ(obj, n_filter) (obj+n_filter)

#define FOREACH(dim_,freq_dim) for (dim_=0; dim_<(freq_dim); dim_++) 

#define GET_NORM(tmp, norm) for ( norm = 0; tmp < 0x40000000; norm++ ) tmp <<= 1;


/*-------------------------- for force alignment -----------------------------------*/
/* network size as function of symbols: 
** n-1 emission HMM n+2 sym type (2: exit states)
*/
#define TOTAL_NET_SIZE(n_sym) (sizeof(TransType)/sizeof(short) /* size of transtype */ \
                               + (n_sym - 1) /* minus space occupied by symlist[0] */ \
                               + (n_sym + 2) * sizeof(SymType)/sizeof(short)) /* space for symbtype */
#define NBR_FIELDS 2  /* used in make_net */
#define NBR_VAR 2 /* number of variances for the name */
#define MAX_NBR_STATES (MAX_NBR_EM_STATES + 1)
#define MEM_SIZE_FORCE_ALIGNMENT 256
  
#endif
