/*=======================================================================
 
 *
 * pmc_f.cpp
 *
 * PMC processing.
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

 PMC processing.
 
======================================================================*/

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "pack.h"
#include "jac_one.h"
*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "pmc_f_user.h"
#include "pmc_f.h"
#include "pack_user.h"


/* 
extern void prt_vec(char *str, short *v, unsigned short n)

{
  int i;
  printf("%s ", str);
  for (i=0; i<n; i++) printf("%5d ", v[i]);
  printf("\n");
}
*/

/*================================================================
  pmc_log_add

  This function accepts as input the log2 value of the speech model
  power spectrum and the log2 value of the noise estimate power
  spectrum, both in long Q9 format.  It outputs an approximation of
  the log-add value log2( speech ps + noise ps ).  The approximation
  should be much faster than the present implementation, because it
  does not require converting to linear power spectra.

  This function also accepts a pointer to the short beta value for
  doing regression, and returns a quick estimate of the beta factor
  in Q15, in order to handle all pmc power spectra computations in
  this function.

  ----------------------------------------------------------------*/
/* parameters to specify the log-add operation */

/* -10 in Q9, limit where the correction term is negligible */
/* #define PMCLIMIT  -5120 */

/* -5 in Q9, where to split the evaluation of the log polynomial function */
/* #define LOGSPLIT  -2560 */

/* midranges of polynomial evaluations */
/* -7.5, for lower segment */
/* #define LOOFFSET  -3840 */

/* -2.5 for upper segment */
/* #define HIOFFSET  -1280  */

/* coefficients for two subdivisions of log2(1+2^x) */
/* first for -10<=x<-5 , then -5<=x<=0, Q19 */
/* fist segment is Q19 */
/* second segment is Q15 */
/* #define LOG_ORDER  3 */
/*
static const short coef_log[2][4] = { { 265, 1223, 2826, 4014 },
	       		              { 211, 1503, 4955, 7665 } };
*/
/* coefficients for 2^-x, Q15 for -1<x<=0 */
/* #define TWO_ORDER 2 */

/*
static const short coef_twomx[3] = { 5615, -21873, 32702 };
*/

/*--------------------------------*/
static short pmc_log_add( long splog, long nslog, short *beta )
{
   long diff, tmp;
   short alog, blog, slog, stmp;
   short correction, lsum;
   short poly, idx, sdiff, result;
   
   /* short values for splog and nslog */
   /* these should not limit */
   if( splog > 32767 )
      alog = 32767;
   else if( splog < -32768 )
      alog = -32768;
   else
      alog = (short)splog;

   slog = alog;
   
   if( nslog > 32767 )
      blog = 32767;
   else if( nslog < -32768 )
      blog = -32768;
   else
      blog = (short)nslog;


   /* alog holds max value */
   if( alog >= blog )
   {
      /* difference in Q9 */
      diff = (long)blog - alog;
      
      /* blog too small to be noticeable */
      if( diff < PMCLIMIT )
      {
	 *beta = 32767;
	 return alog;
      }
   }
   else
   {
      /* difference in Q9 */
      diff = (long)alog - blog;
      
      /* alog too small to be noticeable */
      if( diff < PMCLIMIT )
      {
	 *beta = 0;
	 return blog;
      }

      /* swap blog and alog, blog is greater */
      stmp = blog;
      blog = alog;
      alog = stmp;
   }

   /* calculate an approximation to log-add */

   /* choose which cubic polynomial to use */
   if( diff < LOGSPLIT )
   {
      poly = 0;
      stmp = (short)(diff - LOOFFSET );
   }  
   else
   {
      poly = 1;
      stmp = (short)( diff - HIOFFSET );
   }

   /* correction factor polynomial evaluation */
   /*  Qx is Q19 for poly=0, and Q15 for poly=1 */
   correction = coef_log[poly][0];
   for( idx = 1; idx <= LOG_ORDER; idx++ )
   {
      /* Qx * Q9 = Q9+x */
      tmp = correction * stmp;

      /* next coefficient at Q9+x */
      tmp += (coef_log[poly][idx] << 9 );

      /* readjust result to Qx */
      //      tmp += 1<<8;
      //      tmp >>= 9;

      /* in Qx, should never overflow */
      //      correction = (short)tmp;
      correction = (short)LONG_RIGHT_SHIFT_N(tmp, 9);
   }

   /* readjust correction back to Q9 */
   if( poly == 0 )
   {
      /* correction was Q19 */
      correction += (1<<9);
      correction = (unsigned short)correction >> 10;
   }
   else
   {
      /* correction was Q15 */
      correction += (1<<5);
      correction = (unsigned short)correction >> 6;
   }

   /* add correction to larger log value */
   /* parameters should be chosen for no overflow */
   tmp = (long)correction + alog;

   if( tmp > 32767 )
      lsum =  32767;
   else
      lsum = (short)tmp;


   /* do a quick calculation of the beta factor between 0 and 1 */

   /* should be in range 0 to PMCLIMIT */
   /* in Q9 */
   sdiff = slog - lsum;
   if( sdiff >= 0 )
   {
      *beta = 32767;
   }
   else
   {
      /* quick polynomial approx. to 2^sdiff ( sdiff < 0 ) */

      /* work with positive numbers */
      sdiff = -sdiff;
      
      /* fractional portion to Q15*/
      stmp = sdiff & 0x1ff;
      stmp <<= 6;

      result = coef_twomx[0];
      for( idx = 1; idx <= TWO_ORDER; idx++ )
      {
	 /* Q15 coef * Q15 number */
	 tmp = result * stmp;

	 /* add coefficient, Q30 */
	 tmp += (coef_twomx[idx] << 15);

	 /* adjust to Q15 */
	 result = (short) LONG_RIGHT_SHIFT_N(tmp, 15);
      }	 

      /* now scale result by the integer power */
      stmp = sdiff >> 9;
      if( stmp == 0 )
      {
	 *beta = result;
      }
      else
      {
	 result = (unsigned short)result + ( 1 << ( stmp - 1 ) );
	 *beta = ( (unsigned short)result >> stmp );
      }
   }

   return lsum;
}


/*----------------------------------------------------------------
  log_spectral_compensation

  This function compensates an input clean log spectral signal vector
  to compensate for the estimates of additive noise and channel bias.

  This function is used by three functionalities:
  1. PMC: (jac == 0)
  2. JAC compensation: (jac != 0) && log_df == NULL

  3. JAC estimation:   (jac != 0) && log_df != NULL,
  (This setting also calculates the objective function, which is the 
   ratio of the linear channel biased signal to the channel biased signal
   plus noise for each vector component)
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API void log_spectral_compensation(short sp_log_ps[], short reg_sp_log_ps[],
			       short pmc_log_ps[],  short reg_pmc_log_ps[], 
			       const short noise_log_ps[], const short chn[], 
			       short jac, short *log_df, short n_filter)
{
  short i, scale;
  long  tmp, tmp2;
  short beta, norm, tmp_s;

  if (jac) scale = LC;
  else     scale = LOG10TOLOG2;

  /* parallel model combination, log-add approximation */
  for (i = 0; i < n_filter; i++) {

    /* log-add for MFCC */
    short sum_sh = sp_log_ps[i] + chn[i]; /* tried clip, no help */
    
    tmp = sum_sh * scale;   /* log10() to log2(), then div. by ln 10, Q9 * Q13 = Q22 */
    tmp = LONG_RIGHT_SHIFT_N(tmp, 13);                         /* Q 9 */

    tmp2 = noise_log_ps[i] * scale;
    tmp2 = LONG_RIGHT_SHIFT_N(tmp2, 13);
    /* at this point both tmp and tmp2 are bounded within short */

    /* 
       tmp contains log2 of speech power spectrum, in Q9. tmp2 contains
       log2 of noise power spectrum, in Q9. The function below is a
       simplification that should reduce computational load for pmc.
       It produces an approximation of log2( 2^( log2(speech ps)) +
       2^( log2(noise ps)) ), in Q9.  Actually the function has
       parameters that may be modified to provide a better estimate
       than log-add approximation, by taking into consideration a
       global estimate of noise and speech variances.
    */

    pmc_log_ps[i] = pmc_log_add( tmp, tmp2, &beta );


// the old way of doing log-add:
//    /* linear power spectrum larger than 32 bits, compensate */
//
//    tmp3 = MAX(tmp, tmp2);
//
//    /* requires Q9 (512 is 1 in Q9) input for pow_polyfit
//    for ( norm_pow = 0; tmp3 >= 15870; norm_pow++ )  tmp3 -= 512;
//    	  (replaced by next 4 lines)   */
//
//    tmp3 -= 15870;
//    if (tmp3 < 0) norm_pow = 0;
//    else if (tmp3 == 0) norm_pow =1;
//    else norm_pow = (tmp3 >> 9) + 1; 
//
//    tmp  -= 512 * norm_pow;
//    tmp2 -= 512 * norm_pow;
//
//    sp_lin = pow_polyfit( (short) tmp );
//    noise_lin = pow_polyfit( (short) tmp2 );
//
//    /* PMC_GAIN, need longer than long. 
//       So use rational number: 5 / 8 instead.
//       Use PMC gain = 1, not much difference */
//    /* tmp = 5 * ( sp_lin >> 3 ) + noise_lin; */
//    tmp = sp_lin + noise_lin;
//    if ( tmp < 0 ) tmp = 2147483647;  /* overflow clip */
//    pmc_log_ps[i] = log_polyfit( tmp, 0 );         /* log-add */
//
//    pmc_log_ps[i] += norm_pow << 9;


    tmp = pmc_log_ps[i] * LOG2TOLOG10;  /* log2() to log10(), Q9 * Q15 = Q24 */
    tmp =  LONG_RIGHT_SHIFT_N(tmp, 15);                         /* Q 9 */

    if (jac) {
      tmp = tmp * LN10; /* Q9 * Q13 = Q22 */
      tmp =  LONG_RIGHT_SHIFT_N(tmp, 13);                         /* Q 9 */
    }
    pmc_log_ps[i] = (short) tmp;

//    /* beta factor for regression MFCC, 
//       it is simply beta = sp_lin / ( sp_lin + noise_lin ).
//       But has to go through the scaling trouble because of fixed-point long */
//
//    tmp = sp_lin + noise_lin;
//
//    if ( tmp < 0 ) tmp = 2147483647;  /* overflow clip */
//
//    if (tmp != 0 ) {
//      for ( norm = 0; tmp < 0x40000000; norm++ )
//	tmp <<= 1;
//      sp_lin <<= norm;
//      tmp >>= 15;                          /* Q -15 */
//      beta = (short) ( sp_lin / tmp );     /* Q 15 */
//      if ( beta < 0 ) beta = 32767;        /* overflow clip */
//    }
//    else beta = 0;
    
    if (log_df)  log_df[i] = beta;    
    else /* PMC or JAC */
    if ( reg_sp_log_ps[ i ] != 0 ) {
      tmp = reg_sp_log_ps[i] > 0 ? reg_sp_log_ps[i] : - reg_sp_log_ps[i];
      for ( norm = 0; tmp < 0x40000000; norm++ )
	tmp <<= 1;
      tmp_s = (short)(tmp >> 16);
      tmp = beta * tmp_s;  /* Q 15 * Q -16 = Q -1 */
      tmp <<= 1;           /* Q 0 */
      tmp >>= norm;
      reg_pmc_log_ps[ i ] = reg_sp_log_ps[i] > 0 ? tmp : -tmp;

    } else reg_pmc_log_ps[ i ] = 0;
  }
}

/*
** compensated_log_mean
*/



/*----------------------------------------------------------------
  pmc

  This function compensates a clean speech mfcc vector for the effects
  of channel and noise, creating a compensated mfcc vector.  The
  function requires the log spectral estimates of noise and channel.

  It performs log-add approximation of PMC, and can be switched to do
  either JAC or PMC
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API void
pmc(short *sp_mfcc_o,     /* original non-PMC MFCC mean */
    short *noise_log_ps, /* noise log mel energy */
    short *pmc_mfcc,     /* PMC-compensated MFCC mean */
    short *chn,          /* channel estimate  */
    short jac,           /* jac/pmc switch  */
    short n_mfcc, short n_filter, const short *mu_scale_p2, short *scale_mu,
    const short *cosxfm[  MAX_DIM_MFCC16 ],
    short * sp_bias      /* bias between JAC-compensated MFCC and the original MFCC */ ,
    Boolean b8bitMean
    )
{
  short sp_log_ps[ N_FILTER26 ];        /* log power spectrum */
  short reg_sp_log_ps[ N_FILTER26 ];    /* regression coeff */
  short pmc_log_ps[ N_FILTER26 ];
  short reg_pmc_log_ps[ N_FILTER26 ];
  short *po = pmc_mfcc, *sp_mfcc = sp_mfcc_o;
  short sp_clean[ MAX_DIM ] , i; 
  short pmc_mfcc16[ MAX_DIM ];

  if (b8bitMean){
    po = pmc_mfcc16;
    vector_unpacking((ushort*) sp_mfcc_o, pmc_mfcc16, scale_mu, n_mfcc);
    sp_mfcc = pmc_mfcc16;
  }

  if (sp_bias) {
    /* save the clean MFCC */
    for (i=2*n_mfcc - 1; i>= 0; i--)
      sp_clean[i] = sp_mfcc[i]; 
  }

  /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  cos_transform( sp_mfcc, sp_log_ps, mu_scale_p2, n_mfcc,  n_filter, cosxfm );
  /* regression coeff */
  cos_transform( sp_mfcc + n_mfcc, reg_sp_log_ps, mu_scale_p2 + n_mfcc, n_mfcc, n_filter, cosxfm);

  log_spectral_compensation(sp_log_ps, reg_sp_log_ps, pmc_log_ps, reg_pmc_log_ps, noise_log_ps, chn, jac, (short*) NULL, n_filter);
  
  /* log mel power spectrum to cepstrum */

  inverse_cos_transform(pmc_log_ps, po, mu_scale_p2, n_mfcc, n_filter, cosxfm);
  inverse_cos_transform(reg_pmc_log_ps, po + n_mfcc, mu_scale_p2 + n_mfcc, n_mfcc, n_filter, cosxfm);

  if (sp_bias){
    /* record the bias */
    for (i=2*n_mfcc - 1; i>= 0; i--)
      sp_bias[i] = po[i] - sp_clean[i]; 
  }

  if (b8bitMean)
    vector_packing(po, (ushort *) pmc_mfcc,  scale_mu, n_mfcc);

}

