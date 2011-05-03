/*=======================================================================
 dist_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 This header file exposes interface functions and definitions for
 processing distance measures.  Some of the definitions of macros
 are rather lengthy, and perhaps would be better written
 as functions, but the idea was to write code that took minimum of
 time.

 The macros are also difficult to use because the macros have different
 number of parameters depending on setting of BIT8MEAN and BIT8VAR.
 
======================================================================*/

#ifndef DIST_USER_H
#define DIST_USER_H


/*--------------------------------------------------------------
The Windows method of exporting functions from a DLL.
---------------------------------------------------------------*/
#if defined (WIN32) || defined (WINCE)

/* If it is not defined already, define a macro that does
Windows format export of DLL functions */
#ifndef TIESRENGINECOREAPI_API

// The following ifdef block is the standard way of creating macros which
// make exporting from a DLL simpler. All files within this DLL are compiled
// with the TIESRENGINECOREAPI_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// TIESRENGINECOREAPI_API functions as being imported from a DLL,
// whereas this DLL sees symbols defined with this macro as being exported.
#ifdef TIESRENGINECOREAPI_EXPORTS
#define TIESRENGINECOREAPI_API __declspec(dllexport)
#else
#define TIESRENGINECOREAPI_API __declspec(dllimport)
#endif
#endif

#else

#ifndef TIESRENGINECOREAPI_API
#define TIESRENGINECOREAPI_API
#endif

#endif
/*--------------------------------------------------------------------*/


#include "tiesr_config.h"
#include "tiesrcommonmacros.h"
#include "gmhmm_type.h"
#include "search_user.h"

#ifdef BIT8MEAN
#include "pack_user.h"
#endif

#define   LOGPOLYORD   8
#define   MAX_COEF_LOGPOLY  24103         /* 94.15, Q8 */

#define   POWPOLYORD   6

static const short coef_powpoly[ POWPOLYORD + 1 ] = {
  4, 20, 159, 909, 3936, 11357, 16384};


/* log10(2) = 0.30103 in Q 15 */
/*
#define   LOG2TOLOG10  9864               
*/

/* log2(10) = 3.3219 in Q 13 */
/*
#define   LOG10TOLOG2  27213              
*/

/*--------------------------------*/
/*
** Macro interface of gaussian distance calculations,
** implemented as macro, as function will cost more CPU.
*/



/*----------------------------------------------------------------
  GAUSSIAN_DIST

  This macro is defined differently depending on the setting of BIT8MEAN.


  If BIT8MEAN is set, then the macro unpacks the interleaved static/dynamic
  byte mean vector into a non-interleaved vector of shorts.

  It then uses another macro, GAUSSIAN_DIST_MU16, to do the distance calculation.
  
  --------------------------------*/

#ifdef BIT8MEAN

#define GAUSSIAN_DIST(gv, scr, nbr_dim, mixture, i, feature) \
{ \
    short j;\
    long  diff;\
    short *mu8, *invvar, gconst, *feat = (short*)feature;\
    short diff_s;\
    ushort n_mfcc = nbr_dim>>1;\
    short mu16[  MAX_DIM ], *pmu16 = mu16;\
    mu8 = get_mean(gv->base_mu,mixture, nbr_dim, i);\
    invvar = get_var(gv->base_var,mixture, nbr_dim, i);\
    gconst = gv->base_gconst[ mixture[ i * 3 + 3 ] ];\
    vector_unpacking((const unsigned short *)mu8, mu16, gv->scale_mu, n_mfcc);\
    GAUSSIAN_DIST_MU16(scr, feat, j, nbr_dim, diff, diff_s, gconst, pmu16, invvar, n_mfcc, gv->scale_var);\
}

/* ! BIT8MEAN */
#else

/* Mean is already in short */
#define GAUSSIAN_DIST(gv, scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar) \
    GAUSSIAN_DIST_MU16(scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar, gv->n_mfcc, gv->scale_var)

#endif



/*----------------------------------------------------------------
  GAUSSIAN_DIST_MU16

  This macro has different implementation depending on whether BIT8VAR is set
  or not.  If it is not set, then the likelihood calculation can be implemented
  directly.  If it is set, then the likelihood must unpack the variance in-line
  during calculation.
  --------------------------------*/

#ifdef BIT8VAR

/* Both feature and mean vectors are unpacked, but variance is packed */
#define GAUSSIAN_DIST_MU16(scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar, n_mfcc, scale_var) \
    scr = 0; \
    for (j = 0; j < nbr_dim; j++) { \
      long prod;\
      WEIGHTED_DIFF(prod, diff, diff_s, n_mfcc, invvar, scale_var, feat, mu, j); \
      scr -= prod; \
    } \
    scr = LONG_RIGHT_SHIFT_N(scr,10);   /* Q 6 */ \
    scr -= gconst; \
    scr >>= 1; /* 1/2 */

#define  WEIGHTED_DIFF(prod, diff, diff_s, n_mfcc, invvar, scale_var, feat, mu, jx) \
      diff = (long) *feat++ - (long) *mu++; /* Q 11 */ \
      diff = MAX(diff, -32768);    /* clip */ \
      diff = MIN(diff, 32767); \
      diff_s = (short) diff; \
      diff = diff_s * diff_s;      /* Q 22 */ \
      diff_s = (short)LONG_RIGHT_SHIFT_N(diff, 15);         /* Q 7 */ \
      { short invv; \
        if (jx < n_mfcc) invv = invvar[jx] & 0xff00;  /* static */  \
        else  invv = (invvar[jx - n_mfcc] & 0xff) << 8; /* dynamic */ \
        invv >>=  scale_var[jx]; \
        prod = diff_s * invv;    /* Q 7 * Q 9 = Q 16 */ \
      }


/* ! BIT8VAR */
#else

/* Mean and variance are both in short, so do score likelihood calculation directly.
   Arguments n_mfcc and scale_var are not used in this case but are retained for 
   uniformity with the macro definition. */
#define GAUSSIAN_DIST_MU16(scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar, n_mfcc, scale_var) \
    scr = 0; \
    for (j = 0; j < nbr_dim; j++) { \
      diff = (long) *feat++ - (long) *mu++; /* Q 11 */ \
      diff = MAX(diff, -32768);    /* clip */ \
      diff = MIN(diff, 32767); \
      diff_s = (short) diff; \
      diff = diff_s * diff_s;      /* Q 22 */ \
      diff += (1 << 14);           /* round */ \
      diff_s = diff >> 15;         /* Q 7 */ \
      scr -= diff_s * *invvar++;    /* Q 7 * Q 9 = Q 16 */ \
    } \
    scr += (1 << 9);               /*round */ \
    scr >>= 10;                    /* Q 6 */ \
    scr -= gconst; \
    scr >>= 1; /* 1/2 */ 

#endif



/*--------------------------------*/
/* Functional interface */

/* returns Q6 */
TIESRENGINECOREAPI_API short  hlr_gauss_obs_score_f(const short *feature, int pdf_idx,
			     ObsType type, short *comp, 
			     const short n_dim, const gmhmm_type *gv );

TIESRENGINECOREAPI_API short hlr_logsum( short lna, short lnb );

TIESRENGINECOREAPI_API unsigned short hlr_expn( short x, short qpt );

/* exponential operation
   @param x data in Q_qpt
   @param qpt Q value of data x
   @param rNorm norm of returned exponential
   return exponential in Q_rNorm
*/
unsigned short hlr_expn_q( short x, short qpt, short *rNorm);

TIESRENGINECOREAPI_API short hlr_log_one( unsigned short x );

#define GET_NORM(tmp, norm) for ( norm = 0; tmp < 0x40000000; norm++ ) tmp <<= 1;

/*--------------------------------
  Multiplication of a Q15 and a long
  --------------------------------*/
TIESRENGINECOREAPI_API long q15_x(ushort a, long b);

/* -------------------------------
   return long with the same Qnumber of b's.
   @param a, must be smaller than 1, can have minus value, in Q14
  --------------------------------*/
TIESRENGINECOREAPI_API long q14_x(short a, long b);

/*--------------------------------
  div 32 32 return specified Q
  --------------------------------*/
TIESRENGINECOREAPI_API long div32_32_Q(long num, long den, ushort q_prec);

/*--------------------------------
div 32 32 return Q9
---------------------------------*/
TIESRENGINECOREAPI_API long div32_32(long num, long den);

/* fixed-point square root operation */
TIESRENGINECOREAPI_API long fx_sqrt(long x);

/* ---------------------------------------------------------------------------
   polynomial fit to compute log2()
   ---------------------------------------------------------------------- */
TIESRENGINECOREAPI_API short log_polyfit(long value, short compensate);

/* ---------------------------------------------------------------------------
   polynomial fit to compute pow2()
   ---------------------------------------------------------------------- */
TIESRENGINECOREAPI_API long pow_polyfit( short value );

/*----------------------------------------------------------------
  gauss_det_const

  This function calculates the log of the Gaussian constant based on
  the present inverse variance values.  The leading normalization term
  in a Gaussian distribution is given by 1/((2*pi)^N/2 * det(COV)^1/2
  ).  Since in TIESR the covariance is stored as inverse covariance
  terms in which covariances are scaled by dividing by the square of
  the mean vector component scales, and TIESR implements the
  multiplication by the gaussian constant by subtracting the term in
  the log domain and then multiplying by 1/2, the Gaussian constant
  term is calculated as: N*log(2*pi) - sum(log(1/Ci) ).  Correction
  terms are added according to the scaling of the covariance
  components.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API short gauss_det_const(short *cov, ushort power_factor,
			  const short mu_scale_p2[],
			  const short dim) ;

/* This function conducts uniq operation on a vector. Each element may contain its probability.
   In such case, same elements are merged with their prabilities are summed. 
   @param imax_10 pointer to elements to be compared
   @param prob probability of the elements
          the updated probability of an element is the sum of probablities of same elements 
*/
short uniq(short * imax_10, short *prob , short inum);

#endif
